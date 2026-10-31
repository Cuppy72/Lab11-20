#include <stdlib.h>
#include <stdio.h>

#include "patterns.h"
#include "self_errors.h"

int create_table(unsigned int count_symbs, Hash_table **user_table)
{
	Hash_table* new_table = malloc(sizeof(Hash_table));

	if(new_table == NULL)
		return ALLOC_ERROR;

	new_table->count_symbs = 0;
	new_table->max_count = count_symbs;

	new_table->symbols = malloc(sizeof(char) * count_symbs);

	if(new_table->symbols == NULL) {
		free(new_table);
		
		return ALLOC_ERROR;
	}

	new_table->occurrences_num = malloc(sizeof(int) * count_symbs);

	if(new_table->occurrences_num == NULL) {
		free(new_table->symbols);
		free(new_table);

		return ALLOC_ERROR;
	}

	*user_table = new_table;

	return SUCCESS;
}

void free_table(Hash_table *user_table)
{
	free(user_table->symbols);
	free(user_table->occurrences_num);

	free(user_table);
}

static int equal_symbols(char left, char right)
{
	if(left == '\r' && right == '\n')
		return 1;

	if(left == '\n' && right == '\r')
		return 1;

	return left == right;
}

static int to_table(Hash_table *user_table, int pattern_len, char symb, int symb_ind)
{
	if(user_table == NULL || user_table->symbols == NULL || user_table->occurrences_num == NULL)
		return INCORRECT_TABLE;

	if(user_table->max_count == 0)
		return MAX_COUNT_LIMIT_REACHED;
	
	int i = 0;

	for(; i < user_table->count_symbs; i++) {
		char cur_symb = user_table->symbols[i];

		if(equal_symbols(cur_symb, symb)) {
			int shift = pattern_len - symb_ind - 1;

			user_table->symbols[i] = symb;
			user_table->occurrences_num[i] = (1 > shift) ? 1 : shift;

			return SUCCESS;
		}
	}

	int shift = pattern_len - symb_ind - 1;

	user_table->symbols[i] = symb;
	user_table->occurrences_num[i] = (1 > shift) ? 1 : shift;

	user_table->count_symbs++;

	return SUCCESS;
}

static int out_table(Hash_table *user_table, char symb)
{
	if(user_table == NULL || user_table->symbols == NULL || user_table->occurrences_num == NULL)
		return INCORRECT_TABLE;

	if(user_table->count_symbs == 0)
		return EMPTY_TABLE;

	for(int i = 0; i < user_table->count_symbs; i++) {
		char cur_symb = user_table->symbols[i];

		if(equal_symbols(symb, cur_symb))
			return user_table->occurrences_num[i];
	}

	return -1;
}


int fill_table(const char *pattern, int pattern_length, Hash_table *user_table)
{
	if(pattern == NULL) {
		return INCORRECT_STR;
	}

	if(pattern_length == 0)
		return EMPTY_PATTERN;

	if(user_table == NULL) {
		return INCORRECT_TABLE;
	}

	int err;

	for(int i = 0; i < pattern_length; i++) {
		err = to_table(user_table, pattern_length, pattern[i], i);

		if(err != SUCCESS) {
			return err;
		}
	}

	return SUCCESS;
}

int boer_mour(Hash_table *user_table, const char *pattern, int pattern_length, const char *str, int str_len, int *result)
{
	if(user_table == NULL || user_table->symbols == NULL || user_table->occurrences_num == NULL)
		return INCORRECT_TABLE;

	if(pattern == NULL)
		return INCORRECT_STR;

	if(str == NULL)
		return INCORRECT_STR;

	if(pattern_length == 0)
		return EMPTY_PATTERN;

	if(str_len == 0)
		return INCORRECT_STR;

	int i = 0;


	if(pattern_length == 1) {
		int start_ind = -1;

		for(int i = 0; i < str_len; i++)  {
			if(equal_symbols(str[i], pattern[0])) {
				start_ind = i;
				break;
			}
		}

		*result = start_ind;
		return SUCCESS;
	}

	while(i < str_len - pattern_length + 1) {
		int last_ind_str = i + pattern_length - 1;
		int last_ind_pattern = pattern_length - 1;
		
		int flag = 1;

		while(last_ind_pattern != 0) {
			if(str[last_ind_str] == '\0')
				last_ind_str--;

			if(equal_symbols(str[last_ind_str], pattern[last_ind_pattern])) {
				last_ind_str--;
				last_ind_pattern--;

				i--;
			} else {
				flag = 0;
				break;
			}
		}

		if(!flag) {
			int shift = out_table(user_table, str[last_ind_str]);

			if(shift == -1)
				i += pattern_length;
			else
				i += shift;
		} else {
			*result = last_ind_str;

			return SUCCESS;
		}
	}

	*result = -1;

	return SUCCESS;
}
