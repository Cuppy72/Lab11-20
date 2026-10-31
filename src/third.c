#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "self_errors.h"
#include "overflow.h"
#include "patterns.h"

#define SUBSTR_IND 1
#define FILES_NUM_IND 2
#define PATHS_START_IND 3

#define MAX_PATTERN_LEN 128
#define BUFF_SIZE 1024
#define MAX_FILES 64

static int str_len(const char *str)
{
        int len = 0;

        while(*str++ != '\0')
                len++;

        return len;
}

static void print_occurrence(int line_num, int column_num)
{
        printf("Вхождение найдено: строка %d, позиция %d\n", line_num, column_num);
}

static int search_in_file(const char *path, const char *pattern, int pattern_len, int *total_found)
{
	int err;

        FILE *input = fopen(path, "r");

        if(input == NULL)
                return FILE_OPEN_ERROR;

	Hash_table *table;
	err = create_table(pattern_len, &table);

	if(err != SUCCESS)
		return err;

	err = fill_table(pattern, pattern_len, table);
	
	if(err != SUCCESS)
		return err;

	char buffer[BUFF_SIZE];

	int current_line = 1;
	int bytes_read = 0;

	int end_flag = 0;

	while((bytes_read = fread(buffer, 1, sizeof(buffer), input)) > 0 && !end_flag) {
		if(bytes_read < BUFF_SIZE)
			end_flag = 1;
		
		int current_offset = 0;
		int start_ind;

		while(1) {
			if(bytes_read - current_offset < pattern_len)
				break;

			err = boer_mour(table, pattern, pattern_len, buffer + current_offset, bytes_read - current_offset, &start_ind);

			if(err != SUCCESS)
				return err;

			if(start_ind == -1) break;
		
			(*total_found)++;

			int real_math_ind = current_offset + start_ind;
			int display_line = current_line;

			for(int i = 0; i < real_math_ind; i++) {
				if(buffer[i] == '\n') display_line++;
			}

			int real_start_ind = ftell(input) - bytes_read + real_math_ind;
			print_occurrence(display_line, real_start_ind);

			current_offset = real_math_ind + 1;
		}

		int limit = end_flag ? bytes_read : (bytes_read - pattern_len + 1);
		for(int i = 0; i < limit; i++) {
			if(buffer[i] == '\n') current_line++;
		}

		if(!end_flag)
			fseek(input, -pattern_len, SEEK_CUR);
	}

	fclose(input);
	free_table(table);

        return SUCCESS;
}

int find_substr(const char *pattern, int count_files, ...)
{
        int err;
        int pattern_len = str_len(pattern);

        if(pattern_len > MAX_PATTERN_LEN)
                return TOO_LONG_PATTERN;

        va_list args;

        va_start(args, count_files);

        for(int i = 0; i < count_files; i++) {
                char *path = va_arg(args, char *);

                printf("\nФайл \"%s\":\n", path);

                int total_found = 0;

                err = search_in_file(path, pattern, pattern_len, &total_found);

                if(err != SUCCESS) {
                        va_end(args);
                        return err;
                }

                if(total_found == 0)
                        puts("Вхождений не найдено");
                else
                        printf("Всего вхождений: %d\n", total_found);
        }

        va_end(args);

        return SUCCESS;
}

static int err_switch(int err)
{
        switch(err) {
                case NOT_ENOUGH_ARGS:
                        puts("Введено недостаточно аргументов");
                        break;
                case EMPTY_PATTERN:
                        puts("Искомая подстрока не может быть пустой");
                        break;
                case TOO_LONG_PATTERN:
                        puts("Слишком длинная искомая подстрока");
                        break;
                case ALLOC_ERROR:
                        puts("Ошибка во время выделения памяти");
                        break;
                case FILE_OPEN_ERROR:
                        puts("Ошибка во время открытия файла");
                        break;
                case FILE_CLOSE_ERROR:
                        puts("Ошибка во время закрытия файла");
                        break;
		case INCORRECT_STR:
			puts("Файл содержит некорректную строку");
			break;
                case SUCCESS:
			return 0;
                default:
			puts("Произошла ошибка");
        }

	return 1;
}

int main(int argc, char *argv[])
{
        if(argc < 4) {
                puts("Введено недостаточно аргументов");
                return 1;
        }

        char *pattern = argv[SUBSTR_IND];

        if(*pattern == '\0') {
                puts("Искомая подстрока не может быть пустой");
                return 1;
        }

        char *end_ptr;
 
	char ul_min[] = "0";
	char ul_max[] = "64";

	int flag = int_overflow(argv[FILES_NUM_IND], ul_max, ul_min);

	if(flag == NOT_A_NUM) {
		puts("Количество файлов введено некорректно");
		return 1;
	}

	if(flag == 1 || flag == -1) {
		puts("Введено недопустимое количество файлов");
		return 1;
	}

        int count_files = atol(argv[FILES_NUM_IND]);
	
	if(count_files > argc - PATHS_START_IND) {
                puts("Введено недостаточно аргументов");
                return 1;
        }

        int err = SUCCESS;

        if(count_files == 1)
                err = find_substr(pattern, 1, argv[PATHS_START_IND]);
        else if(count_files == 2)
                err = find_substr(pattern, 2, argv[PATHS_START_IND], argv[PATHS_START_IND + 1]);
        else if(count_files == 3)
                err = find_substr(pattern, 3, argv[PATHS_START_IND], argv[PATHS_START_IND + 1],
                                  argv[PATHS_START_IND + 2]);
        else
                err = find_substr(pattern, 4, argv[PATHS_START_IND], argv[PATHS_START_IND + 1],
                                  argv[PATHS_START_IND + 2], argv[PATHS_START_IND + 3]);

        return err_switch(err);
}
