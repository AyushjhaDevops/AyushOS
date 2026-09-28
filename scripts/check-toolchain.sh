#!/bin/bash

set -e

echo "======================================"
echo "       AyushOS Toolchain Check"
echo "======================================"

echo
echo "[1] GCC"
gcc --version | head -n 1

echo
echo "[2] NASM"
nasm --version

echo
echo "[3] GNU LD"
ld --version | head -n 1

echo
echo "[4] Make"
make --version | head -n 1

echo
echo "[5] QEMU"
qemu-system-i386 --version | head -n 1

echo
echo "[6] GDB"
gdb --version | head -n 1

echo
echo "[7] GRUB"
grub-install --version

echo
echo "[8] GRUB Rescue"
grub-mkrescue --version

echo
echo "[9] Xorriso"
xorriso --version | head -n 1

echo
echo "======================================"
echo "Toolchain check completed."
echo "======================================"
