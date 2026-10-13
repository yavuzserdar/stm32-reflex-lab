#ifndef REFLEX_H
#define REFLEX_H
#include <stdbool.h>
#include <stdint.h>

#define REFLEX_ROUNDS 10U
#define REFLEX_TIMEOUT_US 5000000U
typedef enum { IDLE, RELEASE, WAIT, REACT, RESULT, COMPLETE } ReflexState;
typedef enum {
    EVENT_NONE, EVENT_START, EVENT_WAIT, EVENT_GO,
    EVENT_EARLY, EVENT_TIMEOUT, EVENT_SCORE, EVENT_COMPLETE
} ReflexEvent;
typedef struct {
    ReflexState state;
    uint32_t since, delay, signal_at, random;
    uint32_t reaction, best, total, valid, early, timeouts, session;
} Reflex;

void Reflex_Init(Reflex *game);
/* now and edge are unsigned microsecond timestamps, including across wrap. */
ReflexEvent Reflex_Step(Reflex *game, uint32_t now, bool press,
                       uint32_t edge, bool released);
#endif
