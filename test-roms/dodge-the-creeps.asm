%CLOCK_SPEED=1000
%INS_SIZE=2

%SCREEN_WIDTH=64
%SCREEN_HEIGHT=32

%FPS=60
%IPF=[%CLOCK_SPEED% / 60]

%ENEMY_SPAWN_INTERVAL=[8]

JMP #start		; 0x200

%PLAYER_POS=[%OFFSET% + 2]	; 0x202
RES 2				; 0x202

; enemy queue, need 4 slots = 4 * 4 bytes
%ENEMY_COUNT=8
%ENEMY_QUEUE=[%OFFSET% + 2]	; 0x202 + 0x2
RES [%ENEMY_COUNT% * 4] 	; 0x202 + 0x10 = 0x212

%ENEMY_SPRITE_START_ADDR=[%OFFSET% + 2]
$ENEMY_SPRITE=(0b11000000, 0b11000000)
STR $ENEMY_SPRITE$

$PLAYER_SPRITE=($ENEMY_SPRITE$[1], $ENEMY_SPRITE$[0])
%PLAYER_SPRITE_START_ADDR=[%OFFSET% + 2]
STR $PLAYER_SPRITE$

%ENEMY_WIDTH=0x2
%ENEMY_HEIGHT=0x2

test_program:
    LD V0, 0
    LD V1, 0
    LD I, %ENEMY_SPRITE_START_ADDR%
    DRW V0, V1, 0x2
    JMP [%OFFSET%]

%LEFT=0x0
%RIGHT=0x1
%UP=0x2
%DOWN=0x3

%PLAYER_SPEED=0x1

process_input:
    LD V2, %LEFT%
    LD V3, %RIGHT%
    LD V4, %UP%
    LD V5, %DOWN%

    LD I, %PLAYER_POS%
    LD V1, [I]

    SKNP V2
    CALL #move_left
    SKNP V3
    CALL #move_right
    SKNP V4
    CALL #move_up
    SKNP V5
    CALL #move_down

    LD I, %PLAYER_POS%
    LD [I], V1

    RET

move_left:
    SNE V0, 0x0		; sprite touching left side of screen
    RET

    LD V3, %PLAYER_SPEED%
    SUB V0, V3		; re-using V1 for 0x1
    RET


move_right:
    SNE V0, [%SCREEN_WIDTH% - 0x2] ; sprite touching right side of screen
    RET

    ADD V0, %PLAYER_SPEED%
    RET

move_up:
    SNE V1 0x0		; sprite touching top of screen
    RET

    LD V3, %PLAYER_SPEED%
    SUB V1, V3		; re-using V1 for 0x1
    RET
    
move_down:
    SNE V1, [%SCREEN_HEIGHT% - 0x2]	; sprite touching bottom of screen
    RET

    ADD V1, %PLAYER_SPEED%
    RET
 
start:
    ;JMP #test_program

    CALL #r_player_render

    CALL #reset_tick
    LD VE, 0
    RND VD, [%ENEMY_SPAWN_INTERVAL% | [%ENEMY_SPAWN_INTERVAL% - 1]]

    loop:
	; check tick timer
	LD V0, DT
	SE V0, 0
	JMP #loop

	; tick finished, process stuff
	CALL #reset_tick


	; advance enemy spawn timer, spawn if its time
	ADD VE, 1
	SNE VE, VD
	CALL #spawn_enemy

	CALL #update_enemies
	CALL #update_player

	CALL #check_collision

	SNE VF, 1
	JMP [%OFFSET%]

	CALL #render

	JMP #loop

check_collision:
    LD VF, 0

    ; load player position
    LD I, %PLAYER_POS%
    LD V1, [I]

    LD V6, V0
    LD V7, V1

    ; go through each enemy and check for collision
    LD V8, 0x0
    LD V9, 0x0
    cc_loop:
	SNE V8, %ENEMY_COUNT%
	RET

	ADD V8, 0x1
	LD I, %ENEMY_QUEUE%
	ADD I, V9
	LD V3, [I]
	ADD V9, 0x4

	LD VA, 0
	OR VA, V1
	OR VA, V2
	OR VA, V3
	SNE VA, 0
	JMP #cc_loop

	LD V4, V6
	LD V5, V7

	SUB V4, V0
	SUB V5, V1
	SUB V0, V6
	SUB V1, V7
	OR V0, V1
	OR V4, V5
	OR V0, V4

	LD VF, 0
	SNE V0, 0
	LD VF, 1
	SNE V0, 1
	LD VF 1

	SNE VF, 1
	RET

	JMP #cc_loop

update_player:
    LD I, %PLAYER_POS%
    LD V1, [I]
    LD I, %PLAYER_SPRITE_START_ADDR%
    DRW V0, V1, 0x2

    CALL #process_input
    RET

update_enemies:
    LD V5, 0
    LD V4, 0
    JMP #ue_loop

    ; need this above any dereference to it
    update_pos:
	LD I, %ENEMY_QUEUE%
	ADD I, V4
	LD [I], V3

	ADD V4, 0x4

	JMP #ue_loop

    ue_loop:
	SNE V5, %ENEMY_COUNT%
	RET

	ADD V5, 1

	LD I, %ENEMY_QUEUE%
	ADD I, V4

	LD V3, [I]

	; check if an enemy is there
	LD V7, 0
	OR V7, V0
	OR V7, V1
	OR V7, V2
	OR V7, V3
	SNE V7, 0
	JMP [#update_pos + %INS_SIZE% * 3]
    
	LD I, %ENEMY_SPRITE_START_ADDR%
	DRW V0, V1, 0x2		; erase sprite

	; should sprite be deleted or updated
	LD VF, 0
	LD V6, V0
	LD V7, V1
	SUB V6, V2
	SUB V7, V3
	LD V8, 0
	OR V8, V6
	OR V8, VF
	OR V8, V7
	OR V8, VF
	SNE V8, 0
	JMP #delete_enemy

	JMP #change_x

    delete_enemy:
	LD V0, 0
	LD V1, 0
	LD V2, 0
	LD V3, 0
	JMP #update_pos

    change_x:
	LD VF, 0
	LD V6, V0
	SUB V6, V2
	SNE VF, 1
	JMP #subtract_x

	; add_x
	ADD V0, 1
	JMP #change_y

    subtract_x:
	SUB V0, VF
	JMP #change_y

    change_y:
	LD VF, 0
	LD V6, V1
	SUB V6, V3

	SNE VF, 1
	JMP #subtract_y

	; add_y
	ADD V1, 1
	JMP #update_pos

    subtract_y:
	SUB V1, VF
	JMP #update_pos

render:
    LD V4, 0
    LD V5, 0

    r_enemy_loop:
	SNE V4, %ENEMY_COUNT%
	JMP #r_player_render

	ADD V4, 1

	LD I, %ENEMY_QUEUE%
	ADD I, V5
	LD V3, [I]

	ADD V5, 0x4

	; should you even draw the sprite
	LD V6, 0
	OR V6, V0
	OR V6, V1
	OR V6, V2
	OR V6, V3
	SNE V6, 0
	JMP #r_enemy_loop

	LD I, %ENEMY_SPRITE_START_ADDR%
	DRW V0, V1, 0x2

	JMP #r_enemy_loop

    r_player_render:
	LD I, %PLAYER_POS%
	LD V1, [I]
	LD I, %PLAYER_SPRITE_START_ADDR%
	DRW V0, V1, 0x2
	RET


spawn_enemy:
    CALL #reset_enemy_spawn_timer
    LD I, %ENEMY_QUEUE%
    LD V4, 0	; number of enemies and this iterations
    LD VF, 0	; final offset

    find_free_slot:
	SNE V4, %ENEMY_COUNT%
	RET
	
	; load enemy data
	LD V3, [I]
	LD V6, 0x00

	; if any byte: x pos or y pos or target x pos or y pos
	; is non-zero
	OR V6, V0
	OR V6, V1
	OR V6, V2
	OR V6, V3

	; break the loop
	SNE V6, 0
	JMP #outside		; 0x25e

	ADD V4, 0x1		; 0x260
	ADD VF, 0x4		; 0x262
	JMP #find_free_slot	; 0x264

    outside:			; 0x264 + 2 = 0x266
	CALL #spawn		; 0x266

	LD I, %ENEMY_QUEUE%	; 0x268
	ADD I, VF
	LD [I], V3

	; draw the sprite because parity
	LD I, %ENEMY_SPRITE_START_ADDR%
	DRW V0, V1, 0x2

	RET

spawn:
    ; choose random as fuck wall 0 or 1 or 2 or 3
    RND V5, [4 - 1]

    SNE V5, 0
    JMP #spawn_from_wall_0
    SNE V5, 1
    JMP #spawn_from_wall_1
    SNE V5, 2
    JMP #spawn_from_wall_2
    ; SNE V5, 3
    JMP #spawn_from_wall_3

    spawn_from_wall_0:
	RND V0, [%SCREEN_WIDTH% - %ENEMY_WIDTH%]
	LD V1, 0
	; into wall_1 or wall_2 or wall_3
	RND V6, [2 - 1]
	SNE V6, 0
	JMP #wall_1
	SNE V6, 1
	JMP #wall_2
	; SNE V6, 2
	JMP #wall_3


    spawn_from_wall_1:
	RND V1, [%SCREEN_HEIGHT% - %ENEMY_HEIGHT%]
	LD V0, [%SCREEN_WIDTH% - %ENEMY_WIDTH%]
	; into wall_0 or wall_2 or wall_3
	RND V6, [2 - 1]
	SNE V6, 0
	JMP #wall_0
	SNE V6, 1
	JMP #wall_2
	; SNE V6, 2
	JMP #wall_3


    spawn_from_wall_2:
	RND V0, [%SCREEN_WIDTH% - %ENEMY_WIDTH%]
	LD V1, [%SCREEN_HEIGHT% - %ENEMY_HEIGHT%] 
	; into wall_1 or wall_0 or wall_3
	RND V6, [2 - 1]
	SNE V6, 0
	JMP #wall_1
	SNE V6, 1
	JMP #wall_0
	; SNE V6, 2
	JMP #wall_3


    spawn_from_wall_3:
	RND V1, [%SCREEN_HEIGHT% - %ENEMY_HEIGHT%]
	LD V0, 0
	; into wall_1 or wall_2 or wall_0
	RND V6, [2 - 1]
	SNE V6, 0
	JMP #wall_1
	SNE V6, 1
	JMP #wall_2
	; SNE V6, 2
	JMP #wall_0

    wall_0:
	RND V2, [%SCREEN_WIDTH% - %ENEMY_WIDTH%]
	LD V3, 0
	RET

    wall_1:
	RND V3, [%SCREEN_HEIGHT% - %ENEMY_HEIGHT%]
	LD V2, [%SCREEN_WIDTH% - %ENEMY_WIDTH%]
	RET

    wall_2:
	RND V2, [%SCREEN_WIDTH% - %ENEMY_WIDTH%]
	LD V3, [%SCREEN_HEIGHT% - %ENEMY_HEIGHT%]
	RET

    wall_3:
	RND V3, [%SCREEN_HEIGHT% - %ENEMY_HEIGHT%]
	LD V2, 0
	RET

reset_enemy_spawn_timer:
    LD VE, 0
    RND VD, [%ENEMY_SPAWN_INTERVAL% | [%ENEMY_SPAWN_INTERVAL% - 1]]
    RET

reset_tick:
    LD V0, %IPF%
    LD DT, V0
    RET

