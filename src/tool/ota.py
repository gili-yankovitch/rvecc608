#!/usr/bin/python

from Crypto.Cipher import AES
import argparse
import os
import serial

iv = bytes([ 0 ] * AES.block_size)
key = bytes([ x for x in range(AES.block_size) ])

def pad(x, m):
    p = m - (len(x) % m)
    return x + bytes([p] * p)

def conn():
    return serial.Serial("/dev/ttyUSB0",
                    baudrate = 115200,
                    parity = serial.PARITY_NONE,
                    stopbits = serial.STOPBITS_ONE,
                    bytesize = serial.EIGHTBITS)

def main(filename):
    if not os.path.exists(filename):
        print(f"OTA File {filename} does not exist.")

        exit(1)

    with open(filename) as f:
        msg = pad(f.read().encode(), AES.block_size)
    cipher = AES.new(key, AES.MODE_CBC, iv)
    cipher_text = cipher.encrypt(msg)
    print(f"Filename: {filename}")

    #cipher_text = bytes("Hello, world!!!\x00", "ascii")
    c = conn()
    print(f"Sending: {cipher_text}")
    c.write(cipher_text)
    data = c.read(51)
    print(data)
    c.close()

if __name__ == "__main__":
    parser = argparse.ArgumentParser("Sword of Secrets OTA Update")
    parser.add_argument("filename", help = "Update file path")
    args = parser.parse_args()

    main(args.filename)
