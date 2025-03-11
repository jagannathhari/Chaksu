#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <stddef.h>
#include <stdbool.h>

typedef enum
{
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING
} config__ValueType;

typedef struct
{
    union {
        long int_value;
        double double_value;
        int string_value; // contain offset in string pool.
    } value;

    config__ValueType value_type;

} config__Value;

typedef struct
{
    int key;
    config__Value data;
} config__KeyValue;

typedef struct
{
    char *config__memory;
    config__KeyValue *config__data;
} Config;

void config_free(Config *config);
bool config_get_int(Config *config,const char *key, int* res);
bool config_get_decimal(Config *config,const char *key, double* res);
bool config_get_string(Config *config,const char *key, char** res);
Config* config_from_memory(const char *buffer, size_t buffer_len);
Config* config_from_file(const char *file);


#endif //CONFIG_PARSER_H

#ifdef IMPLEMENT_CONFIG_PARSER

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMPLEMENT_VECTOR
#include "./vector.h"

typedef struct
{
    const char *buffer;
    long buffer_len;
    long lines;
    long start;
    long pos;

} config__ScannerState;

typedef enum
{
    TOKEN_KEY,
    TOKEN_VAL_INT,
    TOKEN_VAL_FLOAT,
    TOKEN_VAL_STRING,
    TOKEN_ASSIGNMENT,
    TOKEN_FILE_END,
    TOKEN_UNKNOWN,
} config__TokenType;

typedef struct
{
    int lexeme; // offset of string
    config__TokenType type;
} config__Token;

typedef struct
{
    config__Token current_token;
    config__ScannerState *ss;
} config__Parser;

void config_free(Config *config)
{
    free_vector(config->config__memory);
    free_vector(config->config__data);
    free(config);
}

static int config__substr(char **memory, const char *source, int start, int end)
{
    if (memory == NULL)
        return -1;

    int substr_len = end - start;

    *memory = vector_ensure_capacity(*memory, substr_len + 1);

    const int memory_location = vector_length(*memory);

    strncpy(*memory + memory_location, &source[start], substr_len);

    vector_header(*memory)->length += substr_len;
    vector_append(*memory, '\0');

    return memory_location;
}

static bool config__is_at_end(config__ScannerState *ss)
{
    return ss->pos >= ss->buffer_len;
}

static char config__advance(config__ScannerState *ss)
{
    if (config__is_at_end(ss))
        return '\0';

    return ss->buffer[ss->pos++];
}

static char config__peek(config__ScannerState *ss)
{
    return config__is_at_end(ss) ? '\0' : ss->buffer[ss->pos];
}

static char config__peek_next(config__ScannerState *ss)
{
    if (ss->pos + 1 < ss->buffer_len)
        return ss->buffer[ss->pos + 1];
    return '\0';
}

static void config__eat_line(config__ScannerState *ss)
{
    while (!config__is_at_end(ss) && ss->buffer[ss->pos] != '\n')
        ss->pos++;
    ss->start = ss->pos;
}

static char* config__read_whole_file(const char *file, long int *bytes)
{
    FILE *f = fopen(file, "r");

    if (!f)
        return NULL;

    char *file_data = NULL;
    size_t file_size = 0;

    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (!file_size)
        goto error;

    file_data = malloc(sizeof(*file_data) * file_size + 1);

    if (!file_data)
        goto error;

    if (fread(file_data, 1, file_size, f) != file_size)
        goto error;

    file_data[file_size] = '\0';
    *bytes = file_size;

    fclose(f);
    return file_data;

error:
    fclose(f);
    return NULL;
}

static config__Token config__string(char **memory, config__ScannerState *ss)
{
    config__Token token = {.type = TOKEN_UNKNOWN};
    while (!config__is_at_end(ss) && config__peek(ss) != '"')
        config__advance(ss);

    if (config__is_at_end(ss))
    {
        puts("Invalid string");
        return token;
    }

    config__advance(ss); // consume "
    token =
        (config__Token){.type = TOKEN_VAL_STRING,
                        .lexeme = config__substr(memory, ss->buffer,
                                                 ss->start + 1, ss->pos - 1)};
    ss->start = ss->pos;
    return token;
}

static config__Token config__identifyer(char **memory, config__ScannerState *ss)
{
    while (isalnum(config__peek(ss)) || config__peek(ss) == '_')
        config__advance(ss);
    config__Token token = {
        .type = TOKEN_KEY,
        .lexeme = config__substr(memory, ss->buffer, ss->start, ss->pos)};
    ss->start = ss->pos;
    return token;
}

static config__Token config__assignement(char **memory,
                                         config__ScannerState *ss)
{
    config__Token token = {
        .type = TOKEN_ASSIGNMENT,
        .lexeme = config__substr(memory, ss->buffer, ss->start, ss->pos)};

    ss->start = ss->pos;

    return token;
}

static config__Token config__number(char **memory, config__ScannerState *ss)
{
    bool is_float = false;
    while (isdigit(config__peek(ss)))
        config__advance(ss);

    if (config__peek(ss) == '.')
    {
        is_float = true;
        config__advance(ss); // consume .
        while (isdigit(config__peek(ss)))
            config__advance(ss);
    }
    config__Token token = {
        .type = is_float ? TOKEN_VAL_FLOAT : TOKEN_VAL_INT,
        .lexeme = config__substr(memory, ss->buffer, ss->start, ss->pos)};

    return token;
}

static void config__print_token(char **memory, config__Token token)
{
    char *token_str[] = {"TOKEN_KEY",        "TOKEN_VAL_INT",
                         "TOKEN_VAL_FLOAT",  "TOKEN_VAL_STRING",
                         "TOKEN_ASSIGNMENT", "TOKEN_FILE_END",
                         "TOKEN_UNKNOWN"};

    printf("Token(%s,%s)\n", token_str[token.type], *memory + token.lexeme);
}

// config : key = value
static config__Token config__next_token(char **memory, config__ScannerState *ss)
{
    while (!config__is_at_end(ss))
    {
        char c = config__advance(ss);
        switch (c)
        {
        case '#':
            config__eat_line(ss);
            break;
        case ' ':
        case '\t':
        case '\r':
            ss->start = ss->pos;
            break;
        case '\n': {
            ss->lines++;
            ss->start = ss->pos;
        }
        break;
        case '=':
            return config__assignement(memory, ss);
        case '"':
            return config__string(memory, ss);
        default:
            if (isalpha(c))
            {
                return config__identifyer(memory, ss);
            }
            else if (isdigit(c))
            {
                return config__number(memory, ss);
            }
            goto error;
        }
    }

error:
    if (config__is_at_end(ss))
        return (config__Token){.type = TOKEN_FILE_END, .lexeme = 0};
    return (config__Token){.type = TOKEN_UNKNOWN, .lexeme = 0};
}

static void config__eat(char **memory, config__Parser *parser,
                        config__TokenType to_eat)
{
    if (parser->current_token.type != to_eat)
    {
        parser->current_token =
            (config__Token){.type = TOKEN_UNKNOWN, .lexeme = 0};
        return;
    }
    parser->current_token = config__next_token(memory, parser->ss);
}

static config__Value config__value(char **memory, config__Parser *parser)
{
    config__Value value = {0};
    switch (parser->current_token.type)
    {
    case TOKEN_VAL_STRING: {
        value =
            (config__Value){.value_type = VALUE_STRING,
                            .value.string_value = parser->current_token.lexeme};

        config__eat(memory, parser, TOKEN_VAL_STRING);
        return value;
    }
    break;

    case TOKEN_VAL_INT: {
        int parsed = strtol(*memory + parser->current_token.lexeme, NULL, 10);
        value =
            (config__Value){.value_type = VALUE_INT, .value.int_value = parsed};
        config__eat(memory, parser, TOKEN_VAL_INT);
    }
    break;

    case TOKEN_VAL_FLOAT: {
        float x = strtof(*memory + parser->current_token.lexeme, NULL);
        value =
            (config__Value){.value_type = VALUE_FLOAT, .value.double_value = x};
        config__eat(memory, parser, TOKEN_VAL_FLOAT);
    }
    default:
        break;
    }

    return value;
}

static bool config__get_value(const char *key, const config__ValueType required_type,
                             Config *config, config__Value *result)
{
    if (!key || !result || !config)
        return false;

    size_t len = vector_length(config->config__data);
    const char* memory = config->config__memory;
    for (size_t i = 0; i < len; i++)
    {
        const config__ValueType value_type = config->config__data[i].data.value_type;
        const char *key_tmp = &memory[config->config__data[i].key];
        if(value_type == required_type && strcmp(key, key_tmp) == 0)
        {
            result->value = config->config__data[i].data.value;
            result->value_type = required_type;
            return true;
        }
    }
    return false;
}

bool config_get_int(Config *config,const char *key, int* res)
{
    if (!config)
        return false;

    config__Value  data = {0};
    if(config__get_value(key, VALUE_INT, config, &data))
    {
        *res = data.value.int_value;
        return true;
    }
    return false;
}

bool config_get_decimal(Config *config,const char *key, double* res)
{
    if (!config)
        return false;

    config__Value  data = {0};
    if(config__get_value(key, VALUE_FLOAT, config, &data))
    {
        *res = data.value.double_value;
        return true;
    }

    return false;
 }

bool config_get_string(Config *config,const char *key, char** res)
{
    if (!config)
        return false;

    config__Value  data = {0};
    char *memory = config->config__memory;
    if(config__get_value(key, VALUE_STRING, config, &data))
    {
        *res = &memory[data.value.string_value];
        return true;
    }

    return false;
}

static Config* config__config__helper(const char *buffer, size_t buffer_len)
{
    // expr : key = value;
    // value : string | float | int

    if (!buffer || !buffer_len)
        return NULL;

    config__ScannerState ss = {0};
    config__Parser parser = {0};
    Config *config = calloc(1, sizeof(*config));

    if (!config)
        return NULL;

    config->config__memory = Vector(char);

    if (!config->config__memory)
        return NULL;

    vector_append(config->config__memory, '\0'); // fill first pos with null;
    config__KeyValue *res = Vector(*res);

    if (!res)
        return NULL;

    ss.buffer = buffer;
    ss.buffer_len = buffer_len;

    parser.ss = &ss;
    parser.current_token = config__next_token(&config->config__memory, &ss);

    while (parser.current_token.type != TOKEN_FILE_END)
    {
        switch (parser.current_token.type)
        {
        case TOKEN_KEY: {
            config__KeyValue tmp = {.key = parser.current_token.lexeme};
            config__eat(&config->config__memory, &parser, TOKEN_KEY);
            config__eat(&config->config__memory, &parser, TOKEN_ASSIGNMENT);
            tmp.data = config__value(&config->config__memory, &parser);
            vector_append(res, tmp);
        }
        break;

        default: {
            // TODO: need to do something.
        }
        break;
        }
    }

    config->config__data = res;

    return config;
}

Config* config_from_memory(const char *buffer, size_t buffer_len)
{
    return config__config__helper(buffer, buffer_len);
}

Config* config_from_file(const char *file)
{

    if (!file)
        return NULL;
    long int buffer_len = 0;
    char *buffer = config__read_whole_file(file, &buffer_len);
    Config *config = config_from_memory(buffer, buffer_len);
    free(buffer);
    return config;
}

#endif // IMPLEMENT_CONFIG_PARSER
