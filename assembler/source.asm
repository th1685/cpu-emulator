        ;test source file for assembler
.org 64 STR AI 58 ;set register A to 58
        STM A  AB ;store value in register A (58) at address AB
        STR BA AB ;set register B to value at address AB (58)
loop    CMP LT AB ;compare less than register A with register B
        BRN    sbrtn1 ;branch (if compare flag set) to sbtrn1
        INC DC B ;decrement register B
        JMP UN loop ;jump unconditionally to loop
sbrtn1  STR BA AB ;set register B to value at address AB (58)
        JMP UN loop ;jump back to the loop
