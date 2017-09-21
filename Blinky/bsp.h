/*
 * bsp.h
 *
 *  Created on: 8. 9. 2017
 *      Author: martisek
 */

#ifndef BLINKY_BSP_H_
#define BLINKY_BSP_H_

#include "qp_port.h"

#define BSP_TICKS_PER_SEC    2U

void BSP_init(void);

void BSP_Led_On(void);
void BSP_Led_Off(void);

#endif /* BLINKY_BSP_H_ */
