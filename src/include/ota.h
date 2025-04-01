#ifndef __OTA_H__
#define __OTA_H__

#include <stdint.h>
#include <aes.h>

#define UART_MAGIC 0x13374242

// AES Blocks are 16 bytes. Then data should be what remains of such block size
struct uart_chunk_header_s
{
    uint32_t magic;
    uint16_t total_size;
    uint16_t cksum;
};

struct uart_chunk_s
{
    struct uart_chunk_header_s header;
    uint8_t data[AES_BLOCKLEN - sizeof(struct uart_chunk_header_s)];
};

void updateInit();

void recvChunk();

#endif // __OTA_H__
