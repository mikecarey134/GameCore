#include "doctest.h"
#include "SafeFileName.h"

TEST_CASE("letters, digits, spaces, dashes and underscores make a safe file name")
{
	const char* safe[] = { "Dana", "Old Tom", "x-7_b", "Conan", "COM0", "LPT10", "consul", "a" };
	for (const char* name : safe)
	{
		INFO(name);
		CHECK(checkFileName(name) == FileNameProblem::None);
	}
}

TEST_CASE("an empty name is reported as empty")
{
	CHECK(checkFileName("") == FileNameProblem::Empty);
}

TEST_CASE("characters a file system could misread are rejected")
{
	const char* unsafe[] = { "a/b", "a\\b", "..", "a:b", "a*b", "a?b", "a\"b", "a<b", "a>b", "a|b", "a.b", "a\tb", "caf\xC3\xA9" };
	for (const char* name : unsafe)
	{
		INFO(name);
		CHECK(checkFileName(name) == FileNameProblem::BadCharacter);
	}
}

TEST_CASE("a leading or trailing space is rejected")
{
	CHECK(checkFileName(" Dana") == FileNameProblem::EdgeSpace);
	CHECK(checkFileName("Dana ") == FileNameProblem::EdgeSpace);
	CHECK(checkFileName(" ") == FileNameProblem::EdgeSpace);
}

TEST_CASE("names Windows reserves for devices are rejected in any case")
{
	const char* reserved[] = { "CON", "con", "Prn", "AUX", "nul", "COM1", "com9", "LPT1", "lpt5" };
	for (const char* name : reserved)
	{
		INFO(name);
		CHECK(checkFileName(name) == FileNameProblem::Reserved);
	}
}
