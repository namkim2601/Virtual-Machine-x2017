#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

typedef struct ASM_func{
    int label;
    int num_of_instr;
    int instructions[32][5];
    int num_of_sym;
    int stk_values[32];
} ASM_func;

unsigned char RAM[256];
unsigned char Registers[8];
void execute_func(ASM_func program[]);

int main(int argc, char** argv) {
    FILE *fp; /*filepointer*/
    size_t size; /*filesize*/
    unsigned char *buffer;

    if (argc != 2) {
        fprintf(stderr, "Error: Please provide <path to binary file> as command line argument\n");           
        exit(1);
    }
    if (!(fp = fopen(argv[1], "rb"))) {
        fprintf(stderr, "Error: File could not be found\n");           
        exit(1);
    }
    
    fseek(fp, 0, SEEK_END); 
    size = ftell(fp);  
    if (size == 0) {
        fprintf(stderr, "Error: File is empty\n");
        exit(1);
    }    
    fseek(fp, 0, SEEK_SET); 
    buffer = malloc(size);  

    if (fread(buffer, sizeof *buffer, size, fp) != size){ 
        fprintf(stderr, "Error: There was an Error reading the file\n");
        exit(1);
    }

// Convert file content to hex
    char *buffer_to_hex = malloc(size*2+8);
    for (int i = 0; i < size; i++) {
        char temp[3];
        snprintf(temp, 3, "%02x",  buffer[i]);
        buffer_to_hex[2*i] = temp[0];
        buffer_to_hex[2*i + 1] = temp[1];
    }
    free(buffer);

// Convert hex to binary
    const char binary[16][5] = {"0000", "0001", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111"};
    const char digits[16] = "0123456789abcdef";
    char *bfunc = malloc(size*16+4);
    *bfunc = '\0';
    
    for (int i = 0; i < size*2; i++) {
        const char *d = strchr(digits, tolower(buffer_to_hex[i]));
        if (d)
            strcat(bfunc, binary[d - digits]);
    }
    free(buffer_to_hex);


    if (strlen(bfunc) % 8 != 0) {
        fprintf(stderr, "Error: Total number of bits in file does not accumulate to a whole number of bytes\n");
        exit(1);
    }

    ASM_func program[8];
    for (int i = 0; i < 8; i++) {
        program[i].label = -1;  
        program[i].num_of_instr = 0;
        program[i].num_of_sym = 0;
    }

    // Parse functions
    int func_0_exists = 0;
    int offset = 0;
    while (offset + 7 < strlen(bfunc)) {  
        char binfo[8]; // Opcodes, argument types and values are at most 8 bits

        offset += 5;
        int num_of_instr = bstr_to_int(parse_bits(bfunc, binfo, offset, 5));
        if (num_of_instr == 0) {
            fprintf(stderr, "Error: Function can not have 0 number of instructions\n");
            exit(1);
        }

        ASM_func current_func;
        current_func.num_of_instr = num_of_instr;
        current_func.num_of_sym = 0;
        for (int i = 0; i < 32; i++) {
            current_func.instructions[i][0] = -1;
            current_func.stk_values[i] = -1;
        }

        // Collect stack information of current function
        int offset_cpy = offset;
        for (int i = 0; i < num_of_instr; i++) {
            offset_cpy += 3; 
            int opcode = bstr_to_int(parse_bits(bfunc, binfo, offset_cpy, 3));

            int exists = 0;
            int type1 = -1;
            int arg1 = -1;
            if (opcode != 2) {
                offset_cpy += 2;
                type1 = bstr_to_int(parse_bits(bfunc, binfo, offset_cpy, 2));
                offset_cpy += update_offset(offset_cpy, type1);
                if (type1 == 2 || type1 == 3) {
                    arg1 = bstr_to_int(parse_bits(bfunc, binfo, offset_cpy, 5));
                    for (int j = 0; j < 32; j++) {
                        if (arg1 == current_func.stk_values[j])
                            exists = 1;
                    }

                    if (exists == 1) {
                        current_func.num_of_sym--;
                    } else {
                        current_func.stk_values[current_func.num_of_sym] = arg1;
                    }
                    current_func.num_of_sym++;
                }
            }

            exists = 0;
            int type2 = -1;
            int arg2 = -1;
            if(opcode == 0 || opcode == 3 || opcode == 4) {
                offset_cpy += 2;
                type2 = bstr_to_int(parse_bits(bfunc, binfo, offset_cpy, 2));
                offset_cpy += update_offset(offset_cpy, type2); 
                if (type2 == 2 || type2 == 3) {
                    arg2 = bstr_to_int(parse_bits(bfunc, binfo, offset_cpy, 5));
                    for (int j = 0; j < 32; j++) {
                        if (arg2 == current_func.stk_values[j])
                            exists = 1;
                    }

                    if (exists == 1) {
                        current_func.num_of_sym--;
                    } else {
                        current_func.stk_values[current_func.num_of_sym] = arg2;
                    }
                    current_func.num_of_sym++;
                }
            }
        }

        // Parse instructions
        int return_at_end = 0;
        for (int i = 0; i < num_of_instr; i++) {
            offset += 3;
            int opcode = bstr_to_int(parse_bits(bfunc, binfo, offset, 3));
            
            int type1 = -1;
            int arg1 = -1;
            char arg1_str[10];
            if (opcode != 2) {
                offset += 2;
                type1 = bstr_to_int(parse_bits(bfunc, binfo, offset, 2));
                offset += update_offset(offset, type1);
                arg1 = parse_addr(bfunc, arg1_str, offset, type1);
            }
            
            int type2 = -1;
            int arg2 = -1;
            char arg2_str[10];
            if(opcode == 0 || opcode == 3 || opcode == 4) {
                offset += 2;
                type2 = bstr_to_int(parse_bits(bfunc, binfo, offset, 2));
                offset += update_offset(offset, type2);
                arg2 = parse_addr(bfunc, arg2_str, offset, type2);
            }
            
            switch(opcode) {
                case 0:
                    if (type1 == 0) {
                        fprintf(stderr, "Error: OPCODE 'MOV' destination can not be value typed\n");
                        exit(1);
                    }
                    break;
                case 1:
                    if (type1 != 0) {
                        fprintf(stderr, "Error: OPCODE 'CAL' argument must be a value\n");
                        exit(1);
                    }
                case 2:
                    if (i == 0) {
                        return_at_end = 1;
                    }
                    break;
                case 3:
                    if (type1 == 0) {
                        fprintf(stderr, "Error: OPCODE 'REF' destination can not be value typed\n");
                        exit(1);
                    }
                    if (type2 != 2 && type2 != 3) {
                        fprintf(stderr, "Error: OPCODE 'REF' source must be stack symbol\n");
                        exit(1);
                    }
                    break;
                case 4:
                    if (type1 != 1) {
                        fprintf(stderr, "Error: OPCODE 'ADD' destination must be register\n");
                        exit(1);
                    }
                    if (type2 != 1) {
                        fprintf(stderr, "Error: OPCODE 'ADD' source must be register\n");
                        exit(1);
                    }
                    break;
                case 6:
                    if (type1 != 1) {
                        fprintf(stderr, "Error: OPCODE 'NOT' argument must be a register\n");
                        exit(1);
                    }
                    break;
                case 7:
                    if (type1 != 1) {
                        fprintf(stderr, "Error: OPCODE 'EQU' argument must be a register\n");
                        exit(1);
                    }
                    break;
            }

            current_func.instructions[(num_of_instr-1)-i][0] = opcode;
            if (opcode != 2) {
                current_func.instructions[(num_of_instr-1)-i][1] = type1;
                current_func.instructions[(num_of_instr-1)-i][2] = arg1;
                if(opcode == 0 || opcode == 3 || opcode == 4) {
                    current_func.instructions[(num_of_instr-1)-i][3] = type2;
                    current_func.instructions[(num_of_instr-1)-i][4] = arg2;   
                }
            }                  

        }
        offset += 3;
        current_func.label = bstr_to_int(parse_bits(bfunc, binfo, offset, 3));

        if (return_at_end == 0) {
            fprintf(stderr, "Error: Function label %d is missing return at end\n", current_func.label);
            exit(1);
        }   

        for (int i = 0; i < 8; i++) {
            if (current_func.label == program[i].label) {
                if (program[i].num_of_instr > 0) {
                    fprintf(stderr, "Error: There is more than one function with the label %d\n", program[i].label);
                    exit(1);
                }
            }
        }    
        if (current_func.label == 0) {
            func_0_exists = 1;
        }

        program[current_func.label] = current_func;  
    }
    free(bfunc);
    if (func_0_exists == 0) {
        fprintf(stderr, "Error: Entry point of program can not be found, please provide a function with the label 0\n");
        exit(1);
    }
    
    /* 
            Areas of memory that are unique

            Register 4 - Top of stack
            Register 5 - Label of of currently executing function
            Register 6 - Number of stack symbols in currently executing function
            Register 7 - Program counter

            RAM[255] - Infinite loop detector: Checks for infinite loops within function
            If value at RAM[255] exceeds 255, exit machine
    */

    // Machine starts at function 0
    Registers[5] = 0;
    Registers[6] = program[Registers[5]].num_of_sym;

    // Push entry function's stack values 
    for (int i = 0; i < Registers[6]; i++) {
        RAM[Registers[4]+i] = program[Registers[5]].stk_values[i];
    }

    Registers[4] = 2*Registers[6] + 2;  /* Update top of stack - Whenever a function is called, memory is 
                                        allocated to store stack symbols, stack values and return addresses */

    Registers[7] = 0; // Initialise PC with value 0
    execute_func(program); // Start virtual machine
    return 1;
}

int get_stk_idx(int arg) {
    for (int i = 0; i < Registers[6]; i++) {
        int temp_idx = Registers[4] - (Registers[6]*2) - 2;
        if (arg == RAM[temp_idx+i]) {
            return (temp_idx+i + 1*Registers[6]);
            break;
        }
    }
    fprintf(stderr, "Stack symbol incorrectly parsed\n");
    exit(1);
}

void execute_func(ASM_func program[]) {
    RAM[255] = 0; // Reset infinite loop detector for each function
    if (program[Registers[5]].label != Registers[5]) {
        fprintf(stderr, "Error: Trying to call function that does not exist\n");
        exit(1);
    }
    while (Registers[7] < 32) {
        int opcode = program[Registers[5]].instructions[Registers[7]][0];

        int type1 = -1;
        int arg1 = -1;
        if (opcode != 2) {
            type1 = program[Registers[5]].instructions[Registers[7]][1];
            arg1 = program[Registers[5]].instructions[Registers[7]][2];
        }
        
        int type2 = -1;
        int arg2 = -1;
        if(opcode == 0 || opcode == 3 || opcode == 4) {
            type2 = program[Registers[5]].instructions[Registers[7]][3];
            arg2 = program[Registers[5]].instructions[Registers[7]][4];
        }
       
        switch (opcode) {
            case 0: // MOV
                switch(type1) { // Destination
                    case 1:
                        switch (type2) { // Source
                            case 0:
                                Registers[arg1] = arg2;
                                break;
                            case 1:
                                Registers[arg1] = Registers[arg2];
                                break;
                            case 2:
                                Registers[arg1] = RAM[get_stk_idx(arg2)];
                                break;
                            case 3:
                                Registers[arg1] = RAM[RAM[get_stk_idx(arg2)]];
                                break;
                        }
                        break;
                    case 2:
                        switch (type2) {
                            case 0:
                                RAM[get_stk_idx(arg1)] = arg2;
                                break;
                            case 1:
                                RAM[get_stk_idx(arg1)] = Registers[arg2];
                                break;
                            case 2:
                                RAM[get_stk_idx(arg1)] = RAM[get_stk_idx(arg2)];
                                break;
                            case 3:
                                RAM[get_stk_idx(arg1)] = RAM[RAM[get_stk_idx(arg2)]];
                                break;
                        }
                        break;
                    case 3:
                        switch (type2) {
                            case 0:
                                RAM[RAM[get_stk_idx(arg1)]] = arg2;
                                break;
                            case 1:
                                RAM[RAM[get_stk_idx(arg1)]] = Registers[arg2];
                                break;
                            case 2:
                                RAM[RAM[get_stk_idx(arg1)]] = RAM[get_stk_idx(arg2)];
                                break;
                            case 3:
                                RAM[RAM[get_stk_idx(arg1)]] = RAM[RAM[get_stk_idx(arg2)]];
                                break;
                        }
                        break;
                    }
                break;
                
            case 1:
                if (Registers[4] + 2*program[arg1].num_of_sym + 2 > 254) {
                    fprintf(stderr, "Error: RAM has been exceeded\n");
                    exit(1);
                }

                // Push return addresses first
                RAM[Registers[4] + 2*program[arg1].num_of_sym] = Registers[5];
                RAM[Registers[4] + 2*program[arg1].num_of_sym + 1] = Registers[7];

                // Jump to called function
                Registers[5] = arg1;
                Registers[6] = program[Registers[5]].num_of_sym;

                // Push functions stack values 
                for (int i = 0; i < Registers[6]; i++) {
                    RAM[Registers[4]+i] = program[Registers[5]].stk_values[i];
                }

                Registers[4] = Registers[4] + 2*Registers[6] + 2;  // Update top of stack
                Registers[7] = 0; // Reset PC for new function
                execute_func(program);
                break;
            case 2:
                if (Registers[5] == 0) {
                    exit(0);
                }  
                // Get return addresses
                Registers[7] = RAM[Registers[4] - 1];
                Registers[5] = RAM[Registers[4] - 2];

                // Deallocate stack frame
                for (int i = 0; i < 2*Registers[6]+2; i++) {
                    RAM[Registers[4] - i] = '\0';
                }

                Registers[4] = Registers[4] - 2*Registers[6] - 2; // Update top of stack
                Registers[6] = program[Registers[5]].num_of_sym;  // Update number of symbols in running function
                return;
            case 3:
                switch (type1) {
                    case 1:
                        Registers[arg1] = get_stk_idx(arg2);
                        break;
                    case 2:
                        if (type2 == 2)
                            RAM[get_stk_idx(arg1)] = get_stk_idx(arg2);
                        if (type2 == 3)
                            RAM[get_stk_idx(arg1)] = RAM[get_stk_idx(arg2)];
                        break;
                    case 3:
                        if (type2 == 2)
                            RAM[RAM[get_stk_idx(arg1)]] = get_stk_idx(arg2);
                        if (type2 == 3)
                            RAM[RAM[get_stk_idx(arg1)]] = RAM[get_stk_idx(arg2)];
                        break;
                }
                break;
            case 4:
                if (arg1 == 7) {
                    if ((Registers[7] + Registers[arg2])%256 > 31 || program[Registers[5]].instructions[Registers[7] + Registers[arg2]][0] == -1) { 
                        fprintf(stderr, "Error: Program counter value is out of scope of instructions Func %d, PC %d\n", Registers[5], Registers[7]);
                        exit(1);
                    }
                    // Keep track of how many times program counter is subtracted
                    if (Registers[7] + Registers[arg2] > 255) {
                        // If program counter is subtracted more than 255 times, exit machine
                        if (RAM[255] == 255) { 
                            fprintf(stderr, "Error: Program has entered infinite loop within function. Func %d\n", Registers[5]);
                            exit(1);
                        }
                        RAM[255] += 1;
                    }
                }
                Registers[arg1] = Registers[arg1] + Registers[arg2];
                break;
            case 5:
                switch (type1) {
                    case 0:
                        printf("%u\n", arg1);
                        break;
                    case 1:
                        printf("%u\n", Registers[arg1]);
                        break;
                    case 2:
                        printf("%u\n", RAM[get_stk_idx(arg1)]);
                        break;
                    case 3:
                        printf("%u\n", RAM[RAM[get_stk_idx(arg1)]]);
                        break;
                }
                break; 
            case 6:
                Registers[arg1] = ~(Registers[arg1]);
                break;
            case 7:
                if (Registers[arg1]  == 0) {
                    Registers[arg1] = 1;
                } else {
                    Registers[arg1] = 0;
                }
                break;
        }  
        Registers[7] += 1; // Increment program counter
    }
}