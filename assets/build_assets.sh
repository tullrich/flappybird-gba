# Textures to c arrays
grit images/flappy_background.png -p -pn16 -gt -gB4 -ftc -m -mp1 -gT[0]
grit images/flappy_foreground.png -p -pn16 -gt -gB4 -ftc -m -gT[0]
grit images/flappy_bird_sprite.png -p -pn16 -gt -gB4 -fa -ftc -gT0x7c1f -Mw4 -Mh4 -o flappy_sprites
grit images/flappy_pipe_sprite.png -p -pn16 -gt -gB4 -fa -ftc -gT0x7c1f -Mw4 -Mh16 -o flappy_sprites
grit images/flappy_buttons_sprite.png -p -pn16 -gt -gB4 -fa -ftc -gT0x7c1f -Mw2 -Mh2 -o flappy_sprites
grit images/flappy_text_sprite.png -p -pn16 -gt -gB4 -fa -ftc -gT0x7c1f -Mw4 -Mh4 -o flappy_sprites

# Wav file -> raw pcm -> .rodata of an obj file
ffmpeg -i audio/sfx_die.wav -ac 1 -ar 10512 -f s8 -acodec pcm_s8 -y sfx_die.raw
arm-none-eabi-objcopy -I binary -O elf32-littlearm -B armv4t --rename-section .data=.rodata sfx_die.raw sfx_die.o

ffmpeg -i audio/sfx_wing.wav -ac 1 -ar 10512 -f s8 -acodec pcm_s8 -y sfx_wing.raw
arm-none-eabi-objcopy -I binary -O elf32-littlearm -B armv4t --rename-section .data=.rodata sfx_wing.raw sfx_wing.o

ffmpeg -i audio/sfx_hit.wav -ac 1 -ar 10512 -f s8 -acodec pcm_s8 -y sfx_hit.raw
arm-none-eabi-objcopy -I binary -O elf32-littlearm -B armv4t --rename-section .data=.rodata sfx_hit.raw sfx_hit.o

ffmpeg -i audio/sfx_swooshing.wav -ac 1 -ar 10512 -f s8 -acodec pcm_s8 -y sfx_swooshing.raw
arm-none-eabi-objcopy -I binary -O elf32-littlearm -B armv4t --rename-section .data=.rodata sfx_swooshing.raw sfx_swooshing.o

ffmpeg -i audio/sfx_point.wav -ac 1 -ar 10512 -f s8 -acodec pcm_s8 -y sfx_point.raw
arm-none-eabi-objcopy -I binary -O elf32-littlearm -B armv4t --rename-section .data=.rodata sfx_point.raw sfx_point.o
