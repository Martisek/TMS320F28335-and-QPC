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


char TextMsg2Send1[] = "\nMSG from TMS320F28335... :-) \n";
char TextMsg2Send2[] = "\nHello TI TMS320F28335... ;-) \n";

QEvt const menuEvt = {CONSOLE_TX_MENU, 0, 0};	// static, immutable event (see "const"!!)
ConsoleTxEvent const txEvent = {{CONSOLE_TX_MSG, 0, 0},  TextMsg2Send2};

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
    QTimeEvt_postEvery(&me->timeEvt, (QActive *)me, BSP_TICKS_PER_SEC);
    return Q_TRAN(&Blinky_Off);
}

static QState Blinky_On(Blinky *me, QEvt *e)
{
    QState state;

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
            if (me->countTimer == 0) {
            	//QActive_postFIFO(AO_ConsoleTx, (QEvt*)&txEvent);
                state = Q_TRAN(&Blinky_On);
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
