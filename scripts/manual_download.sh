#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

EF="${EF:-ef}"
OPENOCD="${OPENOCD:-$HOME/.local/openocd-git/bin/openocd}"
OPENOCD_SCRIPTS="${OPENOCD_SCRIPTS:-$HOME/.local/openocd-git/share/openocd/scripts}"
TARGET_CFG="${TARGET_CFG:-target/ti/mspm0.cfg}"
ADAPTER="${ADAPTER:-cmsis-dap}"
SPEED="${SPEED:-4000}"
ELF="${ELF:-build/app.elf}"
HEX="${HEX:-build/app.hex}"
DAPLINK="${DAPLINK:-/media/$USER/DAPLINK}"
SERIAL="${SERIAL:-/dev/ttyACM0}"
SERIAL_TIMEOUT="${SERIAL_TIMEOUT:-5s}"
TRIES="${TRIES:-20}"

cd "$ROOT_DIR"

usage() {
    cat <<EOF
Usage:
  ./scripts/manual_download.sh <command>

Commands:
  status       Show probe, serial, DAPLink, and FAIL.TXT state.
  build        Build firmware with EmbedForge.
  dry-run      Print the resolved OpenOCD flash command.
  flash        Flash with OpenOCD once, then run ef reset automatically.
  flash-loop   Retry OpenOCD flash; reset automatically after success.
  daplink      Copy build/app.hex to the DAPLink disk and sync.
  serial       Read UART logs from ${SERIAL} at 115200.
  serial-follow Keep reading UART logs until Ctrl+C.
  commands     Print raw commands you can copy and run manually.

Common overrides:
  SPEED=4000 ./scripts/manual_download.sh flash
  SPEED=8000 ./scripts/manual_download.sh flash
  TRIES=60 ./scripts/manual_download.sh flash-loop
  SERIAL=/dev/ttyACM1 ./scripts/manual_download.sh serial
  DAPLINK=/media/\$USER/DAPLINK ./scripts/manual_download.sh daplink

This script never runs sudo, chmod, usermod, or udev setup.
EOF
}

require_file() {
    if [[ ! -e "$1" ]]; then
        echo "missing: $1" >&2
        exit 1
    fi
}

require_exe() {
    if [[ ! -x "$1" ]]; then
        echo "not executable: $1" >&2
        exit 1
    fi
}

show_config() {
    cat <<EOF
Project:         ${ROOT_DIR}
EF:              ${EF}
OpenOCD:         ${OPENOCD}
OpenOCD scripts: ${OPENOCD_SCRIPTS}
Target cfg:      ${TARGET_CFG}
Adapter:         ${ADAPTER}
Speed:           ${SPEED} kHz
ELF:             ${ELF}
HEX:             ${HEX}
DAPLink disk:    ${DAPLINK}
Serial:          ${SERIAL}
EOF
}

openocd_flash_args=(
    --adapter "$ADAPTER"
    --file "$ELF"
    --openocd "$OPENOCD"
    --scripts-dir "$OPENOCD_SCRIPTS"
    --target-cfg "$TARGET_CFG"
    --speed "$SPEED"
    --verbose
)

cmd_status() {
    show_config
    echo
    echo "USB:"
    lsusb | grep -Ei '0d28:0204|cmsis|dap|mbed' || true
    echo
    echo "Device nodes:"
    ls -l /dev/ttyACM* /dev/ttyUSB* /dev/hidraw* 2>/dev/null || true
    echo
    echo "DAPLink disk:"
    if [[ -d "$DAPLINK" ]]; then
        find "$DAPLINK" -maxdepth 1 -type f -printf '%f %s bytes\n' | sort
        if [[ -f "$DAPLINK/FAIL.TXT" ]]; then
            echo
            echo "FAIL.TXT:"
            sed -n '1,80p' "$DAPLINK/FAIL.TXT"
        fi
    else
        echo "not mounted: $DAPLINK"
    fi
}

cmd_build() {
    require_exe "$EF"
    "$EF" build --target cmake-arm
}

cmd_dry_run() {
    require_exe "$EF"
    require_exe "$OPENOCD"
    require_file "$ELF"
    "$EF" flash "${openocd_flash_args[@]}" --dry-run
}

cmd_flash() {
    require_exe "$EF"
    require_exe "$OPENOCD"
    require_file "$ELF"
    echo "If OpenOCD reports 'cannot read IDR', press RESET and retry."
    "$EF" flash "${openocd_flash_args[@]}"
    "$EF" reset --adapter "$ADAPTER"
}

cmd_flash_loop() {
    require_exe "$EF"
    require_exe "$OPENOCD"
    require_file "$ELF"
    echo "Retrying ${TRIES} times. Press/release RESET while this runs."
    for i in $(seq 1 "$TRIES"); do
        echo "try ${i}/${TRIES}"
        if "$EF" flash "${openocd_flash_args[@]}"; then
            "$EF" reset --adapter "$ADAPTER"
            exit 0
        fi
        sleep 0.5
    done
    exit 1
}

cmd_daplink() {
    require_file "$HEX"
    if [[ ! -d "$DAPLINK" ]]; then
        echo "DAPLink disk is not mounted: $DAPLINK" >&2
        exit 1
    fi
    rm -f "$DAPLINK/FAIL.TXT" 2>/dev/null || true
    cp "$HEX" "$DAPLINK/app.hex"
    sync
    echo "copied: $HEX -> $DAPLINK/app.hex"
    if [[ -f "$DAPLINK/FAIL.TXT" ]]; then
        echo
        echo "DAPLink reported failure:"
        sed -n '1,80p' "$DAPLINK/FAIL.TXT"
        exit 1
    fi
}

cmd_serial() {
    require_file "$SERIAL"
    stty -F "$SERIAL" 115200 cs8 -cstopb -parenb -ixon -ixoff -crtscts raw -echo
    timeout "$SERIAL_TIMEOUT" cat "$SERIAL"
}

cmd_serial_follow() {
    require_file "$SERIAL"
    stty -F "$SERIAL" 115200 cs8 -cstopb -parenb -ixon -ixoff -crtscts raw -echo
    cat "$SERIAL"
}

cmd_commands() {
    cat <<EOF
cd ${ROOT_DIR}

${EF} build --target cmake-arm

${EF} flash \\
  --adapter ${ADAPTER} \\
  --file ${ELF} \\
  --openocd ${OPENOCD} \\
  --scripts-dir ${OPENOCD_SCRIPTS} \\
  --target-cfg ${TARGET_CFG} \\
  --speed ${SPEED} \\
  --verbose

${EF} reset --adapter ${ADAPTER}

cp ${HEX} ${DAPLINK}/app.hex
sync

stty -F ${SERIAL} 115200 cs8 -cstopb -parenb -ixon -ixoff -crtscts raw -echo
timeout ${SERIAL_TIMEOUT} cat ${SERIAL}

stty -F ${SERIAL} 115200 cs8 -cstopb -parenb -ixon -ixoff -crtscts raw -echo
cat ${SERIAL}
EOF
}

case "${1:-help}" in
    help|-h|--help)
        usage
        ;;
    status)
        cmd_status
        ;;
    build)
        cmd_build
        ;;
    dry-run)
        cmd_dry_run
        ;;
    flash)
        cmd_flash
        ;;
    flash-loop)
        cmd_flash_loop
        ;;
    daplink)
        cmd_daplink
        ;;
    serial)
        cmd_serial
        ;;
    serial-follow)
        cmd_serial_follow
        ;;
    commands)
        cmd_commands
        ;;
    *)
        usage >&2
        exit 2
        ;;
esac
