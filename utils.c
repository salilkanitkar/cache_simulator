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
			Cache->sets[i].blocks[j].memblock_addr = 0;
		}
	}
}

void sort_lru_counter(lru_sort_t *lru_sort, int n)
{
	int i, j;
	lru_sort_t x;

	for (j=1 ; j < n ; j++) {
		x.index = lru_sort[j].index;
		x.lru_counter = lru_sort[j].lru_counter;
		i = j - 1;
		while (i>=0 && lru_sort[i].lru_counter > x.lru_counter) {
			lru_sort[i+1].index = lru_sort[i].index;
			lru_sort[i+1].lru_counter = lru_sort[i].lru_counter;
			i -= 1;
		}
		lru_sort[i+1].index = x.index;
		lru_sort[i+1].lru_counter = x.lru_counter;
	}
}

void print_cache(cache_t *Cache)
{
	int i, j, k;
	lru_sort_t *lru_sort;

#ifdef DEBUG_FLAG
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
#endif

	if (Cache->config.cache_level == L1_LEVEL) {
		printf("===== L1 contents =====");
	} else if (Cache->config.cache_level == L2_LEVEL) {
		printf("===== L2 contents =====");
	}

	printf("\n");
	for (i=0 ; i < Cache->config.num_sets ; i++) {
		printf("Set\t%d:   ", i);

		k = 0;
		lru_sort = (lru_sort_t *)malloc(sizeof(lru_sort_t)*Cache->sets[i].assoc);
		for (j=0 ; j < Cache->sets[i].assoc; j++) {
			if (Cache->sets[i].blocks[j].valid_bit == 1) {
				lru_sort[k].index = j;
				lru_sort[k].lru_counter = Cache->sets[i].blocks[j].lru_counter;
				k += 1;
			}
		}
		sort_lru_counter(lru_sort, k);

		for (j=0 ; j < k ; j++) {
#ifdef DEBUG_FLAG
			printf("    %c", Cache->sets[i].blocks[j].valid_bit ? 'V' : 'I');
#endif
			printf("%8x %c\t", Cache->sets[i].blocks[lru_sort[j].index].tag, Cache->sets[i].blocks[lru_sort[j].index].dirty_bit ? 'D' : ' ');
		}
		printf("\n");
	}
}

/* cache_t *Cache: This argument specifies the Cache that is going to handle this request.
 * opnum: The number of this memory operation
 * mem_addr: The address at which the read request is made.
 * cache_t *next_Cache: If this argument is NULL, it means the next level is Main Memory, otherwise the L2_cache is passed.
 * */
void handle_read_request(cache_t *Cache, int opnum, unsigned int mem_addr, cache_t *next_Cache)
{
	unsigned int tag;
	unsigned int index;
	unsigned int tmp;
	unsigned int memblock_addr;

	int i, j;
	int old_lru_counter, done=0;
	int max_lru_counter_val, max_lru_counter_val_index;

	/* Extract the memory block address */
	tmp = mem_addr;
	memblock_addr = (tmp >> Cache->config.num_blockoffset_bits);
	memblock_addr = (memblock_addr << Cache->config.num_blockoffset_bits);

	/* Extract the index bits from mem_addr */
	tmp = mem_addr;
	index = (tmp >> Cache->config.num_blockoffset_bits) & ~(~0 << Cache->config.num_index_bits);

	/* Extract the tag bits from mem_addr */
	tmp = mem_addr;
	tag = (tmp >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);

	printf("----------------------------------------\n");
	printf("# %d : read %x\n", opnum, mem_addr);
	printf("L%c read : %x (tag %x, index %d)\n", Cache->config.cache_level==L1_LEVEL ? '1': '2', memblock_addr, tag, index);

	for (i=0 ; i < Cache->sets[index].assoc ; i++) {
		if (Cache->sets[index].blocks[i].valid_bit == 1 && Cache->sets[index].blocks[i].tag == tag) {
			/* Cache hit! */
			printf("L%c hit\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
			old_lru_counter = Cache->sets[index].blocks[i].lru_counter;
			for (j=0 ; j < Cache->sets[index].assoc ; j++) {
				if (i != j && Cache->sets[index].blocks[j].valid_bit == 1 && Cache->sets[index].blocks[j].lru_counter < old_lru_counter)
					Cache->sets[index].blocks[j].lru_counter += 1;
			}
			Cache->sets[index].blocks[i].lru_counter = 0;
			Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
			printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
			done = 1;
			break;
		}
	}

	if (done == 0) {
		for (i=0 ; i < Cache->sets[index].assoc ; i++) {
			if (Cache->sets[index].blocks[i].valid_bit == 0) {
				/* Cache Miss - The first empty block in the required set is found. */
				Cache->sets[index].blocks[i].valid_bit = 1;
				Cache->sets[index].blocks[i].dirty_bit = 0;
				Cache->sets[index].blocks[i].tag = tag;
				Cache->sets[index].blocks[i].lru_counter = 0;
				Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
				for (j=0 ; j < Cache->sets[index].assoc ; j++) {
					if (i != j && Cache->sets[index].blocks[j].valid_bit == 1)
						Cache->sets[index].blocks[j].lru_counter += 1;
				}
				done = 1;
				printf("L%c miss\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				printf("L%c victim: none\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				break;
			}
		}
	}

	if (done == 0) {
		/* No Empty block found in the Set. Also no existing block in the set matches the required block address.
		*  Therefore, a victim block must be chosen and be replaced.
		*/
		max_lru_counter_val = 0;
		max_lru_counter_val_index = 0;
		for (i=0 ; i < Cache->sets[index].assoc ; i++) {
			if (Cache->sets[index].blocks[i].valid_bit == 1 && Cache->sets[index].blocks[i].lru_counter > max_lru_counter_val) {
				max_lru_counter_val = Cache->sets[index].blocks[i].lru_counter;
				max_lru_counter_val_index = i;
			}
		}

		printf("L%c miss\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
		printf("L%c victim: %x (tag %x, index %d, %s)\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr, Cache->sets[index].blocks[max_lru_counter_val_index].tag, index, Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit ? "dirty": "clean");
		printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');

		Cache->sets[index].blocks[max_lru_counter_val_index].tag = tag;
		Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr = memblock_addr;
		Cache->sets[index].blocks[max_lru_counter_val_index].lru_counter = 0;
		Cache->sets[index].blocks[max_lru_counter_val_index].valid_bit = 1;
		Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit = 0;

		for (j=0 ; j < Cache->sets[index].assoc ; j++) {
			if (max_lru_counter_val_index != j && Cache->sets[index].blocks[j].valid_bit == 1)
				Cache->sets[index].blocks[j].lru_counter += 1;
		}
	}
}

/* cache_t *Cache: This argument specifies the Cache that is going to handle this request.
 * opnum: The number of this memory operation
 * mem_addr: The address at which the write request is made.
 * cache_t *next_Cache: If this argument is NULL, it means the next level is Main Memory, otherwise the L2_cache is passed.
 * */
void handle_write_request(cache_t *Cache, int opnum, unsigned int mem_addr, cache_t *next_Cache)
{
	unsigned int tag;
	unsigned int index;
	unsigned int tmp;
	unsigned int memblock_addr;

	int i, j;
	int old_lru_counter, done=0;
	int max_lru_counter_val, max_lru_counter_val_index;

	/* Extract the memory block address */
	tmp = mem_addr;
	memblock_addr = (tmp >> Cache->config.num_blockoffset_bits);
	memblock_addr = (memblock_addr << Cache->config.num_blockoffset_bits);

	/* Extract the index bits from mem_addr */
	tmp = mem_addr;
	index = (tmp >> Cache->config.num_blockoffset_bits) & ~(~0 << Cache->config.num_index_bits);

	/* Extract the tag bits from mem_addr */
	tmp = mem_addr;
	tag = (tmp >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);

	printf("----------------------------------------\n");
	printf("# %d : write %x\n", opnum, mem_addr);
	printf("L%c Write : %x  (tag %x, index %d)\n", Cache->config.cache_level==L1_LEVEL ? '1': '2', memblock_addr, tag, index);

	for (i=0 ; i < Cache->sets[index].assoc ; i++) {
		if (Cache->sets[index].blocks[i].valid_bit == 1 && Cache->sets[index].blocks[i].tag == tag) {
			/* Cache hit! */
			printf("L%c hit\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
			old_lru_counter = Cache->sets[index].blocks[i].lru_counter;
			for (j=0 ; j < Cache->sets[index].assoc ; j++) {
				if (i != j && Cache->sets[index].blocks[j].valid_bit == 1 && Cache->sets[index].blocks[j].lru_counter < old_lru_counter) 
					Cache->sets[index].blocks[j].lru_counter += 1;
			}
			Cache->sets[index].blocks[i].lru_counter = 0;
			Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
			Cache->sets[index].blocks[i].dirty_bit = 1;
			printf("L%c update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
			printf("L%c set dirty\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
			done = 1;
			break;
		}
	}

	if (done == 0) {
		for (i=0 ; i < Cache->sets[index].assoc ; i++) {
			if (Cache->sets[index].blocks[i].valid_bit == 0) {
				/* Cache Miss - The first empty block in the required set is found. */
				Cache->sets[index].blocks[i].valid_bit = 1;
				Cache->sets[index].blocks[i].dirty_bit = 1;
				Cache->sets[index].blocks[i].tag = tag;
				Cache->sets[index].blocks[i].lru_counter = 0;
				Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
				for (j=0 ; j < Cache->sets[index].assoc ; j++) {
					if (i != j && Cache->sets[index].blocks[j].valid_bit == 1)
						Cache->sets[index].blocks[j].lru_counter += 1;
				}
				done = 1;
				printf("L%c miss\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				printf("L%c victim: none\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				printf("L%c set dirty\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				break;
			}
		}
	}

	if (done == 0) {
		/* No Empty block found in the Set. Also no existing block in the set matches the required block address.
		*  Therefore, a victim block must be chosen and be replaced.
		*/
		max_lru_counter_val = 0; 
		max_lru_counter_val_index = 0;
		for (i=0 ; i < Cache->sets[index].assoc ; i++) {
			if (Cache->sets[index].blocks[i].valid_bit == 1 && Cache->sets[index].blocks[i].lru_counter > max_lru_counter_val) {
				max_lru_counter_val = Cache->sets[index].blocks[i].lru_counter;
				max_lru_counter_val_index = i;
			}
		}

		printf("L%c miss\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
		printf("L%c victim: %x (tag %x, index %d, %s)\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr, Cache->sets[index].blocks[max_lru_counter_val_index].tag, index, Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit ? "dirty": "clean");
		printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
		printf("L%c set dirty\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');

		Cache->sets[index].blocks[max_lru_counter_val_index].tag = tag;
		Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr = memblock_addr;
		Cache->sets[index].blocks[max_lru_counter_val_index].lru_counter = 0;
		Cache->sets[index].blocks[max_lru_counter_val_index].valid_bit = 1;
		Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit = 1;

		for (j=0 ; j < Cache->sets[index].assoc ; j++) {
			if (max_lru_counter_val_index != j && Cache->sets[index].blocks[j].valid_bit == 1)
				Cache->sets[index].blocks[j].lru_counter += 1;
		}
	}

}
