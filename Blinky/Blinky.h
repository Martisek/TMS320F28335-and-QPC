/*
 * Blinky.h
 *
 *  Created on: 8. 9. 2017
 *      Author: martisek
 */

#ifndef BLINKY_BLINKY_H_
#define BLINKY_BLINKY_H_

#include "bsp.h"
#include "qp_port.h"

#define TICK_NUMBER 2

typedef enum BlinkySig BlinkySig;

enum BlinkySig
{
    DUMMY_SIG       = Q_USER_SIG,
    TIME_OUT_SIG    = Q_USER_SIG + 1,
    MAX_SIG         = Q_USER_SIG + 10
};

void BlinkyCtor(void);

extern QActive *AO_Blinky;

#endif /* BLINKY_BLINKY_H_ */
