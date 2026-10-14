#include "SafeFileName.h"
#include <algorithm>
#include <cctype>

namespace
{
	bool allowed(char c)
	{
		return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' ' || c == '-' || c == '_';
	}

	bool reservedOnWindows(const std::string& name)
	{
		std::string upper = name;
		std::transform(upper.begin(), upper.end(), upper.begin(), [](char c) { return (char)std::toupper((unsigned char)c); });
		if (upper == "CON" || upper == "PRN" || upper == "AUX" || upper == "NUL")
			return true;
		return upper.size() == 4 && (upper.compare(0, 3, "COM") == 0 || upper.compare(0, 3, "LPT") == 0) &&
			upper[3] >= '1' && upper[3] <= '9';
	}
}

FileNameProblem checkFileName(const std::string& name)
{
	if (name.empty())
		return FileNameProblem::Empty;
	if (!std::all_of(name.begin(), name.end(), allowed))
		return FileNameProblem::BadCharacter;
	if (name.front() == ' ' || name.back() == ' ')
		return FileNameProblem::EdgeSpace;
	if (reservedOnWindows(name))
		return FileNameProblem::Reserved;
	return FileNameProblem::None;
}
