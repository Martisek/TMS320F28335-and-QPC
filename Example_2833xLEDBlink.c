//###########################################################################
// Description:
//! \addtogroup f2833x_example_list
//!  <h1>Timer based blinking LED (timed_led_blink)</h1>
//!
//! This example configures CPU Timer0 for a 500 msec period, and toggles the GPIO32
//! LED on the 2833x eZdsp once per interrupt. For testing purposes, this example
//! also increments a counter each time the timer asserts an interrupt.
//!
//!  \b Watch \b Variables \n
//!  - CpuTimer0.InterruptCount
//!
//! \b External \b Connections \n
//!  - Monitor the GPIO32 LED blink on (for 500 msec) and off (for 500 msec) on
//!    the 2833x eZdsp.
//
//###########################################################################
// $TI Release: F2833x/F2823x Header Files and Peripheral Examples V140 $
// $Release Date: March  4, 2015 $
// $Copyright: Copyright (C) 2007-2015 Texas Instruments Incorporated -
//             http://www.ti.com/ ALL RIGHTS RESERVED $
//###########################################################################

#include <string.h>

#include "DSP28x_Project.h"     // Device Headerfile and Examples Include File
#include "qp_port.h"
#include "Blinky.h"
#include "Console_BSP.h"
#include "Console.h"

// Prototype statements for functions found within this file.
__interrupt void cpu_timer0_isr(void);

void ConsoleTxDone(console_handle_tx_t *handle, void *UserData);
void ConsoleRecDone(console_handle_tx_t *handle, void *UserData);

char ConsoleText[] = "\nTest of console in CI module! \n";
char ConsoleRecData[30];
size_t rcBytes;

int strl;
QEvt TxDoneEvent = {CONSOLE_TX_DONE, 0, 0};
QEvt TxMenuGoEvt = {CONSOLE_TX_MENU, 0, 0};

void main(void)
{
	static QEvt const *l_blinkySto[15];
	static QEvt const *l_consoleSto[15];
	static ConsoleTxEvent smallPoolSto[10];

	console_status_t Console_state;
	char indata;

// Step 1. Initialize System Control:
// PLL, WatchDog, enable Peripheral Clocks
// This example function is found in the DSP2833x_SysCtrl.c file.
   InitSysCtrl();

// Step 2. Initialize GPIO:
// This example function is found in the DSP2833x_Gpio.c file and
// illustrates how to set the GPIO to it's default state.
// InitGpio();  // Skipped for this example

// Step 3. Clear all interrupts and initialize PIE vector table:
// Disable CPU interrupts
   DINT;

// Initialize the PIE control registers to their default state.
// The default state is all PIE interrupts disabled and flags
// are cleared.
// This function is found in the DSP2833x_PieCtrl.c file.
   InitPieCtrl();

// Disable CPU interrupts and clear all CPU interrupt flags:
   IER = 0x0000;
   IFR = 0x0000;

// Initialize the PIE vector table with pointers to the shell Interrupt
// Service Routines (ISR).
// This will populate the entire table, even if the interrupt
// is not used in this example.  This is useful for debug purposes.
// The shell ISR routines are found in DSP2833x_DefaultIsr.c.
// This function is found in DSP2833x_PieVect.c.
   InitPieVectTable();

// Interrupts that are used in this example are re-mapped to
// ISR functions found within this file.
   EALLOW;  // This is needed to write to EALLOW protected registers
   PieVectTable.TINT0 = &cpu_timer0_isr;
   EDIS;    // This is needed to disable write to EALLOW protected registers

// Step 4. Initialize the Device Peripheral. This function can be
//         found in DSP2833x_CpuTimers.c
   InitCpuTimers();   // For this example, only initialize the Cpu Timers
#if (CPU_FRQ_150MHZ)
// Configure CPU-Timer 0 to interrupt every 500 milliseconds:
// 150MHz CPU Freq, 50 millisecond Period (in uSeconds)
   ConfigCpuTimer(&CpuTimer0, 150, 250000);
#endif
#if (CPU_FRQ_100MHZ)
// Configure CPU-Timer 0 to interrupt every 500 milliseconds:
// 100MHz CPU Freq, 50 millisecond Period (in uSeconds)
   ConfigCpuTimer(&CpuTimer0, 100, 500000);
#endif

// To ensure precise timing, use write-only instructions to write to the entire register. Therefore, if any
// of the configuration bits are changed in ConfigCpuTimer and InitCpuTimers (in DSP2833x_CpuTimers.h), the
// below settings must also be updated.

  // CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
   CpuTimer0Regs.TCR.bit.TSS = 0;

// Step 5. User specific code, enable interrupts:

// Configure GPIO32 as a GPIO output pin
   EALLOW;
   GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 0;
   GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;

   GpioCtrlRegs.GPBMUX1.bit.GPIO34 = 0;
   GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;

   GpioCtrlRegs.GPBMUX2.bit.GPIO49 = 0;
   GpioCtrlRegs.GPBDIR.bit.GPIO49 = 1;
   EDIS;

   GpioDataRegs.GPACLEAR.bit.GPIO9 = 1; // turn off LED1 on kit


   memset(ConsoleRecData, 0, 30);
   rcBytes = 0;

   strl = strlen(ConsoleText);
   Console_GetDefaultConfig(Console_ConfigPtr);

   (void)Console_Init(Console_ConfigPtr, 150000000);

   // Enable global Interrupts and higher priority real-time debug events:
   EINT;   // Enable Global interrupt INTM
   ERTM;   // Enable Global realtime interrupt DBGM

   (void)Console_TxCreateHandle(Console_ConfigPtr, Console_Tx_HandlePtr, &ConsoleTxDone, (void*)0);
   (void)Console_RxCreateHandle(Console_ConfigPtr, Console_Rx_HandlePtr, &ConsoleRecDone, (void*)0);

   /*
   while (menuPtr != NULL) {
   Console_state = Console_TransferWriteNonBlocking(Console_ConfigPtr, Console_Tx_HandlePtr, menuPtr->MenuText, strlen(menuPtr->MenuText));
   menuPtr = menuPtr->nextMenuMsgItem;
   DELAY_US(1000000);
   }
	*/

   //Console_state = Console_WriteBlocking(Console_ConfigPtr, ConsoleText, strlen(ConsoleText));
   //Console_state = Console_TransferReadNonBlocking(Console_ConfigPtr, Console_Rx_HandlePtr, ConsoleRecData, 10, &rcBytes, 10);
   //(void)Console_WriteBlocking(Console_ConfigPtr, ConsoleText, strlen(ConsoleText));

   QF_init();
   BSP_init();

   QF_zero();

   BlinkyCtor();
   ConsoleTxCtor();

   QF_poolInit(smallPoolSto, sizeof(smallPoolSto), sizeof(smallPoolSto[0]));

   QActive_start(AO_Blinky,    2, l_blinkySto,  Q_DIM(l_blinkySto),  (void*)0, 0, (QEvt*)0);
   QActive_start(AO_ConsoleTx, 1, l_consoleSto, Q_DIM(l_consoleSto), (void*)0, 0, (QEvt*)0);

   QActive_postFIFO(AO_ConsoleTx, &TxMenuGoEvt);

   (void)QF_run();


// Step 6. IDLE loop. Just sit and loop forever (optional):

   for(;;);
}


__interrupt void cpu_timer0_isr(void)
{
   CpuTimer0.InterruptCount++;
   GpioDataRegs.GPBTOGGLE.bit.GPIO49 = 1; // Toggle GPIO32 once per 500 milliseconds
   //QF_tick();
   QF_TICK(&l_cpu_timer0_isr);
   // Acknowledge this interrupt to receive more interrupts from group 1

   CpuTimer0Regs.TCR.bit.TIF = 1;
   PieCtrlRegs.PIEACK.all |= PIEACK_GROUP1;
}

#pragma CODE_SECTION(ConsoleTxDone,"ramfuncs");
void ConsoleTxDone(console_handle_tx_t *handle, void *UserData)
{
	(void*)handle;


	QActive_postFIFO(AO_ConsoleTx, &TxDoneEvent);

	return;
}

#pragma CODE_SECTION(ConsoleRecDone,"ramfuncs");
void ConsoleRecDone(console_handle_tx_t *handle, void *UserData)
{
	(void*)handle;

	GpioDataRegs.GPASET.bit.GPIO9 = 1;

	return;
}


//===========================================================================
// No more.
//===========================================================================
