#!/bin/bash

# Variables
TINYCORE_URL="http://tinycorelinux.net/14.x/x86/release/TinyCore-current.iso"  # TinyCore ISO URL
ISO_FILE="TinyCore.iso"                 # Local TinyCore ISO file
DISK_IMAGE="tinycore_test.img"          # Disk image for TinyCore
DISK_SIZE="128M"                        # Size of the disk image
MOUNT_DIR="mnt_tinycore"                # Mount point for the disk image

# Ensure a binary is provided as an argument
if [ -z "$1" ]; then
    echo "Usage: $0 <path_to_precompiled_binary>"
    exit 1
fi
BINARY="$1"

# Ensure the binary exists
if [ ! -f "$BINARY" ]; then
    echo "Error: Precompiled binary '$BINARY' not found."
    exit 1
fi

# Ensure dependencies are installed
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

# Step 2: Create a Disk Image
echo "Creating disk image..."
dd if=/dev/zero of=$DISK_IMAGE bs=1M count=${DISK_SIZE/M/}
mkfs.ext4 $DISK_IMAGE

# Step 3: Mount and Prepare Disk Image
echo "Mounting disk image..."
mkdir -p $MOUNT_DIR
sudo mount -o loop $DISK_IMAGE $MOUNT_DIR

echo "Preparing disk image for TinyCore Linux..."
sudo mkdir -p $MOUNT_DIR/tce
sudo cp $ISO_FILE $MOUNT_DIR/

# Step 4: Add Precompiled Binary
echo "Adding precompiled binary '$BINARY'..."
sudo cp "$BINARY" $MOUNT_DIR/

# Step 5: Unmount Disk Image
echo "Unmounting disk image..."
sudo umount $MOUNT_DIR
rmdir $MOUNT_DIR

# Step 6: Boot TinyCore Linux in QEMU
echo "Booting TinyCore Linux with QEMU..."
qemu-system-x86_64 \
    -cdrom $ISO_FILE \
    -hda $DISK_IMAGE \
    -m 512M \
    -boot d \
    -nographic
