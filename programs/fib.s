        STR A 0
        STR B 1
        STM B 0xEE

loop:   ADD A 0xEE  ;add A and B, stored in R
        STM R 0xEE  ;store R at 0xEE
        STM B 0xEA  ;store B at 0xEA
        STR A 0xEA  ;load A from 0xEA (A=B)
        STR B 0xEE  ;load B from 0xEE (B=R)
        JMP loop
