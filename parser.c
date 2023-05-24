#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

char* parse_bits(char* bfunc, char* bstr, int offset, int size) {
    if (!bfunc || !bstr) {
        printf("Error: NULL argument is given in function 'parse_bits'\n");
        exit(1);
    }

    for (int i = 0; i < size; i++)
        bstr[i] = bfunc[strlen(bfunc) - offset + i];
    bstr[size] = '\0';
    return bstr;
}

int parse_addr(char* bfunc, char* str, int offset, int type) {
    if (!bfunc || !str) {
        printf("Error: NULL argument is given in function 'parse_addr'\n");
        exit(1);
    }

    int val;
    switch (type) {
        case 0:
            val = bstr_to_int(parse_bits(bfunc, str, offset, 8));
            return val;
        case 1:
            val = bstr_to_int(parse_bits(bfunc, str, offset, 3));
            return val;
        case 2:
            val = bstr_to_int(parse_bits(bfunc, str, offset, 5));
            return val;
        case 3:
            val = bstr_to_int(parse_bits(bfunc, str, offset, 5));
            return val;
        default:
            printf("Error: Illegal argument type in binary file\n");
            exit(1);
    }
}

int bstr_to_int(const char* bstr) {
    if (!bstr) {
        printf("Error: NULL argument is given in function 'bstr_to_int'\n");
        exit(1);
    }

    int val = 0;
    while (*bstr != '\0')
        val = 2 * val + (*bstr++ - '0');
    return val;
}

int update_offset(int offset, int type) {
    switch (type) {
        case 0:
            return 8;
        case 1:
            return 3;
        case 2:
            return 5;
        case 3:
            return 5;
        default: 
            printf("Error: Illegal argument type in binary file\n");
            exit(1);
    }
}
