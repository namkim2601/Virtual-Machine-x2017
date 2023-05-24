#ifndef PARSER_H_   
#define PARSER_H_

char* parse_bits(char* bfunction, char* bstr, int offset, int size);
int parse_addr(char* bfunction, char* str, int offset, int type);
int bstr_to_int(const char* bstr);
int update_offset(int offset, int type);

#endif
