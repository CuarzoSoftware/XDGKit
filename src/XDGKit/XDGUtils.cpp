#include <XDGKit/XDGUtils.h>
#include <sstream>

using namespace XDG;

std::vector<std::string> XDGUtils::splitString(const std::string &input, char delimiter, bool trim) noexcept
{
    std::vector<std::string> result;
    result.reserve(32);
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        if (trim) result.emplace_back(trimString(token));
        else result.emplace_back(token);
    }

    return result;
}
