.PHONY: build src mnt gnu-efi keys ovmf shim

DEVICE ?= /dev/sda
DEVICE_PART ?= $(DEVICE)1

CC = gcc
LD = ld
OCP = objcopy

CFLAGS =\
-Wall \
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

OCPFLAGS =-j .text \
-j .sdata \
-j .data \
-j .rodata \
-j .dynamic \
-j .dynsym  \
-j .rel \
-j .rela \
-j .rel.* -j .rela.* -j .reloc \
--target efi-app-x86_64 \
--subsystem=10




clean:
	sudo rm -rf build/*
	sudo rm -rf usbroot/*

fixdisk:
	#GPT table and create partition
	echo -e "g\nY\nn\n\n\n\nY\nt\n1\nw\n" | sudo fdisk $(DEVICE)
	sleep 3

	#format to FAT32 EFI System
	sudo mkfs.vfat -F 32 -n "EFI System" -s 16 -v $(DEVICE_PART)

build:
	mkdir -p usbroot/EFI/BOOT/

	$(CC) $(CFLAGS) -c src/main.c -o build/main.o
	$(LD) $(LDFLAGS) build/main.o -o build/main.so  $(LDFLAGS_L)

	$(OCP) $(OCPFLAGS) build/main.so build/main.efi

	cp build/main.efi usbroot/EFI/BOOT/BOOTX64.EFI

cp:
	lsblk

	sudo mount $(DEVICE_PART) mnt/
	sudo rm -rf mnt/*
	sudo cp -r usbroot/* mnt/

	sudo tree mnt/
	sudo umount $(DEVICE_PART)

	sudo sync

qemu:
	sudo qemu-system-x86_64 \
	-cpu host \
	-enable-kvm \
	-m 8192 \
	-net none \
	-drive if=pflash,format=raw,unit=0,file=ovmf/OVMF_CODE-pure-efi.fd,readonly=on \
	-drive if=pflash,format=raw,unit=1,file=ovmf/OVMF_VARS-pure-efi.fd \
	-usb -device usb-storage,drive=nuckusb \
    -drive file=$(DEVICE),if=none,format=raw,id=nuckusb




