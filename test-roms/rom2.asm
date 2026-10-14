start:
    LD V0 18
    LD V2 0
    LD V3 0
    LD I 0x50
    CLS
    DRW V2, V3, 5
    LD V2 58
    LD I 0x55
    DRW V2, V3, 5
    JMP #end

end:
    JMP V0 #start
    JMP #end	; infinite loop
