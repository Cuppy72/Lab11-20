#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define ROMAN_COUNT 13
#define ZECKENDORF_LIMIT 100

#define MIN_BASE 2
#define MAX_BASE 36
#define DEFAULT_BASE 10

#define READ_BUFF_SIZE 64

#define UINT_MAX_CAST ((unsigned long long)UINT_MAX)

typedef struct InSource
{
        FILE *stream;
        const char *str;
        size_t pos;
} InSource;

typedef struct RomanEntry
{
        int value;
        const char *symbols;
} RomanEntry;

static int source_getc(InSource *source)
{
        if(source->stream != NULL)
                return fgetc(source->stream);

        if(source->str == NULL || source->str[source->pos] == '\0')
                return EOF;

        return (unsigned char)source->str[source->pos++];
}

static void source_ungetc(InSource *source, int symb)
{
        if(symb == EOF)
                return;

        if(source->stream != NULL) {
                ungetc(symb, source->stream);

                return;
        }

        if(source->pos > 0)
                source->pos--;
}

static int skip_spaces(InSource *source)
{
        int symb;

        while((symb = source_getc(source)) != EOF) {
                if(!isspace(symb))
                        break;
        }

        return symb;
}

static int read_word(InSource *source, char *buff, size_t capacity)
{
        size_t len = 0;
        int symb = skip_spaces(source);

        if(symb == EOF)
                return -1;

        do {
                if(isspace(symb)) {
                        source_ungetc(source, symb);
                        break;
                }

                if(len + 1 < capacity)
                        buff[len++] = (char)symb;

        } while((symb = source_getc(source)) != EOF);

        buff[len] = '\0';

        return (int)len;
}


static int equal_ignore_case(const char *left, const char *right, size_t len)
{
        for(size_t i = 0; i < len; i++) {
                if(toupper((unsigned char)left[i]) != toupper((unsigned char)right[i]))
                        return 0;
        }

        return 1;
}

static int parse_roman(const char *word, int *result)
{
        static const RomanEntry entries[ROMAN_COUNT] =
                {{1000, "M"}, {900, "CM"}, {500, "D"}, {400, "CD"}, {100, "C"},
                 {90, "XC"}, {50, "L"}, {40, "XL"}, {10, "X"}, {9, "IX"},
                 {5, "V"}, {4, "IV"}, {1, "I"}};

        static const int repeatable[ROMAN_COUNT] =
                {1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1};

        int total = 0;
        int prev_value = INT_MAX;

        if(*word == '\0')
                return -1;

        while(*word != '\0') {
                int matched = -1;

                for(int i = 0; i < ROMAN_COUNT; i++) {
                        size_t len = strlen(entries[i].symbols);

                        if(equal_ignore_case(word, entries[i].symbols, len)) {
                                matched = i;
                                break;
                        }
                }

                if(matched < 0)
                        return -1;

                int value = entries[matched].value;

                if(value > prev_value)
                        return -1;

                if(value == prev_value && !repeatable[matched])
                        return -1;

                total += value;
                prev_value = value;
                word += strlen(entries[matched].symbols);
        }

        *result = total;

        return 0;
}

static int digit_value(char symb)
{
        if(isdigit((unsigned char)symb))
                return symb - '0';

        if(islower((unsigned char)symb))
                return tolower((unsigned char)symb) - 'a' + 10;

        if(isupper((unsigned char)symb))
            return toupper((unsigned char)symb) - 'A' + 10;

        return -1;
}

static int parse_by_base(const char *word, int base, int upper, long long *result)
{
        if(base < MIN_BASE || base > MAX_BASE)
                base = DEFAULT_BASE;

        long long total = 0;
        int sign = 1;

        if(*word == '-') {
                sign = -1;
                word++;
        } else if(*word == '+')
            word++;

        if(*word == '\0')
                return -1;

        for(; *word != '\0'; word++) {
                if(upper && islower((unsigned char)*word))
                        return -1;

                if(!upper && isupper((unsigned char)*word))
                        return -1;

                int digit = digit_value(*word);

                if(digit < 0 || digit >= base)
                        return -1;

                total = total * (long long)base + (long long)digit;
        }

        *result = total * sign;

        return 0;
}

static int parse_zeckendorf(const char *word, unsigned int *result)
{
        unsigned long long fib[ZECKENDORF_LIMIT];

        fib[0] = 1;
        fib[1] = 2;

        int count = 2;

        while(count < ZECKENDORF_LIMIT &&
              fib[count - 1] <= (ULLONG_MAX - fib[count - 2]) / 2ULL + fib[count - 2]) {
                unsigned long long next = fib[count - 1] + fib[count - 2];

                if(next < fib[count - 1])
                        break;

                fib[count++] = next;
        }

        size_t len = strlen(word);

        if(len < 2 || word[len - 1] != '1')
                return -1;

        len--;

        unsigned long long total = 0;

        for(size_t i = 0; i < len; i++) {
                if(word[i] != '0' && word[i] != '1')
                        return -1;

                if(i >= (size_t)count)
                        return -1;

                if(word[i] == '1')
                        total += fib[i];
        }

        if(total > UINT_MAX_CAST)
                return -1;

        *result = (unsigned int)total;

        return 0;
}

static int scan_custom(InSource *source, const char *format, va_list args)
{
        int assigned = 0;

        while(*format != '\0') {
                if(*format != '%') {
                        format++;
                        continue;
                }

                format++;

                if(*format == '\0')
                    break;

                if(*format == '%') {
                        format++;
                        continue;
                }

                int peeked = skip_spaces(source);

                if(peeked == EOF)
                        break;

                source_ungetc(source, peeked);

                if(*format == 'R' && *(format + 1) == 'o') {
                        int *target = va_arg(args, int *);

                        if(target == NULL)
                                break;

                        char roman_buff[READ_BUFF_SIZE];

                        if(read_word(source, roman_buff, sizeof(roman_buff)) < 0)
                                break;

                        int value = 0;

                        if(parse_roman(roman_buff, &value) == 0) {
                                *target = value;
                                assigned++;
                        }

                        format += 2;
                        continue;
                }

                if(*format == 'Z' && *(format + 1) == 'r') {
                        unsigned int *target = va_arg(args, unsigned int *);

                        if(target == NULL)
                                break;

                        char zeck_buff[READ_BUFF_SIZE];

                        if(read_word(source, zeck_buff, sizeof(zeck_buff)) < 0)
                                break;

                        unsigned int value = 0;

                        if(parse_zeckendorf(zeck_buff, &value) == 0) {
                                *target = value;
                                assigned++;
                        }

                        format += 2;
                        continue;
                }

                if(*format == 'C' && (*(format + 1) == 'v' || *(format + 1) == 'V')) {
                        int upper = (*(format + 1) == 'V');
                        int base = va_arg(args, int);
                        int *target = va_arg(args, int *);

                        if(target == NULL)
                                break;

                        char num_buff[READ_BUFF_SIZE];

                        if(read_word(source, num_buff, sizeof(num_buff)) < 0)
                                break;

                        long long value = 0;

                        if(parse_by_base(num_buff, base, upper, &value) == 0) {
                                *target = (int)value;
                                assigned++;
                        }

                        format += 2;
                        continue;
                }

                if(strchr("diuxo", *format) != NULL) {
                        int *target = va_arg(args, int *);

                        if(target == NULL)
                                break;

                        char num_buff[READ_BUFF_SIZE];

                        if(read_word(source, num_buff, sizeof(num_buff)) <= 0)
                                break;

                        int radix = (*format == 'x') ? 16 : ((*format == 'o') ? 8 : 10);
                        char *end_ptr = NULL;
                        long value = strtol(num_buff, &end_ptr, radix);

                        if(end_ptr != num_buff && *end_ptr == '\0') {
                                *target = (int)value;
                                assigned++;
                        }

                        format++;
                        continue;
                }

                format++;
        }

        return assigned;
}

int overfscanf(FILE *stream, const char *format, ...)
{
        if(stream == NULL || format == NULL)
                return -1;

        InSource source = { stream, NULL, 0 };
        va_list args;
        int assigned;

        va_start(args, format);
        assigned = scan_custom(&source, format, args);
        va_end(args);

        return assigned;
}

int oversscanf(const char *input, const char *format, ...)
{
        if(input == NULL || format == NULL)
                return -1;

        InSource source = { NULL, input, 0 };
        va_list args;
        int assigned;

        va_start(args, format);
        assigned = scan_custom(&source, format, args);
        va_end(args);

        return assigned;
}

static int demo_sscanf(void)
{
        int roman = 0;
        unsigned int zeck = 0;
        int lower = 0;
        int upper = 0;
        int decimal = 0;

        int assigned = oversscanf("MMXXIV 001010000101 ff FF 12345",
                                  "%Ro %Zr %Cv %CV %d",
                                  &roman, &zeck, 16, &lower, 16, &upper, &decimal);

        printf("oversscanf: присвоено %d значений\n", assigned);
        printf("%%Ro из \"MMXXIV\"          -> %d\n", roman);
        printf("%%Zr из \"001010000101\"    -> %u\n", zeck);
        printf("%%Cv из \"ff\" (база 16)    -> %d\n", lower);
        printf("%%CV из \"FF\" (база 16)    -> %d\n", upper);
        printf("%%d из \"12345\"            -> %d\n", decimal);

        return assigned;
}

static int demo_fscanf(void)
{
        char line[] = "IV 101101 1010\n";

        FILE *tmp_file = tmpfile();

        if(tmp_file == NULL) {
                puts("Не удалось создать временный файл");
                return -1;
        }

        fputs(line, tmp_file);
        rewind(tmp_file);

        int roman = 0;
        unsigned int zeck = 0;
        int binary = 0;

        int assigned = overfscanf(tmp_file, "%Ro %Zr %Cv", &roman, &zeck, 2, &binary);

        fclose(tmp_file);

        printf("\noverfscanf из потока: присвоено %d значений\n", assigned);
        printf("%%Ro из \"IV\"       -> %d\n", roman);
        printf("%%Zr из \"101101\"   -> %u\n", zeck);
        printf("%%Cv из \"1010\"     -> %d\n", binary);

        return assigned;
}

int main(void)
{
        demo_sscanf();
        demo_fscanf();

        return 0;
}
