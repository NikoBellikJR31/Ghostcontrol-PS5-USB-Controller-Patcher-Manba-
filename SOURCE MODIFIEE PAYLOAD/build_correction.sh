#!/usr/bin/env bash
set -euo pipefail

SDK="/c/"
export SCE_PROSPERO_SDK_DIR="$SDK"
export PS5_PAYLOAD_SDK="$SDK"
export PATH="/c/"

/c/msys64/clang64/bin/clang.exe \
  --start-no-unused-arguments \
  -target x86_64-sie-ps5 \
  -isysroot "$SDK" \
  -isystem "$SDK/target/include" \
  -L "$SDK/target/lib" \
  -L "$SDK/target/user/homebrew/lib" \
  -fno-stack-protector -fno-plt -femulated-tls \
  -lc -lkernel_web \
  --end-no-unused-arguments \
  -D__PROSPERO__ -Wall -Wextra -g -O2 -fPIC -fno-stack-protector \
  -o ghost-control-manba-v2-nbjr.elf \
  gc_main.c shellui_pad.c usb_helpers.c \
  controller_nintendo.c controller_xbox.c controller_ds4.c controller_mamba.c \
  -lScePad -lSceUserService -lpthread -ldl \
  --start-no-unused-arguments \
  -lSceLibcInternal -lSceNet \
  --end-no-unused-arguments

ls -l ghost-control-manba-v2-nbjr.elf
