SWOS Expanded Title Menu — five static PNG sprites, build code 0.10.53_01.

Extract this ZIP in the folder that contains AVVIA_SWOS.bat.
The ZIP places these PNG RGBA sprites under assets/title/buttons/:
  career.png
  hall_of_fame.png
  classic_modes.png
  options.png
  exit_game.png

The original approved title background and canonical logo are unchanged and must
already exist in assets/title/. The code commit contains the SDL2 animation and
second-level menu wiring; this ZIP supplies ONLY the new artwork. The source
image is the user-selected 1086x1448 RGB artwork, cropped without resizing;
only the connected black background outside each button was made transparent.
Do not use the reduced-resolution animated GIF as an asset.

Windows build and interaction tests must be completed on the user's PC.
