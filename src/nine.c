#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>

#include "self_errors.h"

#define MIN_BASE 2
#define MAX_BASE 36

#define EPS 1e-9
#define MAX_STEPS 4096


static int prime_divisors(unsigned value, unsigned *factors, int capacity)
{
        int count = 0;

        for(unsigned divisor = 2; (unsigned long long)divisor * divisor <= value; divisor++) {
                if(value % divisor != 0)
                        continue;

                if(count < capacity)
                        factors[count++] = divisor;

                while(value % divisor == 0)
                        value /= divisor;
        }

        if(value > 1) {
                if(count < capacity)
                        factors[count++] = value;
        }

        return count;
}

static unsigned gcd_unsigned(unsigned left, unsigned right)
{
        while(right != 0) {
                unsigned rest = left % right;

                left = right;
                right = rest;
        }

        return left;
}

bool rational_finite_representation(unsigned numerator, unsigned denominator, int base)
{
        if(numerator == 0 || denominator == 0 || numerator >= denominator)
                return false;

        if(base < MIN_BASE || base > MAX_BASE)
                return false;

        unsigned common = gcd_unsigned(numerator, denominator);

        numerator /= common;
        denominator /= common;

        unsigned divisors[MAX_BASE];

        int base_count = prime_divisors((unsigned)base, divisors, MAX_BASE);

        for(int i = 0; i < base_count; i++)
                while(denominator % divisors[i] == 0)
                        denominator /= divisors[i];

        return denominator == 1;
}

static bool fraction_as_rational(double value, unsigned *numerator, unsigned *denominator)
{
        char text[64];

        snprintf(text, sizeof(text), "%.12g", value);

        unsigned whole = 0;
        unsigned frac = 0;
        unsigned scale = 1;

        const char *ptr = text;

        while(*ptr != '\0' && *ptr != '.') {
                if(*ptr < '0' || *ptr > '9')
                        return false;

                whole = whole * 10u + (unsigned)(*ptr - '0');
                ptr++;
        }

        if(*ptr == '.')
                ptr++;

        while(*ptr != '\0') {
                if(*ptr < '0' || *ptr > '9')
                        return false;

                frac = frac * 10u + (unsigned)(*ptr - '0');
                scale *= 10u;
                ptr++;
        }

        unsigned long long total = (unsigned long long)whole * scale + frac;

        if(total >= 0xFFFFFFFFULL)
                return false;

        *numerator = (unsigned)total;
        *denominator = scale;

        return true;
}

static bool finite_by_steps(double fraction, int base, double eps)
{
        double rest = fraction;

        for(int step = 0; step < MAX_STEPS; step++) {
                rest *= base;

                double integer_part;

                rest = modf(rest, &integer_part);

                if(fabs(rest) < eps || fabs(1.0 - rest) < eps)
                        return true;
        }

        return false;
}

static bool finite_representation(double fraction, int base, double eps)
{
        unsigned numerator;
        unsigned denominator;

        if(fraction_as_rational(fraction, &numerator, &denominator))
                return rational_finite_representation(numerator, denominator, base);

        return finite_by_steps(fraction, base, eps);
}

int check_finite_representation(int base, int count, double eps, char **flags, ...)
{
        if(flags == NULL)
                return INCORRECT_FRACTION;

        *flags = NULL;

        if(base < MIN_BASE || base > MAX_BASE)
                return INCORRECT_BASE;

        if(!(eps > 0.0))
                return INCORRECT_FRACTION;

        if(count <= 0)
                return NOT_ENOUGH_ARGS;

        char *out = malloc(sizeof(char) * (size_t)count);

        if(out == NULL)
                return ALLOC_ERROR;

        int status = SUCCESS;

        va_list args;

        va_start(args, flags);

        for(int i = 0; i < count; i++) {
                double fraction = va_arg(args, double);

                if(!(fraction > 0.0 && fraction < 1.0)) {
                        status = INCORRECT_FRACTION;
                        break;
                }

                out[i] = finite_representation(fraction, base, eps);
        }

        va_end(args);

        if(status == SUCCESS)
                *flags = out;
        else
                free(out);

        return status;
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
                case INCORRECT_FRACTION:
                        puts("Дробь должна лежать в интервале (0; 1)");
                        break;
                case NOT_ENOUGH_ARGS:
                        puts("Передано слишком мало аргументов");
                        break;
                case SUCCESS:
                default:
                        return 0;
        }

        return 1;
}

static void print_fractions(const double fractions[], int count, const char flags[])
{
        for(int i = 0; i < count; i++)
                printf("%g - %s\n", fractions[i],
                       flags[i] ? "конечное представление" : "бесконечное представление");
}

int main(void)
{
        const double fractions[] = {0.5, 0.25, 0.2, 1.0 / 3.0, 0.125, 0.6};
        int count = (int)(sizeof(fractions) / sizeof(fractions[0]));

        const int bases[] = {2, 10, 3};

        for(int b = 0; b < 3; b++) {
                char *flags = NULL;

                int status = check_finite_representation(bases[b], count, EPS, &flags,
                                                         fractions[0], fractions[1], fractions[2],
                                                         fractions[3], fractions[4], fractions[5]);

                if(status != SUCCESS) {
                        err_switch(status);
                        continue;
                }

                printf("Система счисления с основанием %d:\n", bases[b]);
                print_fractions(fractions, count, flags);
                puts("");

                free(flags);
        }

        puts("Точная проверка рациональных дробей p/q:");
        printf("3/4 в base 2:    %s\n", rational_finite_representation(3, 4, 2) ? "конечное" : "бесконечное");
        printf("1/3 в base 10:   %s\n", rational_finite_representation(1, 3, 10) ? "конечное" : "бесконечное");
        printf("1/6 в base 12:   %s\n", rational_finite_representation(1, 6, 12) ? "конечное" : "бесконечное");
        printf("7/10 в base 10:  %s\n", rational_finite_representation(7, 10, 10) ? "конечное" : "бесконечное");
        puts("");

        return 0;
}
