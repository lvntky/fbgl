#!/bin/bash

# ----------- CONFIGURATION -----------
XVFB_DISPLAY_NUM=99
XVFB_RESOLUTION="1280x800x24"
XVFB_DISPLAY=":$XVFB_DISPLAY_NUM"
VNC_PORT=5900
LOG_DIR="./logs"
APP_COMMAND="./fbgl_app"
WM_COMMAND="fluxbox"
# -------------------------------------

mkdir -p "$LOG_DIR"

XVFB_LOG="$LOG_DIR/xvfb.log"
VNC_LOG="$LOG_DIR/x11vnc.log"
APP_LOG="$LOG_DIR/app.log"

function check_dependencies {
    echo "[*] Checking dependencies..."
    for cmd in Xvfb x11vnc $WM_COMMAND; do
        if ! command -v "$cmd" &> /dev/null; then
            echo "[!] Missing dependency: $cmd"
            exit 1
        fi
    done
}

function cleanup {
    echo "[*] Cleaning up..."
    pkill -f "Xvfb $XVFB_DISPLAY"
    pkill -f "x11vnc.*$XVFB_DISPLAY"
    pkill -f "$WM_COMMAND"
    echo "[*] Done."
}

function start_xvfb {
    echo "[*] Starting Xvfb on display $XVFB_DISPLAY..."
    Xvfb "$XVFB_DISPLAY" -screen 0 "$XVFB_RESOLUTION" > "$XVFB_LOG" 2>&1 &
    sleep 1
}

function start_window_manager {
    echo "[*] Starting window manager: $WM_COMMAND..."
    DISPLAY="$XVFB_DISPLAY" $WM_COMMAND > /dev/null 2>&1 &
    sleep 1
}

function start_vnc {
    echo "[*] Starting x11vnc on port $VNC_PORT..."
    x11vnc -display "$XVFB_DISPLAY" -nopw -forever -bg -quiet -rfbport "$VNC_PORT" > "$VNC_LOG" 2>&1
}

function run_app {
    echo "[*] Running GUI application: $APP_COMMAND"
    DISPLAY="$XVFB_DISPLAY" $APP_COMMAND > "$APP_LOG" 2>&1
}

# ------- Main Execution -------
trap cleanup EXIT
check_dependencies
cleanup
start_xvfb
start_window_manager
start_vnc
run_app
