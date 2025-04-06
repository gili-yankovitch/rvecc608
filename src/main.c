#include <ch32v003fun.h>
#include <ch32v003_GPIO_branchless.h>
#include <stdio.h>
#include <string.h>
#include <prot.h>
#include <uECC.h>
#include <aes.h>
#include <i2c.h>
#include <atecc608xx.h>
#include <uart.h>
#include <ota.h>

static int readAnalog()
{
    return SysTick->CNT ^ GPIO_analogRead(GPIOv_from_PORT_PIN(GPIO_port_D, 3));
}

static int RNG(uint8_t *dest, unsigned size)
{
  // Use the least-significant bits from the ADC for an unconnected pin (or connected to a source of
  // random noise). This can take a long time to generate random data if the result of analogRead(0)
  // doesn't change very frequently.
  while (size) {
    uint8_t val = 0;
    for (unsigned i = 0; i < 8; ++i) {
      int init = readAnalog() ^ SysTick->CNT;
      int count = 0;
      while (readAnalog() == init) {
        ++count;
      }

      if (count == 0) {
         val = (val << 1) | (init & 0x01);
      } else {
         val = (val << 1) | (count & 0x01);
      }
    }
    *dest = val;
    ++dest;
    --size;
  }
  // NOTE: it would be a good idea to hash the resulting random data using SHA-256 or similar.
  return 1;
}

static void uECCTest()
{
    const struct uECC_Curve_t * curve = uECC_secp160r1();
    uint8_t private1[21];
    uint8_t private2[21];

    uint8_t public1[40];
    uint8_t public2[40];

    uint8_t secret1[20];
    uint8_t secret2[20];

    unsigned long a = SysTick->CNT;
    uECC_make_key(public1, private1, curve);
    unsigned long b = SysTick->CNT;

    printf("Made key 1 in %ld\r\n", b - a);
    a = SysTick->CNT;
    uECC_make_key(public2, private2, curve);
    b = SysTick->CNT;
    printf("Made key 2 in %ld\r\n", b - a);

    a = SysTick->CNT;
    int r = uECC_shared_secret(public2, private1, secret1, curve);
    b = SysTick->CNT;
    printf("Shared secret 1 in %ld\r\n", b - a);

    if (!r) {
        printf("shared_secret() failed (1)\r\n");
        return;
    }

    a = SysTick->CNT;
    r = uECC_shared_secret(public1, private2, secret2, curve);
    b = SysTick->CNT;
    printf("Shared secret 2 in %ld\r\n", b - a);
    if (!r) {
        printf("shared_secret() failed (2)\r\n");
        return;
    }

    if (memcmp(secret1, secret2, 20) != 0) {
        printf("==== FAILED ====\r\n");
    } else {
        printf("==== PASSED ====\r\n");
    }
}

static void aesTest()
{
    struct AES_ctx ctx;
    uint8_t iv[AES_BLOCKLEN] = { 0 };
    uint8_t key[AES_BLOCKLEN];
    uint8_t message[] = "Hello, world!!!!";
    size_t len = strlen((char *)message);

    // Randomize key
    RNG(key, AES_BLOCKLEN);

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, (uint8_t *)message, len);

    printf("Encrypted message: %s\r\n", message);

    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, (uint8_t *)message, len);

    printf("Decrypted message: %s\r\n", message);
}

static void cryptoTest()
{
    uECCTest();
    aesTest();
}


int  __attribute__(( used, section(".main") )) main()
{
    // Enable GPIOs
    funGpioInitAll();

    GPIO_ADCinit();

	GPIO_pinMode(GPIOv_from_PORT_PIN(GPIO_port_D, 3),
	    GPIO_pinMode_I_analog,
	    GPIO_Speed_In);

    if (i2cSetup() < 0)
    {
        printf("ERROR: Error setting up I2C\r\n");

        goto done;
    }

    printf("i2c Setup Complete\r\n");

    // Lock flash from external read/write
    // flashReadProtect();

    // printf("Flash locked.\r\n");

    // Optional: For blinking LED
    funPinMode(PC3, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);

#if 0
    uECC_set_rng(&RNG);
    cryptoTest();
#endif

done:
    for (;;)
    {
#if 0
        printf("Flashing test...\r\n");

        Delay_Ms(1000);
#else
        funDigitalWrite(PC3, FUN_HIGH);
        Delay_Ms(100);
        funDigitalWrite(PC3, FUN_LOW);
        Delay_Ms(100);
#endif
        // handleRequest();
    }

    return 0;
}
