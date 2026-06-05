#!/bin/bash

# Configuration
IDF_PATH="/home/nixon/Project/esp-idf"

# Check if the export script exists
if [ -f "$IDF_PATH/export.sh" ]; then
    echo "Setting up ESP-IDF environment from $IDF_PATH..."
    . "$IDF_PATH/export.sh"
else
    echo "Error: ESP-IDF export script not found at $IDF_PATH/export.sh"
    echo "This script is intended to be run on the server."
fi