#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "main.h"

int blockSize=-1;

int L1_size=-1;
int L1_assoc=-1;
int L1_pref_n=-1;
int L1_pref_m=-1;

int L2_size=-1;
int L2_assoc=-1;
int L2_pref_n=-1;
int L2_pref_m=-1;

char trace_file[MAX_FILENAME_LEN];
FILE *fp_trace=0;
char trace_str[MAX_TRACESTR_LEN];

int validate_params(char *[]);
void print_params();

mem_op_t mem_op;

int main(int argc, char *argv[])
{

	if (argc != 11) {
		printf("Usage:\n./sim_cache <BLOCKSIZE>\n\t<L1_SIZE> <L1_ASSOC> <L1_PREF_N> <L1_PREF_M>\n\t<L2_SIZE> <L2_ASSOC> <L2_PREF_N> <L2_PREF_M>\n\t<tracefile>\n");
		exit(1);
	}
	
	if (validate_params(argv)) {
		printf("Exiting.....\n");
		exit(1);
	}

	print_params();

	while (fgets(trace_str, MAX_TRACESTR_LEN, fp_trace)) {

		sscanf(trace_str, "%c %x\n", &mem_op.opcode, &mem_op.addr);
#ifdef DEBUG_FLAG
		printf("%c %x\n", mem_op.opcode, mem_op.addr);
#endif
	}

	return 0;

}

int validate_params(char *params[])
{
	blockSize = atoi(params[1]);
	if (blockSize <= 0) {
		printf("Invalid value of blockSize!\n");
		return FALSE;
	}

	int l1_s = atoi(params[2]);
	if (l1_s <= 0) {
		printf("Invalid value of L1 Cache Size!\n");
		return FALSE;
	} else {
		L1_size = l1_s;
	}

	int l1_a = atoi(params[3]);
	if (l1_a <= 0) {
		printf("Invalid value of L1 Cache Associativity!\n");
		return FALSE;
	} else {
		L1_assoc = l1_a;
	}

	int l1_n = atoi(params[4]);
	if (l1_n < 0) {
		printf("Invalid value of Number of L1 Prefetch Buffer Streams!\n");
		return FALSE;
	} else if (l1_n == 0) {
		L1_pref_n = 0;
	} else {
		L1_pref_n = l1_n;
	}

	if (L1_pref_n) {
		int l1_m = atoi(params[5]);
		if (l1_m < 0) {
			printf("Invalid value of Number of Blocks in each L1 Prefetch Buffer Stream!\n");
			return FALSE;
		} else if (l1_m == 0) {
			L1_pref_m = 0;
		} else {
			L1_pref_m = l1_m;
		}
	} else {
		L1_pref_m = 0;
	}

	int l2_s = atoi(params[6]);
	if (l2_s < 0) {
		printf("Invalid value of L2 Cache Size!\n");
		return FALSE;
	} else if (l2_s == 0) {
		L2_size = 0;
	} else {
		L2_size = l2_s;
	}

	if (L2_size) {
		int l2_a = atoi(params[7]);
		if (l2_a <= 0) {
			printf("Invalid value of L1 Cache Associativity!\n");
			return FALSE;
		} else {
			L2_assoc = l2_a;
		}

		int l2_n = atoi(params[8]);
		if (l2_n < 0) {
			printf("Invalid value of Number of L2 Prefetch Buffer Streams!\n");
			return FALSE;
		} else if (l2_n == 0) {
			L2_pref_n = 0;
		} else {
			L2_pref_n = l2_n;
		}

		if (L2_pref_n) {
			int l2_m = atoi(params[9]);
			if (l2_m < 0) {
				printf("Invalid value of Number of Blocks in each L2 Prefetch Buffer Stream!\n");
				return FALSE;
			} else if (l2_m == 0) {
				L2_pref_m = 0;
			} else {
				L2_pref_m = l2_m;
			}
		} else {
			L2_pref_m = 0;
		}
	} else {
		L2_assoc = 0;
		L2_pref_n = 0;
		L2_pref_m = 0;
	}

	strcpy(trace_file, params[10]);
	fp_trace = fopen(trace_file, "r");
	if (fp_trace == NULL) {
		printf("Error while opening the trace file!\n");
		return FALSE;
	}

	return TRUE;
}

void print_params()
{
	printf("===== Simulator configuration =====\n");
	printf("BLOCKSIZE:             %d\n", blockSize);
	
	printf("L1_SIZE:               %d\n", L1_size);
	printf("L1_ASSOC:              %d\n", L1_assoc);
	printf("L1_PREF_N:             %d\n", L1_pref_n);
	printf("L1_PREF_M:             %d\n", L1_pref_m);

	printf("L2_SIZE:               %d\n", L2_size);
	printf("L2_ASSOC:              %d\n", L2_assoc);
	printf("L2_PREF_N:             %d\n", L2_pref_n);
	printf("L2_PREF_M:             %d\n", L2_pref_m);

	printf("trace file:            %s\n", trace_file);
}
