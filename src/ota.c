#include <ch32v003fun.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <uart.h>
#include <ota.h>
#include <prot.h>

#define RESET_MAX_JIFFIES 10000000
#define FLASH_ADDR 0x08000000
#define FLASH_SIZE 0x00004000
#define OTA_START_ADDR 0xc0
#define OTA_END_ADDR   0x3000

static uint8_t iv[AES_BLOCKLEN] = { 0 };
static const uint8_t __attribute__(( used, section(".topflash.rodata") )) k[AES_BLOCKLEN] = {
                0x0, 0x1, 0x2, 0x3,
                0x4, 0x5, 0x6, 0x7,
                0x8, 0x9, 0xa, 0xb,
                0xc, 0xd, 0xe, 0xf
            };

static struct AES_ctx ctx;
static const uint8_t __attribute__(( used, section(".topflash.rodata") )) status[] = "VMCSE";

void __attribute__(( section(".topflash.text") )) blink(int n)
{
    int i;

    for (i = 0; i < n; ++i)
    {
        funDigitalWrite(PC3, FUN_HIGH);
        Delay_Ms(100);
        funDigitalWrite(PC3, FUN_LOW);
        Delay_Ms(100);
    }
}

void __attribute__(( section(".topflash.text") )) updateInit()
{
    memset(iv, 0, AES_BLOCKLEN);

    AES_init_ctx_iv(&ctx, (const uint8_t *)&k, (const uint8_t *)&iv);
}

#define SIZE_WRITE 64

void __attribute__(( section(".topflash.text") )) flashTest()
{
    uint32_t rdata;
    uint32_t addr = FLASH_ADDR + 0xc0; // FLASH_SIZE - 129; // sizeof(uint16_t) * 2;
    uint32_t data[16];
    int i;
#if 0
     = {
        0x03020100, 0x07060504, 0x0B0A0908, 
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98
        };
#endif

    for (i = 0; i < sizeof(data); i++)
    {
        *((uint8_t *)data + i) = i;
    }

    // Reading
    flashRead(addr, &rdata, sizeof(rdata));

    printf("[READ #0] Data at address 0x%lx: %lx\r\n", addr, rdata);

    flashPageErase(addr);

    // Reading
    flashRead(addr, &rdata, sizeof(rdata));

    printf("[READ #1] AFTER ERASE Data at address 0x%lx: %lx\r\n", addr, rdata);

    for (i = 0; i < sizeof(data); i += SIZE_WRITE)
    {
        flashWrite(addr + i, (uint8_t *)&data + i, SIZE_WRITE);
    }

    // _flashWrite(addr, &data);

    // Reading
    flashRead(addr, &rdata, sizeof(rdata));

    printf("[READ #2] AFTER WRITE Data at address 0x%lx: %lx\r\n", addr, rdata);

    // Reading
    flashRead(addr + 8, &rdata, sizeof(rdata));

    printf("[READ #3] AFTER WRITE Data at address 0x%lx: %lx\r\n", addr + 8, rdata);

    flashRead(addr + 16, &rdata, sizeof(rdata));

    printf("[READ #4] AFTER WRITE Data at address 0x%lx: %lx\r\n", addr + 16, rdata);
}

uint16_t __attribute__(( section(".topflash.text") )) cksum16(uint8_t * buf, size_t s)
{
    uint32_t sum = 0;
    int i;

    for (i = 0; i < s; i += 2)
    {
        sum += *((uint16_t *)(buf + i));
    }

    return (sum & 0xffff) + (sum >> 16);
}

void __attribute__((noinline, used, section(".topflash.text") )) recvChunk()
{
    struct chunk_s chunk;
    // uint8_t bkp[AES_BLOCKLEN];
    uint16_t cksum;

    read((uint8_t *)&chunk, sizeof(struct chunk_s));

    // blink(2);

    AES_CBC_decrypt_buffer(&ctx, (uint8_t *)&chunk, sizeof(struct chunk_s));

    // NULL checksum
    cksum = chunk.header.cksum;
    chunk.header.cksum = 0;

    // Verify:
    // 1. Magic for sanity
    // 2. Checksum for correctness
    // 3. Chunk address does not overwrite OTA code or ISRVec
    if (chunk.header.magic != OTA_MAGIC)
    {
        _write(0, (const char *)&status[1], 1);

        return;
    }

    if (cksum != cksum16((uint8_t *)&chunk, sizeof(struct chunk_s)))
    {
        _write(0, (const char *)&status[2], 1);

        return;
    }

    if (chunk.header.addr < OTA_START_ADDR)
    {
        _write(0, (const char *)&status[3], 1);

        return;
    }

    if (chunk.header.addr >= OTA_END_ADDR)
    {
        _write(0, (const char *)&status[4], 1);

        return;
    }
#if 0
    if ((chunk.header.magic != OTA_MAGIC)       ||
        (cksum != cksum16((uint8_t *)&chunk, sizeof(struct chunk_s)))                            ||
        (chunk.header.addr < OTA_START_ADDR)    ||
        (chunk.header.addr >= OTA_END_ADDR))
    {
        _write(0, "X", 1);
#ifdef DEBUG
        _write(0, cur, 16);
        _write(0, &c, 2);
#endif
        return;
    }
#endif
    // Write!
    // _flashWrite(FLASH_ADDR + chunk.header.addr, chunk.data);
    // flashPageErase(chunk.header.addr);
    flashWrite(FLASH_ADDR + chunk.header.addr, chunk.data, PAGE_SIZE);

    // Tell other side that this chunk was written successfully.
    _write(0, (const char *)&status[0], 1);
}

bool __attribute__(( noinline, used, section(".topflash.text") )) update_wait()
{
    uint32_t initial_jiffies = SysTick->CNT;

    while (SysTick->CNT - initial_jiffies < RESET_MAX_JIFFIES)
    {
        if (uartAvailable())
        {
            return true;
        }
    }

    return false;
}

void __attribute__(( noinline, used, section(".topflash.text") )) ota()
{
    // No UART on boot
    if (!update_wait())
    {
        return;
    }

    updateInit();

    for (;;)
    {
        // Wait for data
        if (uartAvailable())
        {
            recvChunk();
        }

        // Finish update if uart hangs
        if (!update_wait())
        {
            return;
        }

        // blink(1);
    }
}

int main() __attribute__((used));

void __attribute__(( section(".topflash.text") )) boot()
{
    // Initialize everything
    SystemInit();

    uartInit();

    // flashTest();

    ota();

    // blink(3);

    // Call main()
    main();
}
