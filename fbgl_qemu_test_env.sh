#!/bin/bash

# Variables
TINYCORE_URL="http://tinycorelinux.net/14.x/x86_64/release/CorePure64-current.iso"  # 64-bit TinyCore Linux ISO
ISO_FILE="CorePure64.iso"               # TinyCore Linux ISO file
DISK_IMAGE="tinycore_disk.img"          # Writable disk image for TinyCore
DISK_SIZE="64M"                         # Size of the writable disk
MOUNT_DIR="mnt_tinycore"                # Temporary mount point for ISO
KERNEL_FILE="vmlinuz64"                 # Kernel extracted from ISO
INITRD_FILE="core.gz"                   # Initrd extracted from ISO
BINARY="$1"                             # Precompiled binary passed as argument

# Check if a binary is provided
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

# Step 1: Download 64-bit TinyCore Linux ISO if not already present
if [ ! -f "$ISO_FILE" ]; then
    echo "Downloading 64-bit TinyCore Linux ISO..."
    wget -O $ISO_FILE $TINYCORE_URL
fi

# Step 2: Extract Kernel and Initrd from ISO
if [ ! -f "$KERNEL_FILE" ] || [ ! -f "$INITRD_FILE" ]; then
    echo "Extracting kernel and initrd from TinyCore ISO..."
    mkdir -p $MOUNT_DIR
    sudo mount -o loop $ISO_FILE $MOUNT_DIR
    cp $MOUNT_DIR/boot/vmlinuz64 $KERNEL_FILE
    cp $MOUNT_DIR/boot/corepure64.gz $INITRD_FILE
    sudo umount $MOUNT_DIR
    rmdir $MOUNT_DIR
fi

# Step 3: Create a writable disk image
echo "Creating writable disk image..."
dd if=/dev/zero of=$DISK_IMAGE bs=1M count=${DISK_SIZE/M/} status=progress
mkfs.ext4 $DISK_IMAGE

# Step 4: Mount the disk image and add the binary
echo "Mounting disk image and adding binary..."
mkdir -p $MOUNT_DIR
sudo mount -o loop $DISK_IMAGE $MOUNT_DIR
sudo cp "$BINARY" $MOUNT_DIR/
sudo umount $MOUNT_DIR
rmdir $MOUNT_DIR

# Step 5: Run TinyCore Linux in QEMU with extracted kernel and initrd
echo "Running TinyCore Linux in terminal mode with QEMU..."
qemu-system-x86_64 \
    -kernel $KERNEL_FILE \
    -initrd $INITRD_FILE \
    -append "loglevel=3 tce=sda1 init=/init" \
    -hda $DISK_IMAGE \
    -m 512M \
    -nographic

echo "QEMU exited. Check above for errors or results."
