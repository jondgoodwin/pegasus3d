Pegasus3D browser (Windows, 64-bit)  -  build @HASH@
=====================================================

A 3-D world browser written in the Cone language. This build opens its baked-in world: an island with a
path, a forest, a cottage, a brook and bridge, villagers, weather, sound, and a hot-air balloon you can ride.
It can also join a multiplayer server as a client (see "Multiplayer").

TO RUN
  Unzip the folder anywhere (keep its files together), then double-click browser.exe.
  The program is not code-signed, so Windows SmartScreen will warn "Windows protected your PC":
  click "More info", then "Run anyway". Only do this if you got the zip from a place you trust.

REQUIREMENTS
  - Windows 10 or 11, 64-bit
  - A GPU and current driver with Vulkan 1.3 (NVIDIA, AMD or Intel; update your graphics driver if it does not start)
  - Memory: about 0.6 GB of RAM in use while running (peak 582 MB measured), plus about 1.2 GB of committed virtual memory
  - Disk: about 16 MB unpacked (6.4 MB zip)

KEYS
  W / S or Up / Down     walk forward / back          A / D or Left / Right   turn
  Shift (held)           run                          Space                   jump
  PageUp / PageDown, or drag with the right mouse button     look up / down
  V                      first / third person         Tab                     possess the other mannequin
  Left click             pick what is under the pointer (the launch button in the balloon's basket starts the ride)
  P  pause time          F  time 10x faster           T  next weather
  N  mute sound          H  hide / show the HUD       B  bowl the ball
  F1 throw paint   F2 zap   F3 make dance (at what the camera looks at)
  F11 fullscreen         Escape quit

MULTIPLAYER
  Start with:   browser.exe --connect 40.87.21.70:40000 --name YourName
  Multiplayer is by approval: the server accepts only players whose IP address has been approved.
  To ask to be let in:   browser.exe --request-play --name YourName --message "a line about you"
  (it sends one request over HTTPS and prints what happened; then wait to be approved). Otherwise, run without options for the single-player island.

OPTIONS
  --no-audio         no sound
  --no-vegetation    no trees and plants (lighter on a slow GPU)
  --no-npcs          no villagers
  GPU_POWER_PREFERENCE=low-power  (environment variable) picks the integrated GPU where there are two.

LICENSES
  Pegasus3D and Cone are MIT-licensed. SDL3 is zlib-licensed. The Inter font is under the SIL Open Font License.
  See THIRD-PARTY.txt and the licenses folder.
