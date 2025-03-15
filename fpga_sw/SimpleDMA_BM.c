/***************************** Include Files *********************************/
#include "xaxidma.h"
#include "xparameters.h"
#include "xdebug.h"

#ifdef XPAR_PS7_RAM_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR		XPAR_PS7_RAM_0_S_AXI_BASEADDR
#define MEM_BASE_ADDR		(DDR_BASE_ADDR + 0x10000000)
#endif

#define TX_BUFF_BASE		(MEM_BASE_ADDR + 0x00001000)
#define RX_BUFF_BASE		(MEM_BASE_ADDR + 0x00002000)

#define PKT_LENGTH 0x20
#define START_DATA_COUNT 0xc

u32 *DMA = XPAR_AXIDMA_0_BASEADDR;

int main (void)
{
	int Value;

	u8 *Packet = (u8 *)TX_BUFF_BASE;
	Value = START_DATA_COUNT;
	for (int i=0; i<PKT_LENGTH; i++)
	{
		Packet[i] = Value;
		Value++;
	}
	Xil_DCacheFlushRange((UINTPTR)Packet, PKT_LENGTH);

	u32 TXdata, RXdata;
	u32 TXbusy, RXbusy;

    DMA[0x00/4] = 0x4;
    TXdata = DMA[0x00/4];
    DMA[0x30/4] = 0x4;
    RXdata = DMA[0x30/4];
    DMA[0x18/4] = TX_BUFF_BASE;
    DMA[0x48/4] = RX_BUFF_BASE;
    TXdata = DMA[0x00/4];
    DMA[0x00/4] = TXdata | 0x00000001;
    RXdata = DMA[0x30/4];
    DMA[0x30/4] = RXdata | 0x00000001;
    DMA[0x58/4] = PKT_LENGTH;
    DMA[0x28/4] = PKT_LENGTH;

    TXbusy = 0;
    RXbusy = 0;
    while (((TXbusy & 0x00000002) == 0) | ((RXbusy & 0x00000002) == 0)){
    	TXbusy = DMA[0x04/4];
    	RXbusy = DMA[0x34/4];
    }

	Xil_DCacheInvalidateRange((UINTPTR)TX_BUFF_BASE	, PKT_LENGTH);
	Xil_DCacheInvalidateRange((UINTPTR)RX_BUFF_BASE	, PKT_LENGTH);

	return XST_SUCCESS;
}
