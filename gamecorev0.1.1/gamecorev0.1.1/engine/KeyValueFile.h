#ifndef KEYVALUEFILE_H
#define KEYVALUEFILE_H

#include <map>
#include <string>
#include <utility>
#include <vector>

class KeyValueReader
{
public:
	explicit KeyValueReader(const std::string& text);

	const std::string& problem() const { return problem_; }
	bool text(const std::string& key, std::string& value);
	bool real(const std::string& key, double& value);
	bool whole(const std::string& key, int& value);
	bool fail(const std::string& message);

private:
	std::map<std::string, std::string> values_;
	std::string problem_;
};

class KeyValueWriter
{
public:
	void add(const std::string& key, const std::string& value);
	void add(const std::string& key, int value);
	void add(const std::string& key, double value, int digits);

	std::string text() const;

private:
	std::vector<std::pair<std::string, std::string> > lines_;
};

#endif
