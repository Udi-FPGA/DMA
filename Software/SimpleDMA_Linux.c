#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>

#define DDR_BASE_ADDR 0x10000000  // Replace with the actual DDR base address
#define DDR_SIZE      0x10000      // Size of the DDR region (in bytes)

#define DMA_BASE_ADDR 0x40400000  // Replace with the actual DDR base address
#define DMA_SIZE      0x100      // Size of the DDR region (in bytes)

#define TX_BUFF_BASE		0x00001000
#define RX_BUFF_BASE		0x00002000

#define PKT_LENGTH 0x20
#define START_DATA_COUNT 0xc

int main() {
    /////////////////////////////// DATA memory space ///////////////////////////////

	int mem_fb;
	void *ddr_mem;

	printf("START\n\r");
    // Open /dev/mem to access physical memory
	mem_fb = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fb < 0) {
        perror("Error opening /dev/mem");
        return -1;
    }

    // Map DDR memory into the process's address space
    ddr_mem = mmap(NULL, DDR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fb, DDR_BASE_ADDR);
    if (ddr_mem == MAP_FAILED) {
        perror("Error mapping DDR memory");
        close(mem_fb);
        return -1;
    }

    void *TX_Buff = ddr_mem+TX_BUFF_BASE;
    void *RX_Buff = ddr_mem+RX_BUFF_BASE;
    uint32_t *TxBuff = (uint32_t *)TX_Buff;
    uint32_t *RxBuff = (uint32_t *)RX_Buff;

	printf("\n\rTX buff EMPTY \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++){
    	printf("TX buff %d %x\n\r",i,TxBuff[i]);

    }
	printf("\n\rRX buff EMPTY \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++){
    	printf("RX buff %d %x\n\r",i,RxBuff[i]);

    }
    void *TX_data = ddr_mem+TX_BUFF_BASE;
    uint8_t *Packet = (uint8_t *)TX_data;
	int Value = START_DATA_COUNT;
	for (int i=0; i<PKT_LENGTH; i++)
	{
		Packet[i] = Value;
		Value++;
	}

	printf("\n\rTX buff READY \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++){
    	printf("TX buff %d %x\n\r",i,TxBuff[i]);

    }


    /////////////////////////////// DMA memory space ///////////////////////////////
	int mem_dma;
	void *dma_mem;

	printf("START\n\r");
    // Open /dev/mem to access physical memory
	mem_dma = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_dma < 0) {
        perror("Error opening /dev/mem");
        return -1;
    }

    // Map DMA memory into the process's address space
    dma_mem = mmap(NULL, DMA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_dma, DMA_BASE_ADDR);
    if (ddr_mem == MAP_FAILED) {
        perror("Error mapping DDR memory");
        close(mem_fb);
        return -1;
    }

    uint32_t TXdata, RXdata;
    uint32_t TxBusy, RxBusy;
    uint32_t *DMAregs = (uint32_t *)dma_mem;
    printf ("\n\r START DMA  \n\r");
    DMAregs[0x00/4] = 0x4;
    TXdata = DMAregs[0x00/4];
    DMAregs[0x30/4] = 0x4;
    RXdata = DMAregs[0x30/4];
    DMAregs[0x18/4] = DDR_BASE_ADDR + TX_BUFF_BASE;
    DMAregs[0x48/4] = DDR_BASE_ADDR + RX_BUFF_BASE;
    TXdata = DMAregs[0x00/4];
    DMAregs[0x00/4] = TXdata | 0x00000001;
    RXdata = DMAregs[0x30/4];
    DMAregs[0x30/4] = RXdata | 0x00000001;
    DMAregs[0x58/4] = PKT_LENGTH;
    DMAregs[0x28/4] = PKT_LENGTH;

    TxBusy = 0;
    RxBusy = 0;
    while (((TxBusy & 0x00000002) == 0) | ((RxBusy & 0x00000002) == 0)){
    	TxBusy = DMAregs[0x04/4];
    	RxBusy = DMAregs[0x34/4];
    }

    close(mem_dma);

	printf("\n\rTX buff END \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++){
    	printf("TX buff %d %x\n\r",i,TxBuff[i]);

    }
	printf("\n\rRX buff END \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++){
    	printf("RX buff %d %x\n\r",i,RxBuff[i]);

    }


    close(mem_fb);

    return 0;
}
