#include <ch32v003fun.h>
#include <atecc608xx.h>
#include <stdio.h>

struct atecc608xx_protocol_s req;
bool reqFlag = false;

int handleRequest()
{
    int err = -1;

    if (!reqFlag)
    {
        goto done;
    }

    printf("I2C Read request offset = %d\r\n", req.data[0]);

    switch (req.opcode)
    {
        case COMMAND_OPCODE_INFO:
        {
            break;
        }

        case COMMAND_OPCODE_LOCK:
        {
            break;
        }

        case COMMAND_OPCODE_RANDOM:
        {
            break;
        }

        case COMMAND_OPCODE_READ:
        {
            break;
        }

        case COMMAND_OPCODE_WRITE:
        {
            break;
        }

        case COMMAND_OPCODE_SHA:
        {
            break;
        }

        case COMMAND_OPCODE_GENKEY:
        {
            break;
        }

        case COMMAND_OPCODE_NONCE:
        {
            break;
        }

        case COMMAND_OPCODE_SIGN:
        {
            break;
        }

        case COMMAND_OPCODE_VERIFY:
        {
            break;
        }

        default:
        {
            goto error;
        }
    }

    reqFlag = false;

done:
    err = 0;

error:
    return err;
}
