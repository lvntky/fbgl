#!/bin/bash

# Variables
BUSYBOX_DIR="busybox_root"   # Directory for BusyBox root filesystem
BINARY="fbgl_test"           # Precompiled binary
TARGET_DIR="/mnt/root"       # Target directory for binary on BusyBox
BUSYBOX_IMAGE="busybox.img"  # Image file for BusyBox environment
LOOP_DEVICE="/dev/loop0"     # Loop device for BusyBox image

# Ensure dependencies are installed
if ! command -v qemu-system-x86_64 &>/dev/null; then
  echo "Please install QEMU to run BusyBox."
  exit 1
fi

if ! command -v losetup &>/dev/null; then
  echo "Please install losetup for loop device setup."
  exit 1
fi

# 1. Create a Minimal BusyBox Environment
echo "Setting up BusyBox environment..."

# Download and build BusyBox
if [ ! -d "busybox" ]; then
  git clone https://git.busybox.net/busybox
fi
cd busybox || exit
make defconfig
sed -i 's/# CONFIG_STATIC/CONFIG_STATIC=y/' .config  # Enable static binary
make -j$(nproc)
cd ..

# Create root filesystem
mkdir -p $BUSYBOX_DIR/{bin,dev,etc,proc,sys,usr}

# Install BusyBox binaries
cp busybox/busybox $BUSYBOX_DIR/bin/
chroot $BUSYBOX_DIR /bin/busybox --install -s

# Add init script
cat <<EOF >$BUSYBOX_DIR/init
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
exec /bin/sh
EOF
chmod +x $BUSYBOX_DIR/init

# Create device nodes
sudo mknod $BUSYBOX_DIR/dev/console c 5 1
sudo mknod $BUSYBOX_DIR/dev/null c 1 3

# 2. Prepare Framebuffer
echo "Setting up framebuffer support..."
sudo mknod $BUSYBOX_DIR/dev/fb0 c 29 0

# 3. Create Image File
echo "Creating BusyBox disk image..."
dd if=/dev/zero of=$BUSYBOX_IMAGE bs=1M count=64
mkfs.ext4 $BUSYBOX_IMAGE
sudo mount -o loop $BUSYBOX_IMAGE $TARGET_DIR
sudo cp -r $BUSYBOX_DIR/* $TARGET_DIR/
sudo umount $TARGET_DIR

# 4. Transfer Precompiled Binary
echo "Transferring binary to BusyBox environment..."
sudo mount -o loop $BUSYBOX_IMAGE $TARGET_DIR
sudo cp $BINARY $TARGET_DIR/bin/
sudo umount $TARGET_DIR

# 5. Run in QEMU
echo "Starting BusyBox in QEMU with framebuffer..."
qemu-system-x86_64 -kernel busybox/arch/x86/boot/bzImage \
  -append "root=/dev/sda console=ttyS0" \
  -hda $BUSYBOX_IMAGE \
  -nographic

echo "BusyBox environment setup complete."
