#!/usr/bin/env bash
set -e

LIBTEMPLATE="$(cd "$(dirname "$0")" && pwd)/buildsys-ios/libtemplate/"
ARCH_DIR="$(cd "$(dirname "$1")" && pwd)"
ARCH_NAME="$(basename "$1")"

ARCH_TMP_DIR="$ARCH_DIR/$ARCH_NAME.tmp"
rm -rf "$ARCH_TMP_DIR"
mkdir -p "$ARCH_TMP_DIR"
tar -xjf "$ARCH_DIR/$ARCH_NAME" -C "$ARCH_TMP_DIR"
(
    cd "$ARCH_TMP_DIR"
    rsync -a "$LIBTEMPLATE" libtemplate
    rm "$ARCH_DIR/$ARCH_NAME"
    tar -cjf "$ARCH_DIR/$ARCH_NAME"  --exclude='CoronaSimLogo-256.png' --exclude='world.jpg' --exclude='Icon*.png' ./{libtemplate,template.app}
)
rm -rf "$ARCH_TMP_DIR"
echo "Done!"
