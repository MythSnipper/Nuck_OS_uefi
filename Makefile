.PHONY: build src mnt gnu-efi keys ovmf shim


DEVICE ?= /dev/null

part_prefix ?= p
EFI_PART = $(DEVICE)$(part_prefix)1
OS_PART = $(DEVICE)$(part_prefix)2

#uefi files for qemu
QEMU_UEFI_CODE = OVMF_CODE.fd
QEMU_UEFI_VARS = OVMF_VARS.fd

#tools used
CC = gcc
LD = ld
OCP = objcopy
KERNEL_CC = x86_64-elf-gcc
KERNEL_LD = x86_64-elf-ld
KERNEL_OCP = x86_64-elf-objcopy
AS = nasm

CFLAGS =\
-Wall \
-Wextra \
-Wno-unused-parameter \
-Wno-unused-variable \
-Ignu-efi/inc \
-fpic \
-ffreestanding \
-fno-stack-protector \
-fno-stack-check \
-fshort-wchar \
-mno-red-zone \
-maccumulate-outgoing-args

LDFLAGS =\
-shared \
-Bsymbolic \
-Lgnu-efi/x86_64/lib \
-Lgnu-efi/x86_64/gnuefi \
-Tgnu-efi/gnuefi/elf_x86_64_efi.lds \
gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o

LDFLAGS_L =\
-lgnuefi \
-lefi

OCPFLAGS =\
-j .text \
-j .sdata \
-j .data \
-j .rodata \
-j .dynamic \
-j .dynsym \
-j .rel \
-j .rela \
-j .rel.* \
-j .rela.* \
-j .reloc \
--output-target efi-app-x86_64 \
--subsystem=10

KERNEL_CFLAGS =\
-Wall \
-Wextra \
-Wframe-larger-than=1024 \
-Wno-unused-parameter \
-Wno-unused-variable \
-ffreestanding \
-fno-stack-protector \
-fPIE \
-g \
-m64 \
-mno-red-zone \
-Ignu-efi/inc \
-O0

KERNEL_LDFLAGS =\
-T src/kernel.ld \
-nostdlib \

KERNEL_OCPFLAGS =\
-O binary

KERNEL_OBJS =\
build/kernel_entry.o \
build/kernel.o \
build/isr_asm.o \
build/isr.o \
build/idt.o \
build/ps2.o

debug: givesudo clean build copy-to-usbroot copy-to-device qemu-refresh qemu
run: givesudo clean build copy-to-usbroot copy-to-device qemu-refresh qemu-kvm

givesudo:
	sudo echo vel

clean:
	rm -rf build/*
	rm -rf usbroot/*

# BUILDING ------------------------------------------------------------

#build code into build directory
build:
	mkdir -p usbroot/EFI/BOOT/

	#bootloader
	$(CC) $(CFLAGS) -c src/nuckboot.c -o build/nuckboot.o
	$(LD) $(LDFLAGS) build/nuckboot.o -o build/nuckboot.so  $(LDFLAGS_L)
	$(OCP) $(OCPFLAGS) build/nuckboot.so build/nuckboot.efi

	#os
	nasm -f elf64 src/entry.asm -o build/kernel_entry.o
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c src/kernel.c -o build/kernel.o

	nasm -f elf64 src/isr.asm -o build/isr_asm.o
	$(KERNEL_CC) $(KERNEL_CFLAGS) -mgeneral-regs-only -c src/isr.c -o build/isr.o

	$(KERNEL_CC) $(KERNEL_CFLAGS) -c src/idt.c -o build/idt.o
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c src/ps2.c -o build/ps2.o



	#link kernel as ELF64 object
	$(KERNEL_LD) $(KERNEL_LDFLAGS) \
	-o build/kernel-full.o \
	$(KERNEL_OBJS)

	#objcopy to flat binary
	$(KERNEL_OCP) $(KERNEL_OCPFLAGS) build/kernel-full.o build/kernel-full.bin

# ------------------------------------------------------------

#makes a ram storage device at /dev/ram0
#ramdisk_size = 2097152
ramdisk_size = 1048576 #1 GiB
ramdisk_efipart_size = 500M
ramdisk:
	sudo modprobe brd rd_nr=1 rd_size=$(ramdisk_size)
	lsblk /dev/ram0
	echo -e "g\nn\n\n\n+$(ramdisk_efipart_size)\nt\n1\nn\n\n\n\nw\n" | sudo fdisk /dev/ram0
	sudo mkfs.vfat -F 32 -n NUCKBOOT /dev/ram0p1
	sudo mkfs.ext2 -b 4096 -L NUCKOS /dev/ram0p2

#destroy the ramdisk
uramdisk:
	sudo modprobe -r brd

#copy code and data to usbroot
copy-to-usbroot:
	cp build/nuckboot.efi usbroot/EFI/BOOT/BOOTX64.EFI
	cp gnu-efi/Shell_Full.efi usbroot/EFI/BOOT/SHELLX64.EFI
	cp build/kernel-full.bin usbroot/kernel.bin
	cp data/out/*.nvideo usbroot/

#copy files to device
copy-to-device:
	lsblk

	sudo mount $(EFI_PART) mnt/
	sudo rm -rf mnt/*
	sudo cp -r usbroot/* mnt/
	sudo tree mnt/
	sudo sync
	sudo umount $(EFI_PART)

#copy iso image to usb
copyimg:
	lsblk
	sudo dd if=build/nuck_os.iso of=$(DEVICE) bs=1MiB status=progress
	sudo sync

# TESTING ------------------------------------------------------------

#test in qemu(fast)
qemu-kvm:
	sudo sync
	sudo -E qemu-system-x86_64 \
	-cpu host \
	-enable-kvm \
	-m 8192 \
	-net none \
	-drive if=pflash,format=raw,unit=0,file=ovmf/temp/$(QEMU_UEFI_CODE),readonly=on \
	-drive if=pflash,format=raw,unit=1,file=ovmf/temp/$(QEMU_UEFI_VARS) \
	-usb -device usb-storage,drive=nuckusb \
    -drive file=$(DEVICE),if=none,format=raw,id=nuckusb \

#test in qemu(software emulated, pause on start, needs gdb)
qemu:
	sudo sync
	sudo -E qemu-system-x86_64 \
	-accel tcg \
	-m 8192 \
	-net none \
	-drive if=pflash,format=raw,unit=0,file=ovmf/temp/$(QEMU_UEFI_CODE),readonly=on \
	-drive if=pflash,format=raw,unit=1,file=ovmf/temp/$(QEMU_UEFI_VARS) \
	-usb -device usb-storage,drive=nuckusb \
    -drive file=$(DEVICE),if=none,format=raw,id=nuckusb \
	-s -S \

#refresh UEFI firmware files used for qemu
qemu-refresh:
	rm ovmf/temp/$(QEMU_UEFI_CODE) ovmf/temp/$(QEMU_UEFI_VARS)
	cp ovmf/$(QEMU_UEFI_CODE) ovmf/temp/
	cp ovmf/$(QEMU_UEFI_VARS) ovmf/temp/

# MEDIA ------------------------------------------------------------

convert-source:
	rm -rf data/pre-convert/*

	#bad apple 6572 frames
	mkdir -p data/pre-convert/bad-apple
	ls data/source/bad-apple/*.jpg | parallel --no-notice -j $$(nproc) ' \
		filename=$$(basename {} .jpg); \
		ffmpeg -loglevel panic -i {} data/pre-convert/bad-apple/$$filename.bmp'

	#nuckos logo
	mkdir -p data/pre-convert/logo
	ffmpeg -loglevel panic -i data/source/logo.jpg -vf scale=100:100 data/pre-convert/logo/logo.bmp

	#pointer
	mkdir -p data/pre-convert/pointer
	cp data/source/pointer.png data/pre-convert/pointer/pointer.png

build-nvideo:
	rm -rf data/out/*
	#nuckos logo
	scripts/convert-nvideo data/pre-convert/logo data/out/nuckos_logo.nvideo 1 .bmp

	#bad apple 6572 frames
	scripts/convert-nvideo data/pre-convert/bad-apple data/out/bad_apple.nvideo 0 .bmp

	#pointer
	scripts/convert-nvideo data/pre-convert/pointer data/out/pointer.nvideo 2 .png

