#!/usr/bin/env bash
set -e

echo "=== Building Linux VST3 ==="
cmake --preset linux-release
cmake --build --preset linux

echo "=== Building Windows VST3 ==="
cmake --preset windows-release
cmake --build --preset windows

echo "=== Done ==="
