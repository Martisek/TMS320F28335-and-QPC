/*****************************************************************************
* Product: Console-based BSP, MinGW
* Last updated for version 5.6.0
* Last updated on  2015-12-18
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* https://state-machine.com
* mailto:info@state-machine.com
*****************************************************************************/
#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "bsp.h"



/*..........................................................................*/
void BSP_init(void)
{
    return;
}
/*..........................................................................*/
void BSP_Led_Off(void)
{
    GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
    return;
}
/*..........................................................................*/
void BSP_Led_On(void)
{
    GpioDataRegs.GPBSET.bit.GPIO34 = 1;
    return;
}
/*--------------------------------------------------------------------------*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line)
{
    /* Next two lines for debug only to halt the processor here.
    * YOU need to change this policy for the production release!
    */
    asm(" ESTOP0");
    for(;;) {
    }
}

/*..........................................................................*/
void QF_onStartup(void) {


    CpuTimer0Regs.TCR.bit.TSS = 0;     /* start the system clock tick timer */

    // Enable CPU INT1, which is connected to CPU-Timer 0:
    IER |= M_INT1;

    // Enable PIE: Group 1 interrupt 7 which is connected to CPU-Timer 0:
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

    // Enable higher priority real-time debug events:
    ERTM;   // Enable Global realtime interrupt DBGM
}
/*..........................................................................*/
void QF_onCleanup(void) {                               /* nothing to clear */
}
/*..........................................................................*/
#pragma CODE_SECTION(QF_onIdle, "ramfuncs");      /* place in RAM for speed */
void QF_onIdle(void) {

#ifdef Q_SPY

    if (SciaRegs.SCICTL2.bit.TXRDY != 0) {                     /* TX ready? */
        uint16_t b = QS_getByte();
        if (b != QS_EOD) {                              /* not End-Of-Data? */
            SciaRegs.SCITXBUF = b;                  /* put into the TX FIFO */
        }
    }

#elif defined NDEBUG
    /* Put the CPU and peripherals to the low-power mode.
    * you might need to customize the clock management for your application,
    * see the datasheet for your particular TMS320C2000 device.
    */
    asm(" IDLE");               /* go to IDLE mode with interrupts DISABLED */
#endif

    QF_INT_ENABLE();                        /* always unlock the interrupts */
}






