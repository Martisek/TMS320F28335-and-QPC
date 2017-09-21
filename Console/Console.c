/*
 * Console.c
 *
 *  Created on: 19. 9. 2017
 *      Author: martisek
 */

#include "Console.h"

/* Module variables */
static Console ConsoleObject;
QActive *AO_Console = &ConsoleObject.super;


/*-----------------------------------------------------------------------------------------------*/
char TextMenu_1[] = "1. Menu       	     \n";
char TextMenu_2[] = "2. Enable injector  \n";
char TextMenu_3[] = "3. Disable injector \n";
char TextMenu_4[] = "4. Parameters       \n";
char TextMenu_5[] = "5. Variables        \n";

MenuMsgItem MenuMsgItem_5 = {TextMenu_5, NULL           };
MenuMsgItem MenuMsgItem_4 = {TextMenu_4, &MenuMsgItem_5 };
MenuMsgItem MenuMsgItem_3 = {TextMenu_3, &MenuMsgItem_4 };
MenuMsgItem MenuMsgItem_2 = {TextMenu_2, &MenuMsgItem_3 };
MenuMsgItem MenuMsgItem_1 = {TextMenu_1, &MenuMsgItem_2 };

char ErrorText[] = "\nStav Tx IDLE!!\n";

MenuMsgItem *menuPtr = &MenuMsgItem_1;

/*-----------------------------------------------------------------------------------------------*/
void ConsoleCtor(void);

static QState Console_init(Console *me, QEvt const *e);
static QState Console_active(Console *me, QEvt const *e);
static QState Console_transfer(Console *me, QEvt const *e);
static QState Console_tx_menu(Console *me, QEvt const *e);
static QState Console_tx_msg(Console *me, QEvt const *e);
static QState Console_tx_idle(Console *me, QEvt const *e);
static QState Console_receive(Console *me, QEvt const *e);
static QState Console_rx_msg(Console *me, QEvt const *e);
static QState Console_rx_parsing(Console *me, QEvt const *e);
static QState Console_rx_idle(Console *me, QEvt const *e);

void ConsoleCtor(void)
{
	Console *me = &ConsoleObject;
	QActive_ctor(&me->super, Q_STATE_CAST(&Console_init));
	QTimeEvt_ctor(&me->timeEvt, CONSOLE_TMOUT);
	//QEQueue_init(&me->deferredEvtQueue, (QEvt const**)(me->deferredEvtSto), Q_DIM(me->deferredEvtSto));

	return;
}

static QState Console_init(Console *me, QEvt const *e)
{
	(void)e;
	me->firstMenuMsgItem = &MenuMsgItem_1;
	return Q_TRAN(&Console_transfer);
}

static QState Console_active(Console *me, QEvt const *e)
{
	QState state;

	switch (e->sig) {
		case Q_ENTRY_SIG: {
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		case Q_INIT_SIG: {
			state = Q_TRAN(&Console_transfer);
			break;
		}
		case CONSOLE_RX_MSG: {
			state = Q_TRAN(&Console_receive);
			break;
		}
		default: {
			state = Q_SUPER(&QHsm_top);
			break;
		}
	}

	return state;
}

static QState Console_transfer(Console *me, QEvt const *e)
{
	QState state;

	switch (e->sig) {
		case Q_ENTRY_SIG: {
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		case Q_INIT_SIG: {
			state = Q_TRAN(&Console_tx_menu);
			break;
		}
		default: {
			state = Q_SUPER(&Console_active);
			break;
		}
	}

	return state;
}

static QState Console_tx_idle(Console *me, QEvt const *e)
{
	QState state;

	switch (e->sig) {
		case Q_ENTRY_SIG: {
			Console_WriteBlocking(Console_ConfigPtr, ErrorText, strlen(ErrorText));
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		case CONSOLE_TX_MENU: {
			state = Q_TRAN(&Console_tx_menu);
			break;
		}
		case CONSOLE_TX_MSG: {
			state = Q_TRAN(&Console_tx_msg);
		}
		default: {
			state = Q_SUPER(&Console_transfer);
			break;
		}
	}

	return state;
}

static QState Console_tx_menu(Console *me, QEvt const *e)
{
	QState state;
	MenuMsgItem *menuItem = me->firstMenuMsgItem;
	console_status_t console_state;

	switch (e->sig) {
		case Q_ENTRY_SIG: {
			console_state = Console_TransferWriteNonBlocking(Console_ConfigPtr, Console_Tx_HandlePtr, menuItem->MenuText, strlen(menuItem->MenuText));
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		case CONSOLE_TX_DONE: {
			menuItem = menuItem->nextMenuMsgItem;
			me->firstMenuMsgItem = menuItem;
			if (menuItem != NULL) {
				console_state = Console_TransferWriteNonBlocking(Console_ConfigPtr, Console_Tx_HandlePtr, menuItem->MenuText, strlen(menuItem->MenuText));
				if (console_state != status_Console_OKState)
					state = Q_TRAN(&Console_tx_idle);
				else
					state = Q_HANDLED();
			}
			else {
				state = Q_TRAN(&Console_tx_idle);
			}
			break;
		}
		default: {
			state = Q_SUPER(&Console_transfer);
			break;
		}
	}

	return state;
}

static QState Console_tx_msg(Console *me, QEvt const *e)
{
	(void)e;

	return Q_HANDLED();
}

static QState Console_receive(Console *me, QEvt const *e)
{
	QState state;

	switch (e->sig) {
		case Q_ENTRY_SIG: {
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		case CONSOLE_RX_DONE: {
			state = Q_HANDLED();
			break;
		}
		default: {
			state = Q_SUPER(&Console_active);
			break;
		}
	}

	return state;
}

static QState Console_rx_msg(Console *me, QEvt const *e)
{
	(void)e;

	return Q_HANDLED();
}
static QState Console_rx_parsing(Console *me, QEvt const *e)
{
	(void)e;

	return Q_HANDLED();
}

static QState Console_rx_idle(Console *me, QEvt const *e)
{
	(void)e;

	return Q_HANDLED();
}

/*************************************************************************************************/





