#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include "files.h"
#include "overflow.h"
#include "self_errors.h"

#define INPUT_IND 1
#define FLAG_IND 2
#define OUTPUT_IND 3

#define MIN_CAPACITY 4

typedef struct Employee
{
        unsigned int id;
        char name[BUFF_SIZE];
        char surname[BUFF_SIZE];
        double salary;
        char tail[BUFF_SIZE];
} Employee;

static int is_latin_word(const char *word)
{
        if(*word == '\0')
                return 0;

        while(*word != '\0') {
                if(!isalpha((unsigned char)*word))
                        return 0;

                word++;
        }

        return 1;
}

static int id_validation(char *id, unsigned int *result_id)
{
	char uint_max[BUFF_SIZE];
	char uint_min[] = "0";

	snprintf(uint_max, BUFF_SIZE, "%u", UINT_MAX);

	int flag = int_overflow(id, uint_max, uint_min);

	if(flag == NOT_A_NUM)
		return INCORRECT_ID;

	if(flag == -1 || flag == 1)
		return OVERFLOW;

	*result_id = atoi(id);

	return SUCCESS;
}

static int read_record(FILE *input, Employee *employee, int *end_of_file)
{
        int c = fgetc(input);

        while(c != EOF && isspace(c))
                c = fgetc(input);

        if(c == EOF) {
                *end_of_file = 1;
                return SUCCESS;
        }

        ungetc(c, input);

	char id_buff[BUFF_SIZE];

        int scanned = fscanf(input, "%1023s %1023s %1023s %lf%1023[^\n]", id_buff,
                             employee->name, employee->surname, &employee->salary,
                             employee->tail);

	
	if(scanned < 4)
		return NOT_ENOUGH_ARGS;

       	int err = id_validation(id_buff, &employee->id);

	if(err != SUCCESS)
		return err;

        if(scanned == 5 && employee->tail[0] != '\0')
                return INCORRECT_FORMAT;

        if(fgetc(input) != '\n' && !feof(input))
                return INCORRECT_FORMAT;

        if(!is_latin_word(employee->name) || !is_latin_word(employee->surname))
                return INCORRECT_FORMAT;

        if(employee->salary < 0.0 || isinf(employee->salary) || isnan(employee->salary))
                return INCORRECT_FORMAT;

        *end_of_file = 0;

        return SUCCESS;
}

static int load_employees(FILE *input, Employee **employees, int *count)
{
        int err;
        int capacity = MIN_CAPACITY;
        int size = 0;

        Employee *arr = malloc(sizeof(Employee) * (size_t)capacity);

        if(arr == NULL)
                return ALLOC_ERROR;

        while(1) {
                if(size == capacity) {
                        int new_capacity = capacity * 2;

                        Employee *tmp = realloc(arr, sizeof(Employee) * (size_t)new_capacity);

                        if(tmp == NULL) {
                                free(arr);
                                return ALLOC_ERROR;
                        }

                        arr = tmp;
                        capacity = new_capacity;
                }

                int end_of_file = 0;

                err = read_record(input, &arr[size], &end_of_file);
                if(err != SUCCESS) {
                        free(arr);
                        return err;
                }

                if(end_of_file)
                        break;

                size++;
        }

        if(size == 0) {
                free(arr);
                return EMPTY_FILE;
        }

        *employees = arr;
        *count = size;

        return SUCCESS;
}

static int compare_asc(const void *left, const void *right)
{
        const Employee *emp_left = (const Employee *)left;
        const Employee *emp_right = (const Employee *)right;

        if(emp_left->salary < emp_right->salary)
                return -1;

        if(emp_left->salary > emp_right->salary) {
		puts("Але вы ебнулись");
                return 1;
	}

        int surname_cmp = strcmp(emp_left->surname, emp_right->surname);

        if(surname_cmp != 0)
                return surname_cmp;

        int name_cmp = strcmp(emp_left->name, emp_right->name);

        if(name_cmp != 0)
                return name_cmp;
	
	if(emp_left->id < emp_right->id)
                return -1;

        if(emp_left->id > emp_right->id)
                return 1;

        return 0;
}

static int compare_desc(const void *left, const void *right)
{
        return -compare_asc(left, right);
}

static int dump_employees(FILE *output, const Employee *employees, int count)
{
        for(int i = 0; i < count; i++) {
                int printed = fprintf(output, "%u %s %s %.2lf\n", employees[i].id,
                                      employees[i].name, employees[i].surname,
                                      employees[i].salary);

                if(printed < 0)
                        return WRITE_FILE_ERROR;
        }

        return SUCCESS;
}

static int err_switch(int err)
{
        switch(err) {
                case INCORRECT_FORMAT:
                        puts("Некорретный формат данных во входном файле");
                        break;
                case ALLOC_ERROR:
                        puts("Ошибка во время выделения памяти");
                        break;
                case EMPTY_FILE:
                        puts("Во входном файле отсутствуют записи");
                        break;
                case INCORRECT_FLAG:
                        puts("Введен некорректный флаг");
                        break;
                case FILE_OPEN_ERROR:
                        puts("Ошибка во время открытия файла");
                        break;
                case FILE_CLOSE_ERROR:
                        puts("Ошибка во время закрытия файла");
                        break;
                case WRITE_FILE_ERROR:
                        puts("Ошибка записи данных в файл");
                        break;
		case IDENTITY_FILES:
			puts("В качестве аргументов передан только один уникальный файл");
			break;
		case INCORRECT_ID:
			puts("Введен некорректный id");
			break;
		case OVERFLOW:
			puts("Ошибка переполнения");
			break;
		case NOT_ENOUGH_ARGS:
			puts("Введено недостаточно аргументов");
			break;
                case SUCCESS:
                default:
                        return 0;
        }

        return 1;
}

int main(int argc, char *argv[])
{
        if(argc < 4) {
                puts("Введено недостаточно аргументов");
                return 1;
        }

        char *flag = argv[FLAG_IND];

        if(*flag != '-' && *flag != '/') {
		puts("Некорректно введен флаг");
                return 1;
        }

	flag++;
	
	if(*flag != 'a' && *flag != 'd') {
		puts("Некорректно введен флаг");
		return 1;
	}

	int cmp_res;

	int err = equal_filenames(argv[INPUT_IND], argv[OUTPUT_IND], &cmp_res);
	if(err != SUCCESS)
		return err_switch(err);

	if(cmp_res == 0)
		return err_switch(IDENTITY_FILES);

        FILE *input = fopen(argv[INPUT_IND], "r");
        FILE *output = fopen(argv[OUTPUT_IND], "w");

        err = validate_open_files(input, output);
        if(err != SUCCESS)
                return err_switch(err);

        Employee *employees = NULL;
        int count = 0;

        err = load_employees(input, &employees, &count);
        if(err != SUCCESS) {
                safety_close_files(input, output);
                return err_switch(err);
        }

        if(*flag == 'a')
                qsort(employees, (size_t)count, sizeof(Employee), compare_asc);
        else
                qsort(employees, (size_t)count, sizeof(Employee), compare_desc);

        err = dump_employees(output, employees, count);

        free(employees);

        int close_err = safety_close_files(input, output);
        if(err == SUCCESS)
                err = close_err;

        return err_switch(err);
}
