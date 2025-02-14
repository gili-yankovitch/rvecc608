#include <ch32v003fun.h>
#include <stdio.h>
#include <prot.h>

#define FLASH_ADDR 0x08000000
#define FLASH_SIZE 0x00004000

int main()
{
    uint32_t addr;
    uint32_t data[16] = {
        0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF,
        0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF,
        0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF,
        0x01234567, 0x89ABCDEF, 0x01234567, 0x89ABCDEF
        };
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

    addr = FLASH_ADDR + FLASH_SIZE - 512; // sizeof(uint16_t) * 2;

    // Reading
    rdata = flashRead(addr);

    printf("[READ #0] Data at address 0x%lx: %lx\r\n", addr, rdata);

    flashPageErase(addr);

    // Reading
    rdata = flashRead(addr);

    printf("[READ #1] AFTER ERASE Data at address 0x%lx: %lx\r\n", addr, rdata);

    flashWrite(addr, data);

    // Reading
    rdata = flashRead(addr);

    printf("[READ #2] AFTER WRITE Data at address 0x%lx: %lx\r\n", addr, rdata);

    for (;;) ;

    return 0;
}
