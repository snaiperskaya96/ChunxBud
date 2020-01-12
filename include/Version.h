#pragma once
#ifndef VERSION_H
#define VERSION_H
#include <string>

static constexpr char CurrentVersion[] = "1.0.4";

enum EVersionCompareResult
{
    SameVersion,
    NewerVersion,
    OlderVersion,
    InvalidVersion
};

struct Version 
{
    int Major = -1;
    int Minor = -1;
    int Patch = -1;

    bool IsValid() const { return Major != -1 && Minor != -1 && Patch != -1; }
};

Version ParseVersion(const std::string& VersionString);
EVersionCompareResult CompareVersion(const std::string& VersionString);

#endif