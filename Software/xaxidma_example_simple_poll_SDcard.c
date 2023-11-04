/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xaxidma_example_simple_poll.c
 *
 * This file demonstrates how to use the xaxidma driver on the Xilinx AXI
 * DMA core (AXIDMA) to transfer packets in polling mode when the AXI DMA core
 * is configured in simple mode.
 *
 * This code assumes a loopback hardware widget is connected to the AXI DMA
 * core for data packet loopback.
 *
 * To see the debug print, you need a Uart16550 or uartlite in your system,
 * and please set "-DDEBUG" in your compiler options. You need to rebuild your
 * software executable.
 *
 * Make sure that MEMORY_BASE is defined properly as per the HW system. The
 * h/w system built in Area mode has a maximum DDR memory limit of 64MB. In
 * throughput mode, it is 512MB.  These limits are need to ensured for
 * proper operation of this code.
 *
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 4.00a rkv  02/22/11 New example created for simple DMA, this example is for
 *       	       simple DMA
 * 5.00a srt  03/06/12 Added Flushing and Invalidation of Caches to fix CRs
 *		       648103, 648701.
 *		       Added V7 DDR Base Address to fix CR 649405.
 * 6.00a srt  03/27/12 Changed API calls to support MCDMA driver.
 * 7.00a srt  06/18/12 API calls are reverted back for backward compatibility.
 * 7.01a srt  11/02/12 Buffer sizes (Tx and Rx) are modified to meet maximum
 *		       DDR memory limit of the h/w system built with Area mode
 * 7.02a srt  03/01/13 Updated DDR base address for IPI designs (CR 703656).
 * 9.1   adk  01/07/16 Updated DDR base address for Ultrascale (CR 799532) and
 *		       removed the defines for S6/V6.
 * 9.3   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings are
 *                     available in all examples. This is a fix for CR-965028.
 *       ms   04/05/17 Modified Comment lines in functions to
 *                     recognize it as documentation block for doxygen
 *                     generation of examples.
 * 9.9   rsp  01/21/19 Fix use of #elif check in deriving DDR_BASE_ADDR.
 * 9.10  rsp  09/17/19 Fix cache maintenance ops for source and dest buffer.
 * </pre>
 *
 * ***************************************************************************

 */
/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xparameters.h"
#include "ff.h"
#include "xdebug.h"

#if defined(XPAR_UARTNS550_0_BASEADDR)
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

/******************** Constant Definitions **********************************/

/*
 * Device hardware build related constants.
 */

#define DMA_DEV_ID		XPAR_AXIDMA_0_DEVICE_ID

#ifdef XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif defined (XPAR_MIG7SERIES_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG7SERIES_0_BASEADDR
#elif defined (XPAR_MIG_0_BASEADDR)
#define DDR_BASE_ADDR	XPAR_MIG_0_BASEADDR
#elif defined (XPAR_PSU_DDR_0_S_AXI_BASEADDR)
#define DDR_BASE_ADDR	XPAR_PSU_DDR_0_S_AXI_BASEADDR
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
		 DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR		0x01000000
#else
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#define TX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00100000)
#define RX_BUFFER_BASE		(MEM_BASE_ADDR + 0x00300000)
#define RX_BUFFER_HIGH		(MEM_BASE_ADDR + 0x004FFFFF)

#define MAX_PKT_LEN		0x20

#define TEST_START_VALUE	0xC

#define NUMBER_OF_TRANSFERS	1

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

#if (!defined(DEBUG))
extern void xil_printf(const char *format, ...);
#endif

int XAxiDma_SimplePollExample(u16 DeviceId);
static int CheckData(void);

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XAxiDma AxiDma;


/*****************************************************************************/
/**
* The entry point for this example. It invokes the example function,
* and reports the execution status.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if example fails.
*
* @note		None.
*
******************************************************************************/
FATFS fs;
TCHAR *path = "0:/";
int DataZise;
int main()
{
	FRESULT rc;
	int Status;

	xil_printf("\r\n--- Entering main() --- \r\n");

	rc = f_mount (&fs, path, 1);			/* Mount/Unmount a logical drive */
	if (rc != FR_OK){
		xil_printf(" ERROR : f_mount returned %d\r\n", rc);
	}
	/* Run the poll example for simple transfer */
	Status = XAxiDma_SimplePollExample(DMA_DEV_ID);

	if (Status != XST_SUCCESS) {
		xil_printf("XAxiDma_SimplePoll Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran XAxiDma_SimplePoll Example\r\n");


	rc = f_mount (&fs, path, 0);			/* Mount/Unmount a logical drive */
	if (rc != FR_OK){
		xil_printf(" ERROR : f_mount returned %d\r\n", rc);
	}

	xil_printf("--- Exiting main() --- \r\n");

	return XST_SUCCESS;

}

#if defined(XPAR_UARTNS550_0_BASEADDR)
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600, and data bits to 8
*
* @param	None.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void Uart550_Setup(void)
{

	/* Set the baudrate to be predictable
	 */
	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
			XUN_LCR_8_DATA_BITS);

}
#endif

/*****************************************************************************/
/**
* The example to do the simple transfer through polling. The constant
* NUMBER_OF_TRANSFERS defines how many times a simple transfer is repeated.
*
* @param	DeviceId is the Device Id of the XAxiDma instance
*
* @return
*		- XST_SUCCESS if example finishes successfully
*		- XST_FAILURE if error occurs
*
* @note		None
*
*
******************************************************************************/
int XAxiDma_SimplePollExample(u16 DeviceId)
{
	XAxiDma_Config *CfgPtr;
	int Status;
	int Tries = NUMBER_OF_TRANSFERS;
	int Index;
	u8 *TxBufferPtr;
	u8 *RxBufferPtr;
	u8 Value;
	FRESULT rc;
	FIL Rfp;
	FIL RTfp;
	FIL Wfp;
	int ReadZise;
	UINT br;
	UINT bw;
	u8 *ReadLogFile;



	TxBufferPtr = (u8 *)TX_BUFFER_BASE ;
	RxBufferPtr = (u8 *)RX_BUFFER_BASE;

	/* Initialize the XAxiDma device.
	 */
	CfgPtr = XAxiDma_LookupConfig(DeviceId);
	if (!CfgPtr) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	Status = XAxiDma_CfgInitialize(&AxiDma, CfgPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed %d\r\n", Status);
		return XST_FAILURE;
	}

	if(XAxiDma_HasSg(&AxiDma)){
		xil_printf("Device configured as SG mode \r\n");
		return XST_FAILURE;
	}

	/* Disable interrupts, we use polling mode
	 */
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DEVICE_TO_DMA);
	XAxiDma_IntrDisable(&AxiDma, XAXIDMA_IRQ_ALL_MASK,
						XAXIDMA_DMA_TO_DEVICE);

	Value = TEST_START_VALUE;

	for(Index = 0; Index < MAX_PKT_LEN; Index ++) {
			TxBufferPtr[Index] = Value;

			Value = (Value + 1) & 0xFF;
	}

	rc = f_open (&Rfp, "0:/idma.bin", FA_READ);				/* Open or create a file */
	if (rc != FR_OK){
		xil_printf(" ERROR : f_open returned %d\r\n", rc);
	}
	ReadZise = Rfp.obj.objsize;
	DataZise = ReadZise;
	rc = f_lseek (&Rfp, 0);								/* Move file pointer of the file object */
	if (rc != FR_OK){
		xil_printf(" ERROR : f_lseek returned %d\r\n", rc);
	}
	rc = f_read (&Rfp, TxBufferPtr, ReadZise, &br);			/* Read data from the file */
	if (rc != FR_OK){
		xil_printf(" ERROR : f_read returned %d\r\n", rc);
	}
	rc = f_close (&Rfp);											/* Close an open file object */
	if (rc != FR_OK){
		xil_printf(" ERROR : f_close returned %d\r\n", rc);
	}
	/* Flush the buffers before the DMA transfer, in case the Data Cache
	 * is enabled
	 */
	Xil_DCacheFlushRange((UINTPTR)TxBufferPtr, DataZise);
	Xil_DCacheFlushRange((UINTPTR)RxBufferPtr, DataZise);

	for(Index = 0; Index < Tries; Index ++) {


		Status = XAxiDma_SimpleTransfer(&AxiDma,(UINTPTR) RxBufferPtr,
					DataZise, XAXIDMA_DEVICE_TO_DMA);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XAxiDma_SimpleTransfer(&AxiDma,(UINTPTR) TxBufferPtr,
					DataZise, XAXIDMA_DMA_TO_DEVICE);

		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		while ((XAxiDma_Busy(&AxiDma,XAXIDMA_DEVICE_TO_DMA)) ||
			(XAxiDma_Busy(&AxiDma,XAXIDMA_DMA_TO_DEVICE))) {
				/* Wait */
		}

		Xil_DCacheInvalidateRange((UINTPTR)TxBufferPtr, DataZise);
		Xil_DCacheInvalidateRange((UINTPTR)RxBufferPtr, DataZise);

		rc = f_open (&Wfp, "0:/odma.bin", FA_OPEN_ALWAYS | FA_WRITE);				/* Open or create a file */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_open returned %d\r\n", rc);
		}
		rc = f_write (&Wfp, RxBufferPtr, DataZise, &bw);	/* Write data to the file */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_write returned %d\r\n", rc);
		}
		rc = f_close (&Wfp);											/* Close an open file object */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_close returned %d\r\n", rc);
		}

		Status = CheckData();
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		rc = f_open (&RTfp, "0:/DMA_LOG.TXT", FA_READ);				/* Open or create a file */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_open returned %d\r\n", rc);
		}
		ReadZise = RTfp.obj.objsize;
		rc = f_lseek (&RTfp, 0);								/* Move file pointer of the file object */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_lseek returned %d\r\n", rc);
		}
		rc = f_read (&RTfp, ReadLogFile, ReadZise, &br);			/* Read data from the file */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_read returned %d\r\n", rc);
		}
		rc = f_close (&RTfp);											/* Close an open file object */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_close returned %d\r\n", rc);
		}

		xil_printf("\n\r Log File \n\r");
		xil_printf(ReadLogFile);

	}

	/* Test finishes successfully
	 */
	return XST_SUCCESS;
}



/*****************************************************************************/
/*
*
* This function checks data buffer after the DMA transfer is finished.
*
* @param	None
*
* @return
*		- XST_SUCCESS if validation is successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int CheckData(void)
{
	u8 *TxPacket;
	u8 *RxPacket;
	int Index = 0;
	u8 Value;
	char *PrintString;
	int PrintStringZise;
	FRESULT rc;
	FIL WTfp;
	int OldFileZise;
	UINT bw;

	TxPacket = (u8 *) TX_BUFFER_BASE;
	RxPacket = (u8 *) RX_BUFFER_BASE;
	Value = TEST_START_VALUE;

	/* Invalidate the DestBuffer before receiving the data, in case the
	 * Data Cache is enabled
	 */
	Xil_DCacheInvalidateRange((UINTPTR)TxPacket, DataZise);
	Xil_DCacheInvalidateRange((UINTPTR)RxPacket, DataZise);

	for(Index = 0; Index < DataZise; Index++) {
		if (RxPacket[Index] != TxPacket[Index]) {
			xil_printf("Data error %d: %x != %x\r\n",
			Index, (unsigned int)RxPacket[Index],
				(unsigned int)TxPacket[Index]);
			PrintStringZise = sprintf(PrintString,"Data error %d: %x != %x\r\n",
			Index, (unsigned int)RxPacket[Index],
				(unsigned int)TxPacket[Index]);

//			return XST_FAILURE;
		} else {
			xil_printf("Data OK %d: %x = %x\r\n",
			Index, (unsigned int)RxPacket[Index],
				(unsigned int)TxPacket[Index]);
			PrintStringZise = sprintf(PrintString,"Data OK %d: %x = %x\r\n",
			Index, (unsigned int)RxPacket[Index],
				(unsigned int)TxPacket[Index]);
		}
		rc = f_open (&WTfp, "0:/DMA_LOG.TXT", FA_OPEN_ALWAYS | FA_WRITE);				/* Open or create a file */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_open returned %d\r\n", rc);
		}
		OldFileZise = WTfp.obj.objsize;
		rc = f_lseek (&WTfp, OldFileZise);								/* Move file pointer of the file object */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_lseek returned %d\r\n", rc);
		}
		rc = f_write (&WTfp, PrintString, PrintStringZise, &bw);			/* Read data from the file */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_read returned %d\r\n", rc);
		}
		rc = f_close (&WTfp);											/* Close an open file object */
		if (rc != FR_OK){
			xil_printf(" ERROR : f_close returned %d\r\n", rc);
		}

		Value = (Value + 1) & 0xFF;
	}

	return XST_SUCCESS;
}
