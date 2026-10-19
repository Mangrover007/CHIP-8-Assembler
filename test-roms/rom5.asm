%INS_SIZE=2
%FONT_START_ADDR=0x50
%LEFT=0x0
%RIGHT=0x1
%UP=0x2
%DOWN=0x3

JMP #game_loop

; emulator runs at 1000Hz
; it can execute 1000 instructions per second

; say i want a single tick to be 1 second
; so between two ticks, i need **at least** 1000 instructions
; say DT = 250
; then i would need 4 iterations of this
; 8 bit registers, so 255 is the max value

game_loop:	; 0x200 so logically these are +2
    LD V2, 0
    CALL #reset_timer	; 0x202

    ; when DT = 0, one tick is finished
    ; every 0x24 ticks, call render function
    SE V2, 0x4
    JMP [%OFFSET% + %INS_SIZE% * 3] 

    LD V2, 0
    CALL #render

    LD V0, DT
    SE V0, 0	; tick is not finished -> jmp back ; dont if DT = 0
    JMP [%OFFSET% - %INS_SIZE% * 6]
    CALL #reset_timer
    ADD V2, 0x1
    JMP [%OFFSET% - %INS_SIZE% * 9]

reset_timer:
    LD V0, [0xFF - 0x5]
    LD DT, V0
    RET

render:
    LD V0, 0
    LD V1, 0
    LD I, [%FONT_START_ADDR%]
    DRW V0, V1, 0x5
    RET

main:
    LD V4, [0x40 / 0x2]
    LD V5, [0x20 / 0x2]
    LD I, [%FONT_START_ADDR%]

    ; initial draw
    DRW V4, V5, 0x5
    
    ; erase sprite
    DRW V4, V5, 0x5
    CALL #processInput
    DRW V4, V5, 0x5
    JMP [%OFFSET% - %INS_SIZE% * 4]	; prev instruction

processInput:
    LD V0, [%LEFT%]
    LD V1, [%RIGHT%]
    LD V2, [%UP%]
    LD V3, [%DOWN%]

    SKNP V0
    CALL #move_left
    SKNP V1
    CALL #move_right
    SKNP V2
    CALL #move_up
    SKNP V3
    CALL #move_down

    RET

move_left:
    SNE V4, 0x0		; sprite touching left side of screen
    RET

    SUB V4, V1		; re-using V1 for 0x1
    RET


move_right:
    SNE V4, [0x40 - 0x4] ; sprite touching right side of screen
    RET

    ADD V4, 0x1
    RET

move_up:
    SNE V5 0x0		; sprite touching top of screen
    RET

    SUB V5, V1		; re-using V1 for 0x1
    RET
    
move_down:
    SNE V5, [0x20 - 0x5]	; sprite touching bottom of screen
    RET

    ADD V5, V1		; re-using V1 for 0x1
    RET
    
