#!/bin/bash
# Run on the laptop (Git Bash) from the browser/ folder: send this checkout's source to the VM and build and restart the
# server there (build-on-vm.sh). See ../SERVER.md.   Usage: server/deploy.sh [--no-restart]
set -euo pipefail
KEY="${PEGASUS_VM_KEY:-$HOME/.ssh/id_ed25519_azure_jondgoodwin}"
VM="${PEGASUS_VM:-ubuntu@40.87.21.70}"
here="$(cd "$(dirname "$0")/.." && pwd)"
root="$(git -C "$here" rev-parse --show-toplevel)"
tmp="$(mktemp -d)"
git -C "$root" archive --format=tar.gz -o "$tmp/pegasus3d.tgz" HEAD browser ':!browser/shots'
scp -i "$KEY" "$tmp/pegasus3d.tgz" "$VM:/tmp/pegasus3d.tgz"
ssh -i "$KEY" "$VM" "mkdir -p ~/pegasus/src && cd ~/pegasus/src && tar xzf /tmp/pegasus3d.tgz && bash browser/server/build-on-vm.sh ${1:-}"
rm -rf "$tmp"
