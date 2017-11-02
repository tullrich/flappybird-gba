# Flappy Bird GBA
This project is an exercise in writing a full game for the [Game Boy Advance](http://problemkaputt.de/gbatek.htm).
It condenses many hardware features and best practices into a single codebase.

![Flappy Bird 1](https://user-images.githubusercontent.com/834356/32338416-a314c9b8-bfc2-11e7-87db-8cdaad74bd60.png)

## GBA Hardware and Features
* LCD Video Controller
	* VBlank sync both manually and via BIOS VBlankIntrWait (swi instruction)
	* Tile mapped backgrounds (video mode 0)
	* Object Attributes
	* Affine Object Attributes (rotated/scaled sprites)
	* ERAM->OAM shadow object copy
* Keypad Input
	* Simple keypoll and ERAM shadow
* Timers
	* Cascaded timers
	* Timer inturrupts
* DMA
	* Direct memory access utility functions
* Hardware Inturrupts
	* Basic inturrupt handler (no inturrupt vector table yet)
* Audio Coprocessor
	* 4 Channel DirectSound audio mixer fed via DMA
	* Audio mixing done in the main loop, buffer flip done during VBlank inturrupt
* Sin/Cos Lookup tables
* Binary asset pipeline using grit (images to c array) and objcopy

### Unused
* Windowing
* Effects (Mosiac, Alpha Blending)
* Serial Comms
* SRAM (ROM gamesave)

## VRAM/OAM Layout
```
<-------VRAM(0x06000000)--->
<-------Charblock0(512 tiles)(0x06000000)--->
<xxxxxBG1(83 tiles)[0x0000-0x0A60]xxx>
<-------Charblock1(512 tiles)(0x06004000)--->
<xxxxxBG0(9 tiles)[0x4000-0x4120]xxx>
<-------Charblock2(512 tiles)(0x06008000)--->
<-------Charblock3(512 tiles)(0x0600C000)--->
<xBG0_Map(18x32)[0xF000-0xF800]x>
<xBG1_Map(18x32)[0xF800-0xFC80]x>
<------OVRAM(0x06010000)--->
<-------Charblock4(512 tiles)[0x06010000]--->
<xxxxxBird_Sprite(64 tiles)[0x0000-0x0800]xxx>
<xxxxxPipe_Sprite(128 tiles)[0x4800-0x5800]xxx>
<xxxxxButton_Sprite(60 tiles)[0x5800-0x5F80]xxx>
<xxxxxText_Sprite(144 tiles)[0x5F80-0x7180]xxx>
<-------Charblock5(512 tiles)[0x06014000]--->
<---VRAM_END(0x06017FFF)--->

<--------OAM(0x07000000)--->
<Btn1[0][0x0000]>
<Btn2[1][0x0008]>
<Btn3[2][0x0010]>
<Text1[3][0x0018]>
<Text2[4][0x0020]>
<Text3[5][0x0028]>
<Player[6][0x0030]>
<Pipe[7][0x0038]>
<Pipe[8][0x0040]>
<Pipe[9][0x0048]>
<Pipe[10][0x0050]>
<----OAM_END(0x070003FF)--->
```

## Dependencies
* CMake
* [devkitARM](https://sourceforge.net/projects/devkitpro/files/devkitARM/)
	* GCC - Arm7tdmi cross compiler
	* Grit - image conversion utility
	* gbafix - ROM header generator
* ffmpeg - wavefront audio to headerless (raw) PCM audio
* Assets:
	* [Flappy Bird Spritesheet](https://www.spriters-resource.com/mobile/flappybird/sheet/59537/)
	* [Flappy Bird Audio Clips](https://www.sounds-resource.com/mobile/flappybird/sound/5309/)

## Building
This project uses CMake. To build run the following from the top level project directory (on linux):
```
cd assets
sh build_assets.sh
mkdir ../build
cd ../build
cmake ..
make
```

## References
* GBA
	* http://problemkaputt.de/gbatek.htm
	* https://www.cs.rit.edu/~tjh8300/CowBite/CowBiteSpec.htm
	* http://www.coranac.com/tonc/text/hardware.htm
	* http://cs.umw.edu/~finlayson/class/spring16/cpsc305/
	* https://www.gamasutra.com/view/feature/131491/gameboy_advance_resource_management.php
	* http://forum.gbadev.org/viewtopic.php?t=4063
* GBA Audio
	* http://deku.gbadev.org/program/sound1.html
	* http://www.belogic.com/gba/index.php
* ARM
	* https://sourceware.org/cgen/gen-doc/arm-thumb-insn.html
	* http://infocenter.arm.com/help/topic/com.arm.doc.ddi0210c/DDI0210B.pdf
	* http://x86asm.net/articles/fixed-point-arithmetic-and-tricks/
* Assets
	* https://www.spriters-resource.com/mobile/flappybird/sheet/59537/
	* https://www.sounds-resource.com/mobile/flappybird/sound/5309/
