/*   -*- buffer-read-only: t -*- vi: set ro:
 *
 *  DO NOT EDIT THIS FILE   (smtp-states-fsm.c)
 *
 *  It has been AutoGen-ed
 *  From the definitions    /home/nrx/programming/cpp/C/smtp/server/src/lib/smtp/smtp-states.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (C) 1992-2018 Bruce Korb - all rights reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name ``Bruce Korb'' nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * AutoFSM IS PROVIDED BY Bruce Korb ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bruce Korb OR ANY OTHER CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define DEFINE_FSM
#include "smtp-states-fsm.h"
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
/* START === USER HEADERS === DO NOT CHANGE THIS COMMENT */
/* END   === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#ifndef NULL
#  define NULL 0
#endif

/**
 *  Enumeration of the valid transition types
 *  Some transition types may be common to several transitions.
 */
typedef enum {
    AUTOFSM_TR_BEGIN_TRANSACTION_HELO,
    AUTOFSM_TR_BEGIN_TRANSACTION_QUIT,
    AUTOFSM_TR_BEGIN_TRANSACTION_RCPT,
    AUTOFSM_TR_BEGIN_TRANSACTION_RSET,
    AUTOFSM_TR_BEGIN_TRANSACTION_TEST,
    AUTOFSM_TR_CLIENT_INIT_HELO,
    AUTOFSM_TR_CLIENT_INIT_MAIL,
    AUTOFSM_TR_CLIENT_INIT_QUIT,
    AUTOFSM_TR_CLIENT_INIT_RSET,
    AUTOFSM_TR_CLIENT_INIT_TEST,
    AUTOFSM_TR_INIT_HELO,
    AUTOFSM_TR_INIT_QUIT,
    AUTOFSM_TR_INIT_RSET,
    AUTOFSM_TR_INIT_TEST,
    AUTOFSM_TR_INVALID,
    AUTOFSM_TR_READ_DATA_END,
    AUTOFSM_TR_READ_DATA_READ,
    AUTOFSM_TR_TRANSACTION_DATA,
    AUTOFSM_TR_TRANSACTION_HELO,
    AUTOFSM_TR_TRANSACTION_QUIT,
    AUTOFSM_TR_TRANSACTION_RCPT,
    AUTOFSM_TR_TRANSACTION_RSET,
    AUTOFSM_TR_TRANSACTION_TEST
} te_autofsm_trans;
#define AUTOFSM_TRANSITION_CT  23

/**
 *  State transition handling map.  Map the state enumeration and the event
 *  enumeration to the new state and the transition enumeration code (in that
 *  order).  It is indexed by first the current state and then the event code.
 */
typedef struct autofsm_transition t_autofsm_transition;
struct autofsm_transition {
    te_autofsm_state  next_state;
    te_autofsm_trans  transition;
};
static const t_autofsm_transition
autofsm_trans_table[ AUTOFSM_STATE_CT ][ AUTOFSM_EVENT_CT ] = {

  /* STATE 0:  AUTOFSM_ST_INIT */
  { { AUTOFSM_ST_INIT, AUTOFSM_TR_INIT_TEST },      /* EVT:  TEST */
    { AUTOFSM_ST_INIT, AUTOFSM_TR_INIT_RSET },      /* EVT:  RSET */
    { AUTOFSM_ST_CLIENT_INIT, AUTOFSM_TR_INIT_HELO }, /* EVT:  HELO */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  MAIL */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  RCPT */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  DATA */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  READ */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  END */
    { AUTOFSM_ST_DONE, AUTOFSM_TR_INIT_QUIT }       /* EVT:  QUIT */
  },


  /* STATE 1:  AUTOFSM_ST_CLIENT_INIT */
  { { AUTOFSM_ST_CLIENT_INIT, AUTOFSM_TR_CLIENT_INIT_TEST }, /* EVT:  TEST */
    { AUTOFSM_ST_INIT, AUTOFSM_TR_CLIENT_INIT_RSET }, /* EVT:  RSET */
    { AUTOFSM_ST_CLIENT_INIT, AUTOFSM_TR_CLIENT_INIT_HELO }, /* EVT:  HELO */
    { AUTOFSM_ST_BEGIN_TRANSACTION, AUTOFSM_TR_CLIENT_INIT_MAIL }, /* EVT:  MAIL */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  RCPT */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  DATA */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  READ */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  END */
    { AUTOFSM_ST_DONE, AUTOFSM_TR_CLIENT_INIT_QUIT } /* EVT:  QUIT */
  },


  /* STATE 2:  AUTOFSM_ST_BEGIN_TRANSACTION */
  { { AUTOFSM_ST_BEGIN_TRANSACTION, AUTOFSM_TR_BEGIN_TRANSACTION_TEST }, /* EVT:  TEST */
    { AUTOFSM_ST_INIT, AUTOFSM_TR_BEGIN_TRANSACTION_RSET }, /* EVT:  RSET */
    { AUTOFSM_ST_CLIENT_INIT, AUTOFSM_TR_BEGIN_TRANSACTION_HELO }, /* EVT:  HELO */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  MAIL */
    { AUTOFSM_ST_TRANSACTION, AUTOFSM_TR_BEGIN_TRANSACTION_RCPT }, /* EVT:  RCPT */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  DATA */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  READ */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  END */
    { AUTOFSM_ST_DONE, AUTOFSM_TR_BEGIN_TRANSACTION_QUIT } /* EVT:  QUIT */
  },


  /* STATE 3:  AUTOFSM_ST_TRANSACTION */
  { { AUTOFSM_ST_TRANSACTION, AUTOFSM_TR_TRANSACTION_TEST }, /* EVT:  TEST */
    { AUTOFSM_ST_INIT, AUTOFSM_TR_TRANSACTION_RSET }, /* EVT:  RSET */
    { AUTOFSM_ST_CLIENT_INIT, AUTOFSM_TR_TRANSACTION_HELO }, /* EVT:  HELO */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  MAIL */
    { AUTOFSM_ST_TRANSACTION, AUTOFSM_TR_TRANSACTION_RCPT }, /* EVT:  RCPT */
    { AUTOFSM_ST_READ_DATA, AUTOFSM_TR_TRANSACTION_DATA }, /* EVT:  DATA */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  READ */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  END */
    { AUTOFSM_ST_DONE, AUTOFSM_TR_TRANSACTION_QUIT } /* EVT:  QUIT */
  },


  /* STATE 4:  AUTOFSM_ST_READ_DATA */
  { { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  TEST */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  RSET */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  HELO */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  MAIL */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  RCPT */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID },     /* EVT:  DATA */
    { AUTOFSM_ST_READ_DATA, AUTOFSM_TR_READ_DATA_READ }, /* EVT:  READ */
    { AUTOFSM_ST_CLIENT_INIT, AUTOFSM_TR_READ_DATA_END }, /* EVT:  END */
    { AUTOFSM_ST_INVALID, AUTOFSM_TR_INVALID }      /* EVT:  QUIT */
  }
};


#define AutofsmFsmErr_off     19
#define AutofsmEvInvalid_off  75
#define AutofsmStInit_off     83


static char const zAutofsmStrings[184] =
/*     0 */ "** OUT-OF-RANGE **\0"
/*    19 */ "FSM Error:  in state %d (%s), event %d (%s) is invalid\n\0"
/*    75 */ "invalid\0"
/*    83 */ "init\0"
/*    88 */ "client_init\0"
/*   100 */ "begin_transaction\0"
/*   118 */ "transaction\0"
/*   130 */ "read_data\0"
/*   140 */ "test\0"
/*   145 */ "rset\0"
/*   150 */ "helo\0"
/*   155 */ "mail\0"
/*   160 */ "rcpt\0"
/*   165 */ "data\0"
/*   170 */ "read\0"
/*   175 */ "end\0"
/*   179 */ "quit";

static const size_t aszAutofsmStates[5] = {
    83,  88,  100, 118, 130 };

static const size_t aszAutofsmEvents[10] = {
    140, 145, 150, 155, 160, 165, 170, 175, 179, 75 };


#define AUTOFSM_EVT_NAME(t)   ( (((unsigned)(t)) >= 10) \
    ? zAutofsmStrings : zAutofsmStrings + aszAutofsmEvents[t])

#define AUTOFSM_STATE_NAME(s) ( (((unsigned)(s)) >= 5) \
    ? zAutofsmStrings : zAutofsmStrings + aszAutofsmStates[s])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

static int autofsm_invalid_transition( te_autofsm_state st, te_autofsm_event evt );

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * */
/**
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
static int
autofsm_invalid_transition( te_autofsm_state st, te_autofsm_event evt )
{
    /* START == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */
    char const * fmt = zAutofsmStrings + AutofsmFsmErr_off;
    fprintf( stderr, fmt, st, AUTOFSM_STATE_NAME(st), evt, AUTOFSM_EVT_NAME(evt));
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return EXIT_FAILURE;
}

/**
 *  Step the FSM.  Returns the resulting state.  If the current state is
 *  AUTOFSM_ST_DONE or AUTOFSM_ST_INVALID, it resets to
 *  AUTOFSM_ST_INIT and returns AUTOFSM_ST_INIT.
 */
te_autofsm_state
autofsm_step(
    te_autofsm_state autofsm_state,
    te_autofsm_event trans_evt,
    void* cookie )
{
    te_autofsm_state nxtSt;
    te_autofsm_trans trans;

    if ((unsigned)autofsm_state >= AUTOFSM_ST_INVALID) {
        return AUTOFSM_ST_INIT;
    }

#ifndef __COVERITY__
    if (trans_evt >= AUTOFSM_EV_INVALID) {
        nxtSt = AUTOFSM_ST_INVALID;
        trans = AUTOFSM_TR_INVALID;
    } else
#endif /* __COVERITY__ */
    {
        const t_autofsm_transition * ttbl =
            autofsm_trans_table[ autofsm_state ] + trans_evt;
        nxtSt = ttbl->next_state;
        trans = ttbl->transition;
    }


    switch (trans) {
    case AUTOFSM_TR_BEGIN_TRANSACTION_HELO:
        /* START == BEGIN_TRANSACTION_HELO == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_BEGIN_TRANSACTION_HELO();
        /* END   == BEGIN_TRANSACTION_HELO == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_BEGIN_TRANSACTION_QUIT:
        /* START == BEGIN_TRANSACTION_QUIT == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_BEGIN_TRANSACTION_QUIT();
        /* END   == BEGIN_TRANSACTION_QUIT == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_BEGIN_TRANSACTION_RCPT:
        /* START == BEGIN_TRANSACTION_RCPT == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_BEGIN_TRANSACTION_RCPT();
        /* END   == BEGIN_TRANSACTION_RCPT == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_BEGIN_TRANSACTION_RSET:
        /* START == BEGIN_TRANSACTION_RSET == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_BEGIN_TRANSACTION_RSET();
        /* END   == BEGIN_TRANSACTION_RSET == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_BEGIN_TRANSACTION_TEST:
        /* START == BEGIN_TRANSACTION_TEST == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_BEGIN_TRANSACTION_TEST();
        /* END   == BEGIN_TRANSACTION_TEST == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_CLIENT_INIT_HELO:
        /* START == CLIENT_INIT_HELO == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_CLIENT_INIT_HELO();
        /* END   == CLIENT_INIT_HELO == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_CLIENT_INIT_MAIL:
        /* START == CLIENT_INIT_MAIL == DO NOT CHANGE THIS COMMENT */
 //       nxtSt = HANDLE_CLIENT_INIT_MAIL();
        /* END   == CLIENT_INIT_MAIL == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_CLIENT_INIT_QUIT:
        /* START == CLIENT_INIT_QUIT == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_CLIENT_INIT_QUIT();
        /* END   == CLIENT_INIT_QUIT == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_CLIENT_INIT_RSET:
        /* START == CLIENT_INIT_RSET == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_CLIENT_INIT_RSET();
        /* END   == CLIENT_INIT_RSET == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_CLIENT_INIT_TEST:
        /* START == CLIENT_INIT_TEST == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_CLIENT_INIT_TEST();
        /* END   == CLIENT_INIT_TEST == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_INIT_HELO:
        /* START == INIT_HELO == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_INIT_HELO();
        /* END   == INIT_HELO == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_INIT_QUIT:
        /* START == INIT_QUIT == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_INIT_QUIT();
        /* END   == INIT_QUIT == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_INIT_RSET:
        /* START == INIT_RSET == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_INIT_RSET();
        /* END   == INIT_RSET == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_INIT_TEST:
        /* START == INIT_TEST == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_INIT_TEST();
        /* END   == INIT_TEST == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_INVALID:
        /* START == INVALID == DO NOT CHANGE THIS COMMENT */
//        exit(autofsm_invalid_transition(autofsm_state, trans_evt));
        /* END   == INVALID == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_READ_DATA_END:
        /* START == READ_DATA_END == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_READ_DATA_END();
        /* END   == READ_DATA_END == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_READ_DATA_READ:
        /* START == READ_DATA_READ == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_READ_DATA_READ();
        /* END   == READ_DATA_READ == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_TRANSACTION_DATA:
        /* START == TRANSACTION_DATA == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_TRANSACTION_DATA();
        /* END   == TRANSACTION_DATA == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_TRANSACTION_HELO:
        /* START == TRANSACTION_HELO == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_TRANSACTION_HELO();
        /* END   == TRANSACTION_HELO == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_TRANSACTION_QUIT:
        /* START == TRANSACTION_QUIT == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_TRANSACTION_QUIT();
        /* END   == TRANSACTION_QUIT == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_TRANSACTION_RCPT:
        /* START == TRANSACTION_RCPT == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_TRANSACTION_RCPT();
        /* END   == TRANSACTION_RCPT == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_TRANSACTION_RSET:
        /* START == TRANSACTION_RSET == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_TRANSACTION_RSET();
        /* END   == TRANSACTION_RSET == DO NOT CHANGE THIS COMMENT */
        break;


    case AUTOFSM_TR_TRANSACTION_TEST:
        /* START == TRANSACTION_TEST == DO NOT CHANGE THIS COMMENT */
//        nxtSt = HANDLE_TRANSACTION_TEST();
        /* END   == TRANSACTION_TEST == DO NOT CHANGE THIS COMMENT */
        break;


    default:
        /* START == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
        autofsm_invalid_transition(autofsm_state, trans_evt);
        /* END   == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
    }


    /* START == FINISH STEP == DO NOT CHANGE THIS COMMENT */
    /* END   == FINISH STEP == DO NOT CHANGE THIS COMMENT */

    return nxtSt;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of smtp-states-fsm.c */
