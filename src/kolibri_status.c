#include "kolibri.h"

const char *kb_status_str(kb_status status)
{
    switch (status) {
    case KB_OK:
        return "ok";
    case KB_ERR_TRUNCATED:
        return "truncated input";
    case KB_ERR_TOO_LARGE:
        return "output buffer too small";
    case KB_ERR_INVALID:
        return "invalid argument";
    case KB_ERR_LZ4_ZERO_OFFSET:
        return "LZ4 block: zero match offset";
    case KB_ERR_LZ4_OFFSET_RANGE:
        return "LZ4 block: match offset before start of output";
    case KB_ERR_UNSUPPORTED:
        return "unsupported compression format";
    case KB_ERR_OVERFLOW:
        return "receive buffer overflow";
    case KB_ERR_NOT_FOUND:
        return "key not found";
    case KB_ERR_LIMIT:
        return "decompressed size exceeds protocol limit";
    case KB_ERR_TIMEOUT:
        return "request timed out";
    case KB_ERR_CLOSED:
        return "connection closed";
    case KB_ERR_STATE:
        return "not valid in this session state";
    default:
        break;
    }
    return "unknown status";
}
