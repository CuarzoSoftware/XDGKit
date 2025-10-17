#include <CZ/XDG/XDGLog.h>

using namespace CZ;

const CZ::CZLogger &XDGLogger() noexcept
{
    static CZLogger logger { "XDGKit", "CZ_XDGKIT_LOG_LEVEL" };
    return logger;
}
