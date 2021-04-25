#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void hexdump(const char *s, size_t len, size_t row_len, int show_chars, int colors)
{
	int b = 0;
	int xc_offset = 0;
	int cw = 0;
	int is_printable = 0;

	for (int i = 0; i < len; ++i) {
		if (b%row_len == 0)
			printf("[%04x]\t", i);

		if ((isalpha(s[i]) || ispunct(s[i]) || isdigit(s[i])) && colors > 0) {
			is_printable = 1;
			printf("\033[01;9%dm", colors);
		} else
			is_printable = 0;

		printf("%02X\033[0m ", s[i] & 0xFF);

		b++;

		if (b == row_len/2)
			printf("  ");

		if (b%row_len == 0 || i + 1 == len) {
			if (show_chars) {
				for (int p = 0; p < (3*row_len) - (3*b); ++p)
					printf(" ");

				printf("\t| ");

				if (i + 1 == len)
					xc_offset = ((i - row_len) + 1) + (row_len - b);
				else
					xc_offset = (i - row_len) + 1;

				cw = 0;
				for (int x = xc_offset; x < i + 1; ++x) {
					if (isalpha(s[x]) || ispunct(s[x]) || isdigit(s[x]))
						if ((int)s[x] == 0x20)
							printf(".");
						else {
							if ((isalpha(s[x]) || ispunct(s[x]) || isdigit(s[x])) && colors > 0)
								printf("\033[01;9%dm", colors);
							printf("%c\033[0m", s[x]);
						}
					else
						printf(".");
					cw++;
				}


				for (int p = 0; p < (row_len - cw); ++p)
					printf(" ");

				printf(" |");
			}

			printf("\n");
			b = 0;
		}
	}

	printf("\n");
}
