#include <utility>

#include "keys.hpp"

static tb_event tb_ev;

namespace keys {

bool poll_event(event &ev)
{
    if (!tb_poll_event(&tb_ev))
        return false;

    ev.type = type(tb_ev.type);
    ev.key  = key(tb_ev.key);
    ev.ch   = tb_ev.ch;

    ev.w    = tb_ev.w;
    ev.h    = tb_ev.h;
    ev.x    = tb_ev.x;
    ev.y    = tb_ev.y;

    return true;
}

/* ns keys */
}
