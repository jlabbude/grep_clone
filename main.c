#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    char** splitees;
    int splitees_count;
    char** splits;
    int splits_count;
} Splits;

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
    int cap_spltee = 4;
    int cap_splt = 4;
    int count_spltee = 0;
    int count_splt = 0;
    char** splittees = malloc(cap_spltee * sizeof(char*));
    char** splits = malloc(cap_splt * sizeof(char*));

    if (!splittees || !splits) {
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
                append(&splittees, &count_spltee, &cap_spltee, word);
            }
            const char split[2] = {filestr[i], '\0'};
            append(&splits, &count_splt, &cap_splt, split);
            start = i + 1;
        }
    }

    if (start < strlen(filestr)) {
        append(&splittees, &count_spltee, &cap_spltee, &filestr[start]);
    }

    const Splits result = {splittees, count_spltee, splits, count_splt};
    return result;
}

void free_splits(const Splits* spli) {
    for (int i = 0; i < spli->splitees_count; i++) {
        free(spli->splitees[i]);
    }
    free(spli->splitees);

    for (int i = 0; i < spli->splits_count; i++) {
        free(spli->splits[i]);
    }
    free(spli->splits);
}

char** find(const char* match, const Splits split, int* matches_count) {
    int cap = 4;
    int count = 0;
    char** matches = malloc(cap * sizeof(char*));
    if (!matches) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    for (int i = 0; i < split.splitees_count; i++) {
        if (strcmp(split.splitees[i], match) == 0) {
            append(&matches, &count, &cap, split.splitees[i]);
        }
    }

    for (int i = 0; i < split.splits_count; i++) {
        if (strcmp(split.splits[i], match) == 0) {
            append(&matches, &count, &cap, split.splits[i]);
        }
    }

    *matches_count = count;
    return matches;
}

int main(const int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Invalid arguments\n");
        return 1;
    }
    const char* seekout = argv[2];
    char* filestr = read_to_str(argv[1]);
    if (!filestr) return 1;
    const Splits spli = split(filestr);
    int match_count = 0;
    char** matches = find(seekout, spli, &match_count);
    printf("Matches: ");
    for (int i  = 0; i < match_count; i++) {
        printf("\033[31m%s\033[0m ",matches[i]);
    }
    printf("\nMatch count: %d", match_count);

    free_splits(&spli);
    free(filestr);
    return 0;
}
