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

mem_op_t mem_op;

cache_t L1_cache;
cache_t L2_cache;

sim_res_t sim_res;

int validate_params(char *[]);
void print_params();
void initialize_cache_sim_params(sim_res_t *);
void calculate_and_print_cache_sim_params(sim_res_t *);

int main(int argc, char *argv[])
{
	unsigned int opnum=0;
	cache_block_t retblock;

	if (argc != 11) {
		printf("Usage:\n./sim_cache <BLOCKSIZE>\n\t<L1_SIZE> <L1_ASSOC> <L1_PREF_N> <L1_PREF_M>\n\t<L2_SIZE> <L2_ASSOC> <L2_PREF_N> <L2_PREF_M>\n\t<tracefile>\n");
		exit(1);
	}
	
	if (validate_params(argv)) {
		printf("Exiting.....\n");
		exit(1);
	}

	print_params();

	L1_cache.config.cache_level = L1_LEVEL;
	set_cache_params(&L1_cache);
	allocate_cache(&L1_cache);
#ifdef DEBUG_FLAG
	print_cache(&L1_cache);
#endif

	L2_cache.config.cache_level = L2_LEVEL;
	if (L2_size) {
		set_cache_params(&L2_cache);
		allocate_cache(&L2_cache);
#ifdef DEBUG_FLAG
		print_cache(&L2_cache);
#endif
	}

	initialize_cache_sim_params(&sim_res);

	while (fgets(trace_str, MAX_TRACESTR_LEN, fp_trace)) {

		sscanf(trace_str, "%c %x\n", &mem_op.opcode, &mem_op.addr);
#ifdef DEBUG_FLAG
		printf("%c %x\n", mem_op.opcode, mem_op.addr);
#endif
		opnum += 1;

		if (mem_op.opcode == 'r') {
			if (L2_size) {
				retblock = handle_read_request(&L1_cache, opnum, mem_op.addr, &L2_cache, 0);
			} else {
				retblock = handle_read_request(&L1_cache, opnum, mem_op.addr, NULL, 0);
			}
		} else if (mem_op.opcode == 'w') {
			if (L2_size) {
				retblock = handle_write_request(&L1_cache, opnum, mem_op.addr, &L2_cache, 0);
			} else {
				retblock = handle_write_request(&L1_cache, opnum, mem_op.addr, NULL, 0);
			}
		}

	}

	print_cache(&L1_cache);
	if (L2_size)
		print_cache(&L2_cache);

	calculate_and_print_cache_sim_params(&sim_res);

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

void initialize_cache_sim_params(sim_res_t *sim_res)
{
	sim_res->L1_reads = 0;
	sim_res->L1_read_misses	= 0;
	sim_res->L1_writes = 0;
	sim_res->L1_write_misses = 0;
	sim_res->L1_miss_rate = 0;
	sim_res->L1_writebacks = 0;
	sim_res->L1_prefetches = 0;

	sim_res->L2_reads_not_L1_prefetch = 0;
	sim_res->L2_read_misses_not_L1_prefetch = 0;
	sim_res->L2_read_L1_prefetch = 0;
	sim_res->L2_read_miss_L1_prefetch = 0;

	sim_res->L2_writes = 0;
	sim_res->L2_write_misses = 0;
	sim_res->L2_miss_rate = 0;
	sim_res->L2_writebacks = 0;
	sim_res->L2_prefetches = 0;

	sim_res->tot_mem_traffic = 0;
}

void calculate_and_print_cache_sim_params(sim_res_t *sim_res)
{

	sim_res->L1_miss_rate = (float)(sim_res->L1_read_misses + sim_res->L1_write_misses) / (float)(sim_res->L1_reads + sim_res->L1_writes);

	if (L2_size)
		sim_res->L2_miss_rate = (float)(sim_res->L2_read_misses_not_L1_prefetch) / (float) (sim_res->L2_reads_not_L1_prefetch);

	if (L2_size) 
		sim_res->tot_mem_traffic = sim_res->L2_read_misses_not_L1_prefetch + sim_res->L2_read_miss_L1_prefetch + sim_res->L2_write_misses + sim_res->L2_writebacks + sim_res->L2_prefetches;
	else
		sim_res->tot_mem_traffic = sim_res->L1_read_misses + sim_res->L1_write_misses + sim_res->L1_writebacks + sim_res->L1_prefetches;

	printf("===== Simulation results (raw) =====\n");
	printf("a. number of L1 reads:                 %d\n", sim_res->L1_reads);
	printf("b. number of L1 read misses:           %d\n", sim_res->L1_read_misses);
	printf("c. number of L1 writes:                %d\n", sim_res->L1_writes);
	printf("d. number of L1 write misses:          %d\n", sim_res->L1_write_misses);
	printf("e. L1 miss rate:                       %f\n", sim_res->L1_miss_rate);
	printf("f. number of L1 writebacks:            %d\n", sim_res->L1_writebacks);
	printf("g. number of L1 prefetches:            %d\n", sim_res->L1_prefetches);

	printf("h. number of L2 reads that did not originate from L1 prefetches:        %d\n", sim_res->L2_reads_not_L1_prefetch);
	printf("i. number of L2 read misses that did not originate from L1 prefetches:  %d\n", sim_res->L2_read_misses_not_L1_prefetch);
	printf("j. number of L2 reads that originated from L1 prefetches:               %d\n", sim_res->L2_read_L1_prefetch);
	printf("k. number of L2 read misses that originated from L1 prefetches:         %d\n", sim_res->L2_read_miss_L1_prefetch);

	printf("l. number of L2 writes:                       %d\n", sim_res->L2_writes);
	printf("m. number of L2 write misses:                 %d\n", sim_res->L2_write_misses);
	if (L2_size)
		printf("n. L2 miss rate:                              %f\n", sim_res->L2_miss_rate);
	else
		printf("n. L2 miss rate:                              0\n");
	printf("o. number of L2 writebacks:                   %d\n", sim_res->L2_writebacks);
	printf("p. number of L2 prefetches:                   %d\n", sim_res->L2_prefetches);

	printf("q. total memory traffic:                      %d\n", sim_res->tot_mem_traffic);

}
