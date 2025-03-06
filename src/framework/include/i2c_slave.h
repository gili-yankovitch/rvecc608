/*
 * Single-File-Header for using the I2C peripheral in slave mode
 *
 * MIT License
 *
 * Copyright (c) 2024 Renze Nicolai
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __I2C_SLAVE_H
#define __I2C_SLAVE_H

#include "ch32v003fun.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

typedef void (*i2c_write_callback_t)(uint8_t offset, uint8_t length);
typedef uint8_t (*i2c_read_callback_t)(uint8_t offset);

struct _i2cSlave
{
    uint8_t writeOffset;
    uint8_t writeSize;
    uint8_t readOffset;
    i2c_write_callback_t writeCb;
    i2c_read_callback_t readCb;
#if 0
    uint8_t position;
    uint8_t size1;
    volatile uint8_t* volatile registers2;
    uint8_t size2;
    bool read_only1;
    i2c_write_callback_t write_callback2;
    i2c_read_callback_t read_callback2;
    bool read_only2;
    bool writing;
#endif

    bool address2matched;
} i2cSlave;

#endif
