; %temp=[2*[1+0xF]]
; %_vbr1=[[0x1+2]*3]

; %_vbr2=[1+2*0b1101]
; %__vbr_3=[[0xD+[%_vbr1%+%_vbr2%-[%temp% / 2]]]*3]
; %__var4=[%__vbr_3%/0b1101]

main:
    JMP #start

%SCREEN_WIDTH=0x40
%SCREEN_HEIGHT=[%SCREEN_WIDTH% / 2]

%SPRITE1=0b10011001
%SPRITE2=0b01100110

$SPRITE_DATA=(
0b10011001,
%SPRITE2%,
[%SPRITE1% ^ %SPRITE2%]
)

%SPRITE_DATA_ADDR=%OFFSET%

STR $SPRITE_DATA$	; store the sprite data directly to memory

start:
    LD V0, [[%SCREEN_WIDTH% - 8] / 2]
    LD V1, [[%SCREEN_HEIGHT% - 3] / 2]
    LD I, [%SPRITE_DATA_ADDR%]
    DRW V0, V1, 3

loop:
    JMP #loop

