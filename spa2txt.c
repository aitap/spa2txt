#include <stdio.h> // printf, snprintf, FILE*
#include <string.h> // strerror, strlen
#include <stdlib.h> // abort
#include <stdint.h> // uint32_t, float32_t
#include <errno.h> // errno

// Based on code by Kurt Oldenburg - 06/28/16

int main (int argc, char** argv) {
	for (argv++/*skip argv[0]*/; *argv/*argv has guaranteed NULL at the end*/; argv++) { 
		// prepare for reading
		FILE* fh = fopen(*argv, "rb");
		if (!fh) {
			printf("%s: %s\n", *argv, strerror(errno));
			return 1;
		}

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

		// TODO: check the header

		// do the comment
		char comment[256];
		if (fseek(fh,30,SEEK_SET)) {
			printf("%s: couldn't seek for comment\n", *argv);
			return 1;
		}
		if (fread(comment, 255, 1, fh) != 1) {
			printf("%s: comment: short read for comment\n", *argv);
			return 1;
		}
		comment[255] = '\0';
		if(fprintf(wfh,"%s\n",comment) < 0) {
			printf("%s.txt: write failed", *argv);
			return 1;
		}

		// get number of points
		int32_t num_points = 0; // FIXME: intel byte order assumed
		if (fseek(fh,564,SEEK_SET)) {
			printf("%s: couldn't seek for num_points\n", *argv);
			return 1;
		}
		if (fread(&num_points, 4, 1, fh) != 1) {
			printf("%s: couldn't read num_points\n", *argv);
			return 1;
		}

		// get max,min wavenumber
		float wn_maxmin[2] = {}; // 32-bit IEEE goes without saying
		if (fseek(fh,576,SEEK_SET)) {
			printf("%s: couldn't seek for max/min wavenumbers\n", *argv);
			return 1;
		}
		if (fread(wn_maxmin, 8, 1, fh) != 1) {
			printf("%s: couldn't read man/min wavenumbers\n", *argv);
			return 1;
		}

		// locate the data
		uint16_t flag=0, offset=0; // FIXME: intel byte order assumed
		if (fseek(fh,288,SEEK_SET)) {
			printf("%s: couldn't seek for data offset\n", *argv);
			return 1;
		}
		while (flag != 3)
			if (fread(&flag, 2, 1, fh) != 1) {
				printf("%s: couldn't read flag\n", *argv);
				return 1;
			}
		if (fread(&offset, 2, 1, fh) != 1) {
			printf("%s: couldn't read data offset\n", *argv);
			return 1;
		}

		// read output spectral data
		float* spectrum = malloc(4*num_points); // once again, 32-bit IEEE floats
		if (!spectrum) abort();
		if (fseek(fh,offset,SEEK_SET)) {
			printf("%s: couldn't seek for data\n", *argv);
			return 1;
		}
		if (fread(spectrum, 4, num_points, fh) != num_points) {
			printf("%s: couldn't read data\n", *argv);
			return 1;
		}
		for (size_t i = 0; i < num_points; i++) {
			if(
				fprintf(
					wfh, "%g\t%g\n",
					wn_maxmin[0] - (wn_maxmin[0]-wn_maxmin[1])*i/num_points,
					spectrum[i]
				) < 0
			) {
				printf("%s: error while writing\n", *argv);
				return 1;
			}
		}
		free(spectrum);

		/* comments are identified by flag=4 and have length computed as abs(spectrum_offset-data_offset) ?! */
	}
	return 0;
}
