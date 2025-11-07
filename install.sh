#!/bin/bash
set -e

echo "Checking dependencies..."

check_and_install() {
    local pkg="$1"
    if pacman -Qi "$pkg" &>/dev/null; then
        echo "✓ $pkg already installed"
    else
        echo "Installing $pkg..."
        sudo pacman -S --noconfirm --needed "$pkg"
    fi
}

check_and_install raylib
check_and_install cjson

echo "Building keyz-ro..."
make build

echo "Installing to /usr/local/bin..."
sudo install -Dm755 build/keyz-ro /usr/local/bin/keyz-ro
make clean

echo "Done. Run with: keyz-ro"

