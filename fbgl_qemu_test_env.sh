#!/bin/bash

# Variables
TINYCORE_URL="http://tinycorelinux.net/14.x/x86/release/TinyCore-current.iso"  # TinyCore Linux ISO URL
ISO_FILE="TinyCore.iso"                 # TinyCore Linux ISO file
DISK_IMAGE="tinycore_disk.img"          # Writable disk image for TinyCore
DISK_SIZE="64M"                         # Size of the writable disk
MOUNT_DIR="mnt_tinycore"                # Temporary mount point for disk
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

# Step 1: Download TinyCore Linux ISO if not already present
if [ ! -f "$ISO_FILE" ]; then
    echo "Downloading TinyCore Linux ISO..."
    wget -O $ISO_FILE $TINYCORE_URL
fi

# Step 2: Create a writable disk image
echo "Creating writable disk image..."
dd if=/dev/zero of=$DISK_IMAGE bs=1M count=${DISK_SIZE/M/} status=progress
mkfs.ext4 $DISK_IMAGE

# Step 3: Mount the disk image and add the binary
echo "Mounting disk image and adding binary..."
mkdir -p $MOUNT_DIR
sudo mount -o loop $DISK_IMAGE $MOUNT_DIR
sudo cp "$BINARY" $MOUNT_DIR/
sudo umount $MOUNT_DIR
rmdir $MOUNT_DIR

# Step 4: Run TinyCore Linux in QEMU
echo "Running TinyCore Linux in QEMU..."
qemu-system-x86_64 \
    -cdrom $ISO_FILE \
    -hda $DISK_IMAGE \
    -m 512M \
    -vga std \
    -display gtk \
    -boot d

echo "QEMU exited. Check above for errors or results."
