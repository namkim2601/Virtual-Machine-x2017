FUNC LABEL 0
    MOV STK A VAL 8
    REF REG 0 STK A
    CAL VAL 7
    PRINT STK A
    RET
FUNC LABEL 7
    MOV STK A REG 0
    MOV REG 0 PTR A
    MOV REG 1 VAL 7
    ADD REG 0 REG 1
    MOV PTR A REG 0
    RET
