#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

const char symbols[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdef";
int stk_values[32];
int num_of_sym = 0;

char* get_addr(char* bfunction, char* str, int offset, int type);
void prepend(char* str, const char* str_to_prepend);

int main(int argc, char** argv) {
    FILE *fp;
    size_t size;
    unsigned char *buffer;

    if (!(fp = fopen(argv[1], "rb"))) {
        printf("Error: The file %s could not be found\n", argv[1]);           
        exit(1);
    }

    fseek(fp, 0, SEEK_END); 
    size = ftell(fp);         
    fseek(fp, 0, SEEK_SET); 
    buffer = malloc(size);

    if (fread(buffer, sizeof *buffer, size, fp) != size){ 
        printf("Error: There was an Error reading the file %s\n", argv[1]);
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

    char *bfunction = malloc(size*8+4);
    *bfunction = '\0';

    for (int i = 0; i < size*2; i++) {
        const char *d = strchr(digits, tolower(buffer_to_hex[i]));
        if (d)
            strcat(bfunction, binary[d - digits]);
    }
    free(buffer_to_hex);
    
    if (strlen(bfunction) % 8 != 0) {
        printf("Error: Total number of bits in file does not accumulate to a whole number of bytes\n");
        exit(1);
    }
    
    char binfo[8]; // Opcodes, argument types and values are at most 8 bits
    int offset = 0;

    char* output_str = malloc(strlen(bfunction) * 2);
    strcpy(output_str, "");

    // Parse functions
    while (offset + 7 < strlen(bfunction)) {  
        offset += 5;
        int num_total_instr = bstr_to_int(parse_bits(bfunction, binfo, offset, 5));

        for (int i = 0; i < 32; i++) {
            stk_values[i] = -1;
        }
        num_of_sym = 0;

        // Collect stack information of current function
        int offset_cpy = offset;
        int idx_first_app[32];
        for (int i = 0; i < num_total_instr; i++) {
            offset_cpy += 3; 
            int opcode = bstr_to_int(parse_bits(bfunction, binfo, offset_cpy, 3));

            int prev_idx = -1;
            int type1 = -1; int arg1 = -1;
            if (opcode != 2) {
                offset_cpy += 2;
                type1 = bstr_to_int(parse_bits(bfunction, binfo, offset_cpy, 2));
                offset_cpy += update_offset(offset_cpy, type1);
                if (type1 == 2 || type1 == 3) {
                    arg1 = bstr_to_int(parse_bits(bfunction, binfo, offset_cpy, 5));
                    for (int j = 0; j < 32; j++) {
                        if (arg1 == stk_values[j])
                            prev_idx = j;
                    }
                    num_of_sym++;
                    if (prev_idx > 0) {
                        num_of_sym--;
                        idx_first_app[prev_idx] = i;
                    } else {
                        stk_values[num_of_sym-1] = arg1;
                        idx_first_app[num_of_sym-1] = i;
                    }

                }
            }

            prev_idx = -1;
            int type2 = -1; int arg2 = -1;
            if(opcode == 0 || opcode == 3 || opcode == 4) {
                offset_cpy += 2;
                type2 = bstr_to_int(parse_bits(bfunction, binfo, offset_cpy, 2));
                offset_cpy += update_offset(offset_cpy, type2); 
                if (type2 == 2 || type2 == 3) {
                    arg2 = bstr_to_int(parse_bits(bfunction, binfo, offset_cpy, 5));
                    for (int j = 0; j < 32; j++) {
                        if (arg2 == stk_values[j])
                            prev_idx = j;
                    }
                    num_of_sym++;
                    if (prev_idx > 0) {
                        num_of_sym--;
                        idx_first_app[prev_idx] = i;
                    } else {
                        stk_values[num_of_sym-1] = arg2;
                        idx_first_app[num_of_sym-1] = i;
                    }

                }
            }
        }

        // Reorder stack values in order of appearence
        for (int i = 0; i < num_of_sym; i++) {     
            for (int j = i+1; j < num_of_sym; j++) {     
                if(idx_first_app[i] > idx_first_app[j]) {    
                    int temp_idx = idx_first_app[i];    
                    idx_first_app[i] = idx_first_app[j];    
                    idx_first_app[j] = temp_idx;    

                    int temp_val = stk_values[i];
                    stk_values[i] = stk_values[j];    
                    stk_values[j] = temp_val;    
                }     
            }     
        }  

        // Parse instructions
        char* function = malloc((num_total_instr+1) * 32);
        strcpy(function, "");
        for (int i = 0; i < num_total_instr; i++) {
            offset += 3;
            int opcode = bstr_to_int(parse_bits(bfunction, binfo, offset, 3));
            char operation[32]; char arg1[16]; char arg2[16];

            int type1 = -1;
            if (opcode != 2) {
                offset += 2;
                type1 = bstr_to_int(parse_bits(bfunction, binfo, offset, 2));
                offset += update_offset(offset, type1);
                memmove(arg1, get_addr(bfunction, arg1, offset, type1), strlen(arg1));
            }
            
            int type2 = -1;
            if(opcode == 0 || opcode == 3 || opcode == 4) {
                offset += 2;
                type2 = bstr_to_int(parse_bits(bfunction, binfo, offset, 2));
                offset += update_offset(offset, type2); 
                memmove(arg2, get_addr(bfunction, arg2, offset, type2), strlen(arg2));
            }
    
            int offset_cpy = offset;
            for (int i = 1; i < num_total_instr; i++) {
                offset_cpy += 3; // opcode
            }
            switch (opcode) {
                case 0:
                    if (type1 == 0) {
                        printf("OPCODE 'MOV' destination can not be value typed\n");
                        exit(1);
                    }
                    strcpy(operation, "    MOV ");
                    break; 
                case 1:
                    strcpy(operation, "    CAL ");
                    break; 
                case 2:
                    strcpy(operation, "    RET");
                    break; 
                case 3:
                    strcpy(operation, "    REF ");
                    break; 
                case 4:
                    strcpy(operation, "    ADD ");
                    break; 
                case 5:
                    strcpy(operation, "    PRINT ");
                    break; 
                case 6:
                    strcpy(operation, "    NOT ");
                    break;
                case 7:
                    strcpy(operation, "    EQU ");
                    break; 
            }
            if (opcode != 2)
                strcat(operation, arg1);
            if(opcode == 0 || opcode == 3 || opcode == 4) {
                strcat(operation, " ");
                strcat(operation, arg2);
            }
            strcat(operation, "\n");

            prepend(function, operation);
        }
        offset += 3;
        int label_num = bstr_to_int(parse_bits(bfunction, binfo, offset, 3));
        char label_num_str[3] = {label_num + '0', '\n'};
        prepend(function, label_num_str);
        prepend(function, "FUNC LABEL ");

        prepend(output_str, function);
        free(function);
    }
    free(bfunction);
    printf("%s", output_str);
    free(output_str);

    return 0;
}

char* get_addr(char* bfunction, char* str, int offset, int type) {
    if (!bfunction || !str) {
        printf("Error: NULL argument is given in function 'get_addr'\n");
        exit(1);
    }

    char val_str[8];
    int val = 0;

    switch (type) {
        case 0:
            val = bstr_to_int(parse_bits(bfunction, str, offset, 8));
            strcpy(str, "VAL ");
            break;
        case 1:
            val = bstr_to_int(parse_bits(bfunction, str, offset, 3));
            strcpy(str, "REG ");
            break;
        case 2:
            val = bstr_to_int(parse_bits(bfunction, str, offset, 5));
            for (int i = 0; i < 32; i++) {
                if (val == stk_values[i]) {
                    char symbol_stk = symbols[(num_of_sym-1) - i];
                    snprintf(val_str, 3, "%c", symbol_stk);
                }
            }
            strcpy(str, "STK ");  
            break;
        case 3:
            val = bstr_to_int(parse_bits(bfunction, str, offset, 5));
            for (int i = 0; i < 32; i++) {
                if (val == stk_values[i]) {
                    char symbol_ptr = symbols[(num_of_sym-1) - i];
                    snprintf(val_str, 3, "%c", symbol_ptr);
                }
            }
            strcpy(str, "PTR "); 
            break;
        default:
            printf("Error: argument 'type' in function 'get_addr' must be >= 0 and <= 3\n");
            exit(1);
    }

    if (type == 0 || type == 1) {
        snprintf(val_str, 4, "%d", val);
    }
    strcat(str, val_str);
    return str;
}

void prepend(char* str, const char* str_to_prepend) {
    if (!str || !str_to_prepend) {
        printf("NULL argument is given in function 'prepend'\n");
        exit(1);
    }
    size_t len = strlen(str_to_prepend);
    memmove(str + len, str, strlen(str) + 1);
    memcpy(str, str_to_prepend, len);
}