#include <stdio.h>
#include <stdlib.h>
#include "kamc_tool.h"

enum DELIMITER {
	OFF,
	ON
};

int lz_decode(unsigned char* in, int* isize, unsigned char* out, int* osize) {
	int i, j;
	int icnt = 0;
	int ocnt = 0;
	unsigned char next_byte_flag = 0xFF;
	int flag_cnt = 0;
	int next_byte = 0;
	int copy_size = 0;
	int ref_distance = 0;
	int delimiter_flag = OFF;

	next_byte_flag = in[icnt];
	icnt++;
#ifdef _DEBUG
	printf("next_byte_flag adress:%06X value:%02X (", icnt-1, next_byte_flag);
	for (j = 7; j >= 0; j--) {
		printf("%1d", (next_byte_flag >> j) & 1);
	}
	printf(")\n");
#endif

	//最初は連続符号化は無いはず
	if ((next_byte_flag >> flag_cnt & 0x3) == 0x3) {
		if (in[icnt] >= 0xC0) {
			next_byte = (in[icnt] - 0xC0) + 8;
			icnt++;
			flag_cnt++;
			delimiter_flag = ON;
		}
	}
	else {
		next_byte = 0;
		while (flag_cnt < 8) {
			next_byte++;
			flag_cnt++;
			if ((next_byte_flag >> flag_cnt & 0x1) == 0x1) {
				delimiter_flag = ON;
				break;
			}
		}
	}

#ifdef _DEBUG
	printf("next_byte:%04X  next_byte_flag:", next_byte);
	if (flag_cnt >= 8)
		printf("/");
	for (j = 7; j >= 0; j--) {
		printf("%1d", (next_byte_flag >> j) & 1);
		if (flag_cnt == j)
			printf("/");
	}
	printf("\n");
#endif

	for (i = 0; i < next_byte; i++) {
		out[ocnt] = in[icnt];
		icnt++;
		ocnt++;
	}
	while (next_byte == 0x46) {
		//8byte以上距離の取得
		next_byte = (in[icnt] - 0xC0) + 8;
		icnt++;

		for (i = 0; i < next_byte; i++) {
			out[ocnt] = in[icnt];
			icnt++;
			ocnt++;
		}
	}

#ifdef _DEBUG
	printf("--------------------------------------------------------------------\n");
#endif

	while (icnt < *isize) {
	//while (icnt < 0x2000) {

		//next_byte_flagの読み込み
		if (flag_cnt >= 8 && delimiter_flag == ON) {
			next_byte_flag = in[icnt];

#ifdef _DEBUG
			printf("next_byte_flag adress:%06X value:%02X (", icnt, next_byte_flag);
			for (j = 7; j >= 0; j--) {
				printf("%1d", (next_byte_flag >> j) & 1);
			}
			printf(")\n");
#endif
			
			icnt++;
			flag_cnt = 0;

			if (((next_byte_flag >> flag_cnt) & 0x1) == 0x0 ){
				delimiter_flag = OFF;

				next_byte = 0;
				while (flag_cnt < 8) {
					next_byte++;
					flag_cnt++;
					delimiter_flag = ON;
					if (((next_byte_flag >> flag_cnt) & 0x1) == 0x1) {
						break;
					}
				}

#ifdef _DEBUG
				printf("next_byte:%04X  next_byte_flag:", next_byte);
				if (flag_cnt >= 8)
					printf("/");
				for (j = 7; j >= 0; j--) {
					printf("%1d", (next_byte_flag >> j) & 1);
					if (flag_cnt == j)
						printf("/");
				}
				printf("\n");
#endif

				for (i = 0; i < next_byte; i++) {
					out[ocnt] = in[icnt];
					icnt++;
					ocnt++;
				}
			}

			if (flag_cnt >= 8) {
				continue;
			}
		}

#ifdef _DEBUG
		printf("code address:%06X value:", icnt);
#endif
		//復号コードが0xFFの場合は終端として終了
		if (in[icnt] == 0xFF) {
#ifdef _DEBUG
			printf("code end\n");
#endif
			break;
		}

		//復号後サイズ＆コピー元までの距離の読み込み
		if (in[icnt] >= 0x80) {
			//1Byteで表すパターン
			copy_size = (in[icnt] >> 4) - 6;
			ref_distance = (in[icnt] & 0xF) + 1;
#ifdef _DEBUG
			printf(" %02X   ", in[icnt]);
#endif
			icnt++;
		}
		else{
			//2Byteで表すパターン
			copy_size = (in[icnt] >> 2) + 3;
			ref_distance = ((in[icnt] & 0x3) << 8) + in[icnt + 1];
#ifdef _DEBUG
			printf(" %02X %02X", in[icnt], in[icnt+1]);
#endif
			icnt += 2;
		}

#ifdef _DEBUG
		printf(" -> decdata address:%06X value:", ocnt);
#endif
		while (copy_size > 0) {
			out[ocnt] = out[ocnt - ref_distance];
#ifdef _DEBUG
			printf(" %02X", out[ocnt]);
#endif
			ocnt++;
			copy_size--;
		}
#ifdef _DEBUG
		printf("\n");
#endif

		//next_byte_flagの読み込み
		if (flag_cnt >= 8 && delimiter_flag == OFF) {
			next_byte_flag = in[icnt];

#ifdef _DEBUG
			printf("next_byte_flag adress:%06X value:%02X (", icnt, next_byte_flag);
			for (j = 7; j >= 0; j--) {
				printf("%1d", (next_byte_flag >> j) & 1);
			}
			printf(")\n");
#endif
			
			icnt++;
			flag_cnt = 0;
			delimiter_flag = OFF;
		}

		if ((next_byte_flag >> flag_cnt & 0x1) == 0x1 && flag_cnt == 7 && delimiter_flag==ON) {
			if (in[icnt+1] < 0xC0 || ((in[icnt+1] < 0x80) && (((in[icnt+1] & 0x3) << 8) + in[icnt + 2] < ocnt))) {
				//連続符号化
				flag_cnt++;
				next_byte = 0;
			}
			else {
				//next_byte_flagの読み込み
				next_byte_flag = in[icnt];

#ifdef _DEBUG
				printf("next_byte_flag adress:%06X value:%02X (", icnt, next_byte_flag);
				for (j = 7; j >= 0; j--) {
					printf("%1d", (next_byte_flag >> j) & 1);
				}
				printf(")\n");
#endif

				icnt++;
				flag_cnt = 0;
				delimiter_flag = OFF;
			}
		}
		else if ((next_byte_flag >> flag_cnt & 0x1) == 0x1 && flag_cnt == 7 && delimiter_flag == OFF) {
			if (in[icnt] >= 0xC0) {
				//8byte以上距離の取得
				next_byte = (in[icnt] - 0xC0) + 8;
				icnt++;
				flag_cnt++;
				delimiter_flag = ON;
			}
			else {
				//連続符号化
				flag_cnt++;
				next_byte = 0;
			}
		}

		if (flag_cnt < 8) {

			//次の符号化コードまでの距離判定
			if (((next_byte_flag >> flag_cnt) & 0x3) == 0x1) {
				if (delimiter_flag == ON) {
					//デリミタの処理
					flag_cnt++;
					delimiter_flag = OFF;

					//8byte未満の距離の取得
					next_byte = 0;
					delimiter_flag = ON;
					while (flag_cnt < 8) {
						next_byte++;
						flag_cnt++;
						if (((next_byte_flag >> flag_cnt) & 0x1) == 0x1) {
							break;
						}
					}
				}
				else {
					if (in[icnt] == 0xFE) {
						//8byte以上距離の取得
						next_byte = (in[icnt] - 0xC0) + 8;
						icnt++;
						flag_cnt++;
					}
					else {
						//連続符号化
						flag_cnt++;
						next_byte = 0;
					}
				}
			}
			else if (((next_byte_flag >> flag_cnt) & 0x3) == 0x3) {
				if (in[icnt] < 0xC0 || ((in[icnt] < 0x80) && (((in[icnt] & 0x3) << 8) + in[icnt + 1] < ocnt))) {
					//連続符号化
					flag_cnt++;
					next_byte = 0;
				}
				else {
					if (delimiter_flag == ON) {
						//デリミタの処理
						flag_cnt++;
						delimiter_flag = OFF;

					}

					if (((next_byte_flag >> flag_cnt) & 0x1) == 1) {
						if (in[icnt] >= 0xC0) {
							//8byte以上距離の取得
							next_byte = (in[icnt] - 0xC0) + 8;
							icnt++;
							flag_cnt++;
							if (((next_byte_flag >> flag_cnt) & 0x1) == 0 && flag_cnt < 8) {
								delimiter_flag = OFF;
							}
							else {
								delimiter_flag = ON;
							}
						}
						else {
							//連続符号化
							flag_cnt++;
							next_byte = 0;
						}
					}
					else {
						//8byte未満の距離の取得
						next_byte = 0;
						while (flag_cnt < 8) {
							next_byte++;
							flag_cnt++;
							delimiter_flag = ON;
							if (((next_byte_flag >> flag_cnt) & 0x1) == 0x1) {
								break;
							}
						}
					}
				}
			}
			else {
				//8byte未満の距離の取得
				next_byte = 0;
				while (flag_cnt < 8) {
					next_byte++;
					flag_cnt++;
					delimiter_flag = ON;
					if (((next_byte_flag >> flag_cnt) & 0x1) == 0x1) {
						break;
					}
				}
			}
		}


		//次Byteが0xFFの場合は終端として終了
		if (next_byte == 0x47) {
#ifdef _DEBUG
			printf("next byte end\n");
#endif
			break;
		}

#ifdef _DEBUG
		printf("next_byte:%04X  next_byte_flag:", next_byte);
		if (flag_cnt >= 8)
			printf("/");
		for (j = 7; j >= 0; j--) {
			printf("%1d", (next_byte_flag >> j) & 1);
			if (flag_cnt == j)
				printf("/");
		}
		printf(" delimiter_flag:%d\n", delimiter_flag);
#endif

		for (i = 0; i < next_byte; i++) {
			out[ocnt] = in[icnt];
			icnt++;
			ocnt++;
		}
		while (next_byte == 0x46) {

			if (flag_cnt >= 8) {
				next_byte_flag = in[icnt];

#ifdef _DEBUG
				printf("next_byte_flag adress:%06X value:%02X (", icnt, next_byte_flag);
				for (j = 7; j >= 0; j--) {
					printf("%1d", (next_byte_flag >> j) & 1);
				}
				printf(")\n");
#endif

				icnt++;
				flag_cnt = 0;

			}

			if ((((((next_byte_flag >> flag_cnt) & 0x3) == 1) && flag_cnt < 7) ||
				((((next_byte_flag >> flag_cnt) & 0x1) == 0) && flag_cnt ==0))
				&& delimiter_flag == ON) {
				delimiter_flag = OFF;
			}

			if (((next_byte_flag >> flag_cnt) & 0x1) == 1) {
				if (in[icnt] < 0xC0) {
					delimiter_flag = OFF;
					flag_cnt++;
					break;
				}
				//8byte以上距離の取得
				next_byte = (in[icnt] - 0xC0) + 8;
				icnt++;
				flag_cnt++;
			}
			else {
				if (delimiter_flag == ON) {
					flag_cnt++;
					delimiter_flag = OFF;
				}
				next_byte = 0;
				while (flag_cnt < 8) {
					next_byte++;
					flag_cnt++;
					delimiter_flag = ON;
					if (((next_byte_flag >> flag_cnt) & 0x1) == 0x1) {
						break;
					}
				}
			}

#ifdef _DEBUG
			printf("next_byte %04X\n", next_byte);
#endif

			for (i = 0; i < next_byte; i++) {
				out[ocnt] = in[icnt];
				icnt++;
				ocnt++;
			}
		}

#ifdef _DEBUG
		printf("--------------------------------------------------------------------\n");
#endif
	}

	i = 1;
	while (i) {
		i *= 2;

		if (ocnt == i ) {
			break;
		}
		else if (ocnt < i ) {
			for (j = ocnt; j < i; j++) {
				out[j] = 0xFF;
			}
			ocnt = i;
			break;
		}
	}

	*osize = ocnt;

	return 0;
}

int lz_encode(unsigned char* in, int* isize, unsigned char* out, int* osize) {
	int i, j;
	int icnt = 0;
	int ocnt = 0;
	unsigned char next_byte_flag = 0xFF;
	int flag_cnt = 0;
	int next_byte = 0xFE;
	int copy_size = 0;
	int quotient = 0;
	int remainder = 0;

	quotient = (int)((*isize - 0x10) / 0x46);
	remainder = (*isize - 0x10) % 0x46;

	if (remainder >= 8) {

		out[ocnt] = next_byte_flag;
		ocnt++;
		out[ocnt] = 0xB + 0xB8;
		ocnt++;
		flag_cnt++;

		for (i = 0; i < 0xB; i++) {
			out[ocnt] = in[icnt];
			icnt++;
			ocnt++;
		}

		out[ocnt] = 0xB0;
		ocnt++;
		flag_cnt += 2;
		icnt += 5;

		out[ocnt] = next_byte;
		ocnt++;

		for (i = 0; i < 0x46; i++) {
			out[ocnt] = in[icnt];
			icnt++;
			ocnt++;
		}
	}
	else if(remainder >= 5) {
		
		out[ocnt] = next_byte_flag;
		ocnt++;
		out[ocnt] = 0xB + 0xB8;
		ocnt++;
		flag_cnt++;

		for (i = 0; i < 0xB; i++) {
			out[ocnt] = in[icnt];
			icnt++;
			ocnt++;
		}

		out[ocnt] = 0x80;
		ocnt++;
		flag_cnt += 2;
		icnt += 2;

		out[ocnt] = next_byte;
		ocnt++;

		for (i = 0; i < 0x46; i++) {
			out[ocnt] = in[icnt];
			icnt++;
			ocnt++;
		}
	}
	else if ((in[4] + in[5] + in[6] + in[7] + in[8] + in[9]) == 0) {

		out[ocnt] = next_byte_flag;
		ocnt++;
		out[ocnt] = 0x5 + 0xB8;
		ocnt++;
		flag_cnt++;

		for (i = 0; i < 0x5; i++) {
			out[ocnt] = in[icnt];
			icnt++;
			ocnt++;
		}

		out[ocnt] = 0x80;
		ocnt++;
		flag_cnt += 2;
		icnt += 2;

		out[ocnt] = next_byte;
		ocnt++;

		for (i = 0; i < 0x46; i++) {
			out[ocnt] = in[icnt];
			icnt++;
			ocnt++;
		}
	}
	else {
		printf("Thies rom is not available\n");
		return -1;
	}

	while (icnt < *isize) {
		if (flag_cnt >= 8) {
			out[ocnt] = next_byte_flag;
			ocnt++;
			flag_cnt = 0;
		}

		if (*isize - icnt >= 0x46) {
			out[ocnt] = next_byte;
			ocnt++;
			flag_cnt++;

			for (i = 0; i < 0x46; i++) {
				out[ocnt] = in[icnt];
				icnt++;
				ocnt++;
			}
		}
		else if (*isize - icnt >= 0x8) {
			next_byte = (*isize - icnt) ;
			out[ocnt] = next_byte + 0xB8;
			ocnt++;
			flag_cnt++;

			for (i = 0; i < next_byte; i++) {
				out[ocnt] = in[icnt];
				icnt++;
				ocnt++;
			}
		}
		else {
			printf("error\n");
			return -1;
		}
	
	}

	out[ocnt] = 0xFF;
	ocnt++;

	*osize = ocnt;

	return 0;
}