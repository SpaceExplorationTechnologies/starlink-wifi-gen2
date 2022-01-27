// MT7620_NAND_ECC_hamming.cpp : 定義主控台應用程式的進入點。
//


/*
 * MTK/Railnk: MT7620 NAND flash programmer utility
 * Only support : 1 page     = 2048 bytes
 *                spare size = 64 or 128 bytes
 *
 * compile:
 * gcc mt7620_nand.c -o mt7620_nand
 *
 * 2013/12/19 yy
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BIT(x,y)	((x & (1<<y)) >> y)
#define CHARBIT(c)	(BIT(c, 0) ^ BIT(c, 1) ^ BIT(c, 2) ^ BIT(c, 3) ^ \
			BIT(c, 4) ^ BIT(c, 5) ^ BIT(c, 6) ^ BIT(c, 7))

#define PAGESIZE	2048
#define ECC_BLOCK	512

/* ecc result*/
unsigned long eccs[16];

/* how many ecc per page ?*/
int ecc_perpage = 0;

void column(unsigned char data[], int _P, unsigned char *p, unsigned char *ps)
{
	int i, P = _P;
	unsigned char result = 0, result_P = 0;

	for(i=0; i<ECC_BLOCK; i++){
		if(i >= P + _P)
			P = P + 2*_P;
		if(i >= P)
			result_P = result_P ^ CHARBIT(data[i]);
		else
			result = result ^ CHARBIT(data[i]);
	}
	*p = result_P;
	*ps= result;
	//printf("%02x %02x\n", result, result_P);
}

void row_P1(unsigned char data[], unsigned char *p1, unsigned char *p1s)
{
	int i;
	unsigned char result = 0, result_P = 0;
	for(i=0; i<ECC_BLOCK; i++){
		result = result ^ BIT(data[i], 0) ^ BIT(data[i], 2) ^ BIT(data[i], 4) ^ BIT(data[i], 6);
		result_P = result_P ^ BIT(data[i], 1) ^ BIT(data[i], 3) ^ BIT(data[i], 5) ^ BIT(data[i], 7);
	}
	*p1 = result_P;
	*p1s = result;
	//printf("P1 = %02x %02x\n", result, result_P);
}

void row_P2(unsigned char data[], unsigned char *p2, unsigned char *p2s)
{
	int i;
	unsigned char result = 0, result_P = 0;
	for(i=0; i<ECC_BLOCK; i++){
		result = result ^ BIT(data[i], 0) ^ BIT(data[i], 1) ^ BIT(data[i], 4) ^ BIT(data[i], 5);
		result_P = result_P ^ BIT(data[i], 2) ^ BIT(data[i], 3) ^ BIT(data[i], 6) ^ BIT(data[i], 7);
	}
	*p2 = result_P;
	*p2s = result;
	//printf("P2 = %02x %02x\n", result, result_P);
}

void row_P4(unsigned char data[], unsigned char *p4, unsigned char *p4s)
{
	int i;
	unsigned char result = 0, result_P = 0;
	for(i=0; i<ECC_BLOCK; i++){
		result = result ^ BIT(data[i], 0) ^ BIT(data[i], 1) ^ BIT(data[i], 2) ^ BIT(data[i], 3);
		result_P = result_P ^ BIT(data[i], 4) ^ BIT(data[i], 5) ^ BIT(data[i], 6) ^ BIT(data[i], 7);
	}
	*p4 = result_P;
	*p4s = result;
	//printf("P4 = %02x %02x\n", result, result_P);
}

/*
 *  generate ecc of per 512 bytes in a single page.
 */
unsigned long ECC(unsigned char data[])
{
	unsigned long ecc = 0;
	unsigned char p, ps;
	int i;

	for(i=0; i<9; i++){
		printf("P%d = ", 1<<(i+3));
		column(data, 1<<i, &p, &ps);
		ecc = ecc | (p << ((2*i)+1)) | (ps << (2 * i));
	}

	row_P1(data, &p, &ps);
	ecc = ecc | (p << ((2*i)+1)) | (ps << (2 * i));
	i++;

	row_P2(data, &p, &ps);
	ecc = ecc | (p << ((2*i)+1)) | (ps << (2 * i));
	i++;

	row_P4(data, &p, &ps);
	ecc = ecc | (p << ((2*i)+1)) | (ps << (2 * i));

	printf("ecc = %08x\n", ecc);
	return ecc;
}


void ECC_pages(unsigned char page_data[])
{
	int i;
	for(i=0;i < ecc_perpage; i++)
		eccs[i] = ECC(&page_data[ECC_BLOCK * i]);		
}

void padding(unsigned char page_data[], int rc)
{
	int i;
	for(i=rc; i<PAGESIZE; i++)
		page_data[i] = 0xff;
}

void write_ecc(FILE *output_fp, int spare_size)
{
	int i;
	for(i=0; i < ecc_perpage; i++){
		fprintf(output_fp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
						0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						(unsigned char)(eccs[i] & 0x000000ff),   
						(unsigned char)((eccs[i] & 0x0000ff00) >> 8),
						(unsigned char)((eccs[i] & 0x00ff0000) >> 16), 
						0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
	}
	/* todo/fixme */
	if(spare_size == 128)
		for(i=0; i< ecc_perpage; i++)
			fprintf(output_fp, "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
						0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
						0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);	
}

void usage(char *bin_name)
{
	printf("\n");
	printf("MT7620 NAND Flash programmer image Utility 0.1\n");
	printf("\n");
	printf("usage: %s spare_size input_filename\n", bin_name);
	printf("ex:\n");
	printf("\t\t %s 64 uboot.img\n", bin_name);
	printf("\t\t %s 128 root_uImage\n", bin_name);
	printf("\n");
}

int main(int argc, char* argv[])
{
	int rc;
	char output_filename[512];
	unsigned char page_data[PAGESIZE];
	int spare_size = 64;
	FILE *input_fp, *output_fp;

	ecc_perpage = PAGESIZE/ECC_BLOCK;

	if(argc != 3){
		usage(argv[0]);
		exit(-1);
	}

	snprintf(output_filename, 256, "%s.output", argv[2]);

	if(!strcmp("128", argv[1])){
		spare_size = 128;
	}else if(!strcmp("64", argv[1])){
		spare_size = 64;
	}else{
		printf("Error: Not support spare area size: %s\n", argv[1]);
		printf("Support spare area size: 64 or 128\n");
		return -1;
	}

	input_fp = fopen(argv[2], "rb");
	if(!input_fp){
		printf("Error: open file %s failed\n", argv[2]);
		return -1;
	}
	output_fp = fopen(output_filename, "wb");
	if(!output_fp){
		printf("Error: open output file %s failed\n", output_filename);
		return -1;
	}
	
	do{
		rc = fread(page_data, 1, PAGESIZE, input_fp);
		if(rc){
			if(rc != PAGESIZE)
				padding(page_data, rc);

			/* write back original data*/
			fwrite(page_data, 1, sizeof(page_data), output_fp);

			/* calculate ECC */
			ECC_pages(page_data);

			/* write ECC back to output file */
			write_ecc(output_fp, spare_size);
		}
		printf(".");
	}while(rc == PAGESIZE);

	fclose(input_fp);
	fclose(output_fp);
	printf("done\nOutput file is \"%s\"\n", output_filename);
	return 0;
}

