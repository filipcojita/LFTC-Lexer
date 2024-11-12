#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "utils.h"
#include "parser.h"

int main(int argc, char *argv[]) {
    // Ensure an input file is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Load the input file into memory
    char *fileContent = loadFile(argv[1]);
    if (!fileContent) {  // Check if file loading was successful
        fprintf(stderr, "Error loading file %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    // Tokenize the input file content
    tokenize(fileContent);

    // Display the tokens (for debugging purposes)
    showTokens();

    // Parse the tokens
    parse();  // parse() will handle syntax errors internally and exit if an error occurs

    // Free the allocated memory for the file content
    free(fileContent);
    
    return EXIT_SUCCESS;
}
