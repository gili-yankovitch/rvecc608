#include <stdint.h>
#include <stdbool.h>

#ifndef __ATECC608XX_H__
#define __ATECC608XX_H__

struct atecc608xx_protocol_s
{
    uint8_t cmd;
    uint8_t len;
    uint8_t opcode;
    uint8_t param1;
    uint8_t param2;
    uint8_t data[250];
};

extern struct atecc608xx_protocol_s req;
extern bool reqFlag;
int handleRequest();

/* Protocol Indices */
#define ATRCC508A_PROTOCOL_FIELD_COMMAND 0
#define ATRCC508A_PROTOCOL_FIELD_LENGTH  1
#define ATRCC508A_PROTOCOL_FIELD_OPCODE  2
#define ATRCC508A_PROTOCOL_FIELD_PARAM1  3
#define ATRCC508A_PROTOCOL_FIELD_PARAM2  4
#define ATRCC508A_PROTOCOL_FIELD_DATA    6

#define ATECC508A_ADDRESS_DEFAULT 0x60 //7-bit unshifted default I2C Address
// 0x60 on a fresh chip. note, this is software definable

#define WORD_ADDRESS_VALUE_COMMAND      0x03    // This is the "command" word address,
//this tells the IC we are going to send a command, and is used for most communications to the IC
#define WORD_ADDRESS_VALUE_IDLE 0x02 // used to enter idle mode

// COMMANDS (aka "opcodes" in the datasheet)
#define COMMAND_OPCODE_INFO     0x30 // Return device state information.
#define COMMAND_OPCODE_LOCK     0x17 // Lock configuration and/or Data and OTP zones
#define COMMAND_OPCODE_RANDOM   0x1B // Create and return a random number (32 bytes of data)
#define COMMAND_OPCODE_READ     0x02 // Return data at a specific zone and address.
#define COMMAND_OPCODE_WRITE    0x12 // Return data at a specific zone and address.
#define COMMAND_OPCODE_SHA              0x47 // Computes a SHA-256 or HMAC/SHA digest for general purpose use by the system.
#define COMMAND_OPCODE_GENKEY   0x40 // Creates a key (public and/or private) and stores it in a memory key slot
#define COMMAND_OPCODE_NONCE    0x16 //
#define COMMAND_OPCODE_SIGN     0x41 // Create an ECC signature with contents of TempKey and designated key slot
#define COMMAND_OPCODE_VERIFY   0x45 // takes an ECDSA <R,S> signature and verifies that it is correctly generated from a given message and public key


#endif
