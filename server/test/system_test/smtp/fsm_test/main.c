#include "smtp-states-fsm.h"
#include <stddef.h>

int main() {
    te_autofsm_state state = AUTOFSM_ST_INIT;
    state = autofsm_step(state, AUTOFSM_EV_HELO, NULL); // goto state AUTOFSM_ST_INIT
    state = autofsm_step(state, AUTOFSM_EV_READ, NULL);// goto state AUTOFSM_ST_INVALID

}
