SRCS:=src/main.c src/prot.c

# Check if riscv64-unknown-elf-gcc exists
ifneq ($(shell which riscv64-unknown-elf-gcc),)
    PREFIX?=riscv64-unknown-elf
# Check if riscv64-linux-gnu-gcc exists
else ifneq ($(shell which riscv64-linux-gnu-gcc),)
    PREFIX?=riscv64-linux-gnu
# Default prefix
else
    PREFIX?=riscv64-elf
endif

# Fedora places newlib in a different location
ifneq ($(wildcard /etc/fedora-release),)
	NEWLIB?=/usr/arm-none-eabi/include
else
	NEWLIB?=/usr/include/newlib
endif


TARGET_MCU?=CH32V003
TARGET_EXT?=c
TARGET=firmware

CH32V003FUN?=ch32v003fun
MINICHLINK?=minichlink

WRITE_SECTION?=flash
SYSTEM_C?=src/framework/ch32v003fun.c

CFLAGS:=$(CFLAGS) -g -Os -flto -ffunction-sections -fdata-sections -fmessage-length=0 -msmall-data-limit=8 -Isrc/include/ -Isrc/framework/include/

CFLAGS_ARCH+=-march=rv32ec -mabi=ilp32e -DCH32V003=1
GENERATED_LD_FILE?=src/framework/generated_ch32v003.ld
TARGET_MCU_LD:=0
LINKER_SCRIPT?=$(GENERATED_LD_FILE)
LDFLAGS+=-L$(CH32V003FUN)/../misc -lgcc

CFLAGS+= \
	$(CFLAGS_ARCH) -static-libgcc \
	-I$(NEWLIB) \
	-I$(CH32V003FUN)/../extralibs \
	-I$(CH32V003FUN) \
	-nostdlib \
	-I. -Wall $(EXTRA_CFLAGS)

LDFLAGS+=-T $(LINKER_SCRIPT) -Wl,--gc-sections
FILES_TO_COMPILE:=$(SYSTEM_C) $(SRCS)

all: firmware.elf

closechlink :
	-killall minichlink

terminal : monitor

FLASH_COMMAND?=$(MINICHLINK) -c /dev/ttyACM0 -w $< $(WRITE_SECTION) -b

$(GENERATED_LD_FILE) :
	$(PREFIX)-gcc -E -P -x c -DTARGET_MCU=$(TARGET_MCU) -DMCU_PACKAGE=$(MCU_PACKAGE) -DTARGET_MCU_LD=$(TARGET_MCU_LD) src/framework/ch32v003fun.ld > $(GENERATED_LD_FILE)

$(TARGET).elf : $(FILES_TO_COMPILE) $(LINKER_SCRIPT) $(EXTRA_ELF_DEPENDENCIES)
	$(PREFIX)-gcc -o $@ $(FILES_TO_COMPILE) $(CFLAGS) $(LDFLAGS)

$(TARGET).bin : $(TARGET).elf
	$(PREFIX)-size $^
	$(PREFIX)-objdump -S $^ > $(TARGET).lst
	$(PREFIX)-objdump -t $^ > $(TARGET).map
	$(PREFIX)-objcopy -O binary $< $(TARGET).bin
	$(PREFIX)-objcopy -O ihex $< $(TARGET).hex

flash : $(TARGET).bin
	$(FLASH_COMMAND)

clean :
	rm -rf $(TARGET).elf $(TARGET).bin $(TARGET).hex $(TARGET).lst $(TARGET).map $(TARGET).hex || true

erase :
	$(MINICHLINK) -p

build : $(TARGET).bin
