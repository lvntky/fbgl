#!/bin/bash

# Variables
BUSYBOX_VERSION="1.36.0"               # BusyBox version to download
PRECOMPILED_KERNEL="https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-6.5.9.tar.xz"
BUSYBOX_DIR="busybox_root"             # Directory for BusyBox root filesystem
BINARY="fbgl_test"                     # Precompiled binary
BUSYBOX_IMAGE="busybox.img"            # Image file for BusyBox environment
TARGET_DIR="/mnt/root"                 # Mount point for the BusyBox image
KERNEL_FILE="vmlinuz"                  # Precompiled kernel file

# Ensure dependencies are installed
REQUIRED_TOOLS=("wget" "qemu-system-x86_64" "dd" "mkfs.ext4" "mount" "losetup")
for tool in "${REQUIRED_TOOLS[@]}"; do
    if ! command -v $tool &>/dev/null; then
        echo "Error: $tool is required but not installed. Please install it first."
        exit 1
    fi
done

# Step 1: Download and Build BusyBox
echo "Setting up BusyBox environment..."
if [ ! -d "busybox-${BUSYBOX_VERSION}" ]; then
    wget https://busybox.net/downloads/busybox-${BUSYBOX_VERSION}.tar.bz2
    tar -xf busybox-${BUSYBOX_VERSION}.tar.bz2
fi
cd busybox-${BUSYBOX_VERSION}
make defconfig
sed -i 's/# CONFIG_STATIC/CONFIG_STATIC=y/' .config  # Enable static binary
make -j$(nproc) install CONFIG_PREFIX=../$BUSYBOX_DIR
cd ..

# Step 2: Set Up BusyBox Root Filesystem
echo "Creating BusyBox root filesystem..."
mkdir -p $BUSYBOX_DIR/{dev,proc,sys,root}
sudo mknod -m 666 $BUSYBOX_DIR/dev/console c 5 1
sudo mknod -m 666 $BUSYBOX_DIR/dev/null c 1 3
echo "#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
exec /bin/sh" > $BUSYBOX_DIR/init
chmod +x $BUSYBOX_DIR/init

# Step 3: Create and Format Disk Image
echo "Creating and formatting disk image..."
dd if=/dev/zero of=$BUSYBOX_IMAGE bs=1M count=64
mkfs.ext4 $BUSYBOX_IMAGE
sudo mount -o loop $BUSYBOX_IMAGE $TARGET_DIR
sudo cp -r $BUSYBOX_DIR/* $TARGET_DIR/
sudo umount $TARGET_DIR

# Step 4: Download Precompiled Kernel
if [ ! -f "$KERNEL_FILE" ]; then
    echo "Downloading precompiled kernel..."
    wget -O $KERNEL_FILE $PRECOMPILED_KERNEL
fi

# Step 5: Transfer Precompiled Binary to BusyBox
echo "Transferring binary to BusyBox..."
sudo mount -o loop $BUSYBOX_IMAGE $TARGET_DIR
sudo cp $BINARY $TARGET_DIR/bin/
sudo umount $TARGET_DIR

# Step 6: Boot with QEMU
echo "Booting BusyBox with QEMU..."
qemu-system-x86_64 \
    -kernel $KERNEL_FILE \
    -append "root=/dev/sda console=ttyS0 init=/init" \
    -hda $BUSYBOX_IMAGE \
    -nographic
