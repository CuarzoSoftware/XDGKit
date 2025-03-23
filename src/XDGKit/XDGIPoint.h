#ifndef XDGIPOINT_H
#define XDGIPOINT_H

#include <XDGKit/XDGNamespace.h>
#include <cstdint>

/**
 * @brief A 2D point.
 *
 * This structure defines a point using its x and y coordinates.
 */
struct XDG::XDGIPoint
{
    int32_t x; /**< The x-coordinate of the point. */
    int32_t y; /**< The y-coordinate of the point. */
};

#endif // XDGIPOINT_H
