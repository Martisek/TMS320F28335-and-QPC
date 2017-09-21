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

typedef enum ConsoleSig ConsoleSig;
typedef struct MenuMsgItem MenuMsgItem;
typedef struct Console Console;

enum ConsoleSig
{
    CONSOLE_DUMMY_SIG 	= Q_USER_SIG,
    CONSOLE_TMOUT   	= Q_USER_SIG + 20,
    CONSOLE_TX_DONE     = Q_USER_SIG + 21,
	CONSOLE_TX_MENU		= Q_USER_SIG + 22,
    CONSOLE_TX_MSG		= Q_USER_SIG + 23,
	CONSOLE_RX_MSG		= Q_USER_SIG + 24,
	CONSOLE_RX_DONE		= Q_USER_SIG + 25,
	CONSOLE_RX_ERROR	= Q_USER_SIG + 26,
	CONSOLE_RX_PARSED	= Q_USER_SIG + 27,
};


struct MenuMsgItem
{
	char *MenuText;
	struct MenuMsgItem *nextMenuMsgItem;
};

struct Console
{
	QActive super;
	QTimeEvt timeEvt;
	MenuMsgItem *firstMenuMsgItem;

	//QEQueue deferredEvtQueue;
	//QEvt *deferredEvtSto[20];
	//char RxBuffer[20];
};

extern QActive *AO_Console;
extern MenuMsgItem *menuPtr;


void ConsoleCtor(void);


#endif /* CONSOLE_CONSOLE_H_ */
