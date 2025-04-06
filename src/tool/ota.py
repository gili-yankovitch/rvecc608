#!/usr/bin/python

from Crypto.Cipher import AES
from struct import pack, unpack
import progressbar
import argparse
import os
import serial

iv = bytes([ 0 ] * AES.block_size)
key = bytes([ x for x in range(AES.block_size) ])

DEBUG = False

START_OFFSET = 0xc0
END_OFFSET = 0x3000
HEADER_SIZE = 8
CHUNK_SIZE = 64
PAD_SIZE = AES.block_size - (HEADER_SIZE + CHUNK_SIZE) % AES.block_size
TOTAL_SIZE = HEADER_SIZE + CHUNK_SIZE + PAD_SIZE

def pad(x, m):
    p = m - (len(x) % m)
    return x + bytes([p] * p)

def conn():
    return serial.Serial("/dev/ttyUSB0",
                    baudrate = 115200,
                    parity = serial.PARITY_NONE,
                    stopbits = serial.STOPBITS_ONE,
                    bytesize = serial.EIGHTBITS)

def cksum16(data):
    sum = 0
    for i in range(0, len(data), 2):
        sum += unpack("H", data[i : i + 2])[0]
    return (sum & 0xffff) + (sum >> 16)

def chunk(data, addr, size):
    c = pack("HHHH",
        0x1337, # Magic
        addr,   # Addr
        size,   # Size
        0       # CKSUM
        ) + data[addr - START_OFFSET : addr - START_OFFSET + CHUNK_SIZE] + bytes([PAD_SIZE] * PAD_SIZE)

    return c[:2 * 3] + pack("H", cksum16(c)) + c[2 * 4:]

def prepare_image(filename):
    # Dissect the file.
    # Start: 0xc0
    # End: 0x3000

    with open(filename, "rb") as f:
        if "hello" in filename:
            # return bytes([x for x in range(START_OFFSET)]) + f.read()[START_OFFSET:END_OFFSET]
            return bytes([x for x in range(256)]) + f.read()[START_OFFSET:END_OFFSET]
        else:
            return f.read()[START_OFFSET:END_OFFSET]

def main(filename):
    if not os.path.exists(filename):
        print(f"OTA File {filename} does not exist.")

        exit(1)

    data = prepare_image(filename)

    # Divide into chunks
    chunks = [ chunk(data, START_OFFSET + addr, len(data)) for addr in range(0, len(data), CHUNK_SIZE) ]

    # Encrypt it all in one go
    plaintext = b"".join(chunks)
    cipher = AES.new(key, AES.MODE_CBC, iv)
    ciphertext = cipher.encrypt(plaintext)

    # Open a serial connection to the device
    c = conn()

    print("Flashing...")

    bar = progressbar.ProgressBar(maxval=len(ciphertext), \
        widgets=[progressbar.Bar('=', '[', ']'), ' ', progressbar.Percentage()])
    bar.start()

    # Write chunk-by-chunk
    for i in range(0, len(ciphertext), TOTAL_SIZE):
        if not DEBUG:
            bar.update(i + TOTAL_SIZE)
        else:
            print(f"Sent chunk {i // TOTAL_SIZE} address {hex(START_OFFSET + (i // TOTAL_SIZE) * CHUNK_SIZE)}...")
        c.write(ciphertext[i:i + TOTAL_SIZE])
        data = c.read(1)

        if DEBUG:
            print("Recvd from chip:", data)

        if data != b"V":
            print("Failed flashing. Reason:",)
            while True:
                print(c.read(1))
    c.close()

    bar.finish()
    print("Done")

if __name__ == "__main__":
    parser = argparse.ArgumentParser("Sword of Secrets OTA Update")
    parser.add_argument("filename", help = "Update file path")
    args = parser.parse_args()

    main(args.filename)
