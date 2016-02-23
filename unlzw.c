/*
 * Program name: 	unlzw - lzw decompressor
 * Program author:	Alex Oprea <ionutalexoprea@gmail.com>
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define NBITS (12)
#define DICT_SIZE (1<<NBITS)
#define DICT_SIZE_INIT (256)

char **d;	/* dictionary */
int *nb;	/* number of bytes for each entry in the dictionary */
int dsize;	/* dictionary size */


/* function headers */

/* functions that handle the dictionary */
void dict_init(void);
void dict_reset(void);
void dict_update(int const code);
void dict_free(void);
/* function that handles codes */
int get_codes(int *code1, int *code2, unsigned char const * const buf, int const num_bytes_next);

/* function that handles decompression */
int decompress_lzw(char const * const file_in, char const * const file_out);


/* dict_init - initialize the dictionary
 * Initializes the dictionary to the values agreed with the coder/compressor.
 */
void dict_init(void)
{
	int i;

	dsize = DICT_SIZE_INIT;
	for (i = 0; i < dsize; i++) {
		d[i] = (char *) calloc(1, sizeof(char));
		d[i][0] = i;
		nb[i] = 1;
	}
}


/* dict_reset - reset the dictionary
 * Reset the dictionary to the initial content.
 */
void dict_reset(void)
{
	int i;

	for (i = DICT_SIZE_INIT; i < DICT_SIZE; i++) {
		if (d[i]) {
			free(d[i]);
			d[i] = NULL;
		}
		nb[i] = 0;
	}

	dsize = DICT_SIZE_INIT;
}


/* dict_update - update the dictionary with a new entry
 * @code: an existing code in the dictionary
 *
 * Update the dictionary with a new conjecture based on the
 * already known entry corresponding to code.
 */
void dict_update(int const code)
{
	/* update the last char in the last conjecture */
	if (dsize - 1 >= DICT_SIZE_INIT) { /* but do not modify the initial dictionary */
		char c, *tmp;
		int pos;

		tmp = d[dsize-1];
		pos = nb[dsize-1] - 1;
		c = d[code][0];
		tmp[pos] = c;
	}

	/* full dictionary */
	if (dsize == DICT_SIZE)
		dict_reset();

	/* insert a new entry in the dictionary */
	nb[dsize] = nb[code] + 1;
	d[dsize] = (char *)calloc(nb[dsize], sizeof(char));
	memcpy(d[dsize], d[code], nb[code]);
	d[dsize][nb[dsize]-1] = d[dsize][0];

	dsize++;
}


/* dict_free - release the memory
 */
void dict_free(void)
{
	int i;

	for (i = 0; i < dsize; i++) {
		if (d[i]) {
			free(d[i]);
			d[i] = NULL;
		}
		nb[i] = 0;
	}
}


/* get_codes - parses 3 bytes to get 2 codes
 * @code1: the first code included in the 3 bytes
 * @code2: the second code included in the 3 bytes
 * @buf: the buffer with the 3 bytes
 * @num_bytes_next: the length of the next buffer
 *
 * If num_bytes_next is not 3, then the input stream is about to end.
 *
 * @return -1 if error
 * @return 0 if code parsing should continue. 2 codes parsed. (regular case)
 * @return 1 if code parsing should stop. 1 code parsed.
 * @return 2 if code parsing should stop. 2 codes parsed.
 */
int get_codes(int *code1, int *code2, unsigned char const * const buf, int const num_bytes_next)
{
	*code1 = *code2 = 0;

	/* error case, should not occur */
	if (num_bytes_next < 0 || num_bytes_next > 3)
		return -1;

	/* case: odd number of codes
	 * last code uses 2 bytes
	 * fread adds 1 byte for end of line/newline feed
	 * all these 3 bytes are in buffer
	 * and num_bytes_next is 0
	 */
	if (num_bytes_next == 0) {
		/* use the last 12 bits from bytes 0 and 1 to build code1 */
		/* *code1 = ((0x0f & buf[0]) << 8) + buf[1]; */
		*code1 = (buf[0] << 4) + ((buf[1] & 0xf0) >> 4);
		/* code2 remains 0 */
		return 1;
	}

	/* code1 is the first 12 bits:
	 * entire first byte and half of the second byte
	 */
	*code1 = (buf[0] << 4) + ((buf[1] & 0xf0) >> 4);

	/* code2 is the last 12 bits:
	 * second half of the second byte and the entire last byte
	 */
	*code2 = ((buf[1] & 0x0f) << 8) + buf[2];

	/* if only end of file will follow, return the 2 codes and stop the parsing */
	if (num_bytes_next == 1)
		return 2;

	/* an entire 3 bytes chunk follows, keep parsing */
	return 0;

}


/* decompress_lzw - lzw decompressor
 * @file_in - filename of the file containing the lzw-compressed input (to be read)
 * @file_out - filename of the file containing the lzw-decompressed output (to be written)
 *
 * This function reads a file specified by filename file_in containing lzw-compressed input
 * and writes a file specified by filename file_out containing the lzw-decompressed text.
 *
 * @return 0 on success
 * @return 1 otherwise
 */
int decompress_lzw(char const * const file_in, char const * const file_out)
{
	FILE *fin = fopen(file_in, "r");
	FILE *fout = fopen(file_out, "w");
	int rv, nbytes, code1, code2;
	unsigned char buf[3], bufnext[3];

	if (!fin) {
		perror("Cannot open input file");
		if (fout)
			fclose(fout);
		return 1;
	}
	if (!fout) {
		perror("Cannot open/create output file");
		if (fin)
			fclose(fin);
		return 1;
	}

	memset(buf, 0, 3);
	memset(bufnext, 0, 3);
	nbytes = fread(buf, sizeof(unsigned char), 3, fin);
	do {
		nbytes = fread(bufnext, sizeof(unsigned char), 3, fin);

		rv = get_codes(&code1, &code2, buf, nbytes);

		if (rv == -1)
			break;

		fwrite(d[code1], sizeof(char), nb[code1], fout);
		dict_update(code1);

		if (rv == 0 || rv == 2) {
			fwrite(d[code2], sizeof(char), nb[code2], fout);
			dict_update(code2);
		}

		if (rv == 1 || rv == 2)
			break;

		memcpy(buf, bufnext, 3);
		memset(bufnext, 0, 3);
	} while (nbytes == 3);

	rv = fclose(fin);
	rv += fclose(fout);

	if (rv)
		return 1;

	return 0;
}


/* main - main function
 *
 * Should be started like:
 *	./unlzw input output
 * where
 *	- input is the compressed file name (to be read)
 *	- output is the decompressed file name (to be written)
 *
 * @return EXIT_SUCCESS on success
 * @return EXIT_FAILURE otherwise
 */
int main(int argc, char *argv[])
{
	int rv;

	if (argc != 3) {
		printf("usage: %s input output\n", argv[0]);
		return EXIT_FAILURE;
	}

	d = (char **) calloc(DICT_SIZE + 1, sizeof(char *));
	nb = (int *) calloc(DICT_SIZE + 1, sizeof(int));

	dict_init();

	rv = decompress_lzw(argv[1], argv[2]);

	dict_free();

	if (rv)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
