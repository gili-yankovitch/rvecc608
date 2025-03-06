#include <ch32v003fun.h>
#include <i2c_slave.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <atecc608xx.h>

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

static volatile uint8_t i2cWriteBuffer[255] = {0x00};

static void onWrite(uint8_t offset, uint8_t length)
{
    uint8_t * buf = (uint8_t *)&i2cWriteBuffer[offset];

    printf("Received: %s\r\n", buf);
}

static uint8_t onRead(uint8_t offset)
{
    uint8_t data[] = "Goodbye!";
    return data[offset];
}

static void SetupI2CSlave(uint8_t address, i2c_write_callback_t write_callback, i2c_read_callback_t read_callback)
{
    i2cSlave.writeOffset = i2cSlave.writeSize = 0;
    i2cSlave.readOffset = 0;
    i2cSlave.writeCb = write_callback;
    i2cSlave.readCb = read_callback;

    // Enable I2C1
    RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

    // Reset I2C1 to init all regs
    RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
    RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;

    I2C1->CTLR1 |= I2C_CTLR1_SWRST;
    I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;

    // Set module clock frequency
    uint32_t prerate = 2000000; // I2C Logic clock rate, must be higher than the bus clock rate
    I2C1->CTLR2 |= (FUNCONF_SYSTEM_CORE_CLOCK/prerate) & I2C_CTLR2_FREQ;

    // Enable interrupts
    I2C1->CTLR2 |= I2C_CTLR2_ITBUFEN | I2C_CTLR2_ITEVTEN | I2C_CTLR2_ITERREN;

    NVIC_EnableIRQ(I2C1_EV_IRQn); // Event interrupt
    NVIC_SetPriority(I2C1_EV_IRQn, 2 << 4);
    NVIC_EnableIRQ(I2C1_ER_IRQn); // Error interrupt
    NVIC_SetPriority(I2C1_ER_IRQn, 2 << 4);

    // Set clock configuration
    uint32_t clockrate = 1000000; // I2C Bus clock rate, must be lower than the logic clock rate
    I2C1->CKCFGR = ((FUNCONF_SYSTEM_CORE_CLOCK/(3*clockrate))&I2C_CKCFGR_CCR) | I2C_CKCFGR_FS; // Fast mode 33% duty cycle
    //I2C1->CKCFGR = ((FUNCONF_SYSTEM_CORE_CLOCK/(25*clockrate))&I2C_CKCFGR_CCR) | I2C_CKCFGR_DUTY | I2C_CKCFGR_FS; // Fast mode 36% duty cycle
    // I2C1->CKCFGR = (FUNCONF_SYSTEM_CORE_CLOCK/(2*clockrate))&I2C_CKCFGR_CCR; // Standard mode good to 100kHz

    // Set I2C address
    I2C1->OADDR1 = address << 1;
    I2C1->OADDR2 = 0;

    // Enable I2C
    I2C1->CTLR1 |= I2C_CTLR1_PE;

    // Acknowledge bytes when they are received
    I2C1->CTLR1 |= I2C_CTLR1_ACK;
}

void I2C1_EV_IRQHandler(void) __attribute__((interrupt));
void I2C1_EV_IRQHandler(void) {
    uint16_t STAR1, STAR2 __attribute__((unused));
    STAR1 = I2C1->STAR1;
    STAR2 = I2C1->STAR2;

    // Write event
    if (STAR1 & I2C_STAR1_RXNE)
    {
        // Start event
        if (STAR1 & I2C_STAR1_ADDR)
        {
            i2cSlave.writeOffset = i2cSlave.writeSize = 0; // Reset position
            // i2cSlave.address2matched = !!(STAR2 & I2C_STAR2_DUALF);
        }

        i2cWriteBuffer[i2cSlave.writeSize++] = I2C1->DATAR;
    }

    // Read event
    if (STAR1 & I2C_STAR1_TXE)
    {
        if (i2cSlave.readCb != NULL)
        {
            I2C1->DATAR = i2cSlave.readCb(i2cSlave.readOffset++);
        }
    }

    // Stop event
    if (STAR1 & I2C_STAR1_STOPF)
    {
        I2C1->CTLR1 &= ~(I2C_CTLR1_STOP); // Clear stop

        if (i2cSlave.writeCb != NULL)
        {
            i2cSlave.writeCb(i2cSlave.writeOffset, i2cSlave.writeSize - i2cSlave.writeOffset);
        }

        i2cSlave.writeOffset = i2cSlave.writeSize;
    }
}

void I2C1_ER_IRQHandler(void) __attribute__((interrupt));
void I2C1_ER_IRQHandler(void) {
    uint16_t STAR1 = I2C1->STAR1;

    if (STAR1 & I2C_STAR1_BERR) { // Bus error
        I2C1->STAR1 &= ~(I2C_STAR1_BERR); // Clear error
    }

    if (STAR1 & I2C_STAR1_ARLO) { // Arbitration lost error
        I2C1->STAR1 &= ~(I2C_STAR1_ARLO); // Clear error
    }

    if (STAR1 & I2C_STAR1_AF) { // Acknowledge failure
        // This is how a read request ends
        i2cSlave.readOffset = 0;
        I2C1->STAR1 &= ~(I2C_STAR1_AF); // Clear error
    }
}

int i2cSetup()
{
    // Initialize I2C slave
    funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
    funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL
    // SetupI2CSlave(ATECC508A_ADDRESS_DEFAULT, i2c_registers, sizeof(i2c_registers), onWrite, onRead, false);
    SetupI2CSlave(ATECC508A_ADDRESS_DEFAULT, onWrite, onRead);

    return 0;
}

