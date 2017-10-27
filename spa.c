#include <stdio.h> // printf, snprintf, FILE*
#include <string.h> // strerror, strlen
#include <stdlib.h> // abort
#include <stdint.h> // uint32_t, float32_t
#include <errno.h> // errno
#include "nicolet_spa.h"

// Based on code by Kurt Oldenburg - 06/28/16
// NOTE: this code assumes that your platform has 32-bit IEEE floats and CHAR_BIT == 8

typedef union {
	uint8_t u8[2];
	uint16_t u16;
} endianness_test;

// byte swapping will occur for each `size`-sized item
static size_t fread_le(void * ptr, size_t size, size_t nmemb, FILE* stream) {
	{
		size_t ret = 0;
		if ((ret = fread(ptr, size, nmemb, stream)) != nmemb) return ret;
	}

	if (((endianness_test){.u16 = 0x0100}).u8[0]) { // host is big-endian => must byteswap
		char item[size]; // yes, this is C99 VLA

		for (size_t i = 0; i < nmemb; i++) {
			memcpy(item, (char*)ptr + i*size, size);
			for (size_t j = 0; j < size; j++)
				*((char*)ptr + i*size + j) = item[size-j-1];
		}
	}

	return nmemb;
}

enum spa_parse_result spa_parse(
	const char * filename,
	char comment[256], size_t * num_points,
	float ** wavelengths, float ** intensities
) {
	*wavelengths = *intensities = NULL; // make it easier to clean up
	enum spa_parse_result ret = spa_ok;
	FILE* fh = fopen(filename, "rb");
	if (!fh) {
		ret = spa_open_error;
		goto cleanup;
	}
	// do the comment
	if (fseek(fh,30,SEEK_SET)) {
		ret = spa_seek_error;
		goto cleanup;
	}
	if (fread(comment, 255, 1, fh) != 1) { // no byte-swapping needed here
		ret = spa_read_error;
		goto cleanup;
	}
	comment[255] = '\0';

	uint32_t local_num_points;
	// get number of points
	if (fseek(fh,564,SEEK_SET)) {
		ret = spa_seek_error;
		goto cleanup;
	}
	if (fread_le(&local_num_points, sizeof(uint32_t), 1, fh) != 1) {
		ret = spa_read_error;
		goto cleanup;
	}
	// hopefully, your either your platform is 64-bit or you don't store 2**32 spectra in the first place
	*num_points = local_num_points;

	// get max,min wavenumber
	float wn_maxmin[2];
	if (fseek(fh,576,SEEK_SET)) {
		ret = spa_seek_error;
		goto cleanup;
	}
	if (fread_le(wn_maxmin, sizeof(float), 2, fh) != 2) {
		ret = spa_read_error;
		goto cleanup;
	}

	// locate the data
	uint16_t flag=0, offset=0;
	if (fseek(fh,288,SEEK_SET)) {
		ret = spa_seek_error;
		goto cleanup;
	}
	do {
		if (fread_le(&flag, sizeof(uint16_t), 1, fh) != 1) {
			ret = spa_read_error;
			goto cleanup;
		}
	} while (flag != 3);
	if (fread_le(&offset, sizeof(uint16_t), 1, fh) != 1) {
		ret = spa_read_error;
	}

	// read output spectral data
	*intensities = calloc(*num_points,sizeof(float));
	*wavelengths = calloc(*num_points,sizeof(float));
	if (!intensities || !wavelengths) {
		ret = spa_alloc_error;
		goto cleanup;
	}
	if (fseek(fh,offset,SEEK_SET)) {
		ret = spa_seek_error;
		goto cleanup;
	}
	if (fread(*intensities, sizeof(float), *num_points, fh) != *num_points) {
		ret = spa_read_error;
		goto cleanup;
	}
	for (size_t i = 0; i < *num_points; i++)
		(*wavelengths)[i] = wn_maxmin[0] - (wn_maxmin[0]-wn_maxmin[1])*i / *num_points;

cleanup:
	if (fh) fclose(fh);
	if (ret) {
		free(*wavelengths);
		free(*intensities);
	}

	return ret;
}
