#include <ch32v003fun.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <uart.h>
#include <ota.h>

static uint8_t prev[AES_BLOCKLEN] = { 0 };
static uint8_t k[AES_BLOCKLEN] = {
                0x0, 0x1, 0x2, 0x3,
                0x4, 0x5, 0x6, 0x7,
                0x8, 0x9, 0xa, 0xb,
                0xc, 0xd, 0xe, 0xf
            };

static struct AES_ctx ctx;

void __attribute__ (( section(".topflash.text") )) updateInit()
{
    // Manually implement CBC
    AES_init_ctx(&ctx, (const uint8_t *)&k);

    memset(prev, 0, AES_BLOCKLEN);
}

void __attribute__((noinline, used, section(".topflash.text") )) recvChunk()
{
    size_t i;
    uint8_t cur[AES_BLOCKLEN];

    read(cur, AES_BLOCKLEN);

    printf("Before: %s\r\n", cur);

    // Decrypt
    AES_ECB_decrypt(&ctx, cur);

    // CBC
    for (i = 0; i < AES_BLOCKLEN; ++i)
    {
        uint8_t c = cur[i];

        cur[i] ^= prev[i];

        // Prepare for next block
        prev[i] = c;
    }

    printf("After: %s\r\n", cur);
}

int main() __attribute__((used));

void __attribute__(( section(".topflash.text") )) boot()
{
    // Initialize everything
    SystemInit();

    uartInit();

    // Call main()
    main();
}
