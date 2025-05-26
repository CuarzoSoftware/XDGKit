#ifndef XDGUTILS_H
#define XDGUTILS_H

#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>

namespace XDG
{
    /**
     * @brief Utilities for data structures.
     */
    namespace XDGUtils
    {
        /**
         * @brief Removes leading and trailing whitespace from a string.
         *
         * @param str The input string to be trimmed.
         * @return A new string with leading and trailing spaces removed.
         */
        inline std::string trimString(const std::string &str) noexcept
        {
            const size_t start { str.find_first_not_of(" \t") };
            const size_t end { str.find_last_not_of(" \t") };
            return (start != std::string::npos && end != std::string::npos) ? str.substr(start, end - start + 1) : "";
        }

        /**
         * @brief Splits a string into a vector of substrings based on a delimiter.
         *
         * @param input The string to split.
         * @param delimiter The character used to split the string.
         * @param trim If `true`, removes leading and trailing spaces from each substring.
         * @return A vector of strings resulting from the split operation.
         */
        std::vector<std::string> splitString(const std::string &input, char delimiter, bool trim = true) noexcept;
        std::vector<std::string> splitString(const std::string_view &input, char delimiter, bool trim = true) noexcept;

        /**
         * @brief Removes duplicate elements from a vector in place.
         *
         * @tparam T The type of elements in the vector.
         * @param vec The vector from which duplicates will be removed.
         */
        template<class T>
        inline void removeDuplicates(std::vector<T> &vec) noexcept
        {
            std::unordered_set<T> seen;
            auto it = vec.begin();

            while (it != vec.end())
            {
                if (seen.find(*it) != seen.end())
                    it = vec.erase(it);
                else
                {
                    seen.insert(*it);
                    ++it;
                }
            }
        }

        // Makes sure the string ends before end and increments by the string len + 1
        inline char *advanceStrPosSafe(char *pos, char *end) noexcept
        {
            while (pos < end && *pos != '\0')
                pos++;

            return (pos < end) ? pos + 1 : nullptr;
        }

        inline char *readSafeAndAdvancePos(void *dst, void *pos, void *end, size_t size) noexcept
        {   
            if ((char*)pos + size > (char*)end)
                return nullptr;

            memcpy(dst, pos, size);
            return (char*)pos + size;
        }

        inline char *advanceSafe(void *pos, void *end, size_t size) noexcept
        {
            if ((char*)pos + size > (char*)end)
                return nullptr;

            return (char*)pos + size;
        }
    }
}

#endif // XDGUTILS_H
