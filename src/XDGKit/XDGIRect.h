#ifndef XDGIRECT_H
#define XDGIRECT_H

#include <XDGKit/XDGNamespace.h>
#include <cstdint>

/**
 * @brief A rectangle.
 *
 * This structure defines a rectangle using two points:
 * the top-left corner (`x0`, `y0`) and the bottom-right corner (`x1`, `y1`).
 */
struct XDG::XDGIRect
{
    int32_t x0; /**< The x-coordinate of the top-left corner. */
    int32_t y0; /**< The y-coordinate of the top-left corner. */
    int32_t x1; /**< The x-coordinate of the bottom-right corner. */
    int32_t y1; /**< The y-coordinate of the bottom-right corner. */
};

#endif // XDGIRECT_H
