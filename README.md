A port of Wine's new experimental WOW64 mode, without 32-bit Wine libraries or binaries to FreeBSD.
Based on the code from Alex (shkhln).

https://github.com/shkhln/freebsd-wine

## Known Issues:
* 32-bit WineD3D performance is severely slow. Please use Damavand with `WINE_D3D_CONFIG="renderer=vulkan"`, DXVK, or the Zink (OpenGL on Vulkan) Mesa driver for impacted 32-bit applications.
* 32-bit OpenGL applications are limited to OpenGL 4.3. 64-bit applications aren't impacted have full OpenGL 4.6 exposed.
* The new WOW64 mode is considered experimental and may have bugs that can lead to unexpected issues with certain applications. Please report any WOW64-related issues to upstream Wine.
