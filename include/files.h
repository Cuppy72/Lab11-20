#ifndef __FILES__
#define __FILES__

#include <stdio.h>

#define BUFF_SIZE 1024

#define SUCCESS 10
#define FILE_OPEN_ERROR 13
#define FILE_CLOSE_ERROR 14
#define WRITE_FILE_ERROR 15

int equal_filenames(char *file_path_1, char *file_path_2);

int validate_open_files(FILE *file_1, FILE *file_2);
int safety_close_files(FILE *file_1, FILE *file_2);

int file_cpy(FILE *source, FILE *destination);

#endif
