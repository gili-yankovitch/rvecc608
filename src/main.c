#include <ch32v003fun.h>
#include <stdio.h>
#include <prot.h>

#define FLASH_ADDR 0x08000000
#define FLASH_SIZE 0x00004000
void flashFlockUnlock();
void flashFlockLock();
void flashUnlock();
void flashLock();
void flashBusy();
void _flashWrite(uint32_t addr, uint32_t * data);

int main()
{
    uint32_t addr;
#if 1
    uint32_t data[16] = {
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98
        };
#else
    uint32_t data[16] = {
        0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF,
        0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF,
        0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF,
        0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF
        };
#endif
    uint32_t rdata;

    SystemInit();

    SetupUART(UART_BRR);

    // Enable GPIOs
    funGpioInitAll();

    // Lock flash from external read/write
    // flashReadProtect();

    // printf("Flash locked.\r\n");

    // Optional: For blinking LED
    funPinMode(PC3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);

    // addr = FLASH_ADDR + FLASH_SIZE - 512; // sizeof(uint16_t) * 2;
    addr = FLASH_ADDR + FLASH_SIZE - 129; // sizeof(uint16_t) * 2;

    // Reading
    flashRead(addr, &rdata, sizeof(rdata));

    printf("[READ #0] Data at address 0x%lx: %lx\r\n", addr, rdata);

    flashPageErase(addr);

    // Reading
    flashRead(addr, &rdata, sizeof(rdata));

    printf("[READ #1] AFTER ERASE Data at address 0x%lx: %lx\r\n", addr, rdata);

    flashWrite(addr, &data, sizeof(data));
    // _flashWrite(addr, &data);

    // Reading
    flashRead(addr, &rdata, sizeof(rdata));

    printf("[READ #2] AFTER WRITE Data at address 0x%lx: %lx\r\n", addr, rdata);


    for (;;) ;

    return 0;
}
