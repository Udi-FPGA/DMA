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

#define BD_SIZE   0x40

#define TX_BD_BASE   0x00000000
#define RX_BD_BASE   0x00000800
#define V_BUFF_BASE 0x00001000
#define U_BUFF_BASE 0x00001400
#define T_BUFF_BASE 0x00001800
#define RX_BUFF_BASE 0x00002000

#define PKT_LENGTH    0x20

int main() {
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

    void *V_Buff = ddr_mem+V_BUFF_BASE;
    void *U_Buff = ddr_mem+U_BUFF_BASE;
    void *T_Buff = ddr_mem+T_BUFF_BASE;
    uint32_t *Vbuff = (uint32_t *)V_Buff;
    uint32_t *Ubuff = (uint32_t *)U_Buff;
    uint32_t *Tbuff = (uint32_t *)T_Buff;
    for (int i=0; i<3; i++){
    memset(Vbuff+(i*PKT_LENGTH/4),0x10+i,PKT_LENGTH);
    memset(Ubuff+(i*PKT_LENGTH/4),0x20+i,PKT_LENGTH);
    memset(Tbuff+(i*PKT_LENGTH/4),0x30+i,PKT_LENGTH);
    }

	printf("TX BUFFERS \n\r");
    for (int i=0; i<3*PKT_LENGTH/4; i++){
    	printf("V_Val %02x  data %x \t",i,Vbuff[i]);
    	printf("U_Val %02x  data %x \t",i,Ubuff[i]);
    	printf("T_Val %02x  data %x \n\r",i,Tbuff[i]);
    }

    void *Tx_BD = ddr_mem+TX_BD_BASE;
    uint32_t *tx_BD = (uint32_t *)Tx_BD;
    memset(tx_BD,0x00,9*BD_SIZE);
	for (int i=0; i<3; i++){
    tx_BD[(i*3*BD_SIZE+0*BD_SIZE+0x00)/4] = DDR_BASE_ADDR+TX_BD_BASE+i*3*BD_SIZE+1*BD_SIZE;
    tx_BD[(i*3*BD_SIZE+0*BD_SIZE+0x08)/4] = DDR_BASE_ADDR+V_BUFF_BASE+i*PKT_LENGTH;
    tx_BD[(i*3*BD_SIZE+0*BD_SIZE+0x18)/4] = 0x08000000+(PKT_LENGTH);
    tx_BD[(i*3*BD_SIZE+0*BD_SIZE+0x1c)/4] = 0x00000000;
    tx_BD[(i*3*BD_SIZE+0*BD_SIZE+0x30)/4] = 0x00000035;
    tx_BD[(i*3*BD_SIZE+1*BD_SIZE+0x00)/4] = DDR_BASE_ADDR+TX_BD_BASE+i*3*BD_SIZE+2*BD_SIZE;
    tx_BD[(i*3*BD_SIZE+1*BD_SIZE+0x08)/4] = DDR_BASE_ADDR+U_BUFF_BASE+i*PKT_LENGTH;
    tx_BD[(i*3*BD_SIZE+1*BD_SIZE+0x18)/4] = 0x00000000+(PKT_LENGTH);
    tx_BD[(i*3*BD_SIZE+1*BD_SIZE+0x1c)/4] = 0x00000000;
    tx_BD[(i*3*BD_SIZE+1*BD_SIZE+0x30)/4] = 0x00000035;
    tx_BD[(i*3*BD_SIZE+2*BD_SIZE+0x00)/4] = DDR_BASE_ADDR+TX_BD_BASE+i*3*BD_SIZE+3*BD_SIZE;
    tx_BD[(i*3*BD_SIZE+2*BD_SIZE+0x08)/4] = DDR_BASE_ADDR+T_BUFF_BASE+i*PKT_LENGTH;
    tx_BD[(i*3*BD_SIZE+2*BD_SIZE+0x18)/4] = 0x04000000+(PKT_LENGTH);
    tx_BD[(i*3*BD_SIZE+2*BD_SIZE+0x1c)/4] = 0x00000000;
    tx_BD[(i*3*BD_SIZE+2*BD_SIZE+0x30)/4] = 0x00000035;
	}

    void *Rx_BD = ddr_mem+RX_BD_BASE;
    uint32_t *rx_BD = (uint32_t *)Rx_BD;
    memset(rx_BD,0x00,9*BD_SIZE);
    for (int i=0; i<9;i++){
    rx_BD[(i*BD_SIZE+0x00)/4] = DDR_BASE_ADDR+RX_BD_BASE+i*BD_SIZE+BD_SIZE;
    rx_BD[(i*BD_SIZE+0x08)/4] = DDR_BASE_ADDR+RX_BUFF_BASE+i*PKT_LENGTH;
    rx_BD[(i*BD_SIZE+0x18)/4] = 0x00000000+PKT_LENGTH;
    rx_BD[(i*BD_SIZE+0x1c)/4] = 0x00000000;
    rx_BD[(i*BD_SIZE+0x30)/4] = 0x00000000;
    }

	printf ("\n\r TX BD pre \n\r");
	for (int j=0; j<9; j++){
		for (int i=0; i<16; i++){
			printf ("BD %08x ",tx_BD[16*j+i]);
		}
	printf ("\n\r");
	}
	printf ("\n\r RX BD pre \n\r");
	for (int j=0; j<9; j++){
		for (int i=0; i<16; i++){
			printf ("BD %08x ",rx_BD[16*j+i]);
		}
	printf ("\n\r");
	}


    /////////////////////////////// SGDMA operation ///////////////////////////////
	int mem_dma;
	void *dma_mem;
    // Open /dev/mem to access physical memory
	mem_dma = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fb < 0) {
        perror("Error opening /dev/mem");
        return -1;
    }

    // Map DDR memory into the process's address space
    dma_mem = mmap(NULL, DMA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_dma, DMA_BASE_ADDR);
    if (dma_mem == MAP_FAILED) {
        perror("Error mapping DDR memory");
        close(mem_dma);
        return -1;
    }

    uint32_t data0;
    uint32_t data1;
    uint32_t *DMAregs = (uint32_t *)dma_mem;
    printf ("\n\r START DMA  \n\r");
    DMAregs[0x00/4] = 0x4;
    DMAregs[0x30/4] = 0x4;
    data0 = DMAregs[0x00/4];
    data1 = DMAregs[0x30/4];
    DMAregs[0x08/4] = DDR_BASE_ADDR+TX_BD_BASE;
    DMAregs[0x38/4] = DDR_BASE_ADDR+RX_BD_BASE;
    DMAregs[0x00/4] = 0x1;
    DMAregs[0x30/4] = 0x1;
    DMAregs[0x40/4] = DDR_BASE_ADDR+RX_BD_BASE+8*BD_SIZE;;
    DMAregs[0x10/4] = DDR_BASE_ADDR+TX_BD_BASE+8*BD_SIZE;

    data0 = 0;
    data1 = 0;
    while (((data0 & 0x00000002) == 0) | ((data1 & 0x00000002) == 0)){
    	data0 = DMAregs[0x04/4];
    	data1 = DMAregs[0x34/4];
    }

    printf ("\n\r RX BD post  \n\r");
	for (int j=0; j<9; j++){
		for (int i=0; i<16; i++){
			printf ("BD %08x ",rx_BD[16*j+i]);
		}
	printf ("\n\r");
	}
    printf ("\n\r TX BD post  \n\r");
	for (int j=0; j<9; j++){
		for (int i=0; i<16; i++){
			printf ("BD %08x ",tx_BD[16*j+i]);
		}
	printf ("\n\r");
	}

    void *RX_Buff = ddr_mem+RX_BUFF_BASE;
    uint32_t *Rx_buff = (uint32_t *)RX_Buff;

	printf("\n\rRX BUFFERS \n\r");
	for (int j=0; j<9; j++){
		for (int i=0; i<PKT_LENGTH/4; i++){
			printf("RX_dat %08x ",Rx_buff[j*(PKT_LENGTH/4)+i]);
		}
		printf ("\n\r");
    }

    close(mem_dma);
    close(mem_fb);

    return 0;
}
