#include "doctest.h"
#include "KeyValueFile.h"

TEST_CASE("written values read back exactly, in the order they were added")
{
	KeyValueWriter writer;
	writer.add("name", std::string("Old Tom"));
	writer.add("count", -42);
	writer.add("third", 1.0 / 3.0, 17);
	writer.add("short", 0.1f, 9);
	CHECK(writer.text() == "name = Old Tom\ncount = -42\nthird = 0.33333333333333331\nshort = 0.100000001\n");

	KeyValueReader reader(writer.text());
	std::string name;
	int count = 0;
	double third = 0.0;
	double brief = 0.0;
	CHECK(reader.text("name", name));
	CHECK(reader.whole("count", count));
	CHECK(reader.real("third", third));
	CHECK(reader.real("short", brief));
	CHECK(name == "Old Tom");
	CHECK(count == -42);
	CHECK(third == 1.0 / 3.0);
	CHECK((float)brief == 0.1f);
	CHECK(reader.problem().empty());
}

TEST_CASE("blank lines, comments and spaces around keys and values are ignored")
{
	KeyValueReader reader("\n# a comment\n  colour   =  deep red  \r\n\n");
	std::string colour;
	CHECK(reader.text("colour", colour));
	CHECK(colour == "deep red");
	CHECK(reader.problem().empty());
}

TEST_CASE("a value keeps every '=' after the first")
{
	KeyValueReader reader("sum = 1 + 1 = 2\n");
	std::string sum;
	CHECK(reader.text("sum", sum));
	CHECK(sum == "1 + 1 = 2");
}

TEST_CASE("a missing key is reported by name")
{
	KeyValueReader reader("a = 1\n");
	std::string value;
	CHECK_FALSE(reader.text("b", value));
	CHECK(reader.problem() == "missing b");
}

TEST_CASE("numbers must be whole numbers or reals with nothing after them")
{
	KeyValueReader reader("w = 12x\nr = 1.5.2\ne = \n");
	int whole = 0;
	double real = 0.0;
	CHECK_FALSE(reader.whole("w", whole));
	CHECK(reader.problem() == "bad whole number for w: 12x");

	KeyValueReader reals("r = 1.5.2\n");
	CHECK_FALSE(reals.real("r", real));
	CHECK(reals.problem() == "bad number for r: 1.5.2");

	KeyValueReader empty("e = \n");
	CHECK_FALSE(empty.whole("e", whole));
	CHECK(empty.problem() == "bad whole number for e: ");
}

TEST_CASE("a line without '=' is reported and the first problem is kept")
{
	KeyValueReader reader("a = 1\nnonsense\n");
	CHECK(reader.problem() == "line without '=': nonsense");
	std::string value;
	CHECK_FALSE(reader.text("zzz", value));
	CHECK(reader.problem() == "line without '=': nonsense");
	CHECK_FALSE(reader.fail("later"));
	CHECK(reader.problem() == "line without '=': nonsense");
}
