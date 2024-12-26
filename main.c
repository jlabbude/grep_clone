#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "grep.h"

char* read_to_str(const char* filename) {
    char* buff = NULL;
    FILE* file = fopen(filename, "rb");
    if (file) {
        fseek(file, 0, SEEK_END);
        const long len = ftell(file);
        fseek(file, 0, SEEK_SET);
        buff = malloc(len + 1);
        if (buff) {
            const size_t res = fread(buff, 1, len, file);
            if (res != len) {
                fprintf(stderr, "File read error\n");
                free(buff);
                fclose(file);
                return NULL;
            }
            buff[len] = '\0';
        }
        fclose(file);
    } else {
        fprintf(stderr, "Unable to open file\n");
        return NULL;
    }
    return buff;
}

void append(char*** array, int* count, int* capacity, const char* val) {
    if (*count == *capacity) {
        *capacity *= 2;
        *array = realloc(*array, *capacity * sizeof(char*));
        if (*array == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }
    (*array)[*count] = strdup(val);
    if ((*array)[*count] == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    (*count)++;
}

bool is_char_split(const char val) {
    return val == ' ' || val == '\n' || val == '\t';
}

Splits split(const char* filestr) {
    int size = 0;
    int cap = 4;
    char** splits = malloc(cap * sizeof(char*));

    if (!splits) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    int start = 0;
    for (int i = 0; filestr[i] != '\0'; i++) {
        if (is_char_split(filestr[i])) {
            if (start < i) {
                char word[i - start + 1];
                strncpy(word, &filestr[start], i - start);
                word[i - start] = '\0';
                append(&splits, &size, &cap, word);
            }
            const char split[2] = {filestr[i], '\0'};
            append(&splits, &size, &cap, split);
            start = i + 1;
        }
    }

    if (start < strlen(filestr)) {
        append(&splits, &size, &cap, &filestr[start]);
    }

    return (Splits) {splits, cap, size};
}

void free_splits(const Splits* spli) {
    for (int i = 0; i < spli->size; i++) {
        free(spli->split_vec[i]);
    }
    free(spli->split_vec);
}

char** find(const char* match, const Splits split, int* matches_count) {
    int cap = 4;
    int count = 0;
    char** matches = malloc(cap * sizeof(char*));
    if (!matches) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < split.size; i++) {
        if (strcmp(split.split_vec[i], match) == 0) {
            int line_start = i;
            int line_end = i;
            while (line_start > 0 && strcmp(split.split_vec[line_start - 1], "\n") != 0) {
                line_start--;
            }
            while (line_end < split.size - 1 && strcmp(split.split_vec[line_end], "\n") != 0) {
                line_end++;
            }
            size_t line_length = 0;
            for (int j = line_start; j <= line_end; j++) {
                line_length += strlen(split.split_vec[j]) + 1;
            }
            char* line_aggregation = malloc(line_length + 1);
            if (!line_aggregation) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            line_aggregation[0] = '\0';
            for (int j = line_start; j <= line_end; j++) {
                strcat(line_aggregation, split.split_vec[j]);
            }
            append(&matches, &count, &cap, line_aggregation);
            free(line_aggregation);
            i = line_end;
        }
    }

    *matches_count = count;
    return matches;
}

void print_line_matches(char** matches, const char* seekout, const int match_count) {
    printf("Matches:\n");
    for (int i = 0; i < match_count; i++) {
        const Splits matchsplt = split(matches[i]);
        for (int j = 0; j < matchsplt.size; j++) {
            char* match = matchsplt.split_vec[j];
            strcmp(match, seekout) == 0 ? printf("\033[0;31m%s\033[0m", match) : printf("%s", match);
        }
        free(matches[i]);
    }
    free(matches);
}

void print_matches(char** matches, const char* seekout, const int match_count) {
    int count = 0;
    for (int i = 0; i < match_count; ++i) {
        const Splits matchsplt = split(matches[i]);
        for (int j = 0; j < matchsplt.size; ++j) {
            const char* match = matchsplt.split_vec[j];
            if (strcmp(match, seekout) == 0) count++;
        }
    }
    printf("Match count: %d", count);
}

void print_help() {
    static const char help[] =
        "Usage: untitled [FILE] [OPTION]\n"
        "No flag: Print amount of matches found in file.\n"
        "\nOptions:\n"
        "  -h | --help    Display this help message.\n"
        "  -l | --lines   Display the program version.\n";
    printf("%s", help);
}

int main(const int argc, char* argv[]) {
    const char* seekout = 0;
    const char* filename = 0;
    if (argc >= 3) {
        filename = argv[1];
        seekout = argv[2];
    } else if (argc >= 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
        print_help();
        return 1;
    } else {
        fprintf(stderr, "Please input an input file and a string to search\n Use the flag -h or --help to see usage.");
        return 1;
    }

    char* filestr = read_to_str(filename);
    if (!filestr) return 1;

    const Splits spli = split(filestr);

    int match_count = 0;
    char** matches = find(seekout, spli, &match_count);

    if (argc >= 4) {
        const char* arg = argv[3];
        if (strcmp(arg, "-l") == 0 || strcmp(arg, "--lines") == 0) {
            print_line_matches(matches, seekout, match_count);
            return 0;
        } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
            print_help();
            return 0;
        } else {
            fprintf(stderr, "Unknown flag. -h or --help for help");
            return 1;
        }
    } else if (argc == 3) print_matches(matches, seekout, match_count);

    regex_tokenizer(0);

    free_splits(&spli);
    free(filestr);
    return 0;
}
