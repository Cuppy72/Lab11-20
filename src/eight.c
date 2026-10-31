#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "self_errors.h"

#define MIN_BASE 2
#define MAX_BASE 36

static int digit_value(char symb)
{
        if('0' <= symb && symb <= '9')
                return symb - '0';

        if('a' <= symb && symb <= 'z')
                return symb - 'a' + 10;

        if('A' <= symb && symb <= 'Z')
                return symb - 'A' + 10;

        return -1;
}

static char value_digit(int value)
{
        if(value < 10)
                return (char)('0' + value);

        return (char)('a' + value - 10);
}

static int check_number(const char *number, int base)
{
        if(number == NULL || *number == '\0')
                return INCORRECT_NUMBER;

        for(const char *ptr = number; *ptr != '\0'; ptr++) {
                int value = digit_value(*ptr);

                if(value < 0 || value >= base)
                        return INCORRECT_NUMBER;
        }

        return SUCCESS;
}

int add_columns(const char *left, const char *right, int base, char **result)
{
        if(base < MIN_BASE || base > MAX_BASE || result == NULL)
                return INCORRECT_BASE;

        int err = check_number(left, base);

        if(err != SUCCESS)
                return err;

        err = check_number(right, base);

        if(err != SUCCESS)
                return err;

        size_t len_left = strlen(left);
        size_t len_right = strlen(right);
        size_t max_len = (len_left > len_right ? len_left : len_right) + 1;

        char *out = malloc(max_len + 1);

        if(out == NULL)
                return ALLOC_ERROR;

        int carry = 0;
        size_t pos = max_len;
        size_t ia = len_left;
        size_t ib = len_right;

        while(ia > 0 || ib > 0 || carry > 0) {
                int sum = carry;

                if(ia > 0)
                        sum += digit_value(left[--ia]);

                if(ib > 0)
                        sum += digit_value(right[--ib]);

                carry = sum / base;
                out[--pos] = value_digit(sum % base);
        }

        if(pos == max_len)
            out[--pos] = '0';

        memmove(out, out + pos, max_len - pos + 1);
        out[max_len - pos] = '\0';

        *result = out;

        return SUCCESS;
}

int sum_in_base(int base, int count, char **result, ...)
{
        if(result == NULL)
                return INCORRECT_BASE;

        if(base < MIN_BASE || base > MAX_BASE)
                return INCORRECT_BASE;

        if(count <= 0)
                return NOT_ENOUGH_ARGS;

        char *acc = NULL;
        int err = SUCCESS;

        va_list args;

        va_start(args, result);

        for(int i = 0; i < count; i++) {
                const char *number = va_arg(args, const char *);

                if(number == NULL) {
                        err = INCORRECT_NUMBER;
                        break;
                }

                char *next = NULL;

                err = add_columns((acc != NULL) ? acc : "0", number, base, &next);

                free(acc);

                if(err != SUCCESS) {
                        acc = NULL;
                        break;
                }

                acc = next;
        }

        va_end(args);

        if(err == SUCCESS)
                *result = acc;
        else
                free(acc);

        return err;
}

static int err_switch(int err)
{
        switch(err) {
                case ALLOC_ERROR:
                        puts("Ошибка выделения памяти");
                        break;
                case INCORRECT_BASE:
                        puts("Некорректное основание системы счисления");
                        break;
                case INCORRECT_NUMBER:
                        puts("Некорректная запись числа");
                        break;
                case NOT_ENOUGH_ARGS:
                        puts("Передано слишком мало аргументов");
                        break;
                case SUCCESS:
			return 0;
                default:
                        puts("Произошла ошибка");
        }

        return 1;
}

static int demo_sum(int base, int count, const char *numbers[])
{
        char *result = NULL;

        int err = sum_in_base(base, count, &result,
                                 numbers[0], numbers[1], numbers[2]);

        if(err != SUCCESS)
                return err_switch(err);

        printf("Сумма в системе счисления %d: %s\n", base, result);
        free(result);

        return SUCCESS;
}

int main(void)
{
        const char *hex_numbers[] = {"ff", "1", "aaaaaaaaaa"};

        if(demo_sum(16, 3, hex_numbers) != SUCCESS)
                return 1;

        const char *bin_numbers[] = {"1111", "1", "0"};

        if(demo_sum(2, 3, bin_numbers) != SUCCESS)
                return 1;

        const char *d36_numbers[] = {"z", "10", "100000000000000000000000000000000000000"};

        if(demo_sum(36, 3, d36_numbers) != SUCCESS)
                return 1;

        char *result = NULL;

        int err = sum_in_base(10, 2, &result, "12", "g5");

        if(err != SUCCESS) {
                puts("Ожидаемая ошибка (некорректный символ для базы 10):");

                return err_switch(err);
        }

        free(result);

        return 0;
}
