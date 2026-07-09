#!/bin/bash

# Target destination
DEST="nixon@192.168.99.10:~/Project/xiaozhi-esp32"

# Get project root (one level up from scripts/ directory)
PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

echo "Syncing from $PROJECT_ROOT to $DEST..."

# Sync current state to target
# -a: archive mode (preserves permissions, etc.)
# -v: verbose
# --delete: delete files in target that are no longer in source
# --exclude: skip large/unnecessary directories
rsync -avz --delete \
    --exclude=".git/" \
    --exclude="build/" \
    --exclude=".vscode/" \
    --exclude=".devcontainer/" \
    --exclude=".cache/" \
    --exclude="sdkconfig" \
    --exclude="sdkconfig.old" \
    --exclude="managed_components/" \
    --exclude="dependencies.lock" \
    "$PROJECT_ROOT/" "$DEST/"

echo "Sync complete!"