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

#define BD_SIZE 0x40
#define PKT_LENGTH 0x20
#define BD_NUM     6
#define START_DATA_COUNT 0xc

u32 *DMA = XPAR_AXIDMA_0_BASEADDR;

int main (void)
{

	u32 Data0, Data1;
	u32 *TxBDprt    = (u32 *)TX_BD_SPACE_BASE;
	u32 *RxBDprt    = (u32 *)RX_BD_SPACE_BASE;
	u32 *TxBbuffprt = (u32 *)TX_BUFF_BASE;
	u32 *RxBbuffprt = (u32 *)RX_BUFF_BASE;
	u32 TxBDadd = TX_BD_SPACE_BASE;
	u32 RxBDadd = RX_BD_SPACE_BASE;
	u32 TxBuffadd = TX_BUFF_BASE;
	u32 RxBuffadd = RX_BUFF_BASE;

	int Value;

	u8 *Packet = (u8 *)TX_BUFF_BASE;
	Value = START_DATA_COUNT;
	for (int i=0; i<PKT_LENGTH*BD_NUM; i++)
	{
		Packet[i] = Value;
		Value++;
	}
	Xil_DCacheFlushRange((UINTPTR)Packet, PKT_LENGTH*BD_NUM);

	for (int i=0; i<BD_NUM; i++){
		TxBDadd  += BD_SIZE;
		RxBDadd  += BD_SIZE;
		TxBDprt[((i*BD_SIZE) + 0x00)/4] = TxBDadd;
		RxBDprt[((i*BD_SIZE) + 0x00)/4] = RxBDadd;
		TxBDprt[((i*BD_SIZE) + 0x04)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x04)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x08)/4] = TxBuffadd;
		RxBDprt[((i*BD_SIZE) + 0x08)/4] = RxBuffadd;
		TxBDprt[((i*BD_SIZE) + 0x0C)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x0C)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x10)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x10)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x14)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x14)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x18)/4] = 0x00000000 +  PKT_LENGTH;
		RxBDprt[((i*BD_SIZE) + 0x18)/4] = 0x00000000 +  PKT_LENGTH;
		TxBDprt[((i*BD_SIZE) + 0x1C)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x1C)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x20)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x20)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x24)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x24)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x28)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x28)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x2C)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x2C)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x30)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x30)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x34)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x34)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x38)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x38)/4] = 0x00000000;
		TxBDprt[((i*BD_SIZE) + 0x3C)/4] = 0x00000000;
		RxBDprt[((i*BD_SIZE) + 0x3C)/4] = 0x00000000;
	    TxBuffadd += PKT_LENGTH;
	    RxBuffadd += PKT_LENGTH;
	}
	Xil_DCacheFlushRange((UINTPTR)TxBDprt, BD_SIZE*BD_NUM);
	Xil_DCacheFlushRange((UINTPTR)RxBDprt, BD_SIZE*BD_NUM);

    DMA[0x00/4] = 0x4;
    DMA[0x30/4] = 0x4;
    DMA[0x08/4] = TX_BD_SPACE_BASE;
    DMA[0x38/4] = RX_BD_SPACE_BASE;
    Data0 = DMA[0x00/4];
    DMA[0x00/4] = Data0 | 0x00000001;
    Data1 = DMA[0x30/4];
    DMA[0x30/4] = Data1 | 0x00000001;
    DMA[0x40/4] = RX_BD_SPACE_BASE + BD_SIZE*BD_NUM - BD_SIZE;
    DMA[0x10/4] = TX_BD_SPACE_BASE + BD_SIZE*BD_NUM - BD_SIZE;

    Data0 = 0;
    Data1 = 0;
    while (((Data0 & 0x00000002) == 0) | ((Data1 & 0x00000002) == 0)){
	    Data0 = DMA[0x04/4];
	    Data1 = DMA[0x34/4];
    }

	Xil_DCacheInvalidateRange((UINTPTR)TxBDprt, BD_SIZE*BD_NUM);
	Xil_DCacheInvalidateRange((UINTPTR)RxBDprt, BD_SIZE*BD_NUM);
	Xil_DCacheInvalidateRange((UINTPTR)TxBbuffprt, PKT_LENGTH*BD_NUM);
	Xil_DCacheInvalidateRange((UINTPTR)RxBbuffprt, PKT_LENGTH*BD_NUM);

	return XST_SUCCESS;
}
