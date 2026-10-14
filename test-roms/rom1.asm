; variable assigned to integer values
%count=0x5
%screen_width=0x40

%START_ADDRESS=0x200

; math expression inside []
%screen_height=[%screen_width% / 2]

; variable assigned to a list, which contains pairs of coordinates
; this is a list of lists, which evaluate math expressions inside
%sprite_positions={
    {[%screen_width% - 8 / 2], [%screen_height% - 5 / 2]},
    {0, 0},
    {[%screen_width% - 8], 0},
    {0, [%screen_height% - 5]},
    {[%screen_widht% - 8], [%screen_height% - 5]}
}

; NOTE: all variables will be accessed with %var_name% which will
; be stored as a map element in a std::map<std::string, void*>

; list variables will naturally decay to their first element
; in other words, if %list% is a list variable, then the following
; are equivalent
; %list% | [%list% + 0]
; and in c++, it would just be iterator.begin()

; to access lsit variables then is simple math [%list% + i] for
; some integer i

; sprite data that needs to be stored to memory
%sprite_data={
    0b10011001,
    0b01100110
}

%SPRITE_START_ADDRESS=0x200


load_sprite:
    LD I, %SPRITE_START_ADDRESS%
    LD V0, [%sprite_data% + 0]
    LD V1, [%sprite_data% + 1]
    LD [I], V1


; main function
main:
    call #load_sprite
    call #start

start:
    CLS
    LD V0, 0x0		; iteration count
    LD I, 0x50		; sprite data (positioned at screen center)
    LD V1, [%screen_width% - 8 / 2]
    LD V2, [%screen_height% - 5 / 2]
    call #loop
    RET

loop:
    SNE V0, %count%	; skip the next instruction if V0 != 0x5
    RET
    ADD V0, 1		; increase iteration count
    DRW V1, V2, 0x5	; draw sprite
    ADD I, 0b101	; increment idx register for next sprite
    ADD V1, 0x08	; adjust x position for next sprite
    JMP #loop
