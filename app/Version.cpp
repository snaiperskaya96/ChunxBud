#include "Version.h"

// hack alert
void std::__throw_logic_error(char const*)
{
    abort();
}

void std::__throw_out_of_range_fmt(char const*, ...)
{
    abort();
}

void std::__throw_length_error(char const*)
{
    abort();
}

Version ParseVersion(const std::string& VersionString)
{
    Version Out;
    size_t Dots[2] = { 0 };
    for (int I = 0; I < 2; I++)
    {
        Dots[I] = VersionString.find('.', (I == 0 ? 0 : Dots[I - 1] + 1));
        if (Dots[I] == std::string::npos)
        {
            return Out;
        }
    }

    
    Out.Major = std::atoi(VersionString.substr(0, Dots[0]).c_str());
    Out.Minor = std::atoi(VersionString.substr(Dots[0] + 1, Dots[1] - 1).c_str());
    Out.Patch = std::atoi(VersionString.substr(Dots[1] + 1).c_str());

    return Out;
}

EVersionCompareResult CompareVersion(const std::string& VersionString)
{
    std::string CurVersion = std::string(CurrentVersion);
    Version CurrentVersion = ParseVersion(CurVersion);
    Version ToBeCompared = ParseVersion(VersionString);

    if (!ToBeCompared.IsValid())
    {
        return EVersionCompareResult::InvalidVersion;
    }

    if (ToBeCompared.Major > CurrentVersion.Major)
    {
        return EVersionCompareResult::NewerVersion;
    }

    if (ToBeCompared.Major < CurrentVersion.Major)
    {
        return EVersionCompareResult::OlderVersion;
    }

    // if major are equal
    if (ToBeCompared.Minor > CurrentVersion.Minor)
    {
        return EVersionCompareResult::NewerVersion;
    }

    if (ToBeCompared.Minor < CurrentVersion.Minor)
    {
        return EVersionCompareResult::OlderVersion;
    }

    // if minor are equal
    if (ToBeCompared.Patch > CurrentVersion.Patch)
    {
        return EVersionCompareResult::NewerVersion;
    }

    if (ToBeCompared.Patch < CurrentVersion.Patch)
    {
        return EVersionCompareResult::OlderVersion;
    }

    return EVersionCompareResult::SameVersion;
}