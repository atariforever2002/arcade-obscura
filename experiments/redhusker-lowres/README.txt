RED HUSKER LOW-RESOLUTION FEASIBILITY TEST — October 25, 2026

Extract RHLOW into C:\MAME so C:\MAME\RHLOW\LOW.BAT exists.
This is a NEW isolated folder. Do not merge it over the main MAME folder.
From C:\MAME> type RHLOW\LOW and press Enter.
No coin or play needed: watch the demo. Check picture size, colors, smoothness.
Press Esc to return. Photograph the FPS and video-driver lines.
If the display is unreadable, press Esc. Do not adjust the CRT.
If there is a no-VESA-mode error, stop and photograph it.
HIGH.BAT is a matched 640x480 control, only if the low mode works.
From C:\MAME> use RHLOW\HIGH (from inside RHLOW just use HIGH).
Both tests intentionally disable sound and fix frame skipping at zero.
Compare only these two runs, not earlier sound-enabled results.

Executable is the unchanged, previously software-validated MAME52.EXE,
SHA256 b002afa5835e1a4541aea931094a31ade02ae5f4ae0732760a9e8fff54ad3653.
This package changes the requested video mode, not the executable.
320x240 writes 153600 bytes per full 16-bit frame vs 614400 at 640x480.
That is 75% fewer output bytes, NOT a promised 4x overall speedup.
Rygar's 256x224 source fits without source-pixel downsampling. Pixel
repeat spacing changes; visual quality and CRT coverage remain unverified.
The exact scaler passes added host tests at height 240. Its existing DOS
runtime validation does not establish Matrox low-mode support.
MAME may fall back from linear VESA to banked VESA at the same size.
Banked mode bypasses this custom fill routine. Selected driver/log and
picture coverage must therefore be checked before accepting this test.
No custom register timings, 15kHz modes, or video BIOS changes are used.
Fresh test configuration uses default keyboard controls and local save
folders. Cabinet ROMs are read from C:\MAME\ROMS; original configs,
scores, executables and GLAUNCH settings are not modified by this package.
LOW.LOG/HIGH.LOG retain the latest respective test's video log.
