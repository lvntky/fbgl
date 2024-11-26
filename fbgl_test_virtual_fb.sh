#!/bin/bash

# Check if Xvfb is installed
if ! command -v Xvfb &> /dev/null; then
    echo "Xvfb not found. Install it with your package manager (e.g., sudo pacman -S xorg-server-xvfb)."
    exit 1
fi

# Set variables
PRECOMPILED_BINARY="./fbgl_test" # Replace with your precompiled binary's path
DISPLAY_NUMBER=":99"             # Virtual framebuffer display number

# Check if the precompiled binary exists
if [ ! -f "$PRECOMPILED_BINARY" ]; then
    echo "Precompiled binary '$PRECOMPILED_BINARY' not found."
    exit 1
fi

# Start virtual framebuffer
echo "Starting Xvfb on display ${DISPLAY_NUMBER}..."
Xvfb $DISPLAY_NUMBER -screen 0 1024x768x24 &
XVFB_PID=$!

# Wait for Xvfb to start
sleep 2

# Set the DISPLAY environment variable
export DISPLAY=$DISPLAY_NUMBER

# Run the precompiled binary inside the virtual framebuffer
echo "Running $PRECOMPILED_BINARY in the virtual framebuffer environment..."
$PRECOMPILED_BINARY

# Capture the exit code of the binary
BINARY_EXIT_CODE=$?

# Stop Xvfb
echo "Stopping Xvfb..."
kill $XVFB_PID

# Exit with the binary's exit code
exit $BINARY_EXIT_CODE
