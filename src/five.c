#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_BASE 36
#define MIN_BASE 2
#define DEFAULT_BASE 10

#define ROMAN_COUNT 13
#define ZECKENDORF_LIMIT 100

static const int roman_values[ROMAN_COUNT] =
        {1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1};
static const char *roman_symbols[ROMAN_COUNT] =
        {"M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"};


static int build_zeckendorf(unsigned int value, char *digits, int capacity)
{
        unsigned long long fib[ZECKENDORF_LIMIT];

        fib[0] = 1;
        fib[1] = 2;

        int count = 2;

        while(fib[count - 1] <= (unsigned long long)value && count < ZECKENDORF_LIMIT) {
                fib[count] = fib[count - 1] + fib[count - 2];
                count++;
        }

        unsigned long long rest = (unsigned long long)value;
        int pos = 0;

        for(int i = count - 1; i >= 0; i--) {
                if(rest >= fib[i]) {
                        rest -= fib[i];
                        digits[pos++] = '1';
                } else
                        digits[pos++] = '0';

                if(pos >= capacity)
                        return -1;
        }

        if(pos == 0)
                digits[pos++] = '0';

        for(int i = 0; i < pos / 2; i++) {
                char swap = digits[i];

                digits[i] = digits[pos - 1 - i];
                digits[pos - 1 - i] = swap;
        }

        digits[pos++] = '1';
        digits[pos] = '\0';

        return pos;
}



static int read_digit_value(char symb)
{
        if(isdigit((unsigned char)symb))
                return symb - '0';

        if(islower((unsigned char)symb))
                return symb - 'a' + 10;

        if(isupper((unsigned char)symb))
                return symb - 'A' + 10;

        return -1;
}

static int parse_by_base(const char *str, int base, long long *result)
{
        if(base < MIN_BASE || base > MAX_BASE)
                base = DEFAULT_BASE;

        long long total = 0;
        int sign = 1;

        if(*str == '-') {
                sign = -1;
                str++;
        } else if(*str == '+')
            str++;

        if(*str == '\0')
                return 0;

        while(*str != '\0') {
                int digit = read_digit_value(*str);

                if(digit < 0 || digit >= base)
                    return 0;

                total = total * (long long)base + (long long)digit;
		
		if(total > LLONG_MAX / (long long)base - (long long)digit)
			return 0;

                str++;
        }

        *result = total * sign;

        return 1;
}



static int count_ll(long long value)
{
        char scratch[32];

        return (int)sprintf(scratch, "%lld", value);
}
static int count_roman(int value)
{
        int written = 0;

        if(value <= 0)
                return count_ll(value);

        for(int i = 0; i < ROMAN_COUNT; i++)
                while(value >= roman_values[i]) {
                        written += (int)strlen(roman_symbols[i]);
                        value -= roman_values[i];
                }

        return written;
}


static int count_by_base(int value, int base)
{
        if(base < MIN_BASE || base > MAX_BASE)
                base = DEFAULT_BASE;

        int count = value < 0 ? 1 : 0;
        unsigned int rest = (value < 0) ? (unsigned int)(-(long long)value)
                                        : (unsigned int)value;

        if(rest == 0)
                return 1;

        while(rest > 0) {
                rest /= (unsigned int)base;
                count++;
        }

        return count;
}

typedef struct OutSink
{
        FILE *stream;
        char *buffer;
        size_t capacity;
        size_t length;
} OutSink;

static void sink_write(OutSink *sink, const char *data, size_t len)
{
        for(size_t i = 0; i < len; i++) {
                if(sink->buffer != NULL) {
                        if(sink->length + 1 < sink->capacity)
                                sink->buffer[sink->length] = data[i];

                        sink->length++;
                } else
                        fputc(data[i], sink->stream);
        }
}

static int sink_printf(OutSink *sink, const char *format, ...)
{
        va_list args;
        int written;

        va_start(args, format);
        written = vsnprintf(NULL, 0, format, args);
        va_end(args);

        if(written > 0) {
                char stack_chunk[128];
                char *chunk = stack_chunk;
                size_t need = (size_t)written + 1;
                int allocated = 0;

                if(need > sizeof(stack_chunk)) {
                        chunk = (char *)malloc(need);

                        if(chunk == NULL)
                                return written;

                        allocated = 1;
                }

                va_start(args, format);
                vsnprintf(chunk, need, format, args);
                va_end(args);

                sink_write(sink, chunk, (size_t)written);

                if(allocated)
                        free(chunk);
        }

        return written;
}

static int print_roman_sink(OutSink *output, int value)
{
        if(value <= 0)
                return sink_printf(output, "%d", value);

        int written = 0;

        for(int i = 0; i < ROMAN_COUNT; i++) {
                while(value >= roman_values[i]) {
                        written += sink_printf(output, "%s", roman_symbols[i]);
                        value -= roman_values[i];
                }
        }

        return written;
}

static int print_zeckendorf_sink(OutSink *output, unsigned int value)
{
        char digits[ZECKENDORF_LIMIT + 2];

        if(build_zeckendorf(value, digits, ZECKENDORF_LIMIT) < 0)
                return sink_printf(output, "%u", value);

        return sink_printf(output, "%s", digits);
}

static int print_by_base_sink(OutSink *output, int value, int base, int upper)
{
        if(base < MIN_BASE || base > MAX_BASE)
                base = DEFAULT_BASE;

        char digits[64];
        int count = 0;

        unsigned int rest = (value < 0) ? (unsigned int)(-(long long)value)
                                        : (unsigned int)value;

        if(rest == 0)
                digits[count++] = '0';

        while(rest > 0) {
                unsigned int digit = rest % (unsigned int)base;

                if(digit < 10)
                        digits[count++] = (char)('0' + digit);
                else
                        digits[count++] = upper ? (char)('A' + digit - 10)
                                                : (char)('a' + digit - 10);

                rest /= (unsigned int)base;
        }

        if(value < 0)
                sink_write(output, "-", 1);

        for(int i = count - 1; i >= 0; i--)
                sink_write(output, &digits[i], 1);

        return count + (value < 0 ? 1 : 0);
}

static void print_mem_dump_sink(OutSink *output, const void *value, size_t size)
{
        const unsigned char *bytes = (const unsigned char *)value;
        char bits[9];

        for(size_t i = 0; i < size; i++) {
                for(int b = 0; b < 8; b++)
                        bits[b] = (char)('0' + ((bytes[i] >> (7 - b)) & 1u));

                bits[8] = '\0';

                sink_write(output, bits, 8);

                if(i + 1 < size)
                        sink_write(output, " ", 1);
        }
}

static int custom_format(OutSink *output, const char *format, va_list args)
{
        int written = 0;

        while(*format != '\0') {
                if(*format != '%') {
                        sink_write(output, format, 1);
                        written++;
                        format++;
                        continue;
                }

                format++;

                if(*format == '\0') {
                        sink_write(output, "%", 1);
                        written++;
                        break;
                }

                if(*format == '%') {
                    sink_write(output, "%", 1);
                    written++;
                    format++;
                    continue;
                }

                if(*format == 'R' && *(format + 1) == 'o') {
                        written += print_roman_sink(output, va_arg(args, int));
                        format += 2;
                        continue;
                }

                if(*format == 'Z' && *(format + 1) == 'r') {
                        written += print_zeckendorf_sink(output,
                                                         va_arg(args, unsigned int));
                        format += 2;
                        continue;
                }

                if(*format == 'C' && (*(format + 1) == 'v' || *(format + 1) == 'V')) {
                        int value = va_arg(args, int);
                        int base = va_arg(args, int);

                        written += print_by_base_sink(output, value, base,
                                                      *(format + 1) == 'V');
                        format += 2;
                        continue;
                }

                if((*format == 't' && *(format + 1) == 'o') ||
                   (*format == 'T' && *(format + 1) == 'O')) {
                        const char *str = va_arg(args, const char *);
                        int base = va_arg(args, int);

                        long long value = 0;

                        if(str == NULL)
                                written += sink_printf(output, "(null)");
                        else if(parse_by_base(str, base, &value))
                                written += sink_printf(output, "%lld", value);
                        else
                                written += sink_printf(output, "%s", str);

                        format += 2;
                        continue;
                }

                if(*format == 'm' && (*(format + 1) == 'i' || *(format + 1) == 'u' ||
                                      *(format + 1) == 'd' || *(format + 1) == 'f')) {
                        char flag = *(format + 1);

                        if(flag == 'i') {
                                int value = va_arg(args, int);

                                print_mem_dump_sink(output, &value, sizeof(value));
                                written += (int)sizeof(int) * 8 + (int)sizeof(int) - 1;
                        } else if(flag == 'u') {
                                unsigned int value = va_arg(args, unsigned int);

                                print_mem_dump_sink(output, &value, sizeof(value));
                                written += (int)sizeof(unsigned int) * 8
                                           + (int)sizeof(unsigned int) - 1;
                        } else if(flag == 'd') {
                                double value = va_arg(args, double);

                                print_mem_dump_sink(output, &value, sizeof(value));
                                written += (int)sizeof(double) * 8 + (int)sizeof(double) - 1;
                        } else {
                                float value = (float)va_arg(args, double);

                                print_mem_dump_sink(output, &value, sizeof(value));
                                written += (int)sizeof(float) * 8 + (int)sizeof(float) - 1;
                        }

                        format += 2;
                        continue;
                }

                if(strchr("dDiuXxocsefFgGp", *format) != NULL) {
                        written += sink_printf(output, "%c%c", '%', *format);
                        format++;
                        continue;
                }

                sink_write(output, "%", 1);
                written++;
        }

        return written;
}

static int count_format(const char *format, va_list args)
{
        int written = 0;

        while(*format != '\0') {
                if(*format != '%') {
                        written++;
                        format++;
                        continue;
                }

                format++;

                if(*format == '\0') {
                        written++;
                        break;
                }

                if(*format == '%') {
                        written++;
                        format++;
                        continue;
                }

                if(*format == 'R' && *(format + 1) == 'o') {
                        written += count_roman(va_arg(args, int));
                        format += 2;
                        continue;
                }

                if(*format == 'Z' && *(format + 1) == 'r') {
                        char digits[ZECKENDORF_LIMIT + 2];
                        unsigned int value = va_arg(args, unsigned int);

                        if(build_zeckendorf(value, digits, ZECKENDORF_LIMIT) > 0)
                                written += (int)strlen(digits);
                        else
                                written += count_ll((long long)value);

                        format += 2;
                        continue;
                }

                if(*format == 'C' && (*(format + 1) == 'v' || *(format + 1) == 'V')) {
                        int value = va_arg(args, int);
                        int base = va_arg(args, int);

                        written += count_by_base(value, base);
                        format += 2;
                        continue;
                }

                if((*format == 't' && *(format + 1) == 'o') ||
                   (*format == 'T' && *(format + 1) == 'O')) {
                        const char *str = va_arg(args, const char *);
                        int base = va_arg(args, int);

                        long long value = 0;

                        if(str == NULL)
                                written += 6;
                        else if(parse_by_base(str, base, &value))
                                written += count_ll(value);
                        else
                                written += (int)strlen(str);

                        format += 2;
                        continue;
                }

                if(*format == 'm' && (*(format + 1) == 'i' || *(format + 1) == 'u' ||
                                      *(format + 1) == 'd' || *(format + 1) == 'f')) {
                        char flag = *(format + 1);

                        if(flag == 'i') {
                                (void)va_arg(args, int);
                                written += (int)sizeof(int) * 8 + (int)sizeof(int) - 1;
                        } else if(flag == 'u') {
                                (void)va_arg(args, unsigned int);
                                written += (int)sizeof(unsigned int) * 8
                                           + (int)sizeof(unsigned int) - 1;
                        } else if(flag == 'd') {
                                (void)va_arg(args, double);
                                written += (int)sizeof(double) * 8 + (int)sizeof(double) - 1;
                        } else {
                                (void)va_arg(args, double);
                                written += (int)sizeof(float) * 8 + (int)sizeof(float) - 1;
                        }

                        format += 2;
                        continue;
                }

                if(strchr("dDiuXxocsefFgGp", *format) != NULL) {
                        written += 2;
                        format++;
                        continue;
                }

                written++;
        }

        return written;
}

int overfprintf(FILE *file, const char *format, ...)
{
        if(file == NULL || format == NULL)
                return -1;

        OutSink sink = { file, NULL, 0, 0 };
        va_list args;
        int written;

        va_start(args, format);
        written = custom_format(&sink, format, args);
        va_end(args);

        return written;
}

int oversprintf(char *buffer, const char *format, ...)
{
        if(buffer == NULL || format == NULL)
                return -1;

        va_list args;
        int needed;

        va_start(args, format);
        needed = count_format(format, args);
        va_end(args);

        size_t capacity = (size_t)needed + 1;

        OutSink sink = { NULL, buffer, capacity, 0 };

        va_start(args, format);
        int written = custom_format(&sink, format, args);
        va_end(args);

        buffer[sink.length] = '\0';

        return written;
}

static int demo_output(void)
{
        puts("Демонстрация работы overfprintf:");
        overfprintf(stdout, "Римское число: %Ro\n", 2024);
        overfprintf(stdout, "Цекендорфово представление: %Zr\n", 100u);
        overfprintf(stdout, "В системе счисления 16 (нижний регистр): %Cv\n", -255, 16);
        overfprintf(stdout, "В системе счисления 16 (верхний регистр): %CV\n", -255, 16);
        overfprintf(stdout, "Перевод из 16-ричной в десятичную: %to\n", "ff", 16);
        overfprintf(stdout, "Перевод из 16-ричной в десятичную: %TO\n", "FF", 16);
        overfprintf(stdout, "Дамп int: %mi\n", 1);
        overfprintf(stdout, "Дамп unsigned int: %mu\n", 1u);
        overfprintf(stdout, "Дамп double: %md\n", 1.0);
        overfprintf(stdout, "Дамп float: %mf\n", 1.0f);

        return 0;
}

static int demo_sprintf(void)
{
        char buffer[128];

        oversprintf(buffer, "%Ro", 9);
        printf("oversprintf(\"%%Ro\", 9) = \"%s\"\n", buffer);

        oversprintf(buffer, "%Zr", 20u);
        printf("oversprintf(\"%%Zr\", 20) = \"%s\"\n", buffer);

        oversprintf(buffer, "%Cv", 255, 2);
        printf("oversprintf(\"%%Cv\", 255, 2) = \"%s\"\n", buffer);

        return 0;
}

int main(void)
{
        demo_output();
        demo_sprintf();

        return 0;
}
