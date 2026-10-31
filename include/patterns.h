#ifndef __PATTERN__
#define __PATTERN__

typedef struct {
	char *symbols;
	unsigned int *occurrences_num;
	
	unsigned int count_symbs;
	unsigned int max_count;
} Hash_table;

int create_table(unsigned int count_symbs, Hash_table **user_table);
void free_table(Hash_table *user_table);

int fill_table(const char *pattern, int pattern_length, Hash_table *user_table);

int boer_mour(Hash_table *user_table, const char *pattern, int pattern_length, const char *str, int str_len, int *result);

#endif
