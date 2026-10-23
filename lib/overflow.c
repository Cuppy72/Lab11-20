#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#include "overflow.h"

static int correct_int(char *num, int *len_num, int *lead_zeros, char *sign)
{
	if(*num == '+' || *num == '-') {
		*sign = *num;
		num++;
	}

	char flag = 1;
	while(*num != '\0') {
		if(!isdigit(*num))
			return 0;

		if(flag && *num == '0') {
			num++;
			(*lead_zeros)++;
			continue;
		} else
			flag = 0;
		
		(*len_num)++;
		num++;
	}

	if(flag) {
		(*len_num) = 1;
		(*lead_zeros)--;
		return 1;
	}

	return 1;
}

int int_overflow(char *user_num, char *max_num, char *min_num)
{
	int len_user = 0, len_max = 0, len_min = 0;
	int user_lead_zeros = 0, max_lead_zeros = 0, min_lead_zeros = 0;

	char *ptr_user = user_num, *ptr_max = max_num, *ptr_min = min_num;
	char sign_user = '+', sign_max = '+', sign_min = '+';

	if(!correct_int(ptr_user, &len_user, &user_lead_zeros, &sign_user))
		return -2;
	if(!correct_int(ptr_max, &len_max, &max_lead_zeros, &sign_max))
		return -2;
	if(!correct_int(ptr_min, &len_min, &min_lead_zeros, &sign_min))
		return -2;

	if (sign_user == '+') {
		if (sign_max == '-' || len_user > len_max) return 1; 
		if (sign_max == '+' && len_user < len_max) return 0; 
		if (sign_max == '+' && len_user == len_max) {
			int i = user_lead_zeros, j = max_lead_zeros;

			if(user_num[i] == '-' || user_num[i] == '+')
				i++;
			if(max_num[j] == '-' || max_num[j] == '+')
				j++;
			
			while (user_num[i] != '\0' && max_num[j] != '\0') {
				if (user_num[i] > max_num[j]) return 1;
				if (user_num[i] < max_num[j]) return 0;
				i++;
				j++;
			}
			
			return 0;
		}

		return 0;
	} else if (sign_user == '-') {
		if (sign_min == '+' || len_user > len_min) return -1; 
		if (sign_min == '-' && len_user < len_min) return 0; 
		if (sign_min == '-' && len_user == len_min) {
			int i = user_lead_zeros, j = min_lead_zeros;

			if(user_num[i] == '-' || user_num[i] == '+')
				i++;
			if(min_num[j] == '-' || min_num[j] == '+')
				j++;

			while (user_num[i] != '\0' && min_num[j] != '\0') {
				if (user_num[i] > min_num[j]) return -1;
				if (user_num[i] < min_num[j]) return 0;
				i++;
				j++;
			}
			
			return 0;
		}

		return 0;
	}
}

