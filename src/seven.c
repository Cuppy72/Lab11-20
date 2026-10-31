#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#include "self_errors.h"

#define MAX_VECTORS 16
#define MAX_DIM 8

typedef struct Vector
{
        double *coords;
        int dim;
} Vector;

typedef double (*NormFunc)(const Vector *vector, void *context, int *err);

static int vector_create(Vector *vector, int dim)
{
        if(vector == NULL)
                return INCORRECT_POINTER;

        if(dim <= 0 || dim > MAX_DIM)
                return INCORRECT_DIM;

        double *coords = malloc(sizeof(double) * (size_t)dim);

        if(coords == NULL)
                return ALLOC_ERROR;

        vector->coords = coords;
        vector->dim = dim;

        return SUCCESS;
}

static void vector_destroy(Vector *vector)
{
        if(vector != NULL) {
                free(vector->coords);
                vector->coords = NULL;
                vector->dim = 0;
        }
}

static double norm_inf(const Vector *vector, void *context, int *err)
{
        (void)context;

        if(vector == NULL || vector->coords == NULL) {
                *err = INCORRECT_POINTER;

                return 0.0;
        }

        double best = 0.0;

        for(int i = 0; i < vector->dim; i++) {
                double module = fabs(vector->coords[i]);

                if(module > best)
                        best = module;
        }

        *err = SUCCESS;

        return best;
}

static double norm_p(const Vector *vector, void *context, int *err)
{
        double p = *(double *)context;

        if(vector == NULL || vector->coords == NULL) {
                *err = INCORRECT_POINTER;

                return 0.0;
        }

        if(p < 1.0) {
                *err = INCORRECT_DIM;

                return 0.0;
        }

        double sum = 0.0;

        for(int i = 0; i < vector->dim; i++)
                sum += pow(fabs(vector->coords[i]), p);

        *err = SUCCESS;

        return pow(sum, 1.0 / p);
}

static double norm_a(const Vector *vector, void *context, int *err)
{
        const double *matrix = (const double *)context;

        if(vector == NULL || vector->coords == NULL || matrix == NULL) {
                *err = INCORRECT_POINTER;

                return 0.0;
        }

        int dim = vector->dim;
        double *ax = malloc(sizeof(double) * (size_t)dim);

        if(ax == NULL) {
                *err = ALLOC_ERROR;

                return 0.0;
        }

        for(int i = 0; i < dim; i++) {
                double sum = 0.0;

                for(int j = 0; j < dim; j++)
                        sum += matrix[(size_t)i * (size_t)dim + (size_t)j] * vector->coords[j];

                ax[i] = sum;
        }

        double result = 0.0;

        for(int i = 0; i < dim; i++)
            result += ax[i] * vector->coords[i];

        free(ax);

        if(result < 0.0) {
                *err = INCORRECT_MATRIX;

                return 0.0;
        }

        *err = SUCCESS;

        return result;
}

int longest_vectors(int dim, double epsilon, int count_norms, ...)
{
        if(dim <= 0 || dim > MAX_DIM || count_norms <= 0 || count_norms > MAX_VECTORS)
                return INCORRECT_DIM;

        if(epsilon < 0.0)
                return INCORRECT_DIM;

        va_list args;

        va_start(args, count_norms);

        int count_vectors = va_arg(args, int);

        if(count_vectors <= 0 || count_vectors > MAX_VECTORS) {
                va_end(args);

                return INCORRECT_DIM;
        }

        Vector *vectors = malloc(sizeof(Vector) * (size_t)count_vectors);

        if(vectors == NULL) {
                va_end(args);

                return ALLOC_ERROR;
        }

        for(int i = 0; i < count_vectors; i++)
                vectors[i].coords = NULL;

        for(int i = 0; i < count_vectors; i++)
                vectors[i] = va_arg(args, Vector);

        NormFunc *norms = malloc(sizeof(NormFunc) * (size_t)count_norms);
        void **contexts = malloc(sizeof(void *) * (size_t)count_norms);
        int *answers = malloc(sizeof(int) * (size_t)count_norms);

        if(norms == NULL || contexts == NULL || answers == NULL) {
                free(vectors);
                free(norms);
                free(contexts);
                free(answers);
                va_end(args);

                return ALLOC_ERROR;
        }

        for(int i = 0; i < count_norms; i++) {
                norms[i] = va_arg(args, NormFunc);
                contexts[i] = va_arg(args, void *);
                answers[i] = -1;
        }

        va_end(args);

        int status = SUCCESS;

        for(int n = 0; n < count_norms && status == SUCCESS; n++) {
                if(norms[n] == NULL) {
                        status = INCORRECT_POINTER;
                        break;
                }

                double *lengths = malloc(sizeof(double) * (size_t)count_vectors);

                if(lengths == NULL) {
                        status = ALLOC_ERROR;
                        break;
                }

                double best = 0.0;
                int best_count = 0;

                for(int v = 0; v < count_vectors; v++) {
                        int err = SUCCESS;

                        lengths[v] = norms[n](&vectors[v], contexts[n], &err);

                        if(err != SUCCESS) {
                                status = err;
                                break;
                        }

                        if(best_count == 0 || lengths[v] > best + epsilon) {
                                best = lengths[v];
                                best_count = 1;
                        } else if(lengths[v] >= best - epsilon) {
                                best_count++;

                                if(lengths[v] > best)
                                        best = lengths[v];
                        }
                }

                if(status != SUCCESS) {
                        free(lengths);
                        break;
                }

                int *picked = malloc(sizeof(int) * (size_t)best_count);

                if(picked == NULL) {
                        free(lengths);
                        status = ALLOC_ERROR;
                        break;
                }

                int pos = 0;

                for(int v = 0; v < count_vectors && pos < best_count; v++) {
                        if(lengths[v] >= best - epsilon)
                                picked[pos++] = v;
                }

                answers[n] = pos;

                printf("Норма N%d: максимальная длина %.4f, векторы:", n + 1, best);

                for(int i = 0; i < pos; i++) {
                        printf(" V%d(", picked[i]);

                        for(int c = 0; c < dim; c++)
                                printf("%s%.2f", (c == 0) ? "" : ", ", vectors[picked[i]].coords[c]);

                        printf(")");
                }

                puts("");

                free(picked);
                free(lengths);
        }

        for(int i = 0; i < count_vectors; i++)
                vector_destroy(&vectors[i]);

        free(vectors);
        free(norms);
        free(contexts);
        free(answers);

        return status;
}

static int err_switch(int err)
{
        switch(err) {
                case ALLOC_ERROR:
                        puts("Ошибка выделения памяти");
                        break;
                case INCORRECT_DIM:
                        puts("Некорректная размерность или количество аргументов");
                        break;
                case INCORRECT_POINTER:
                        puts("Передан нулевой указатель");
                        break;
                case INCORRECT_MATRIX:
                        puts("Матрица не является положительно определённой");
                        break;
                case SUCCESS:
                default:
                        return 0;
        }

        return 1;
}

static Vector make_vector(int dim, const double *source)
{
        Vector vector;

        vector.coords = NULL;
        vector.dim = 0;

        if(vector_create(&vector, dim) != SUCCESS)
                return vector;

        for(int i = 0; i < dim; i++)
            vector.coords[i] = source[i];

        return vector;
}

int main(void)
{
        double p_value = 2.0;

        double matrix[MAX_DIM * MAX_DIM] = {
                1.25, 0.0, 0.0,
                0.0, 1.5, 0.0,
                0.0, 0.0, 1.75
        };

        double eps = 1e-9;

        Vector v1 = make_vector(3, (const double[]){1.0, 2.0, 3.0});
        Vector v2 = make_vector(3, (const double[]){3.0, 0.0, 4.0});
        Vector v3 = make_vector(3, (const double[]){-3.0, 4.0, 0.0});
        Vector v4 = make_vector(3, (const double[]){0.5, 0.5, 0.5});

        if(v1.coords == NULL || v2.coords == NULL || v3.coords == NULL || v4.coords == NULL)
                return err_switch(ALLOC_ERROR);

        puts("Демонстрация longest_vectors (три нормы):");

        int status = longest_vectors(3, eps, 3,
                                     4, v1, v2, v3, v4,
                                     norm_inf, NULL,
                                     norm_p, &p_value,
                                     norm_a, matrix);

        if(status != SUCCESS)
                return err_switch(status);

        return 0;
}
