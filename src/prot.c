#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define FLASH_OBKEYR ((volatile uint32_t *)0x40022008)
#define FLASH_KEYR ((volatile uint32_t *)0x40022004)
#define FLASH_MODEKEYR ((volatile uint32_t *)0x40022024)
#define FLASH_CTLR ((volatile uint32_t *)0x40022010)
#define FLASH_STATR ((volatile uint32_t *)0x4002200C)
#define FLASH_FLOCK   ((volatile uint32_t*)0x40022020)
#define RDPRT_KEY 0x000000A5
#ifndef FLASH_KEY1
#define FLASH_KEY1 0x45670123
#endif

#ifndef FLASH_KEY2
#define FLASH_KEY2 0xCDEF89AB
#endif

#define FLASH_ADDR ((volatile uint32_t *)0x40022014)
#define FLASH_OBR ((volatile uint32_t *)0x4002201C)
#define FLASH_RDPR (*FLASH_OBR & (1 << 1))
#define RDPR ((volatile uint16_t *)0x1FFFF800)
#define RDPR_ON 0x1331
#define RDPR_OFF 0x5AA5

#define FLASH_BUFRST_BIT     (1 << 19)
#define FLASH_BUFLOAD_BIT    (1 << 18)
#define FLASH_FTER_BIT       (1 << 17)
#define FLASH_FTPG_BIT       (1 << 16)
#define FLASH_STRT_BIT       (1 << 6)
#define FLASH_LOCK_BIT       (1 << 7) // LOCK bit
#define FLASH_PER_BIT        (1 << 1) // Page Erase bit
#define FLASH_PG_BIT         (1 << 0) // Program mode bit
#define FLASH_BSY_BIT        (1 << 0) // Busy bit
#define FLASH_EOP_BIT        (1 << 5) // End of programming bit


static void flashFlockUnlock()
{
    *FLASH_MODEKEYR = FLASH_KEY1;
    *FLASH_MODEKEYR = FLASH_KEY2;
    *FLASH_FLOCK = 0x00000000; // Unlock Flash if locked
}

static inline void flashFlockLock()
{
    *FLASH_FLOCK = 0x1; // Unlock Flash if locked
}

static void flashUnlock()
{
    *FLASH_KEYR = FLASH_KEY1;
    *FLASH_KEYR = FLASH_KEY2;
}

void flashLock()
{
    *FLASH_CTLR |= FLASH_LOCK_BIT;  // Set LOCK bit
}

static inline void flashUnlockOBKEYR()
{
    *FLASH_OBKEYR = FLASH_KEY1;
    *FLASH_OBKEYR = FLASH_KEY2;
}

static inline void enableFlashProgramming()
{
    *FLASH_CTLR |= FLASH_PG_BIT; // Set PG bit
}

static inline void disableFlashProgramming()
{
    *FLASH_CTLR &= ~FLASH_PG_BIT; // Set PG bit
}

static inline void flashBusy()
{
    while (((*FLASH_STATR) & FLASH_BSY_BIT)) ;
}

static inline void flashEOP()
{
    while ((((*FLASH_STATR) & FLASH_BSY_BIT)) && (!((*FLASH_STATR) & FLASH_EOP_BIT))) ;
    *FLASH_STATR |= FLASH_EOP_BIT;
}

void flashPageErase(uint32_t address)
{
    // #1
    flashUnlock();

    // #2
    flashFlockUnlock();

    // #3
    flashBusy();

    // #4
    *FLASH_CTLR |= FLASH_FTER_BIT;

    // #5
    *FLASH_ADDR = address;

    // #6
    *FLASH_CTLR |= FLASH_STRT_BIT;

    // #7
    flashEOP();

    flashFlockLock();
    flashLock();
}

void _flashPageErase(uint32_t address)
{
    flashUnlock();  // Unlock flash for writing
    flashBusy();

    flashBusy();  // Ensure flash is not busy

    *FLASH_CTLR |= FLASH_PER_BIT;  // Enable page erase mode
    *FLASH_ADDR = address;  // Set the address to erase
    *FLASH_CTLR |= (1 << 6); // Start the erase operation

    flashBusy();  // Wait for erase to complete

    // Clear EOP flag
    if (*FLASH_STATR & FLASH_EOP_BIT) {
        *FLASH_STATR |= FLASH_EOP_BIT;
    }

    *FLASH_CTLR &= ~FLASH_PER_BIT;  // Disable page erase mode
    flashLock();  // Lock flash again
}

uint32_t flashRead(uint32_t addr)
{
    return *(volatile uint32_t *)addr;
}

void flashWrite(uint32_t addr, uint32_t * data)
{
    int i;

    // Unlock Flash if needed
    flashUnlock();
    flashBusy();

    flashFlockUnlock();
    flashBusy();

    *FLASH_CTLR |= FLASH_FTPG_BIT;
    *FLASH_CTLR |= FLASH_BUFRST_BIT;

    flashEOP();

    // Write data to address
    for (i = 0; i < 16; i++)
    {
        *((volatile uint32_t *)addr + i) = data[i];
        *FLASH_CTLR |= FLASH_BUFLOAD_BIT;

        flashEOP();
    }

    *FLASH_ADDR = addr;

    *FLASH_CTLR |= FLASH_STRT_BIT;

    flashEOP();

    *FLASH_CTLR &= ~FLASH_FTPG_BIT;

    flashFlockLock();

    flashLock();
}

#define RAM_ADDR 0x20000000
#define RAM_SIZE 0x800
void _flashWrite(uint32_t addr, uint16_t data)
{
    void (* fptr)(uint32_t addr, uint16_t data) = (void *)RAM_ADDR;

    // Copy function to RAM
    memcpy(fptr, _flashWrite, RAM_SIZE);

    fptr(addr, data);
}

static void userSelectProg()
{
    flashUnlock();
    flashUnlockOBKEYR();

    flashBusy();

    // Set OBG
    *FLASH_CTLR |= 1 << 4;
}

static void _flashReadProtect()
{
    *FLASH_KEYR = RDPRT_KEY;
    *RDPR = RDPR_ON;
}

static void _flashReadUnprotect()
{
    *RDPR = RDPR_OFF;
}

int flashReadProtect()
{
    if (!FLASH_RDPR)
    {
        userSelectProg();
        _flashReadProtect();
    }

    return 0;
}

int flashReadUnprotect()
{
    if (FLASH_RDPR)
    {
        userSelectProg();
        _flashReadUnprotect();
    }

    return 0;
}
