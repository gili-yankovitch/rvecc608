#include <ch32v003fun.h>
#include <i2c_slave.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define I2C_INITIAL_ADDR 0x36

static volatile uint8_t i2c_registers[255] = {0x00};

static void onWrite(uint8_t reg, uint8_t length)
{
}

static void onRead(uint8_t reg)
{
}

int i2cSetup()
{
    // Initialize I2C slave
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL
    SetupI2CSlave(I2C_INITIAL_ADDR, i2c_registers, sizeof(i2c_registers), onWrite, onRead, false);

    return 0;
}

