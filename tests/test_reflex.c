#include "reflex.h"
#include <assert.h>
#include <stdio.h>

static uint32_t start(Reflex *g, uint32_t at)
{
    assert(Reflex_Step(g, at, true, at, false) == EVENT_START);
    assert(Reflex_Step(g, at + 10, false, 0, false) == EVENT_NONE);
    assert(g->state == RELEASE);
    at += 30000;
    assert(Reflex_Step(g, at, false, 0, true) == EVENT_WAIT);
    assert(g->delay >= 2000000 && g->delay <= 5000000);
    at += g->delay;
    assert(Reflex_Step(g, at - 1, false, 0, true) == EVENT_NONE);
    assert(Reflex_Step(g, at, false, 0, true) == EVENT_GO);
    return at;
}

int main(void)
{
    Reflex g;
    Reflex_Init(&g);
    assert(Reflex_Step(&g, 0, false, 0, true) == EVENT_NONE);
    uint32_t now = start(&g, 100000);
    /* Timestamp captured in IRQ wins over foreground processing delay. */
    assert(Reflex_Step(&g, now + 400000, true, now + 250000, false) == EVENT_SCORE);
    assert(g.reaction == 250000 && g.best == 250000 && g.valid == 1);
    for (unsigned i = 1; i < REFLEX_ROUNDS; ++i) {
        now += 2000000;
        assert(Reflex_Step(&g, now, false, 0, true) == EVENT_NONE);
        assert(Reflex_Step(&g, now, false, 0, true) == EVENT_WAIT);
        now += g.delay;
        assert(Reflex_Step(&g, now, false, 0, true) == EVENT_GO);
        now += 200000;
        ReflexEvent event = Reflex_Step(&g, now, true, now, false);
        assert(event == (i == 9 ? EVENT_COMPLETE : EVENT_SCORE));
    }
    assert(g.valid == 10 && g.best == 200000 && g.total / g.valid == 205000);
    assert(g.state == COMPLETE);
    now = start(&g, now + 100000);
    assert(g.session == 2 && g.valid == 0 && g.early == 0 && g.timeouts == 0);
    assert(Reflex_Step(&g, now + REFLEX_TIMEOUT_US, false, 0, true) == EVENT_TIMEOUT);
    assert(g.timeouts == 1 && g.valid == 0);

    Reflex_Init(&g);
    Reflex_Step(&g, 100, true, 100, false);
    Reflex_Step(&g, 30100, false, 0, true);
    /* Even if poll is late, an edge preceding LED activation is early. */
    assert(Reflex_Step(&g, 6000000, true, 40000, false) == EVENT_EARLY);
    assert(g.early == 1 && g.valid == 0);

    Reflex_Init(&g);
    now = start(&g, UINT32_MAX - 10000);
    /* Cross wrap during reaction itself too. */
    g.signal_at = UINT32_MAX - 100000;
    assert(Reflex_Step(&g, 200000, true, 149999, false) == EVENT_SCORE);
    assert(g.reaction == 250000);

    Reflex_Init(&g);
    now = start(&g, 0);
    assert(Reflex_Step(&g, now + REFLEX_TIMEOUT_US + 1000, true,
                       now + REFLEX_TIMEOUT_US - 1, false) == EVENT_SCORE);
    Reflex_Init(&g);
    now = start(&g, 0);
    assert(Reflex_Step(&g, now + REFLEX_TIMEOUT_US, true,
                       now + REFLEX_TIMEOUT_US, false) == EVENT_TIMEOUT);
    puts("PASS: 10 rounds, statistics, restart, early press, timeout, captured edge, timer wrap.");
    return 0;
}
