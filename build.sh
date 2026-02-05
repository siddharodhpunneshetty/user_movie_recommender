#!/bin/bash
# Build script for Render - compiles C code on Linux
echo "Installing GCC..."
apt-get update && apt-get install -y gcc

echo "Compiling recommender.c..."
gcc -o recommender recommender.c -lm

echo "Build complete!"
