#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>

#include "self_errors.h"

#define MAX_DEGREE 64

static bool nearly_equal(double left, double right, double eps)
{
        return fabs(left - right) <= eps;
}

int rebase_polynomial(double eps, double a, double **coeffs, int degree, ...)
{
        if(coeffs == NULL)
                return INCORRECT_DEGREE;

        *coeffs = NULL;

        if(!(eps > 0.0))
                return INCORRECT_EPS;

        if(degree < 0 || degree > MAX_DEGREE)
                return INCORRECT_DEGREE;

        double *result = malloc(sizeof(double) * (size_t)(degree + 1));

        if(result == NULL) 
                return ALLOC_ERROR;

        va_list args;
        va_start(args, degree);

        for(int i = 0; i <= degree; i++)
                result[i] = va_arg(args, double);

        va_end(args);

        for(int i = 0; i < degree; i++) {
                for(int j = degree - 1; j >= i; j--) {
                        result[j] = result[j] + a * result[j + 1];
		}
	}

        for(int k = 0; k <= degree; k++)
                if(nearly_equal(result[k], 0.0, eps))
                        result[k] = 0.0;

        *coeffs = result;

        return SUCCESS;
}

static int err_switch(int err)
{
        switch(err) {
                case ALLOC_ERROR:
                        puts("Ошибка выделения памяти");
                        break;
                case INCORRECT_DEGREE:
                        puts("Некорректная степень многочлена");
                        break;
                case INCORRECT_EPS:
                        puts("Эпсилон должен быть положительным числом");
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

static void print_source_polynomial(const double coeffs[], int degree)
{
        printf("f(x) = %g", coeffs[0]);

        for(int i = 1; i <= degree; i++)
                printf(" + %gx^%d", coeffs[i], i);

        puts("");
}

static void print_rebased_polynomial(const double coeffs[], int degree, double a)
{
        if(degree < 0) {
                puts("g(x) = 0");

                return;
        }

        printf("g(x) = %g", coeffs[0]);

        for(int i = 1; i <= degree; i++)
                printf(" + %g*(x - %g)^%d", coeffs[i], a, i);

        puts("");
}

static double evaluate_source(const double coeffs[], int degree, double x)
{
        double value = coeffs[degree];

        for(int i = degree - 1; i >= 0; i--)
                value = value * x + coeffs[i];

        return value;
}

static double evaluate_rebased(const double coeffs[], int degree, double a, double x)
{
        double value = coeffs[degree];

        for(int i = degree - 1; i >= 0; i--)
                value = value * (x - a) + coeffs[i];

        return value;
}

static int demo_one(void)
{
        double *coeffs = NULL;

        int status = rebase_polynomial(1e-9, 2.0, &coeffs, 3,
                                       1.0, 2.0, 3.0, 4.0);

        if(status != SUCCESS)
                return err_switch(status);

        const double source[] = {1.0, 2.0, 3.0, 4.0};

        print_source_polynomial(source, 3);
        print_rebased_polynomial(coeffs, 3, 2.0);

        double check_x = 3.5;

        printf("Контроль при x = %g: f(x) = %g, g(x) = %g\n",
               check_x, evaluate_source(source, 3, check_x),
               evaluate_rebased(coeffs, 3, 2.0, check_x));

        free(coeffs);

        return SUCCESS;
}

static int demo_two(void)
{
        double *coeffs = NULL;

        int status = rebase_polynomial(1e-9, -1.0, &coeffs, 4,
                                       5.0, -3.0, 0.0, 0.0, 1.0);

        if(status != SUCCESS)
                return err_switch(status);

        const double source[] = {5.0, -3.0, 0.0, 0.0, 1.0};

        print_source_polynomial(source, 4);
        print_rebased_polynomial(coeffs, 4, -1.0);

        double check_x = 2.25;

        printf("Контроль при x = %g: f(x) = %g, g(x) = %g\n",
               check_x, evaluate_source(source, 4, check_x),
               evaluate_rebased(coeffs, 4, -1.0, check_x));

        free(coeffs);

        return SUCCESS;
}

static int demo_three(void)
{
        double *coeffs = NULL;

        int status = rebase_polynomial(1e-9, 3.0, &coeffs, 0, 7.0);

        if(status != SUCCESS)
                return err_switch(status);

        puts("Постоянный многочлен:");
        print_rebased_polynomial(coeffs, 0, 3.0);

        free(coeffs);

        return SUCCESS;
}

int main(void)
{
        if(demo_one() != SUCCESS)
                return 1;

        puts("");

        if(demo_two() != SUCCESS)
                return 1;

        puts("");

        if(demo_three() != SUCCESS)
                return 1;

        puts("");

        return 0;
}
