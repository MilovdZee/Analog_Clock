#!/bin/bash

# Configuration targeting the WEMOS D1 Mini Clone
PORT=/dev/ttyUSB0
BOARD_FQBN="esp8266:esp8266:d1_mini:eesz=4M3M"

# Parse optional flags
NO_WRITE=false
for arg in "$@"; do
  if [ "$arg" == "--noWrite" ]; then
    NO_WRITE=true
  fi
  if [ "$arg" == "--format" ]; then
    FORMAT=true
  fi
done

PATH=./bin:$PATH
if [[ ! -f bin/arduino-cli ]]; then
  echo "Downloading and installing the arduino-cli..."
  mkdir -p ./bin
  curl -fsSL https://raw.githubusercontent.com/arduino/arduino-cli/master/install.sh | BINDIR=./bin sh
fi

# TOTAL CACHE ERASE
# This clears the global cache that was causing the symbol mismatch
echo "Erasing all Arduino caches..."
rm -rf ~/.cache/arduino/sketches
rm -rf ~/.cache/arduino/cores
rm -rf ./build_cache

# Isolated Local Storage for Tools
LOCAL_DATA="/tmp/arduino_clock_cli"
mkdir -p "$LOCAL_DATA"
mkdir -p "./build_cache"
CLI_CONFIG="--config-dir $LOCAL_DATA"

# Add the official ESP8266 board repository to local configuration index
arduino-cli $CLI_CONFIG config add board_manager.additional_urls https://arduino.esp8266.com/stable/package_esp8266com_index.json

# Ensure core 3.1.2 (or stable equivalent) is installed locally
if [[ ! -d "$LOCAL_DATA/packages/esp8266/hardware/esp8266" ]]; then
  echo "Installing private ESP8266 Core..."
  arduino-cli $CLI_CONFIG core update-index
  arduino-cli $CLI_CONFIG core install esp8266:esp8266
fi

# Dynamically locate tools inside the dynamically generated ESP8266 tool paths
MKLITTLEFS_TOOL=$(find $LOCAL_DATA/packages/esp8266/tools/mklittlefs -name mklittlefs -type f | head -n 1)
ESPTOOL_TOOL=$(find $LOCAL_DATA/packages/esp8266 -name esptool.py -type f | head -n 1)

echo "Creating data image for in LittleFS partition..."
# Configured for standard 4MB modules matching 2MB Sketch / 2MB File System partition tables
# --size 0x2fa000 (3121152 bytes), --block 8192, --page 256
"$MKLITTLEFS_TOOL" \
  -p 256 \
  -b 8192 \
  -s 0x2fa000 \
  -c data \
  data.img

if [ "$NO_WRITE" = false ]; then
  if [ "$FORMAT" = true ]; then
    echo "Performing TOTAL HARDWARE FLASH ERASE..."
    # Completely wipes the 4MB module back to a clean factory state
    python3 "$ESPTOOL_TOOL" --port $PORT erase_flash
  fi

  echo "Write data image to LittleFS partition..."
  # The file system boundary for a 2MB Sketch layout starts at flash offset address 0x200000
  python3 "$ESPTOOL_TOOL" \
    --port $PORT \
    --baud 921600 \
    write_flash 0x100000 data.img
else
  echo "Skipping LittleFS flash (--noWrite enabled)"
fi

# Compile with clean build path
echo "Compiling..."
# 'eespsz=4M2M' sets up the internal 4 Megabyte Flash Layout partitioned with 3 Megabytes for LittleFS
arduino-cli $CLI_CONFIG compile --fqbn ${BOARD_FQBN} \
  -j 0 \
  --build-path "./build_cache" \
  --libraries ~/Arduino/libraries \
  ./Analog_Clock.ino

if [ "$NO_WRITE" = false ]; then
  echo "Uploading firmware..."
  arduino-cli $CLI_CONFIG upload \
    --fqbn ${BOARD_FQBN} \
    --port $PORT \
    --input-dir "./build_cache"
else
  echo "Skipping firmware flash (--noWrite enabled)"
fi
