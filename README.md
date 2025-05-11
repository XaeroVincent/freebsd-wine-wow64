A port of Wine's new experimental WOW64 mode, without 32-bit Wine libraries or binaries to FreeBSD.

Based on the code from Alex (shkhln).

https://github.com/shkhln/freebsd-wine

> This version also includes the https://gitlab.winehq.org/wine/wine/-/merge_requests/5213/diffs patch, which fixes Cyberpunk 2077, Unreal Engine and some Unity games.

## Known Issues:
* 32-bit WineD3D performance is extremely poor. Please use Damavand with `WINE_D3D_CONFIG="renderer=vulkan"`, DXVK, or the Zink (OpenGL on Vulkan) Mesa driver for impacted 32-bit applications.
* 32-bit OpenGL applications are currently limited to OpenGL 4.3. 64-bit applications aren't impacted and have full OpenGL 4.6 exposed to them.
* The new WOW64 mode is considered experimental and may have bugs that can lead to unexpected issues with certain applications. Please report any WOW64-related issues to upstream Wine.
