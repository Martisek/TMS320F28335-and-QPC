/*
 * Console_BSP.h
 *
 *  Created on: 12. 9. 2017
 *      Author: martisek
 */

#ifndef CONSOLE_CONSOLE_BSP_H_
#define CONSOLE_CONSOLE_BSP_H_

#include "DSP28x_Project.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*< Help macro >*/
#define GET_BRR_VALUE(baud,mcuclk) 	((uint16_t)(((mcuclk>>2)/((baud)*8))-1))
#define MAKE_STATUS(group, code) 	((((group)*100) + (code)))

#define StatusGroup_Console		1

/*< Function-like macros >*/
#define	Console_EnableTx(config, enable) 	((config->sciReg->SCICTL1.bit.TXENA) = enable)
#define Console_EnableRx(config, enable) 	((config->sciReg->SCICTL1.bit.RXENA) = enable)
#define Console_EnableTxInt(config, enable)	((config->sciReg->SCIFFTX.bit.TXFFIENA) = enable)
#define Console_EnableRxInt(config, enable) ((config->sciReg->SCIFFRX.bit.RXFFIENA) = enable)


/*< In-module typedefs >*/
typedef struct console_handle_tx console_handle_tx_t;
typedef struct console_handle_rx console_handle_rx_t;
typedef struct console_config console_config_t;
typedef struct console_flags console_flags_t;
typedef struct console_rx_flags console_rx_flags_t;
typedef enum console_status console_status_t;
typedef enum console_parity_mode console_parity_mode_t;
typedef enum console_stop_bit_count console_stop_bit_count_t;
typedef enum console_loopback_enable console_loopback_enable_t;

typedef void (*console_transfer_callback_tx_t)(console_handle_tx_t *handle, void *UserData);	/*< Callback function for Tx ready interrupt >*/
typedef void (*console_transfer_callback_rx_t)(console_handle_rx_t *handle, void *UserData);	/*< Callback function for Tx ready interrupt >*/


/*! Console parity mode enumeration */
enum console_parity_mode
{
    console_ParityDisabled 	= 0, /*!< Parity disabled */
    console_ParityEven 		= 1,     /*!< Parity enabled, type even, bit setting: PE|PT = 10 */
    console_ParityOdd 		= 2,      /*!< Parity enabled, type odd,  bit setting: PE|PT = 11 */
};

/*! Console stop bit count enumeration */
enum console_stop_bit_count
{
	console_OneStopBit = 0, /*!< One stop bit */
	console_TwoStopBit = 1, /*!< Two stop bits */
};

/*! Console loop back enable enumeration */
enum console_loopback_enable
{
	console_loopback_dis  	= 0,
	console_loopback_enab	= 1,
};

enum console_status
{
	status_Console_OKState				= MAKE_STATUS(StatusGroup_Console,  0), /*!< Console is OK */
	status_Console_TxBusy 				= MAKE_STATUS(StatusGroup_Console,  1),	/*!< Transmitter is busy. */
	status_Console_RxBusy 				= MAKE_STATUS(StatusGroup_Console,  2),	/*!< Receiver is busy. */
	status_Console_TxIdle 				= MAKE_STATUS(StatusGroup_Console,  3), /*!< UART transmitter is idle. */
	status_Console_RxIdle 				= MAKE_STATUS(StatusGroup_Console,  4), /*!< UART receiver is idle. */
	status_Console_RxError 				= MAKE_STATUS(StatusGroup_Console,  5), /*!< Error happens on UART. */
	status_Console_RxRingBufferOverrun 	= MAKE_STATUS(StatusGroup_Console,  6),	/*!< UART RX software ring buffer overrun. */
	status_Console_RxHardwareOverrun 	= MAKE_STATUS(StatusGroup_Console,  7), /*!< UART RX receiver overrun. */
	status_Console_RxFramingError 		= MAKE_STATUS(StatusGroup_Console,  8), /*!< UART framing error. */
	status_Console_RxParityError 		= MAKE_STATUS(StatusGroup_Console,  9), /*!< UART parity error. */
	status_Console_BaudrateNotSupport 	= MAKE_STATUS(StatusGroup_Console, 10),	/*!< Baudrate is not support in current clock source */
	status_Console_BitCountNotSupport 	= MAKE_STATUS(StatusGroup_Console, 11),	/*!< Number of bits in frame is not supported by this HW */
	status_Console_SciHWNotFound 		= MAKE_STATUS(StatusGroup_Console, 12),
	status_Console_TimeoutOVFL			= MAKE_STATUS(StatusGroup_Console, 13),
	status_Console_HandlePassingError	= MAKE_STATUS(StatusGroup_Console, 14),
};

struct console_rx_flags
{
	unsigned console_Rx_sumErr					: 	1;
	unsigned console_Rx_overrunErr				:	1;
	unsigned console_Rx_framingErr				:	1;
	unsigned console_Rx_parityErr				:	1;
	unsigned console_Rx_dataRegFull				:	1;
};


/*! Tx handle struct */
struct console_handle_tx
{
	volatile struct SCI_REGS *sciReg;

	volatile char *txData;
	volatile size_t txDataSize;
	volatile size_t nchar;

	volatile console_status_t tx_transfer_status;

	console_transfer_callback_tx_t consoleTxCallBackFci;
	void *userTxData;

	volatile uint16_t txState;
};

/*! Rx handle struct */
struct console_handle_rx
{
	volatile struct SCI_REGS *sciReg;
	volatile char *rxData;
	volatile size_t rxDataSize;
	//volatile QEQueue rxQueue;
	volatile uint16_t rxTimeUsPerChar;
	volatile size_t nchar;
	volatile size_t *countRecChars;

	volatile console_status_t rx_transfer_status;

	console_transfer_callback_tx_t consoleRxCallBackFci;
	void *userRxData;

	console_rx_flags_t rxStateFlags;

	volatile uint16_t rxstate;
};

struct console_config
{
	volatile struct SCI_REGS *sciReg;
    uint32_t baudRate_Bps;         /*!< UART baud rate  */
    console_parity_mode_t parityMode; /*!< Parity mode, disabled (default), even, odd */
    console_stop_bit_count_t stopBitCount; /*!< Number of stop bits, 1 stop bit (default) or 2 stop bits  */
    console_loopback_enable_t loopBackEnable;
    uint16_t console_bit_count;
    bool enableFifo;
    bool enableTx; /*!< Enable TX */
    bool enableRx; /*!< Enable RX */
};




struct console_flags
{
	unsigned console_HWTxDataRegEmptyFlag		:	1; 	/*!< TX data register empty flag. */
    unsigned console_HWTransmissionCompleteFlag :	1; 	/*!< Transmission complete flag. */
    unsigned console_HWRxDataRegFullFlag 		: 	1; 	/*!< RX data register full flag. */
    unsigned console_HWRxOverrunFlag 			: 	1;  /*!< RX overrun flag. */
    unsigned console_HWFramingErrorFlag 		: 	1;	/*!< Frame error flag, sets if logic 0 was detected */
    unsigned console_HWParityErrorFlag 			:	1;  /*!< If parity enabled, sets upon parity error detection */
    unsigned console_HWRxFifoOverflowFlag		:	1;
    unsigned console_HWTxFifoDataMatchFlag		:	1;
    unsigned console_TxInProcessFlag			:   1;
    unsigned console_RxInProcessFlag			: 	1;
};





/*---------------------------------------------------------------------------------------------------------*/

extern volatile console_handle_tx_t *Console_Tx_HandlePtr;
extern volatile console_handle_rx_t *Console_Rx_HandlePtr;
extern volatile console_config_t *Console_ConfigPtr;


/*---------------------------------------------------------------------------------------------------------*/

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




#endif /* CONSOLE_CONSOLE_BSP_H_ */
