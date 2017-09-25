/*
 * Console_BSP.c
 *
 *  Created on: 12. 9. 2017
 *      Author: martisek
 */
#include "Console_BSP.h"

/*-----------------------------------------------------------------------------------------------*/
/*
 * Public function prototypes
 */
console_status_t Console_Init(const console_config_t *config, uint32_t srcClock_Hz);
uint32_t Console_GetStatusFlags(const console_config_t *config);
console_status_t Console_TxCreateHandle(const console_config_t *config, console_handle_tx_t *handle, console_transfer_callback_tx_t callback, void *userData);
console_status_t Console_RxCreateHandle(const console_config_t *config, console_handle_rx_t *handle, console_transfer_callback_rx_t callback, void *userData);
console_status_t Console_WriteChar(const console_config_t *config, char txdata);
console_status_t Console_ReadChar(const console_config_t *config, char *rxdata, uint16_t timeout_ms);
console_status_t Console_WriteBlocking(const console_config_t *config, char *txdata, size_t length);
console_status_t Console_ReadBlocking(const console_config_t *config, char *rxdata, size_t length);
console_status_t Console_TransferWriteNonBlocking(const console_config_t *config, console_handle_tx_t *handle, char *txdata, size_t length);
console_status_t Console_TransferReadNonBlocking(const console_config_t *config, console_handle_rx_t *handle, char *rxdata, size_t length, size_t *recievedBytes, uint16_t timeoutmpy);
console_status_t Console_TransferGetSentCount(volatile console_handle_tx_t *handle, size_t *count);
void Console_AbortSending(const console_config_t *config, volatile console_handle_tx_t *handle);
void Console_GetDefaultConfig(console_config_t *config);

/*-----------------------------------------------------------------------------------------------*/
/*
 * Interrupt handle functions
 */
__interrupt void ConsoleSendingISR(void);
__interrupt void ConsoleReceivingISR(void);
__interrupt void ConsoleRxTimeoutISR(void);

/*-----------------------------------------------------------------------------------------------*/
/*
 * Private (static) function prototypes
 */

/*-----------------------------------------------------------------------------------------------*/
/*
 * Global module objects
 */
volatile console_handle_tx_t Console_Tx_Handle;
volatile console_handle_tx_t *Console_Tx_HandlePtr = &Console_Tx_Handle;	/*< Must be used in main loop as well as in other modules if necessary!! Do not implement own handle variable!!! >*/

volatile console_handle_rx_t Console_Rx_Handle;
volatile console_handle_rx_t *Console_Rx_HandlePtr = &Console_Rx_Handle;	/*< Must be used in main loop as well as in other modules if necessary!! Do not implement own handle variable!!! >*/

volatile console_config_t Console_Config;
volatile console_config_t *Console_ConfigPtr = &Console_Config;

volatile console_flags_t Console_Flags;
volatile console_flags_t *Console_FlagsPtr = &Console_Flags;

volatile uint16_t ReceivingDone;

/**
 * @brief Initializes a UART instance with a user configuration structure and peripheral clock.
 *
 * @param[in] config
 * @param[in] srcClock_Hz
 * @retval status_Console_SciHWNotFound
 */
console_status_t Console_Init(const console_config_t *config, uint32_t srcClock_Hz)
{
	unsigned int intStat; /*< State variable for intterupt restoring >*/

	if (config->sciReg != &SciaRegs && config->sciReg != &ScibRegs && config->sciReg != &ScicRegs)
		return status_Console_SciHWNotFound;

	intStat = __disable_interrupts();	/*< start of critical section >*/
	InitSciGpio();

	EALLOW;  // This is needed to write to EALLOW protected registers
		PieVectTable.TINT2 = &ConsoleRxTimeoutISR;
	EDIS;

	EALLOW;
		PieVectTable.SCITXINTA = &ConsoleSendingISR;
		PieVectTable.SCIRXINTA = &ConsoleReceivingISR;
	EDIS;

	config->sciReg->SCICCR.all = 0x0000;
	switch (config->parityMode) {
		case console_ParityDisabled: {
			config->sciReg->SCICCR.bit.PARITYENA = 0;
			break;
		}
		case console_ParityEven: {
			config->sciReg->SCICCR.bit.PARITYENA = 1;
			config->sciReg->SCICCR.bit.PARITY = 1;
			break;
		}
		case console_ParityOdd: {
			config->sciReg->SCICCR.bit.PARITYENA = 1;
			config->sciReg->SCICCR.bit.PARITY = 0;
			break;
		}
		default:{
			config->sciReg->SCICCR.bit.PARITYENA = 0;
			break;
		}
	}

	switch (config->stopBitCount) {
		case console_OneStopBit: {
			config->sciReg->SCICCR.bit.STOPBITS = 0;
			break;
		}
		case console_TwoStopBit: {
			config->sciReg->SCICCR.bit.STOPBITS = 1;
			break;
		}
		default: {
			config->sciReg->SCICCR.bit.STOPBITS = 0;
			break;
		}
	}

	switch (config->loopBackEnable) {
		case console_loopback_dis: {
			config->sciReg->SCICCR.bit.LOOPBKENA = 0;
			break;
		}
		case console_loopback_enab: {
			config->sciReg->SCICCR.bit.LOOPBKENA = 1;
			break;
		}
		default:{
			config->sciReg->SCICCR.bit.LOOPBKENA = 0;
			break;
		}
	}

	if (config->console_bit_count > 8 || config->console_bit_count == 0)
		return status_Console_BitCountNotSupport;

	config->sciReg->SCICCR.all |= ((config->console_bit_count - 1) & 0x0007);

	config->sciReg->SCICTL1.all = 0x0000;

	// setup FIFO
	config->sciReg->SCIFFTX.all = 0x8020;
	config->sciReg->SCIFFRX.all = 0x0042;
	config->sciReg->SCIFFCT.all = 0x00;
	config->sciReg->SCIFFTX.bit.SCIFFENA = 1;	// 1 = FIFO extended function is enabled
	config->sciReg->SCIFFTX.bit.TXFIFOXRESET = 0;
	config->sciReg->SCIFFTX.bit.TXFIFOXRESET = 1;
	config->sciReg->SCIFFRX.bit.RXFIFORESET = 0;
	config->sciReg->SCIFFRX.bit.RXFIFORESET = 1;

	config->sciReg->SCIHBAUD = (GET_BRR_VALUE(config->baudRate_Bps, srcClock_Hz) >> 8) & 0x00FF;
	config->sciReg->SCILBAUD = (GET_BRR_VALUE(config->baudRate_Bps, srcClock_Hz) & 0x00FF);

	// soft mode - it solves data corruption when debugger accesses DSP
	config->sciReg->SCIPRI.bit.SOFT = 0x1;

	Console_EnableTxInt(config, 0); // Tx interrupt is just disabled
	Console_EnableRxInt(config, 0); // Rx interrupt is just disabled

	// release from reset and allow RX and TX
	config->sciReg->SCICTL1.all =0x0023;


	PieCtrlRegs.PIECTRL.bit.ENPIE = 1;
	PieCtrlRegs.PIEIER9.bit.INTx2 = 1;
	PieCtrlRegs.PIEIER9.bit.INTx1 = 1;
	IER = IER | 0x2100;	// Interrupt enable for SCI Rx, Tx as well as TMR2

	Console_EnableTx(config, 1); // Tx is just enabled
	Console_EnableRx(config, 1); // Rx is just enabled

	__restore_interrupts(intStat);

	return status_Console_OKState;
}

/**
 * @brief
 *
 *
 */
void Console_GetDefaultConfig(console_config_t *config)
{
	config->sciReg = &SciaRegs;
	config->baudRate_Bps = 19200;
	config->console_bit_count = 8;
	config->parityMode = console_ParityDisabled;
	config->stopBitCount = console_OneStopBit;
	config->loopBackEnable = console_loopback_dis;

	return;
}


/**
 * @brief Send one character into console.
 *
 *
 */
console_status_t Console_WriteChar(const console_config_t *config, char txdata)
{
	if (config->sciReg->SCIFFTX.bit.TXFFIENA == 1) {
			return status_Console_TxBusy;
	}

	if (((config->sciReg->SCIFFTX.all & 0x1F00) != 0x0000) && config->sciReg->SCIFFTX.bit.SCIFFENA == 0) {
		config->sciReg->SCIFFTX.bit.TXFIFOXRESET = 0;
		config->sciReg->SCIFFTX.bit.TXFIFOXRESET = 1;
	}

	Console_EnableTxInt(config, 0); // Tx interrupt is just disabled
	while (config->sciReg->SCIFFTX.bit.TXFFST != 0) {
		__asm("  NOP");
	}
	config->sciReg->SCITXBUF = txdata;

	return status_Console_OKState;
}


/**
 * @brief Send data block into console.
 *
 * @param[in]
 * @param[out]
 * @param retval
 */
console_status_t Console_WriteBlocking(const console_config_t *config, char *txdata, size_t length)
{
	size_t i = 0;

	if (config->sciReg->SCIFFTX.bit.TXFFIENA == 1) {
				return status_Console_TxBusy;
		}

	if (((config->sciReg->SCIFFTX.all & 0x1F00) != 0x0000) && config->sciReg->SCIFFTX.bit.SCIFFENA == 0) {
		config->sciReg->SCIFFTX.bit.TXFIFOXRESET = 0;
		config->sciReg->SCIFFTX.bit.TXFIFOXRESET = 1;
	}

	Console_EnableTxInt(config, 0); // Tx interrupt is just disabled

	for (i = 0; i < length; i++) {
		while (config->sciReg->SCIFFTX.bit.TXFFST != 0) {
			__asm("  NOP");
		}
		config->sciReg->SCITXBUF = *(txdata+i);
	}

	return status_Console_OKState;
}



uint32_t Console_GetStatusFlags(const console_config_t *config)
{
	(void)config;
	return 0;
}

console_status_t Console_ReadChar(const console_config_t *config, char *rxdata, uint16_t timeout_ms)
{
	uint32_t tick_us = 0;

	if (config->sciReg->SCIFFRX.bit.RXFFIENA == 1) {
		return status_Console_RxBusy;
	}

	Console_EnableRxInt(config, 0);	// Disable Rx interrupt

	while (config->sciReg->SCIFFRX.bit.RXFFST != 1) {
		tick_us++;
		DELAY_US(1);
		if ((tick_us >> 10) > timeout_ms) {
		  return status_Console_TimeoutOVFL;
		}
	}

	*rxdata = config->sciReg->SCIRXBUF.all;

	return status_Console_OKState;
}

console_status_t Console_ReadBlocking(const console_config_t *config, char *rxdata, size_t length)
{

	return status_Console_OKState;
}

console_status_t Console_TxCreateHandle(const console_config_t *config, console_handle_tx_t *handle, console_transfer_callback_tx_t callback, void *userData)
{
	(void*)config;

	if (handle != &Console_Tx_Handle) {
		return status_Console_HandlePassingError;
	}

	handle->nchar = 0;
	handle->consoleTxCallBackFci = callback;
	handle->userTxData = userData;
	handle->txState = status_Console_TxIdle;

	return status_Console_OKState;
}

console_status_t Console_RxCreateHandle(const console_config_t *config, console_handle_rx_t *handle, console_transfer_callback_rx_t callback, void *userData)
{
	handle->consoleRxCallBackFci = callback;
	handle->userRxData = userData;
	handle->rxTimeUsPerChar = (uint16_t)(1000000/(config->baudRate_Bps/config->console_bit_count));
	handle->rxstate = status_Console_RxIdle;
	handle->rx_transfer_status = status_Console_OKState;

	return status_Console_OKState;
}

console_status_t Console_TransferWriteNonBlocking(const console_config_t *config, console_handle_tx_t *handle, char *txdata, size_t length)
{

	if (handle->txState != status_Console_TxIdle) {
    	return status_Console_TxBusy;
    }

    Console_Tx_Handle.txState = status_Console_TxBusy;

	handle->txData = txdata;
	handle->txDataSize = length;
	handle->nchar = 0;

	config->sciReg->SCIFFTX.bit.TXFIFOXRESET = 0;
	config->sciReg->SCIFFTX.bit.TXFIFOXRESET = 1;

	Console_EnableTxInt(config, 1);	// Enable Tx interrupt
	return status_Console_OKState;
}

console_status_t Console_TransferReadNonBlocking(const console_config_t *config, console_handle_rx_t *handle, char *rxdata, size_t length, size_t *recievedBytes, uint16_t timeoutmpy)
{
	uint32_t timeout_period;

	if (handle->rxstate != status_Console_RxIdle)
		return status_Console_RxBusy;

	timeout_period = (uint32_t)(handle->rxTimeUsPerChar * timeoutmpy);
	ConfigCpuTimer(&CpuTimer2, 150, timeout_period);


	handle->rxData = rxdata;
	handle->rxDataSize = length;
	handle->nchar = 0;
	handle->countRecChars = recievedBytes;

	handle->rxstate = status_Console_RxBusy;

	//CpuTimer2Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
	config->sciReg->SCIFFRX.bit.RXFIFORESET = 0;
	config->sciReg->SCIFFRX.bit.RXFIFORESET = 1;
	Console_EnableRxInt(config, 1);	// Enable Rx interrupt

	return status_Console_OKState;
}

console_status_t Console_TransferGetSentCount(volatile console_handle_tx_t *handle, size_t *count)
{
	return status_Console_OKState;
}

void Console_AbortSending(const console_config_t *config, volatile console_handle_tx_t *handle)
{
	return;
}

/*-----------------------------------------------------------------------------------------------*/

/**
 * @brief Interrupt service rutine for serial console data sending.
 *
 *
 */
#pragma CODE_SECTION(ConsoleSendingISR,"ramfuncs");
__interrupt void ConsoleSendingISR(void)
{
	__disable_interrupts();
	unsigned int i;

	if (Console_Tx_Handle.nchar >= Console_Tx_Handle.txDataSize) {
		SciaRegs.SCIFFTX.bit.TXFFIENA = 0;	// Interrupt from FIFO sending is stopped
		Console_Tx_Handle.txState = status_Console_TxIdle;
		(Console_Tx_Handle.consoleTxCallBackFci)(Console_Tx_HandlePtr, Console_Tx_HandlePtr->userTxData);
	}
	else {
		while (i++ < 16 && Console_Tx_Handle.nchar < Console_Tx_Handle.txDataSize) {
			SciaRegs.SCITXBUF = Console_Tx_Handle.txData[Console_Tx_Handle.nchar];
			Console_Tx_Handle.nchar ++;
		}
	}

	SciaRegs.SCIFFTX.bit.TXFFINTCLR = 1;
	PieCtrlRegs.PIEACK.all |=  PIEACK_GROUP9;
	__enable_interrupts();

	return;
}


/**
 * @brief Interrupt service rutine for serial console data receiving.
 *
 */
#pragma CODE_SECTION(ConsoleSendingISR,"ramfuncs");
__interrupt void ConsoleReceivingISR(void)
{
	unsigned int i;

	CpuTimer2Regs.TCR.bit.TIE = 1;
	CpuTimer2Regs.TCR.bit.TSS = 1;	// Stop timer
	CpuTimer2Regs.TCR.bit.TRB = 1;	// Reload counter value
	CpuTimer2Regs.TCR.bit.TSS = 0; 	// Start timer

	for (i = 0; i < SciaRegs.SCIFFRX.bit.RXFFST; i++) {
		Console_Rx_Handle.rxData[Console_Rx_Handle.nchar] = SciaRegs.SCIRXBUF.bit.RXDT;
		Console_Rx_Handle.nchar ++;

		if (SciaRegs.SCIRXST.bit.RXERROR == 1) {
			if (SciaRegs.SCIRXST.bit.FE == 1) {
				Console_Rx_Handle.rx_transfer_status = status_Console_RxFramingError;
			}

			if (SciaRegs.SCIRXST.bit.PE == 1) {
				Console_Rx_Handle.rx_transfer_status = status_Console_RxParityError;
			}

			if (SciaRegs.SCIRXST.bit.OE == 1) {
				Console_Rx_Handle.rx_transfer_status = status_Console_RxHardwareOverrun;
			}

			Console_Rx_Handle.rxstate = status_Console_RxIdle;
			CpuTimer2Regs.TCR.bit.TSS = 1;		// Stop timer
			SciaRegs.SCIFFRX.bit.RXFFIENA = 0;	// Stop receiving

		}

		if (Console_Rx_Handle.nchar >= Console_Rx_Handle.rxDataSize-1) {
			Console_Rx_Handle.rxData[Console_Rx_Handle.nchar] = '/0';
			Console_Rx_Handle.rxstate = status_Console_RxIdle;
			CpuTimer2Regs.TCR.bit.TSS = 1;		// Stop timer
			SciaRegs.SCIFFRX.bit.RXFFIENA = 0;	// Stop receiving
			break;
		}
	}

	*Console_Rx_Handle.countRecChars = Console_Rx_Handle.nchar;
	SciaRegs.SCIFFRX.bit.RXFFINTCLR = 1;
	PieCtrlRegs.PIEACK.all |=  PIEACK_GROUP9;
	return;
}


#pragma CODE_SECTION(ConsoleRxTimeoutISR,"ramfuncs");
__interrupt void ConsoleRxTimeoutISR(void)
{
	SciaRegs.SCIFFRX.bit.RXFFIENA = 0;
	ReceivingDone = 111;

	Console_Rx_Handle.rxData[Console_Rx_Handle.nchar] = '/0';

	(Console_Rx_Handle.consoleRxCallBackFci)(Console_Rx_HandlePtr, Console_Rx_HandlePtr->userRxData);

	CpuTimer2Regs.TCR.bit.TIF = 1;
	CpuTimer2Regs.TCR.bit.TIE = 0;
	CpuTimer2Regs.TCR.bit.TSS = 1;

	return;
}








