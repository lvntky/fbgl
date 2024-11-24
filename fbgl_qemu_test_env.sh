#!/bin/bash

# Configuration
KERNEL_VERSION="6.1.1"
BUSYBOX_VERSION="1.36.1"
WORKDIR="$(pwd)/fbgl-test-env"
KERNEL_URL="https://cdn.kernel.org/pub/linux/kernel/v6.x/linux-${KERNEL_VERSION}.tar.xz"
BUSYBOX_URL="https://busybox.net/downloads/busybox-${BUSYBOX_VERSION}.tar.bz2"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m' # No Color

# Error handling
set -e
trap 'echo -e "${RED}Error: Script failed${NC}" >&2' ERR

echo -e "${GREEN}Setting up FBGL test environment...${NC}"

# Create working directory
mkdir -p "${WORKDIR}"
cd "${WORKDIR}"

# Function to check if a package is installed
check_package() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed${NC}"
        exit 1
    fi
}

# Check required packages
required_packages=(wget make gcc g++ qemu-system-x86_64 cpio gzip bc flex bison libelf-dev)
for package in "${required_packages[@]}"; do
    check_package $package
done

# Download and extract kernel
if [ ! -d "linux-${KERNEL_VERSION}" ]; then
    echo "Downloading Linux kernel..."
    wget -q --show-progress "${KERNEL_URL}"
    tar xf "linux-${KERNEL_VERSION}.tar.xz"
    rm "linux-${KERNEL_VERSION}.tar.xz"
fi

# Configure and build kernel
cd "linux-${KERNEL_VERSION}"
if [ ! -f ".config" ]; then
    echo "Configuring kernel..."
    make defconfig
    # Enable framebuffer support
    cat >> .config << EOF
CONFIG_FB=y
CONFIG_FB_VESA=y
CONFIG_FB_EFI=y
CONFIG_FB_SIMPLE=y
CONFIG_FRAMEBUFFER_CONSOLE=y
EOF
fi

echo "Building kernel..."
make -j$(nproc) bzImage

cd "${WORKDIR}"

# Download and build Busybox
if [ ! -d "busybox-${BUSYBOX_VERSION}" ]; then
    echo "Downloading Busybox..."
    wget -q --show-progress "${BUSYBOX_URL}"
    tar xf "busybox-${BUSYBOX_VERSION}.tar.bz2"
    rm "busybox-${BUSYBOX_VERSION}.tar.bz2"
fi

cd "busybox-${BUSYBOX_VERSION}"
if [ ! -f ".config" ]; then
    echo "Configuring Busybox..."
    make defconfig
    # Build statically
    sed -i 's/# CONFIG_STATIC is not set/CONFIG_STATIC=y/' .config
fi

echo "Building Busybox..."
make -j$(nproc)
make install

# Create initial ramdisk
cd "${WORKDIR}"
mkdir -p initramfs
cd initramfs
mkdir -p {bin,sbin,etc,proc,sys,usr/{bin,sbin},dev}

# Copy Busybox
cp -a ../busybox-${BUSYBOX_VERSION}/_install/* .

# Create init script
cat > init << 'EOF'
#!/bin/sh
mount -t proc none /proc
mount -t sysfs none /sys
mount -t devtmpfs none /dev

# Create framebuffer device
mknod /dev/fb0 c 29 0

# Set up environment
export PATH=/bin:/sbin:/usr/bin:/usr/sbin

# Create test directory
mkdir -p /test
cd /test

# Your FBGL test program will be copied here
# Wait for it to be copied and then execute it

echo "Ready for FBGL testing!"
exec /bin/sh
EOF

chmod +x init

# Create initial ramdisk
find . | cpio -H newc -o | gzip > "${WORKDIR}/initramfs.cpio.gz"

# Create test script
cd "${WORKDIR}"
cat > run-test.sh << 'EOF'
#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 <fbgl-test-program>"
    exit 1
fi

TEST_PROGRAM="$1"
KERNEL="linux-6.1.1/arch/x86/boot/bzImage"
INITRD="initramfs.cpio.gz"

if [ ! -f "$TEST_PROGRAM" ]; then
    echo "Error: Test program not found: $TEST_PROGRAM"
    exit 1
fi

qemu-system-x86_64 \
    -kernel "$KERNEL" \
    -initrd "$INITRD" \
    -append "console=ttyS0 root=/dev/ram0 rw init=/init" \
    -nographic \
    -enable-kvm \
    -device virtio-vga \
    -cpu host \
    -m 1G

EOF

chmod +x run-test.sh

echo -e "${GREEN}FBGL test environment setup complete!${NC}"
echo "To run a test program:"
echo "1. Build your FBGL program"
echo "2. Run: ./run-test.sh <path-to-your-program>"