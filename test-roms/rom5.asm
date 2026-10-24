%INS_SIZE=2
%FONT_START_ADDR=0x50
%LEFT=0x0
%RIGHT=0x1
%UP=0x2
%DOWN=0x3

; instructions per frame
%IPF=[16]

JMP #game_loop

; emulator runs at 1000Hz
; it can execute 1000 instructions per second

; say i want a single tick to be 1 second
; so between two ticks, i need **at least** 1000 instructions
; say DT = 250
; then i would need 4 iterations of this
; 8 bit registers, so 255 is the max value

; i want 1/60th of a second
; 1/60th of a second ~= 16.667 milliseconds
; in other words, around 16 instructions between frames

; 16 instructions per frame = 62.5 FPS

; i want 1/144th of a second
; 1/144th of a second ~= 6.9 milliseconds
; in other words, around 7 instructions between frames

game_loop:	; 0x200 so logically these are +2
    ; initial setup
	LD V4, [0x40 / 0x2]
	LD V5, [0x20 / 0x2]
	LD I, [%FONT_START_ADDR%]
	
	; initial draw
	CALL #render

    LD VE, 0
    CALL #reset_timer	; 0x202

    ; when DT = 0, one tick is finished
    ; every 0x24 ticks, call render function
    SE VE, 0x1
    JMP [%OFFSET% + %INS_SIZE% * 4] 

    LD VE, 0
    CALL #update
    CALL #render

    LD V0, DT
    SE V0, 0	; tick is not finished -> jmp back ; dont if DT = 0
    JMP [%OFFSET% - %INS_SIZE% * 7]
    CALL #reset_timer
    ADD VE, 0x1
    JMP [%OFFSET% - %INS_SIZE% * 10]

reset_timer:
    LD V0, [%IPF%]
    LD DT, V0
    RET

render:
    DRW V4, V5, 0x5
    RET

update:
    CALL #render 	; erase sprite
    CALL #processInput
    RET

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
    
