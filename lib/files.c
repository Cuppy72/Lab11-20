#include <stdio.h>
#include <string.h>

#include "files.h"

#define BUFF_SIZE 1024

#define SUCCESS 10
#define FILE_OPEN_ERROR 13
#define FILE_CLOSE_ERROR 14
#define WRITE_FILE_ERROR 15


int safety_close_files(FILE *file_1, FILE *file_2)
{
	int status_code_1, status_code_2;

	status_code_1 = fclose(file_1);
	status_code_2 = fclose(file_2);

	if(status_code_1 || status_code_2)
		return FILE_CLOSE_ERROR;

	return SUCCESS;
}

static char *pick_filename(char *file_path)
{
	char *name = strrchr(file_path, '/');

	if(name == NULL)
		name = file_path;
	else
		name++;

	return name;
}

int equal_filenames(char *file_path_1, char *file_path_2)
{
	char *filename_1 = pick_filename(file_path_1);
	char *filename_2 = pick_filename(file_path_2);

	if(strcmp(filename_1, filename_2) == 0)
		return 1;

	return 0;
}

int validate_open_files(FILE *file_1, FILE *file_2)
{
	int status_code;

	if(file_1 == NULL && file_2 != NULL) {
		status_code = fclose(file_2);

		if(status_code)
			return FILE_CLOSE_ERROR;

		return FILE_OPEN_ERROR;

	} else if(file_1 != NULL && file_2 == NULL) {
		status_code = fclose(file_1);

		if(status_code)
			return FILE_CLOSE_ERROR;

		return FILE_OPEN_ERROR;

	} else if(file_1 == NULL && file_2 == NULL)
		return FILE_OPEN_ERROR;

	return SUCCESS;
}

int file_cpy(FILE *source, FILE *destination)
{
	int err;

	err = validate_open_files(source, destination);
	if(err != SUCCESS)
		return err;

	char buff[BUFF_SIZE];
	int bytes_read, bytes_write;

	while((bytes_read = fread(buff, sizeof(char), BUFF_SIZE, source)) > 0) {
		bytes_write = fwrite(buff, sizeof(char), bytes_read, destination);

		if(bytes_read != bytes_write) {
			err = safety_close_files(source, destination);
			if(err != SUCCESS)
				return err;

			return WRITE_FILE_ERROR;
		}
	}

	return SUCCESS;
}

