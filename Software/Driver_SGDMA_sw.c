/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"

#ifdef XPAR_PS7_RAM_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_PS7_RAM_0_S_AXI_BASEADDR
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x1000000)
#endif

#define TX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00000000)
#define RX_BD_SPACE_BASE	(MEM_BASE_ADDR + 0x00001000)
#define TX_BUFF_BASE		(MEM_BASE_ADDR + 0x00002000)
#define RX_BUFF_BASE		(MEM_BASE_ADDR + 0x00003000)

#define PKT_LENGTH 0x20
#define BD_NUM     6
#define START_DATA_COUNT 0xc

int main (void)
{
	int Value;

	u8 *Packet = (u8 *)TX_BUFF_BASE;
	Value = START_DATA_COUNT;
	for (int i=0; i<PKT_LENGTH*BD_NUM; i++)
	{
		Packet[i] = Value;
		Value++;
	}
	Xil_DCacheFlushRange((UINTPTR)Packet, PKT_LENGTH*BD_NUM);

	int status;
	XAxiDma_Config *Config;
	XAxiDma InstancePtr;

	Config = XAxiDma_LookupConfig(XPAR_AXI_DMA_0_DEVICE_ID);
	status = XAxiDma_CfgInitialize(&InstancePtr, Config);

	XAxiDma_BdRing *TxBDringptr;
	XAxiDma_BdRing *RxBDringptr;

	TxBDringptr = XAxiDma_GetTxRing(&InstancePtr);
	RxBDringptr = XAxiDma_GetRxRing(&InstancePtr);

	status = XAxiDma_BdRingCreate(TxBDringptr, TX_BD_SPACE_BASE,
			TX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT, BD_NUM);
	status = XAxiDma_BdRingCreate(RxBDringptr, RX_BD_SPACE_BASE,
			RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT, BD_NUM);

	XAxiDma_Bd *TxBDptr;
	XAxiDma_Bd *RxBDptr;
	UINTPTR TxBlockAdd = TX_BUFF_BASE;
	UINTPTR RxBlockAdd = RX_BUFF_BASE;

	status = XAxiDma_BdRingAlloc(TxBDringptr, BD_NUM, &TxBDptr);
	status = XAxiDma_BdRingAlloc(RxBDringptr, BD_NUM, &RxBDptr);

	for (int i=0;i<BD_NUM;i++){
		status = XAxiDma_BdSetBufAddr(TxBDptr, TxBlockAdd);
		status = XAxiDma_BdSetLength(TxBDptr, PKT_LENGTH, TxBDringptr->MaxTransferLen);
		XAxiDma_BdSetCtrl(TxBDptr, XAXIDMA_BD_CTRL_ALL_MASK);

		status = XAxiDma_BdSetBufAddr(RxBDptr, RxBlockAdd);
		status = XAxiDma_BdSetLength(RxBDptr, PKT_LENGTH, RxBDringptr->MaxTransferLen);

		TxBlockAdd += PKT_LENGTH;
		RxBlockAdd += PKT_LENGTH;

		TxBDptr = (XAxiDma_Bd *)XAxiDma_BdRingNext(TxBDringptr, TxBDptr);
		RxBDptr = (XAxiDma_Bd *)XAxiDma_BdRingNext(RxBDringptr, RxBDptr);
	}
	Xil_DCacheFlushRange((UINTPTR)TX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT*BD_NUM);
	Xil_DCacheFlushRange((UINTPTR)RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT*BD_NUM);

	status = XAxiDma_BdRingToHw(RxBDringptr, BD_NUM, RxBDptr);
	status = XAxiDma_BdRingToHw(TxBDringptr, BD_NUM, TxBDptr);

	status = XAxiDma_BdRingStart(RxBDringptr);
	status = XAxiDma_BdRingStart(TxBDringptr);

	while ((XAxiDma_Busy(&InstancePtr, XAXIDMA_DMA_TO_DEVICE))||
			(XAxiDma_Busy(&InstancePtr, XAXIDMA_DEVICE_TO_DMA)));

	Xil_DCacheInvalidateRange(TX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT*BD_NUM);
	Xil_DCacheInvalidateRange(RX_BD_SPACE_BASE, XAXIDMA_BD_MINIMUM_ALIGNMENT*BD_NUM);
	Xil_DCacheInvalidateRange(TX_BUFF_BASE, PKT_LENGTH*BD_NUM);
	Xil_DCacheInvalidateRange(RX_BUFF_BASE, PKT_LENGTH*BD_NUM);




	return XST_SUCCESS;
}
