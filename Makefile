.PHONY: build src mnt gnu-efi keys ovmf shim

DEVICE ?= /dev/null
EFI_PART = $(DEVICE)1
EFI_PART_SIZE = 600M#MiB
MAIN_PART = $(DEVICE)2

#iso image building shenanigans
IMAGE_SIZE = 2048#in MiB
IMAGE_NAME = nuck_os.iso

CC = gcc
LD = ld
OCP = objcopy
KERNEL_CC = x86_64-elf-gcc
KERNEL_LD = x86_64-elf-ld
AS = nasm

CFLAGS =\
-Wall \
-Wextra \
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
-j .dynsym  \
-j .rel \
-j .rela \
-j .rel.* -j .rela.* -j .reloc \
--target efi-app-x86_64 \
--subsystem=10



KERNEL_CFLAGS =\
-Wall \
-Wextra \
-ffreestanding \
-fno-stack-protector \
-fpie \
-nostdlib \
-m64 \
-mno-red-zone \
-Ignu-efi/inc \
-O0

KERNEL_LDFLAGS =\
-nostdlib \
-e kernel_main \
--oformat binary


all: clean build copy-usbroot copydisk qemu

clean:
	sudo rm -rf build/*
	sudo rm -rf usbroot/*

#build code into build directory
build:
	mkdir -p usbroot/EFI/BOOT/

	$(CC) $(CFLAGS) -c src/nuckboot.c -o build/nuckboot.o
	$(LD) $(LDFLAGS) build/nuckboot.o -o build/nuckboot.so  $(LDFLAGS_L)

	$(OCP) $(OCPFLAGS) build/nuckboot.so build/nuckboot.efi

	#kernel
	$(KERNEL_CC) $(KERNEL_CFLAGS) -c src/kernel.c -o build/kernel.o

	#link kernel with kernel entry
	$(KERNEL_LD) $(KERNEL_LDFLAGS) \
	build/kernel.o \
	-o build/kernel-full.bin

#copy code and data to usbroot
copy-usbroot:
	cp build/nuckboot.efi usbroot/EFI/BOOT/BOOTX64.EFI
	cp build/kernel-full.bin usbroot/kernel.bin
	cp data/*.nvideo usbroot/

#reconstruct usb partitions and format
disk:
	echo -e "+${EFI_PART_SIZE}"
	#GPT table and create partition
	echo -e "g\nY\nn\n1\n\n+${EFI_PART_SIZE}\nY\nt\n1\nn\n2\n\n\nt\n2\n222\nw\n" | sudo fdisk $(DEVICE)
	sudo sync

	#format to FAT32 EFI System
	sudo mkfs.vfat -F 32 -n "EFI System" -s 16 -v $(EFI_PART)

	#format to ext2
	echo -e "y\n" | sudo mkfs.ext2 $(MAIN_PART)
	sudo sync

#build iso image
img:
	sudo rm -rf build/*.iso
	sudo dd if=/dev/zero of=build/$(IMAGE_NAME) bs=1MiB count=$(IMAGE_SIZE) status=progress

	LOOP_DEV=$$(sudo losetup -f --show build/$(IMAGE_NAME)); \
	echo -e "g\nn\n\n\n+${EFI_PART_SIZE}\nt\n1\nn\n\n\n\nt\n2\n222\nw" | sudo fdisk $$LOOP_DEV; \
	sudo sync; \
	sudo losetup -d $$LOOP_DEV; \
	\
	LOOP_DEV=$$(sudo losetup -f --show -P build/$(IMAGE_NAME)); \
	sudo lsblk; \
	sudo mkfs.vfat -F 32 -n "EFI System" -s 16 -v $${LOOP_DEV}p1; \
	echo -e "y\n" | sudo mkfs.ext2 $${LOOP_DEV}p2; \
	sudo sync; \
	\
	sudo mount $${LOOP_DEV}p1 mnt/; \
	sudo rm -rf mnt/*; \
	sudo cp -r usbroot/* mnt/; \
	sudo tree mnt/; \
	sudo umount $${LOOP_DEV}p1; \
	\
	sudo mount $${LOOP_DEV}p2 mnt/; \
	sudo rm -rf mnt/*; \
	sudo cp -r usbmain/* mnt/; \
	sudo tree mnt/; \
	sudo umount $${LOOP_DEV}p2; \
	\
	sudo losetup -d $$LOOP_DEV

#copy files to usb
copydisk:
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

#test in qemu
qemu:
	sudo sync
	sudo qemu-system-x86_64 \
	-cpu host \
	-enable-kvm \
	-m 8192 \
	-net none \
	-drive if=pflash,format=raw,unit=0,file=ovmf/OVMF_CODE-pure-efi.fd,readonly=on \
	-drive if=pflash,format=raw,unit=1,file=ovmf/OVMF_VARS-pure-efi.fd \
	-usb -device usb-storage,drive=nuckusb \
    -drive file=$(DEVICE),if=none,format=raw,id=nuckusb

#test in qemu(slow)
qemu-slow:
	sudo sync
	sudo qemu-system-x86_64 \
	-icount shift=10,sleep=on \
	-m 8192 \
	-net none \
	-drive if=pflash,format=raw,unit=0,file=ovmf/OVMF_CODE-pure-efi.fd,readonly=on \
	-drive if=pflash,format=raw,unit=1,file=ovmf/OVMF_VARS-pure-efi.fd \
	-usb -device usb-storage,drive=nuckusb \
    -drive file=$(DEVICE),if=none,format=raw,id=nuckusb






IF ?= /dev/null
OF ?= /dev/null

#convert downloaded youtube video "video.mp4" to bmp images
convert-video-yt:
	rm data/frames/*
	ffmpeg -i data/video.mp4 -vf scale=640:360 -r 2 data/frames/frame_%04d.bmp

#convert jpg images to bmp frames
convert-bad-apple-frames:
	rm data/bad-apple-source/bmpframes/*
	for file in data/bad-apple-source/frames-bad-apple/*.jpg; do \
		filename=$$(basename $$file .jpg); \
		ffmpeg -loglevel panic -i $$file data/bad-apple-source/bmpframes/$$filename.bmp; \
	done

#convert bmp frames to black and white bmp video
convert-bad-apple:
	rm -rf data/frames/*
	cp -r data/bad-apple-source/bmpframes/* data/frames/
	./scripts/convert-bad-apple


#opt: ffmpeg -i logo.jpg -vf scale=100:100 logo.bmp
convert-oslogo:
	python scripts/convert-logo.py










