#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <ctype.h>

#include "overflow.h"

#define FLAG_IND 1
#define STR_IND 2
#define SEED_IND 3
#define EXTRA_START_IND 4

#define BUFF_SIZE 15

#define SUCCESS 10
#define NOT_ENOUGH_ARGS 11
#define INCORRECT_FLAG 12
#define ALLOC_ERROR 13
#define INCORRECT_SEED 14
#define INCORRECT_SYMB 15
#define INCORRECT_COUNT_ARGC 16
#define NOT_A_NUM -2
#define OVERFLOW 17

static int is_ascii(unsigned char symb)
{
	if(0 <= symb && symb <= 127)
		return 1;
	
	return 0;
}

static int str_len(char *str)
{
	char *str_ptr = str;

	while(*str_ptr != '\0') {
		if(!is_ascii((unsigned char)(*str_ptr)))
			return -1;

		str_ptr++;
	}

	return str_ptr - str;
}

int l_flag(int len, char **result)
{
	*result = NULL;

        printf("Длина строки: %d\n", len);

        return SUCCESS;
}

int r_flag(const char *str, int len, char **result)
{
        char *reversed = malloc(sizeof(char) * (size_t)(len + 1));

        if(reversed == NULL)
                return ALLOC_ERROR;

        for(int i = 0; i < len; i++)
                reversed[i] = str[len - i - 1];

        reversed[len] = '\0';
	*result = reversed;

        return SUCCESS;
}

int u_flag(const char *str, int len, char **result)
{
        char *updated = malloc(sizeof(char) * (len + 1));

        if(updated == NULL)
                return ALLOC_ERROR;

        for(int i = 0; i < len; i++) {
                if(i % 2 == 1)
                        updated[i] = (char)toupper((unsigned char)str[i]);
                else
                        updated[i] = str[i];
        }

        updated[len] = '\0';

        *result = updated;

        return SUCCESS;
}

int n_flag(const char *str, int len, char **result)
{
        char *reordered = malloc(sizeof(char) * (size_t)(len + 1));

        if(reordered == NULL)
                return ALLOC_ERROR;

        int pos = 0;

        for(int i = 0; i < len; i++)
                if(isdigit((unsigned char)str[i]))
                        reordered[pos++] = str[i];

        for(int i = 0; i < len; i++)
                if(isalpha((unsigned char)str[i]))
                        reordered[pos++] = str[i];

        for(int i = 0; i < len; i++)
                if(!isdigit((unsigned char)str[i]) && !isalpha((unsigned char)str[i]))
                        reordered[pos++] = str[i];

        reordered[pos] = '\0';

        *result = reordered;

        return SUCCESS;
}

static int seed_validation(char *seed, int *result_seed)
{
	char uint_max[BUFF_SIZE];
	char uint_min[] = "0";

	snprintf(uint_max, BUFF_SIZE, "%u", UINT_MAX);

	int flag = int_overflow(seed, uint_max, uint_min);

	if(flag == NOT_A_NUM)
		return INCORRECT_SEED;

	if(flag == -1 || flag == 1)
		return OVERFLOW;

	*result_seed = atoll(seed);

	return SUCCESS;
}

static int total_str_length(int argc, char *argv[], int *res_length)
{
	*res_length = str_len(argv[STR_IND]);
	
	if(*res_length < 0)
		return INCORRECT_SYMB;

	for(int i = EXTRA_START_IND; i < argc; i++) {
		int cur_len = str_len(argv[i]); 
		
		if(cur_len < 0)
			return INCORRECT_SYMB;

		*res_length += cur_len;
	}

	return SUCCESS;
}

static int contains(int array[], int size_array, int num)
{
	for(int i = 0; i < size_array; i++) {
		if(num == array[i])
			return 1;
	}

	return 0;
}

int c_flag(int argc, char *argv[], char **result)
{
	if(argc < 4)
		return NOT_ENOUGH_ARGS;

	char *seed = argv[SEED_IND];
	int result_seed;

	int err;

	err = seed_validation(seed, &result_seed);

	if(err != SUCCESS)
		return err;
	
	int count_strings = argc - EXTRA_START_IND + 1;
	int total_len = 0;

	err = total_str_length(argc, argv, &total_len);
	
	if(err != SUCCESS)
		return err;

	char *concatenated = malloc(sizeof(char) * (size_t)(total_len + 1));

	if(concatenated == NULL)
		return ALLOC_ERROR; 

	int *indexes = calloc((size_t)(count_strings), sizeof(int));

	if(indexes == NULL) {
		free(concatenated);

		return ALLOC_ERROR;
	}

	srand(result_seed);
	
	int last_ind_position = 0;
	int last_char_position = 0;

	for(int i = 0; i < count_strings; i++) {
		int cur;

		while((cur = rand() % count_strings + 1) && contains(indexes, count_strings, cur))
			;

		indexes[last_ind_position++] = cur;

		char *cur_str;

		if(cur == 1)
			cur_str = argv[STR_IND];
		else
			cur_str = argv[EXTRA_START_IND - 2 + cur];

                while(*cur_str != '\0')
                        concatenated[last_char_position++] = *cur_str++;
        }

        concatenated[last_char_position] = '\0';

        free(indexes);

        *result = concatenated;

        return SUCCESS;
}

void err_switch(int err, char flag)
{
        switch(err) {
                case NOT_ENOUGH_ARGS:
                        puts("Введено недостаточно аргументов");
                        break;
                case INCORRECT_FLAG:
                        printf("Введен некорректный флаг - %c\n", flag);
                        break;
                case INCORRECT_SEED:
                        puts("Введено некорректное числовое значение seed");
                        break;
                case ALLOC_ERROR:
                        puts("Ошибка во время выделения памяти");
                        break;
		case INCORRECT_SYMB:
			puts("Введенный символ не содержится в таблице ASCII");
			break;
		case INCORRECT_COUNT_ARGC:
			puts("Введено некорректное количество аргументов для текущего флага");
			break;
		case OVERFLOW:
			puts("Значение seed выходит за границы unsigned int");
			break;
                case SUCCESS:
                default:
                        return;
        }
}

int first_validate(int argc, int *len, char *str)
{
	if(argc != 3)
		return INCORRECT_COUNT_ARGC;

	*len = str_len(str);
	if(*len < 0)
		return INCORRECT_SYMB;

	return SUCCESS;
}

int switch_flag(char flag, int argc, char *argv[], char **result)
{
	int err;

	int len = 0;
	char *str = argv[STR_IND];

	if(flag != 'c') {
		err = first_validate(argc, &len, str);

		if(err != SUCCESS)
			return err;
	}

	switch(flag) {
                case 'l':
                        err = l_flag(len, result);
                        break;
                case 'r':
                        err = r_flag(str, len, result);
                        break;
                case 'u':
                        err = u_flag(str, len, result);
                        break;
                case 'n':
                        err = n_flag(str, len, result);
                        break;
                case 'c':
			err = c_flag(argc, argv, result);
			break;
                default:
                        err = INCORRECT_FLAG;
        }

	return err;
}

int main(int argc, char *argv[])
{
        if(argc < 3) {
                puts("Введено недостаточно аргументов");
                return 1;
        }

        char *flag = argv[FLAG_IND];

        if(*flag != '-' && *flag != '/') {
                puts("Некорректно введен флаг");
                return 1;
        }

        flag++;

	char *result;

	int err = switch_flag(*flag, argc, argv, &result);
        err_switch(err, *flag);

        if(*(flag + 1) != '\0')
                puts("Может быть исполнен только один флаг");

	if(err == SUCCESS && result != NULL) {
		printf("%s\n", result);

		free(result);
	}

        return (err == SUCCESS) ? 0 : 1;
}
