#include "reflex.h"
#include <string.h>

static uint32_t random_next(Reflex *g)
{
    uint32_t x = g->random;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    g->random = x;
    return x;
}

void Reflex_Init(Reflex *g)
{
    memset(g, 0, sizeof(*g));
    g->state = IDLE;
}

ReflexEvent Reflex_Step(Reflex *g, uint32_t now, bool press,
                       uint32_t edge, bool released)
{
    switch (g->state) {
    case IDLE:
    case COMPLETE:
        if (press) {
            uint32_t session = g->session + 1U;
            Reflex_Init(g);
            g->session = session;
            g->random = edge ^ 0xA341316CU;
            if (g->random == 0U) g->random = 1U;
            g->best = UINT32_MAX;
            g->state = RELEASE;
            return EVENT_START;
        }
        break;
    case RELEASE:
        if (released && !press) {
            g->delay = 2000000U + random_next(g) % 3000001U;
            g->since = now;
            g->state = WAIT;
            return EVENT_WAIT;
        }
        break;
    case WAIT:
        /* An edge captured before the light is always a false start. */
        if (press) {
            g->early++;
            g->state = RESULT;
            g->since = now;
            return EVENT_EARLY;
        }
        if ((uint32_t)(now - g->since) >= g->delay) {
            g->signal_at = now;
            g->state = REACT;
            return EVENT_GO;
        }
        break;
    case REACT:
        if (press && (uint32_t)(edge - g->signal_at) < REFLEX_TIMEOUT_US) {
            g->reaction = edge - g->signal_at;
            g->total += g->reaction;
            if (g->reaction < g->best) g->best = g->reaction;
            g->valid++;
            g->since = now;
            g->state = g->valid == REFLEX_ROUNDS ? COMPLETE : RESULT;
            return g->state == COMPLETE ? EVENT_COMPLETE : EVENT_SCORE;
        }
        if ((uint32_t)(now - g->signal_at) >= REFLEX_TIMEOUT_US) {
            g->timeouts++;
            g->state = RESULT;
            g->since = now;
            return EVENT_TIMEOUT;
        }
        break;
    case RESULT:
        if ((uint32_t)(now - g->since) >= 1500000U) g->state = RELEASE;
        break;
    }
    return EVENT_NONE;
}
