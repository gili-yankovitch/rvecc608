#include <ch32v003fun.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <uart.h>
#include <ota.h>
#include <prot.h>

#define FLASH_ADDR 0x08000000
#define FLASH_SIZE 0x00004000

static uint8_t iv[AES_BLOCKLEN] = { 0 };
static uint8_t k[AES_BLOCKLEN] = {
                0x0, 0x1, 0x2, 0x3,
                0x4, 0x5, 0x6, 0x7,
                0x8, 0x9, 0xa, 0xb,
                0xc, 0xd, 0xe, 0xf
            };

static struct AES_ctx ctx;

void __attribute__(( section(".topflash.text") )) updateInit()
{
    // Manually implement CBC
    AES_init_ctx(&ctx, (const uint8_t *)&k);

    memset(iv, 0, AES_BLOCKLEN);
}

void __attribute__((noinline, used, section(".topflash.text") )) flashTest()
{
    uint32_t rdata;
    uint32_t addr = FLASH_ADDR + FLASH_SIZE - 129; // sizeof(uint16_t) * 2;
    uint32_t data[16] = {
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98,
        0x76543210, 0xFEDCBA98, 0x76543210, 0xFEDCBA98
        };

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
    size_t i;
    struct chunk_s chunk;
    uint8_t bkp[AES_BLOCKLEN];
    uint8_t * cur = (uint8_t *)&chunk;
    uint16_t cksum;
    uint16_t c;

    read(cur, AES_BLOCKLEN);
    memcpy(bkp, cur, AES_BLOCKLEN);

    // Decrypt
    AES_ECB_decrypt(&ctx, cur);

    // CBC
    for (i = 0; i < AES_BLOCKLEN; ++i)
    {
        cur[i] ^= iv[i];
    }

    // NULL checksum
    cksum = chunk.header.cksum;
    chunk.header.cksum = 0;
    c = cksum16(cur, sizeof(struct chunk_s));

    // Verify
    if ((chunk.header.magic != OTA_MAGIC) || (cksum != c))
    {
        _write(0, "X", 1);
#ifdef DEBUG
        _write(0, cur, 16);
        _write(0, &c, 2);
#endif
    }
    else
    {
        _write(0, "V", 1);

        // Copy current to iv
        memcpy(iv, bkp, AES_BLOCKLEN);
    }
}

void __attribute__(( noinline, used, section(".topflash.text") )) ota()
{
    for (;;)
    {
        if (uartAvailable())
        {
            recvChunk();
        }
    }
}

int main() __attribute__((used));

void __attribute__(( section(".topflash.text") )) boot()
{
    // Initialize everything
    SystemInit();

    uartInit();

    flashTest();

    updateInit();

    ota();

    // Call main()
    main();
}
