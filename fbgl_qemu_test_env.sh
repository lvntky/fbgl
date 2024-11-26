#!/bin/bash

# Variables
TINYCORE_URL="http://tinycorelinux.net/14.x/x86/release/TinyCore-current.iso"  # TinyCore Linux ISO URL
ISO_FILE="TinyCore.iso"                 # Local TinyCore ISO file
DISK_IMAGE="tinycore_test.img"          # Disk image for TinyCore
DISK_SIZE="128M"                        # Size of the disk image
MOUNT_DIR="mnt_tinycore"                # Temporary mount point for the ISO
KERNEL_FILE="vmlinuz"                   # Extracted kernel file
INITRD_FILE="core.gz"                   # Extracted initrd file
BINARY="$1"                             # Precompiled binary passed as argument

# Ensure a binary is provided as an argument
if [ -z "$BINARY" ]; then
    echo "Usage: $0 <path_to_precompiled_binary>"
    exit 1
fi

# Ensure the binary exists
if [ ! -f "$BINARY" ]; then
    echo "Error: Binary file '$BINARY' not found."
    exit 1
fi

# Ensure required tools are installed
REQUIRED_TOOLS=("wget" "qemu-system-x86_64" "dd" "mkfs.ext4" "mount" "losetup")
for tool in "${REQUIRED_TOOLS[@]}"; do
    if ! command -v $tool &>/dev/null; then
        echo "Error: $tool is required but not installed. Please install it first."
        exit 1
    fi
done

# Step 1: Download TinyCore Linux ISO
if [ ! -f "$ISO_FILE" ]; then
    echo "Downloading TinyCore Linux ISO..."
    wget -O $ISO_FILE $TINYCORE_URL
fi

# Step 2: Extract Kernel and Initrd from ISO
if [ ! -f "$KERNEL_FILE" ] || [ ! -f "$INITRD_FILE" ]; then
    echo "Extracting kernel and initrd from TinyCore ISO..."
    mkdir -p $MOUNT_DIR
    sudo mount -o loop $ISO_FILE $MOUNT_DIR
    cp $MOUNT_DIR/boot/vmlinuz $KERNEL_FILE
    cp $MOUNT_DIR/boot/core.gz $INITRD_FILE
    sudo umount $MOUNT_DIR
    rmdir $MOUNT_DIR
fi

# Step 3: Create a Disk Image
echo "Creating disk image..."
dd if=/dev/zero of=$DISK_IMAGE bs=1M count=${DISK_SIZE/M/} status=progress
mkfs.ext4 $DISK_IMAGE

# Step 4: Mount and Prepare Disk Image
echo "Mounting disk image and adding binary..."
mkdir -p $MOUNT_DIR
sudo mount -o loop $DISK_IMAGE $MOUNT_DIR

# Add precompiled binary to the disk
sudo mkdir -p $MOUNT_DIR/tce
sudo cp "$BINARY" $MOUNT_DIR/
sudo umount $MOUNT_DIR
rmdir $MOUNT_DIR

# Step 5: Boot TinyCore Linux in QEMU
echo "Booting TinyCore Linux with QEMU..."
qemu-system-x86_64 \
    -kernel $KERNEL_FILE \
    -initrd $INITRD_FILE \
    -append "loglevel=3 tce=sda1 init=/init" \
    -hda $DISK_IMAGE \
    -m 512M \
    -vga std \
    -nographic

echo "QEMU exited. Check above for errors or results."
