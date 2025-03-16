#include <ch32v003fun.h>
#include <atecc608xx.h>
#include <i2c.h>
#include <stdio.h>
#include <string.h>

volatile struct atecc608xx_protocol_s req;
volatile bool reqFlag = false;

struct __attribute__((packed)) config_s
{
    uint32_t SN;
    uint32_t RevNum;
    uint8_t AES_Enable;
    uint8_t I2C_Enable;
    uint8_t Reserved0;
    uint8_t I2C_Address;
    uint8_t GPIO_Control;
    uint8_t Reserved1;
    uint8_t CounterMatch;
    uint8_t ChipMode;
    uint16_t SlotConfig[SLOTS_CONFIG_NUM];
    uint64_t Counter0;
    uint64_t Counter1;
    uint8_t UseLock;
    uint8_t ViolateKeyPermission;
    uint16_t SecureBoot;
    uint8_t KdflvLoc;
    uint16_t KdflvStr;
    uint64_t Reserved2;
    uint8_t UserExtra;
    uint8_t UserExtraAdd;
    uint8_t LockValue;
    uint8_t LockConfig;
    uint16_t SlotLocked;
} config = {
    .SN = 0x67452301,
    .RevNum = 0x02600000,
    .AES_Enable = 1,
    .I2C_Enable = 1,
    .Reserved0 = 0,
    .I2C_Address = 0x6c,
    .GPIO_Control = 0x03,
    .Reserved1 = 0,
    .CounterMatch = 0,
    .ChipMode = 1,
    .SlotConfig = { 0 },
    .Counter0 = 0x00000000FFFFFFFF,
    .Counter1 = 0x00000000FFFFFFFF,
    .UseLock = 0,
    .ViolateKeyPermission = 0,
    .SecureBoot = 0xF703,
    .KdflvLoc = 0,
    .KdflvStr = 0x7669,
    .Reserved2 = 0,
    .UserExtra = 0,
    .UserExtraAdd = 0,
    .LockValue = 0,
    .LockConfig = 0,
    .SlotLocked = 0xffff
};

static int zone_read(uint8_t zone, uint16_t addr, size_t len, void * res)
{
    int err = -1;

    switch (zone)
    {
        case ZONE_CONFIG | 0b10000000:
        case ZONE_CONFIG:
        {
            if ((addr + len) > sizeof(struct config_s))
            {
                goto error;
            }

            memcpy(res, (uint8_t *)&config + addr, len);

            break;
        }

        case ZONE_OTP | 0b10000000:
        case ZONE_OTP:
        {
            break;
        }

        case ZONE_DATA | 0b10000000:
        case ZONE_DATA:
        {
            break;
        }

        default:
        {
            break;
        }
    }

    err = 0;
error:
    return err;
}

int handleRequest()
{
    int err = -1;
    if (!reqFlag)
    {
        goto done;
    }

    printf("I2C Read request offset = %d\r\n", req.opcode);

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
            printf("COMMAND_OPCODE_READ = %02x, Zone: %d Addr: %d\r\n", req.opcode, req.param1, req.param2);
            if (zone_read(req.param1, req.param2, req.len, (void *)i2cReadBuffer) < 0)
            {
                memset((void *)i2cReadBuffer, 0, 255);
            }

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
