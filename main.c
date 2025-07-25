#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

ssize_t my_getline(char **lineptr, size_t *n, FILE *stream) {
    size_t pos = 0;
    int c;

    if (*lineptr == NULL || *n == 0) {
        *n = 128;
        *lineptr = malloc(*n);
        if (*lineptr == NULL) return -1;
    }

    while ((c = fgetc(stream)) != EOF) {
        if (pos + 1 >= *n) {
            *n *= 2;
            char *new_ptr = realloc(*lineptr, *n);
            if (!new_ptr) return -1;
            *lineptr = new_ptr;
        }

        (*lineptr)[pos++] = c;

        if (c == '\n') break;
    }

    if (pos == 0 && c == EOF) return -1;

    (*lineptr)[pos] = '\0';
    return pos;
}

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum {
    META_COMMAND_EXIT,
    META_COMMAND_HELP,
    META_COMMAND_UNRECOGNIZED_COMMAND,
} MetaCommandResult;

InputBuffer *new_input_buffer() {
    InputBuffer *input_buffer = malloc(sizeof(InputBuffer));
    if(input_buffer == NULL) {
        perror("InputBuffer memory allocation failed");
        return NULL;
    }
    
    input_buffer->buffer = NULL;
    input_buffer->buffer_length = 0;
    input_buffer->input_length = 0;
    return input_buffer;
}

void print_prompt() {
    printf("db > ");
}

void read_input(InputBuffer *input_buffer) {
    ssize_t bytes_read = my_getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);
    if (bytes_read <= 0) {
        perror("Error reading input");
        return;
    }
    if(input_buffer->buffer[bytes_read - 1] == '\n') {
        input_buffer->input_length = bytes_read - 1; // Exclude newline character
        input_buffer->buffer[bytes_read - 1] = '\0'; // Remove newline character
    } else {
        input_buffer->input_length = bytes_read; // No newline character
    }

}

MetaCommandResult do_meta_command(InputBuffer *input_buffer) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        return META_COMMAND_EXIT;
    } else if (strcmp(input_buffer->buffer, ".help") == 0) {
        return META_COMMAND_HELP;
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

void close_input_buffer(InputBuffer *input_buffer) {
    if (input_buffer) {
        free(input_buffer->buffer);
        free(input_buffer);
    }
}

int main( int argc, char *argv[] ) {
    
    InputBuffer *input_buffer = new_input_buffer();

    while (1){
        print_prompt();
        read_input(input_buffer);

        switch (do_meta_command(input_buffer)) {
            case META_COMMAND_EXIT:
                close_input_buffer(input_buffer);
                return 0;
            case META_COMMAND_HELP:
                printf("Available commands:\n");
                printf(".exit - Exit the program\n");
                printf(".help - Show this help message\n");
                break;
            case META_COMMAND_UNRECOGNIZED_COMMAND:
                printf("Unrecognized command: %s\n, type .help for a list of commands", input_buffer->buffer);
                break;
        }
    }
}