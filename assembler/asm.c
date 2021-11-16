/*2-pass assembler
 * pass 1: collect labels and store memory addresses through:
 *   a) processing of directives (e.g. .org, etc)
 *   b) collecting labels and assigning memory addresses
 * pass 2: assemble into machine code through:
 *   b) substitution of labels as memory addresses
 *   c) substitution of mnemonics as machine instructions
 *   d) format as binary file (e.g. removal of label/directive column, spaces and newlines
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char** labels;
    int* addresses;
} TABLE;

long int file_size(FILE* fp) {
    fseek(fp, 0L, SEEK_END); /*find file size*/
    long int filesize = ftell(fp);

    fseek(fp, 0L, SEEK_SET);

    return filesize;
}

void* read_file(char* filename) {
    /*read file into memory*/
    FILE* fp = fopen(filename, "rb"); /*open file*/
    if(fp == NULL) {
        fprintf(stderr, "could not open file %s\n", filename);
        return NULL;
    }

    long int filesize = file_size(fp);
    fprintf(stdout, "source code file size: %ld\n", filesize);

    fclose(fp); /*file loaded into memory, no longer needed*/
}

void collect_labels(void* source, TABLE labels) {
    /*collect the labels and match them to memory addresses*/

    return;
}

void substitute_labels(void* source, TABLE labels) {
    return;
}

void process_mnemonics(void* source) {
    return;
}

void format_file_data(void* source) {
    return;
}

void output_file_data(void* source, char* filename) {
    FILE* fp = fopen(filename, "wb+");
    /*copy assembled data into file*/
    long int filesize = file_size(fp);
    fprintf(stdout, "output file size: %ld\n", filesize);
    fclose(fp);

    return;
}

int main(int argc, char* argv[]) {
    /*check input*/
    if(argc!=3) {fprintf(stderr, "usage: asm -<input file> -<output file>\n"); return 1;}

    /*pass 1: read source file into memory, process directives, collect labels*/
    fprintf(stdout, "loading source file %s\n", argv[1]);
    void* source_code = read_file(argv[1]); /*read source*/

    fprintf(stdout, "generating table\n");
    TABLE labels; /*a table for matching labels with addresses*/
    collect_labels(source_code, labels);

    /*pass 2: substitute labels as addresses, process mnemonics into instructions*/
    fprintf(stdout, "assembling source\n");
    substitute_labels(source_code, labels);
    process_mnemonics(source_code);
    format_file_data(source_code);
       
    /*output assembled binary*/
    output_file_data(source_code, argv[2]);

    return 0;
}

