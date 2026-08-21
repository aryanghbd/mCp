// code to parse and deal with JSON-RPC messages, including reading from stdin and writing to stdout, as well as logging to stderr.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#define CHUNK_SIZE 1024 // read 1kb at a time lol
#define MAX_METHOD_NAME_LENGTH 256


bool validate_jsonrpc_request(const char *jsonrpc_message) {
    // check if the message starts with '{' and ends with '}'
    size_t len = strlen(jsonrpc_message);
    if (len < 2 || jsonrpc_message[0] != '{' || jsonrpc_message[len - 1] != '}') {
        // bad shape.
        return false;
    }

    // we need the following 3 fields: jsonrpc, method and id.
    if (strstr(jsonrpc_message, "\"jsonrpc\"") == NULL ||
        strstr(jsonrpc_message, "\"method\"") == NULL ||
        strstr(jsonrpc_message, "\"id\"") == NULL) {
        return false;
    }

    // the jsonrpc field must be equal to the value "2.0"
    const char *jsonrpc_field = strstr(jsonrpc_message, "\"jsonrpc\"");
    if (jsonrpc_field != NULL) {
        //assuming not null, go to the actual value.
        const char *value_start = strchr(jsonrpc_field, ':');
        if (value_start != NULL) {
            value_start++; // move past the colon
            while (*value_start == ' ' || *value_start == '\t') {
                value_start++; // skip whitespace
            }
            if (strncmp(value_start, "\"2.0\"", 5) != 0) {
                // surface error?
                perror("Invalid JSON-RPC version");
                return false;
                // return some kind of err
            }
        }
        else {
            perror("Invalid JSON-RPC message: missing value for jsonrpc field");
            return false;
        }
    }
    else {
        perror("Invalid JSON-RPC message: missing jsonrpc field");
        return false;
    }

    // next we need to make sure that the method field is a string, and the id field is either a string or a number

    const char *method_field = strstr(jsonrpc_message, "\"method\"");
    if (method_field != NULL) {
        const char *value_start = strchr(method_field, ':');
        if (value_start != NULL) {
            // get over the colon and whitespace
            value_start++;
            while (*value_start == ' ' || *value_start == '\t') {
                value_start++; // get to the meat and potatoes
            }

            // we've gotten to the value, it should start with a double quote and end with a double quote

            if (*value_start != '"') {
                perror("Invalid JSON-RPC message: method field is not a string");
                return false;
            }
            // otherwise, read char by char until we find the closing double quote, making sure to handle escaped quotes properly
            value_start++; 

            // Adhere to a max length for a method name to prevent buffer overflow, that way we either hit the limit or hit the closing quote
            while (*value_start != '"' && (value_start - method_field) < MAX_METHOD_NAME_LENGTH) {
                if (*value_start == '\\') {
                    // skip the next character, since it's escaped
                    value_start++;
                }
                value_start++;
            }
        }
        else {
            perror("Invalid JSON-RPC message: missing value for method field");
            return false;
        }
    }
    else {
        perror("Invalid JSON-RPC message: missing method field");
        return false; 
    }

    return true;
}

char *read_jsonrpc_message() {
    // allocate a 1KB buffer to read the message info
    char *buffer = malloc(CHUNK_SIZE);
    
    // read one 
    size_t bytes_read = fread(buffer, 1, CHUNK_SIZE - 1, stdin);
    buffer[bytes_read] = '\0'; // null-terminate the string to use as C string later.

    
    // if we read 0 bytes, either EOF or error, return NULL to indicate failure, since errors go to stderr anyway.
    if (bytes_read == 0) {
        free(buffer);
        return NULL;
    }

    //otherwise we're in business, proceed to parse the JSON-RPC message

    //first we need to validate the validity of the structure

    // json-rpc REQUESTS obey this format: {"jsonrpc": "2.0", "method": "method_name", "params": { ... }, "id": 1}, validate shape
    return buffer;
}

int main(void) {
    char *message = read_jsonrpc_message();
    if (message) {
        printf("Read JSON-RPC message: %s\n", message);
        free(message);
    } else {
        // specify error too
        fprintf(stderr, "Failed to read JSON-RPC message: %s\n", strerror(errno));
    }
    return 0;
}
