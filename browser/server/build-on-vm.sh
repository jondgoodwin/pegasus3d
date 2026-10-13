#!/bin/bash
# Run ON THE VM, after the source is in ~/pegasus/src (deploy.sh sends it): build the server and restart the service.
# See ../SERVER.md. Needs the Cone compiler in ~/pegasus/cone (built there, shared libLLVM) and the SDL3 stand-in.
set -euo pipefail
cd "$HOME/pegasus/src/browser"
mkdir -p "$HOME/pegasus/bin" "$HOME/pegasus/run"
# The stand-in for libSDL3 (no window, no GPU, no sound: the server needs none): the linker finds it by LIBRARY_PATH
cc -O2 -Wall -fPIC -shared -o "$HOME/pegasus/bin/libSDL3.so" server/sdl3shim.c
LIBRARY_PATH="$HOME/pegasus/bin" nice -n 10 python3 "$HOME/pegasus/cone/tools/congo/congo.py" build --release
cp build/release/browser "$HOME/pegasus/bin/pegasus-server.new"
mv -f "$HOME/pegasus/bin/pegasus-server.new" "$HOME/pegasus/bin/pegasus-server"
if [ "${1:-}" != "--no-restart" ]; then
  sudo systemctl restart pegasus3d-server
  sleep 15
  systemctl is-active pegasus3d-server
fi
