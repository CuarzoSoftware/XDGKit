#ifndef XDGMAP_H
#define XDGMAP_H

#include <unordered_map>

namespace XDG
{
    /**
     * @brief Alias for std::unordered_map
     */
    template <class Key, class T>
    using XDGMap = std::unordered_map<Key, T>;
}

#endif // XDGMAP_H
