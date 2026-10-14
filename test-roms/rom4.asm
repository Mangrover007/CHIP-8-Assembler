%FONT_START_ADDR=0x50
%SCREEN_WIDTH=0x40
%SCREEN_HEIGHT=0x20
%FONT_WIDTH=0x8
%FONT_HEIGHT=0x5
%INS_SIZE=0x2


main:
    CALL #start		; 200
    JMP #loop		; 202

render_next_font:
    LD V0, [%SCREEN_WIDTH%]	; 204
    LD V1, [%SCREEN_HEIGHT%]	; 206

    LD V2, V5			; 208
    ADD V2, [%FONT_WIDTH%] ; final position of last bit of sprite
    SUB V0, V2			; 20c
    SE VF, 0x0			; 20e
    JMP [%OFFSET% + %INS_SIZE% * 2] ; 210
    LD V5, 0 		; reset the x pos to start of screen 212
    ADD V6, [%FONT_HEIGHT% + 2]	; 214

    DRW V5, V6, [%FONT_HEIGHT%]	; 216
    ADD I, V3			; 218
    ADD V5, [%FONT_WIDTH% + 2]
    RET				; 21a

start:
    LD I, [%FONT_START_ADDR%]   ; 21c

    LD V3, [%FONT_HEIGHT%]	; 21e

    LD V5, 0	; x pos		; 220
    LD V6, 0	; y pos		; 222

    LD V4, 0 	; iterator	; 224
    SNE V4, [0xF + 0x1]		; 226
    RET				; 228
    CALL #render_next_font	; 22a
    ADD V4, 0x1			; 22c
    JMP [%OFFSET% - %INS_SIZE% * 5] ; 23e

loop:
    JMP #loop			; 230

