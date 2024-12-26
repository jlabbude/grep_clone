#ifndef GREP_H
#define GREP_H

typedef struct splits {
    char** split_vec;
    int cap;
    int size;
} Splits;

void append(char*** array, int* count, int* capacity, const char* val);

Splits regex_tokenizer(char* re_arg);

#endif