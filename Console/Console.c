/*
 * Console_Tx.c
 *
 *  Created on: 19. 9. 2017
 *      Author: martisek
 */

#include "Console.h"

/* Module variables */
static Console_Tx ConsoleObjectTx;
QActive *AO_ConsoleTx = &ConsoleObjectTx.super;

static Console_Rx ConsoleObjectRx;
QActive *AO_ConsoleRx = &ConsoleObjectRx.super;

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

char InfoText_1[] = "\nStav Tx IDLE!!\n";

MenuMsgItem *menuPtr = &MenuMsgItem_1;
MenuMsgItem *menuItem;

/*-----------------------------------------------------------------------------------------------*/
/*< Function prototypes >*/
void ConsoleTxCtor(void);
void ConsoleRxCtor(void);

/*< Static function for TX active object >*/
static QState Console_tx_init(Console_Tx *me, QEvt const *e);
static QState Console_transfer(Console_Tx *me, QEvt const *e);
static QState Console_tx_menu(Console_Tx *me, QEvt const *e);
static QState Console_tx_msg(Console_Tx *me, QEvt const *e);
static QState Console_tx_idle(Console_Tx *me, QEvt const *e);
static QState Console_tx_error(Console_Tx *me, QEvt const *e);

/*< Static function for RX active object >*/
static QState Console_rx_init(Console_Rx *me, QEvt const *e);
static QState Console_receive(Console_Rx *me, QEvt const *e);
static QState Console_rx_parsing(Console_Rx *me, QEvt const *e);
static QState Console_rx_idle(Console_Rx *me, QEvt const *e);
static QState Console_rx_error(Console_Rx *me, QEvt const *e);

/*-----------------------------------------------------------------------------------------------*/
/*																								 */
/*< Function for Tx part (active object) >*/
void ConsoleTxCtor(void)
{
	Console_Tx *me = &ConsoleObjectTx;
	QActive_ctor(&me->super, Q_STATE_CAST(&Console_tx_init));
	QTimeEvt_ctor(&me->timeEvt, CONSOLE_TMOUT);
	QEQueue_init(&me->deferredEvtQueue, (QEvt const**)(me->deferredEvtSto), Q_DIM(me->deferredEvtSto));

	return;
}

static QState Console_tx_init(Console_Tx *me, QEvt const *e)
{
	(void)e;
	me->firstMenuMsgItem = &MenuMsgItem_1;
	return Q_TRAN(&Console_transfer);
}

static QState Console_transfer(Console_Tx *me, QEvt const *e)
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
			state = Q_TRAN(&Console_tx_idle);
			break;
		}

		case CONSOLE_TX_MENU: {
			if (QEQueue_getNFree(&me->deferredEvtQueue) > 0) {
				QActive_defer(&me->super, &me->deferredEvtQueue, e);
			}
			state = Q_HANDLED();
			break;
		}
		case CONSOLE_TX_MSG: {
			if (QEQueue_getNFree(&me->deferredEvtQueue) > 0) {
				QActive_defer(&me->super, &me->deferredEvtQueue, e);
			}
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

static QState Console_tx_idle(Console_Tx *me, QEvt const *e)
{
	QState state;
	ConsoleTxEvent const *TxEvent = (ConsoleTxEvent*)e;
	console_status_t console_state;
	menuItem = me->firstMenuMsgItem;

	switch (e->sig) {
		case Q_ENTRY_SIG: {
			Console_WriteBlocking(Console_ConfigPtr, InfoText_1, strlen(InfoText_1));
			TxEvent = (ConsoleTxEvent*)QActive_recall((QActive*)me, &me->deferredEvtQueue);
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		case CONSOLE_TX_MSG: {
			console_state = Console_TransferWriteNonBlocking(Console_ConfigPtr, Console_Tx_HandlePtr, TxEvent->textstring, strlen(TxEvent->textstring));
			if (console_state != status_Console_OKState)
				state = Q_TRAN(&Console_tx_error);
			else
				state = Q_TRAN(&Console_tx_msg);
			break;
		}
		case CONSOLE_TX_MENU: {
			console_state = Console_TransferWriteNonBlocking(Console_ConfigPtr, Console_Tx_HandlePtr, menuItem->MenuText, strlen(menuItem->MenuText));
			if (console_state != status_Console_OKState)
				state = Q_TRAN(&Console_tx_error);
			else
				state = Q_TRAN(&Console_tx_menu);
			break;
		}
		default: {
			state = Q_SUPER(&Console_transfer);
			break;
		}
	}

	return state;
}

static QState Console_tx_menu(Console_Tx *me, QEvt const *e)
{
	QState state;
	console_status_t console_state;

	switch (e->sig) {
		case Q_ENTRY_SIG: {
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		case CONSOLE_TX_DONE: {
			menuItem = menuItem->nextMenuMsgItem;
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

static QState Console_tx_msg(Console_Tx *me, QEvt const *e)
{
	QState state;
	ConsoleTxEvent *TxEvent = (ConsoleTxEvent*)e;

	switch (e->sig) {
		case Q_ENTRY_SIG: {
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		case CONSOLE_TX_DONE: {
			state = Q_TRAN(&Console_tx_idle);
			break;
		}
		default: {
			state = Q_SUPER(&Console_transfer);
			break;
		}
	}
	return state;
}

static QState Console_tx_error(Console_Tx *me, QEvt const *e)
{
	QState state;

	switch(e->sig) {
		case Q_ENTRY_SIG: {
			state = Q_HANDLED();
			break;
		}
		case Q_EXIT_SIG: {
			state = Q_HANDLED();
			break;
		}
		default: {
			state = Q_SUPER(&Console_transfer);
			break;
		}
	}

	return state;
}

/*-----------------------------------------------------------------------------------------------*/
/*																								 */
/*< Functions for Rx part (active object) >*/
static QState Console_rx_init(Console_Rx *me, QEvt const *e)
{
	(void)e;
	return Q_TRAN(&Console_receive);
}

static QState Console_receive(Console_Rx *me, QEvt const *e)
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
			state = Q_TRAN(&Console_rx_idle);
			break;
		}
		default: {
			state = Q_SUPER(&QHsm_top);
			break;
		}
	}

	return state;
}

static QState Console_rx_parsing(Console_Rx *me, QEvt const *e)
{
	(void)e;

	return Q_HANDLED();
}

static QState Console_rx_idle(Console_Rx *me, QEvt const *e)
{
	(void)e;

	return Q_HANDLED();
}

/*************************************************************************************************/





