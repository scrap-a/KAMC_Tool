#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kamc_tool.h"

int main(int argc, char** argv) {
	int ret=0;
	FILE* fin, * fout;
	unsigned char *in;
	unsigned char *out;
	int isize, osize;
	char option[8];
	int c1 = 0;
	int c2 = 0;
	
	if (argc < 4) {
		fprintf(stderr, "kcm_tool.exe option [in.bin] [out.bin]\n");
		fprintf(stderr, " option:\n");
		fprintf(stderr, "  e:encode\n");
		fprintf(stderr, "  d:decode\n");
		fprintf(stderr, " example:\n");
		fprintf(stderr, "kcm_tool.exe d ANTADV.BIN ""Adventure (1984) (Konami) (J).rom""\n");
		return -1;
	}
	for (c1 = 0; c1 <= (int)strlen(argv[1]) && c1 <= 8; c1++) {
		if (argv[1][c1] == 'd' || argv[1][c1] == 'D') {
			option[c2] = argv[1][c1];
			c2++;
		}
		if (argv[1][c1] == 'e' || argv[1][c1] == 'E') {
			option[c2] = argv[1][c1];
			c2++;
		}
	}
	argv[1][c2] = '\0';

	if ((fin = fopen(argv[2], "rb")) == 0) {
		fprintf(stderr, "input file open error:%s\n",argv[2]);
		return -1;
	}
	if ((fout = fopen(argv[3], "wb")) == 0) {
		fprintf(stderr, "output file open error:%s\n",argv[3]);
		return -1;
	}

	in  = (unsigned char*)malloc(sizeof(unsigned char) * MAX_SIZE);
	out = (unsigned char*)malloc(sizeof(unsigned char) * MAX_SIZE);

	memset(in,  0, MAX_SIZE);
	memset(out, 0, MAX_SIZE);

	fseek(fin, 0, SEEK_END);
	isize = (int)ftell(fin);
	fseek(fin, 0, SEEK_SET);

	isize = isize < MAX_SIZE ? isize : MAX_SIZE;
	fread(in, sizeof(unsigned char), isize, fin);
	osize = isize;

	for (c1 = 0; c1 < (int)strlen(option); c1++) {

		if (option[c1] == 'd' || option[c1] == 'D') {
			lz_decode(in, &isize, out, &osize);
		}
		if (option[c1] == 'e' || option[c1] == 'E') {
			lz_encode(in, &isize, out, &osize);
		}

		memcpy(in, out, osize);
		isize = osize;
	}	

	fwrite(out, sizeof(unsigned char), osize, fout);

	free(in);
	free(out);
	fclose(fin);
	fclose(fout);

	return ret;
}