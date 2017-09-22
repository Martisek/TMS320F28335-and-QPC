/**
*
*   \brief Source file for blinky example
*
*/


#include "blinky.h"


typedef struct Blinky Blinky;

struct Blinky
{
    QActive super;
    QTimeEvt timeEvt;
    uint16_t countTimer;
};

// global prototypes
static Blinky blinkyObject;
QActive *AO_Blinky = &blinkyObject.super;

//
static QState Blinky_init(Blinky *me, QEvt *e);
static QState Blinky_On(Blinky *me, QEvt *e);
static QState Blinky_Off(Blinky *me, QEvt *e);

void BlinkyCtor(void);

ConsoleTxEvent txEvent;
char TextMsg2Send[] = "\nMSG from TMS320F28335... :-) \n";

/********************************************************/

void BlinkyCtor(void)
{
    Blinky *me = &blinkyObject;
    QActive_ctor(&me->super, Q_STATE_CAST(&Blinky_init));
    QTimeEvt_ctor(&me->timeEvt, TIME_OUT_SIG);

    return;
}

static QState Blinky_init(Blinky *me, QEvt *e)
{
    me->countTimer = TICK_NUMBER;
    //QTimeEvt_arm_(&me->timeEvt, &me->super, BSP_TICKS_PER_SEC);
    QTimeEvt_postEvery(&me->timeEvt, (QActive *)me, BSP_TICKS_PER_SEC);
    return Q_TRAN(&Blinky_Off);
}

static QState Blinky_On(Blinky *me, QEvt *e)
{
    QState state;
    txEvent.super.sig = CONSOLE_TX_MSG;
    txEvent.textstring = TextMsg2Send;

    switch(e->sig){
        case Q_ENTRY_SIG: {
            me->countTimer = TICK_NUMBER;
            BSP_Led_On();
            state = Q_HANDLED();
            break;
        }

        case Q_EXIT_SIG: {
            state = Q_HANDLED();
            break;
        }

        case TIME_OUT_SIG: {
            me->countTimer--;
            if (me->countTimer == 0) {
            	QActive_postFIFO(AO_ConsoleTx, (QEvt*)&txEvent);
                state = Q_TRAN(&Blinky_Off);
            }
            else
                state = Q_HANDLED();
            break;
        }

        default: {
            state = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return state;
}

static QState Blinky_Off(Blinky *me, QEvt *e)
{
    QState state;

    switch(e->sig){
        case Q_ENTRY_SIG: {
            me->countTimer = TICK_NUMBER;
            BSP_Led_Off();
            state = Q_HANDLED();
            break;
        }

        case Q_EXIT_SIG: {
            state = Q_HANDLED();
            break;
        }

        case TIME_OUT_SIG: {
            me->countTimer--;
            if (me->countTimer == 0)
                state = Q_TRAN(&Blinky_On);
            else
                state = Q_HANDLED();
            break;
        }

        default: {
            state = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return state;
}
