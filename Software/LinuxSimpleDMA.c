#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>

#define DDR_BASE_ADDR 0x10000000  // Replace with the actual DDR base address
#define DDR_SIZE      0x4000      // Size of the DDR region (in bytes)

#define DMA_BASE_ADDR 0x40400000  // Replace with the actual DDR base address
#define DMA_SIZE      0x100      // Size of the DDR region (in bytes)

#define TX_BUFF_BASE 0x1000
#define RX_BUFF_BASE 0x2000

#define PKT_LENGTH    0x20

int main() {
    int mem_fd;
    void *ddr_mem;
    uint8_t value = 0xc;
    uint32_t data;

    // Open /dev/mem to access physical memory
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("Error opening /dev/mem");
        return -1;
    }

    // Map DDR memory into the process's address space
    ddr_mem = mmap(NULL, DDR_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, DDR_BASE_ADDR);
    if (ddr_mem == MAP_FAILED) {
        perror("Error mapping DDR memory");
        close(mem_fd);
        return -1;
    }

    void *Tx_Check = ddr_mem+TX_BUFF_BASE;
    uint32_t *TXdata = (uint32_t *)Tx_Check;
    printf ("\n\r read empty  TX BUFFER  \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++)
	{
		data = TXdata[i];
		printf ("find add %d data %x\n\r",i,data);
	}

    void *Tx_addr = ddr_mem+TX_BUFF_BASE;
    printf ("\n\r Load TX BUFFER  \n\r");
    uint8_t *Packet = (uint8_t *)Tx_addr;
	for (int i=0; i<PKT_LENGTH; i++)
	{
		Packet[i] = value;
		value++;
	}
//    msync(Tx_addr, PKT_LENGTH*4, MS_SYNC);

//    void *Tx_Check = ddr_mem+TX_BUFF_BASE;
//    uint32_t *TXdata = (uint32_t *)Tx_Check;
    printf ("\n\r read Loaded  TX BUFFER  \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++)
	{
		data = TXdata[i];
		printf ("find add %d data %x\n\r",i,data);
	}

    void *Rx_Check = ddr_mem+RX_BUFF_BASE;
    uint32_t *RXdata = (uint32_t *)Rx_Check;
    printf ("\n\r read empty  RX BUFFER  \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++)
	{
		data = RXdata[i];
		printf ("find add %d data %x\n\r",i,data);
	}

    int dma_fd;
    void *dma_mem;
    dma_fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (mem_fd < 0) {
        perror("Error opening /dev/mem");
        return -1;
    }
    // Map DDR memory into the process's address space
    dma_mem = mmap(NULL, DMA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dma_fd, DMA_BASE_ADDR);
    if (ddr_mem == MAP_FAILED) {
        perror("Error mapping DDR memory");
        close(mem_fd);
        return -1;
    }
    uint32_t *DMAregs = (uint32_t *)dma_mem;
    printf ("\n\r START DMA  \n\r");
    DMAregs[0x00/4] = 0x4;
    DMAregs[0x30/4] = 0x4;
    DMAregs[0x18/4] = DDR_BASE_ADDR+TX_BUFF_BASE;
    DMAregs[0x48/4] = DDR_BASE_ADDR+RX_BUFF_BASE;
    DMAregs[0x00/4] = 0x1;
    DMAregs[0x30/4] = 0x1;
    DMAregs[0x58/4] = PKT_LENGTH;
    DMAregs[0x28/4] = PKT_LENGTH;


//    void *Rx_Check = ddr_mem+RX_BUFF_BASE;
//    uint32_t *RXdata = (uint32_t *)Rx_Check;
    printf ("\n\r read Moved  RX BUFFER  \n\r");
    for (int i=0; i<PKT_LENGTH/4; i++)
	{
		data = RXdata[i];
		printf ("find add %d data %x\n\r",i,data);
	}

    // Unmap memory and close file descriptor
    if (munmap(dma_mem, DMA_SIZE) < 0) {
        perror("Error unmapping memory");
    }

    close(dma_fd);

    // Unmap memory and close file descriptor
    if (munmap(ddr_mem, DDR_SIZE) < 0) {
        perror("Error unmapping memory");
    }

    close(mem_fd);


    return 0;
}
