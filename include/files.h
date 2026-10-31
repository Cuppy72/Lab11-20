#ifndef __FILES__
#define __FILES__

#include <stdio.h>

#define BUFF_SIZE 1024

int equal_filenames(char *file_path_1, char *file_path_2, int *result);

int validate_open_files(FILE *file_1, FILE *file_2);
int safety_close_files(FILE *file_1, FILE *file_2);

int file_cpy(FILE *source, FILE *destination);

#endif
