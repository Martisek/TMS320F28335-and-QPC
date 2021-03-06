/*
 * Console.h
 *
 *  Created on: 19. 9. 2017
 *      Author: martisek
 */

#ifndef CONSOLE_CONSOLE_H_
#define CONSOLE_CONSOLE_H_

#include "Console_BSP.h"
#include "qp_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum ConsoleSig ConsoleSig;
typedef struct MenuMsgItem MenuMsgItem;

typedef struct Console_Tx Console_Tx;
typedef struct Console_Rx Console_Rx;

typedef struct ConsoleTxEvent ConsoleTxEvent;

enum ConsoleSig
{
    CONSOLE_DUMMY_SIG 	= Q_USER_SIG,
    CONSOLE_TMOUT   	= Q_USER_SIG + 20,
    CONSOLE_TX_DONE     = Q_USER_SIG + 21,	// if sending via interrupt is completly done
	CONSOLE_TX_MENU		= Q_USER_SIG + 22,	// command to send all menu
    CONSOLE_TX_MSG		= Q_USER_SIG + 23, 	// command to send string
	CONSOLE_RX_MSG		= Q_USER_SIG + 24,
	CONSOLE_RX_DONE		= Q_USER_SIG + 25,
	CONSOLE_RX_ERROR	= Q_USER_SIG + 26,
	CONSOLE_RX_PARSED	= Q_USER_SIG + 27,
	CONSOLE_RX_TMOUT    = Q_USER_SIG + 28,
};


struct MenuMsgItem
{
	char *MenuText;
	struct MenuMsgItem *nextMenuMsgItem;
};

/*< Active object struct for console Tx >*/
struct Console_Tx
{
	QActive super;					/*< Active object class >*/
	QTimeEvt timeEvt;				/*< Time event - timeout for sending all data >*/
	MenuMsgItem *firstMenuMsgItem;	/*< Pointer to first menu item >*/
	QEQueue deferredEvtQueue; 		/*< Thread-safe queue object for event deferring >*/
	QEvt *deferredEvtSto[20];		/*< Actual queue for queue object >*/

};

/*< Active object struct for console Rx >*/
struct Console_Rx
{
	QActive super;
	QTimeEvt timeEvt;
	//QEQueue deferredEvtQueue;
	//QEvt *deferredEvtSto[20];
	char *RxBuffer;
	size_t numRxWords;
};

struct ConsoleTxEvent
{
	QEvt super;
	char *textstring;
};


/*-----------------------------------------------------------------------------------------------*/

extern QActive *AO_ConsoleTx;
extern QActive *AO_ConsoleRx;
extern MenuMsgItem *menuPtr;


void ConsoleTxCtor(void);
void ConsoleRxCtor(void);


#endif /* CONSOLE_CONSOLE_H_ */
