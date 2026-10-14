#ifndef SAFEFILENAME_H
#define SAFEFILENAME_H

#include <string>

enum class FileNameProblem { None, Empty, BadCharacter, EdgeSpace, Reserved };

FileNameProblem checkFileName(const std::string& name);

#endif
