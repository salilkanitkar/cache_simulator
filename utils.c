#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

#include "main.h"

void set_cache_params(cache_t *Cache)
{
	if (Cache->config.cache_level == L1_LEVEL) {
		Cache->config.size = L1_size;
		Cache->config.assoc = L1_assoc;
		Cache->config.pref_n = L1_pref_n;
		Cache->config.pref_m = L1_pref_m;
	} else if (Cache->config.cache_level == L2_LEVEL) {
		Cache->config.size = L2_size;
		Cache->config.assoc = L2_assoc;
		Cache->config.pref_n = L2_pref_n;
		Cache->config.pref_m = L2_pref_m;
	}

	Cache->config.num_sets = (int)(Cache->config.size / (Cache->config.assoc * blockSize));
	Cache->config.num_index_bits = (int)(log(Cache->config.num_sets) / log(2));
	Cache->config.num_blockoffset_bits = (int)(log(blockSize) / log(2));
	Cache->config.num_tag_bits = NUM_ADDR_BITS - (Cache->config.num_index_bits + Cache->config.num_blockoffset_bits);

}

void allocate_cache(cache_t *Cache)
{
	int i, j;

	Cache->sets = (cache_set_t *)malloc(sizeof(cache_set_t)*Cache->config.num_sets);
	if (!Cache->sets) {
		printf("Memory Allocation Failed!\n");
		exit(1);
	}

	for (i=0 ; i < Cache->config.num_sets ; i++) {
		Cache->sets[i].assoc = Cache->config.assoc;
		Cache->sets[i].blocks = (cache_block_t *)malloc(sizeof(cache_block_t)*Cache->sets[i].assoc);
		if (!Cache->sets[i].blocks) {
			printf("Memory Allocation Failed\n");
			exit(1);
		}

		for (j=0 ; j < Cache->sets[i].assoc ; j++) {
			Cache->sets[i].blocks[j].valid_bit = 0;
			Cache->sets[i].blocks[j].dirty_bit = 0;
			Cache->sets[i].blocks[j].lru_counter = 0;
			Cache->sets[i].blocks[j].tag = 0;
		}
	}
}

void print_cache(cache_t *Cache)
{
	int i,j;

	if (Cache->config.cache_level == L1_LEVEL) {
		printf("L1 Cache:\n");
	} else if (Cache->config.cache_level == L2_LEVEL) {
		printf("L2 Cache:\n");
	}

	printf("Size: %d\n", Cache->config.size);
	printf("Associativity: %d\n", Cache->config.assoc);
	printf("Number of Prefetch Buffers: %d\n", Cache->config.pref_n);
	printf("Number of Blocks in each Prefetch Buffer: %d\n", Cache->config.pref_m);
	printf("Number of Sets: %d\n", Cache->config.num_sets);
	printf("Number of Index Bits: %d\n", Cache->config.num_index_bits);
	printf("Number of Block Offset Bits: %d\n", Cache->config.num_blockoffset_bits);
	printf("Number of Tag Bits: %d\n", Cache->config.num_tag_bits);

	if (Cache->config.cache_level == L1_LEVEL) {
		printf("==== L1 contents ====");
	} else if (Cache->config.cache_level == L2_LEVEL) {
		printf("==== L2 contents ====");
	}

	printf("\n");
	for (i=0 ; i < Cache->config.num_sets ; i++) {
		printf("Set    %d:", i);
		for (j=0 ; j < Cache->sets[i].assoc ; j++) {
			printf("    %c", Cache->sets[i].blocks[j].valid_bit ? 'V' : 'I');
			printf("    %x %c", Cache->sets[i].blocks[j].tag, Cache->sets[i].blocks[j].dirty_bit ? 'D' : 'C');
		}
		printf("\n");
	}
}
