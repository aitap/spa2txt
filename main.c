#include <stdio.h> // printf, snprintf, FILE*
#include <string.h> // strerror, strlen
#include <stdlib.h> // abort
#include <errno.h> // errno
#include "nicolet_spa.h"


int main (int argc, char** argv) {
	if (!argc) return 0;

	for (argv++/*skip argv[0]*/; *argv/*argv has guaranteed NULL at the end*/; argv++) { 
		// prepare for writing
		FILE* wfh = NULL;
		{
			size_t textfile_len = strlen(*argv)/*without \0*/ + 5/*.txt\0*/;
			char* textfile = malloc(textfile_len);
			if (!textfile) abort();
			if (snprintf(textfile, textfile_len, "%s%s", *argv, ".txt") >= textfile_len)
				abort();
			wfh = fopen(textfile, "w");
			if (!wfh) {
				printf("%s: %s\n", textfile, strerror(errno));
				return 1;
			}
			free(textfile);
		}

		// prepare for reading
		char comment[256];
		size_t num_points;
		float * wavelengths, * intensities;
		enum spa_parse_result ret = spa_parse(*argv, comment, &num_points, &wavelengths, &intensities);
		if (ret) {
			printf("%s: parse error, code %d\n", *argv, (int)ret);
			return 2;
		}

		if (fprintf(wfh, "%s\n", comment) < 0) {
			printf("%s.txt: error while writing\n", *argv);
			return 3;
		}
		for (size_t i = 0; i < num_points; i++) {
			if(fprintf(wfh, "%g\t%g\n", wavelengths[i], intensities[i]) < 0) {
				printf("%s: error while writing\n", *argv);
				return 3;
			}
		}
		free(wavelengths);
		free(intensities);
	}
	return 0;
}
