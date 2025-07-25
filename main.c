#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

typedef struct {
    int id;
    char username[COLUMN_USERNAME_SIZE];
    char email[COLUMN_EMAIL_SIZE];
} Row;

typedef struct {
    char *buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_HELP,
    META_COMMAND_UNRECOGNIZED_COMMAND,
} MetaCommandResult;

typedef enum {
    STATEMENT_SELECT,
    STATEMENT_INSERT,
    STATEMENT_UPDATE,
    STATEMENT_DELETE,
} StatementType;

typedef struct {
    StatementType type; // Placeholder for statement type
    Row row_to_insert; // Placeholder for row to insert
} Statement;

typedef enum {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
} PrepareResult;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

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

PrepareResult prepare_statement(InputBuffer *input_buffer, Statement *statement) {
    
    if(strncmp(input_buffer->buffer, "select", 6) == 0) {
        int args = sscanf(input_buffer->buffer, "select %d %s %s", &(statement->row_to_insert.id), statement->row_to_insert.username, statement->row_to_insert.email);
        if (args < 3) {
            return PREPARE_SYNTAX_ERROR;
        }
        statement->type = STATEMENT_SELECT;
        return PREPARE_SUCCESS;
    } else if (strncmp(input_buffer->buffer, "insert", 6) == 0) {
        statement->type = STATEMENT_INSERT;
        return PREPARE_SUCCESS;
    } else if (strncmp(input_buffer->buffer, "update", 6) == 0) {
        statement->type = STATEMENT_UPDATE;
        return PREPARE_SUCCESS;
    } else if (strncmp(input_buffer->buffer, "delete", 6) == 0) {
        statement->type = STATEMENT_DELETE;
        return PREPARE_SUCCESS;
    } else {
        return PREPARE_UNRECOGNIZED_STATEMENT;
    }
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

void close_input_buffer(InputBuffer *input_buffer) {
    if (input_buffer) {
        free(input_buffer->buffer);
        free(input_buffer);
    }
}

void execute_statement(Statement* statement) {
  switch (statement->type) {
    case (STATEMENT_INSERT):
      printf("This is where we would do an insert.\n");
      break;
    case (STATEMENT_SELECT):
      printf("This is where we would do a select.\n");
      break;
  }
}

MetaCommandResult do_meta_command(InputBuffer *input_buffer) {
    if (strcmp(input_buffer->buffer, ".exit") == 0) {
        close_input_buffer(input_buffer);
        exit(EXIT_SUCCESS);
    } else if (strcmp(input_buffer->buffer, ".help") == 0) {
        return META_COMMAND_HELP;
    } else {
        return META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}


int main( int argc, char *argv[] ) {
    
    InputBuffer *input_buffer = new_input_buffer();

    while (1){
        print_prompt();
        read_input(input_buffer);

        if (input_buffer->buffer[0] == '.'){
            switch (do_meta_command(input_buffer)) {
                case META_COMMAND_SUCCESS:
                    continue;
                case META_COMMAND_HELP:
                    printf("Available commands:\n");
                    printf(".exit - Exit the program\n");
                    printf(".help - Show this help message\n");
                    continue;
                case META_COMMAND_UNRECOGNIZED_COMMAND:
                    printf("Unrecognized command: %s, type .help for a list of commands.\n", input_buffer->buffer);
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, &statement)) {
            case PREPARE_SUCCESS:
                break;
            case PREPARE_UNRECOGNIZED_STATEMENT:
                printf("Unrecognized keyword at start of '%s'.\n", input_buffer->buffer);
                continue;
            case PREPARE_SYNTAX_ERROR:
                printf("Syntax error. Could not parse statement: '%s'.\n", input_buffer->buffer);
                continue;
        }

        printf("Executed successfully\n");
    }
}