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
		Cache->sets[i].blocks = (cache_block_t *)malloc(sizeof(cache_block_t)*Cache->config.assoc);
		if (!Cache->sets[i].blocks) {
			printf("Memory Allocation Failed\n");
			exit(1);
		}

		for (j=0 ; j < Cache->config.assoc ; j++) {
			Cache->sets[i].blocks[j].valid_bit = 0;
			Cache->sets[i].blocks[j].dirty_bit = 0;
			Cache->sets[i].blocks[j].lru_counter = 0;
			Cache->sets[i].blocks[j].tag = 0;
			Cache->sets[i].blocks[j].memblock_addr = 0;
		}
	}

	if (Cache->config.pref_n) {
	Cache->sb = (sb_t *)malloc(sizeof(sb_t)*Cache->config.pref_n);
	if (!Cache->sb) {
		printf("Memory Allocation Failed!\n");
		exit(1);
	}
	
	for (i=0 ; i < Cache->config.pref_n ; i++) {
		Cache->sb[i].sb_valid_bit = 0;
		Cache->sb[i].lru_counter = i;
		Cache->sb[i].sb_buf = (cache_block_t *)malloc(sizeof(cache_block_t)*Cache->config.pref_m);
		if (!Cache->sb[i].sb_buf) {
			printf("Memory Allocation Failed!\n");
			exit(1);
		}

		for (j=0 ; j < Cache->config.pref_m ; j++) {
			Cache->sb[i].sb_buf[j].valid_bit = 0;
			Cache->sb[i].sb_buf[j].dirty_bit = 0;
			Cache->sb[i].sb_buf[j].lru_counter = 0;
			Cache->sb[i].sb_buf[j].tag = 0;
			Cache->sb[i].sb_buf[j].memblock_addr = 0;
		}
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

void sb_sort_lru_counter(int *tmp1, int *tmp2, int n)
{
	int i, j;
	int x, y;

	for (j=1 ; j < n ; j++) {
		x = tmp1[j];
		y = tmp2[j];
		i = j - 1;
		while (i>=0 && tmp1[i] > x) {
			tmp1[i+1] = tmp1[i];
			tmp2[i+1] = tmp2[i];
			i -= 1;
		}
		tmp1[i+1] = x;
		tmp2[i+1] = y;
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
		lru_sort = (lru_sort_t *)malloc(sizeof(lru_sort_t)*Cache->config.assoc);
		for (j=0 ; j < Cache->config.assoc; j++) {
			if (Cache->sets[i].blocks[j].valid_bit == 1) {
				lru_sort[k].index = j;
				lru_sort[k].lru_counter = Cache->sets[i].blocks[j].lru_counter;
				k += 1;
			} else {
				lru_sort[k].index = j;
				lru_sort[k].lru_counter = -1;
				k += 1;
			}
		}
		sort_lru_counter(lru_sort, k);

		for (j=0 ; j < k ; j++) {
#ifdef DEBUG_FLAG
			printf("    %c", Cache->sets[i].blocks[j].valid_bit ? 'V' : 'I');
#endif
			if (Cache->sets[i].blocks[j].valid_bit)
				printf("%8x %c", Cache->sets[i].blocks[lru_sort[j].index].tag, Cache->sets[i].blocks[lru_sort[j].index].dirty_bit ? 'D' : ' ');
			else
				printf("--------  ");

			printf("\t");
		}
		printf("\n");
	}

	int sb_flag = 0;

	if (Cache->config.pref_n && Cache->config.cache_level == L1_LEVEL) {
		printf("===== L1-SB contents =====");
			sb_flag = 1;
	} else if (Cache->config.pref_n && Cache->config.cache_level == L2_LEVEL) {
		printf("===== L2-SB contents =====");
		sb_flag = 1;
	}

	int *tmp1, *tmp2;
	tmp1 = (int *)malloc(sizeof(int) * Cache->config.pref_n);
	tmp2 = (int *)malloc(sizeof(int) * Cache->config.pref_n);

	if (sb_flag) {
		for (i=0 ; i < Cache->config.pref_n ; i++) {
			tmp1[i] = Cache->sb[i].lru_counter;
			tmp2[i] = i;
		}
		sb_sort_lru_counter(tmp1, tmp2, Cache->config.pref_n);

		for (i=0 ; i < Cache->config.pref_n ; i++) {
			printf("\n");
			for (j=0 ; j < Cache->config.pref_m ; j++) {
				printf("\t%8x", Cache->sb[tmp2[i]].sb_buf[j].memblock_addr);
			}
		}
		printf("\n");
	}

}

void copy_block(cache_block_t *dest, cache_block_t *src)
{
	dest->valid_bit = src->valid_bit;
	dest->dirty_bit = src->dirty_bit;
	dest->lru_counter = src->lru_counter;
	dest->tag = src->tag;
	dest->memblock_addr = src->memblock_addr;
}

/* cache_t *Cache: This argument specifies the Cache that is going to handle this request.
 * opnum: The number of this memory operation
 * mem_addr: The address at which the read request is made.
 * cache_t *next_Cache: If this argument is NULL, it means the next level is Main Memory, otherwise the L2_cache is passed.
 * */
cache_block_t handle_read_request(cache_t *Cache, int opnum, unsigned int mem_addr, cache_t *next_Cache, int is_prefetch)
{
	unsigned int tag;
	unsigned int index;
	unsigned int tmp;
	unsigned int memblock_addr;

	int i, j, k;
	int old_lru_counter, done=0;
	int max_lru_counter_val, max_lru_counter_val_index;

	int sb_hit=0, sb_old_lru_counter=0;
	int sb_max_lru_counter_val=0, sb_max_lru_counter_val_index=0;

	cache_block_t retblock;

	cache_block_t tmp_retblock;
	int tmp_memblock_addr=0, tmp_tag=0;

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

	if (opnum != -1) { ;
#ifdef DEBUG_OP
		printf("----------------------------------------\n");
		printf("# %d : read %x\n", opnum, mem_addr);
#endif
	}

#ifdef DEBUG_OP
	printf("L%c read : %x (tag %x, index %d)\n", Cache->config.cache_level==L1_LEVEL ? '1': '2', memblock_addr, tag, index);
#endif

	if (Cache->config.cache_level == L1_LEVEL)
		sim_res.L1_reads += 1;

	for (i=0 ; i < Cache->config.assoc ; i++) {
		if (Cache->sets[index].blocks[i].valid_bit == 1 && Cache->sets[index].blocks[i].tag == tag) {
			/* Cache hit! */
#ifdef DEBUG_OP
			printf("L%c hit\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
			old_lru_counter = Cache->sets[index].blocks[i].lru_counter;
			for (j=0 ; j < Cache->config.assoc ; j++) {
				if (i != j && Cache->sets[index].blocks[j].valid_bit == 1 && Cache->sets[index].blocks[j].lru_counter < old_lru_counter)
					Cache->sets[index].blocks[j].lru_counter += 1;
			}
			Cache->sets[index].blocks[i].lru_counter = 0;
			Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
#ifdef DEBUG_OP
			printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
			done = 1;
			copy_block(&retblock, &Cache->sets[index].blocks[i]);
			return retblock;
		}
	}


	if (done == 0) {

		for (i=0 ; i < Cache->config.assoc ; i++) {
			if (Cache->sets[index].blocks[i].valid_bit == 0) {
				/* Cache Miss - The first empty block in the required set is found. */
#ifdef DEBUG_OP
				printf("L%c miss\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				printf("L%c victim: none\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
				/* Check for a Hit in the first block of all stream buffers. */
				sb_hit = 0;
				for (j = 0 ; j < Cache->config.pref_n ; j++) {
					if (Cache->sb[j].sb_buf[0].valid_bit == 1 && Cache->sb[j].sb_buf[0].memblock_addr == memblock_addr) {
						sb_hit = 1;
						break;
					}
				}

				if (sb_hit == 0) { ;
					if (Cache->config.pref_n) {
						/* Request has missed in Cache as well as in first entry of all Stream Buffers. */
#ifdef DEBUG_OP
						printf("L%c-SB miss, ", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
						sb_max_lru_counter_val = Cache->config.pref_n - 1;
						sb_max_lru_counter_val_index = 0;
						for (j=0 ; j < Cache->config.pref_n ; j++) {
							if (Cache->sb[j].lru_counter == sb_max_lru_counter_val) {
								sb_max_lru_counter_val_index = j;
								break;
							}
						}
#ifdef DEBUG_OP
						printf("select LRU buffer #%d, ", sb_max_lru_counter_val_index);

						printf("update LRU\n");
#endif
						for (j=0 ; j < Cache->config.pref_n ; j++) {
							if (j != sb_max_lru_counter_val_index)
								Cache->sb[j].lru_counter += 1;
						}
					}

					/* Fetch the cache block from next level of memory hierarchy. */
					if (next_Cache) {
						sim_res.L2_reads_not_L1_prefetch += 1;
						retblock = handle_read_request(next_Cache, -1, mem_addr, NULL, 0);
						copy_block(&Cache->sets[index].blocks[i], &retblock);
					}

					Cache->sets[index].blocks[i].valid_bit = 1;
					Cache->sets[index].blocks[i].dirty_bit = 0;
					Cache->sets[index].blocks[i].tag = tag;
					Cache->sets[index].blocks[i].lru_counter = 0;
					Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
					for (j=0 ; j < Cache->config.assoc ; j++) {
						if (i != j && Cache->sets[index].blocks[j].valid_bit == 1)
							Cache->sets[index].blocks[j].lru_counter += 1;
					}

#ifdef DEBUG_OP					
					printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
					copy_block(&retblock, &Cache->sets[index].blocks[i]);

					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_read_misses += 1;

					if (Cache->config.pref_n) {
						/* Prefetch X+1 to X+M blocks in LRU selected Stream Buffer. */
						tmp_memblock_addr = memblock_addr;
						tmp_tag = 0;
						Cache->sb[sb_max_lru_counter_val_index].sb_valid_bit = 1;
						Cache->sb[sb_max_lru_counter_val_index].lru_counter = 0;
						for (j=0 ; j < Cache->config.pref_m ; j++) {
							tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
							tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
#ifdef DEBUG_OP
							printf("L%c-SB #%d prefetch %x\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_max_lru_counter_val_index, tmp_memblock_addr);
#endif
							if (Cache->config.cache_level == L1_LEVEL)
								sim_res.L1_prefetches += 1;
							else if (Cache->config.cache_level == L2_LEVEL)
								sim_res.L2_prefetches += 1;

							if (next_Cache) {
								sim_res.L2_read_L1_prefetch += 1;
								tmp_retblock = handle_read_request(next_Cache, -1, tmp_memblock_addr, NULL, 1);
								copy_block(&Cache->sb[sb_max_lru_counter_val_index].sb_buf[j], &tmp_retblock);
							}
	                                	        Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].valid_bit = 1;
                                        		Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].tag = tmp_tag;
	                                        	Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].memblock_addr = tmp_memblock_addr;
						}
					}

					if (is_prefetch == 1 && Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_read_miss_L1_prefetch += 1;
					else if (is_prefetch == 0 && Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_read_misses_not_L1_prefetch += 1;

					done = 1;

					break;
				} else { ;
					/* Request missed in Cache but hit in first entry of one of the Stream Buffers. */
					sb_hit = j;

#ifdef DEBUG_OP	
					printf("L%c-SB #%d hit, pop first entry, update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_hit);
#endif
					sb_old_lru_counter = Cache->sb[sb_hit].lru_counter;
					for (j=0 ; j < Cache->config.pref_n ; j++) {
						if (sb_hit != j && Cache->sb[j].lru_counter < sb_old_lru_counter) 
							Cache->sb[j].lru_counter += 1;
					}
					Cache->sb[sb_hit].lru_counter = 0;

					copy_block(&Cache->sets[index].blocks[i], &Cache->sb[sb_hit].sb_buf[0]);
					Cache->sets[index].blocks[i].valid_bit = 1;
					Cache->sets[index].blocks[i].dirty_bit = 0;
					Cache->sets[index].blocks[i].tag = tag;
					Cache->sets[index].blocks[i].lru_counter = 0;
					Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
					for (j=0 ; j < Cache->config.assoc ; j++) {
						if (i != j && Cache->sets[index].blocks[j].valid_bit == 1)
							Cache->sets[index].blocks[j].lru_counter += 1;
					}
					
#ifdef DEBUG_OP
					printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
					copy_block(&retblock, &Cache->sets[index].blocks[i]);

					/*if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_read_misses += 1;*/

					tmp_memblock_addr = memblock_addr;
					tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
					tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
					for (j = 1 ; j < Cache->config.pref_m ; j++) {
						copy_block(&Cache->sb[sb_hit].sb_buf[j-1], &Cache->sb[sb_hit].sb_buf[j]);

						Cache->sb[sb_hit].sb_buf[j-1].valid_bit = Cache->sb[sb_hit].sb_buf[j].valid_bit;
						//Cache->sb[sb_hit].sb_buf[j-1].valid_bit = 1;
						Cache->sb[sb_hit].sb_buf[j-1].tag = tmp_tag;
						Cache->sb[sb_hit].sb_buf[j-1].memblock_addr = tmp_memblock_addr;

						tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
						tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
						
					}

#ifdef DEBUG_OP
					printf("L%c-SB #%d prefetch %x\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_hit, tmp_memblock_addr);
#endif

					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_prefetches += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_prefetches += 1;

					if (next_Cache) {
						sim_res.L2_read_L1_prefetch += 1;
						tmp_retblock = handle_read_request(next_Cache, -1, tmp_memblock_addr, NULL, 1);
						copy_block(&Cache->sb[sb_hit].sb_buf[j-1], &tmp_retblock);
					}
					Cache->sb[sb_hit].sb_buf[j-1].valid_bit = 1;
					Cache->sb[sb_hit].sb_buf[j-1].tag = tmp_tag;
					Cache->sb[sb_hit].sb_buf[j-1].memblock_addr = tmp_memblock_addr;

					/*if (is_prefetch == 1 && Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_read_miss_L1_prefetch += 1;
					else if (is_prefetch == 0 && Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_read_misses_not_L1_prefetch += 1;*/

					done = 1;
					break;
				}
			}
		}
	}

	if (done == 0) {

		/* No Empty block found in the Set. Also no existing block in the set matches the required block address.
		*  Therefore, a victim block must be chosen and be replaced.
		*/
		max_lru_counter_val = 0;
		max_lru_counter_val_index = 0;
		for (i=0 ; i < Cache->config.assoc ; i++) {
			if (Cache->sets[index].blocks[i].valid_bit == 1 && Cache->sets[index].blocks[i].lru_counter > max_lru_counter_val) {
				max_lru_counter_val = Cache->sets[index].blocks[i].lru_counter;
				max_lru_counter_val_index = i;
			}
		}

#ifdef DEBUG_OP
		printf("L%c miss\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
		printf("L%c victim: %x (tag %x, index %d, %s)\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr, Cache->sets[index].blocks[max_lru_counter_val_index].tag, index, Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit ? "dirty": "clean");
#endif

		if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
		for (j=0 ; j < Cache->config.pref_n ; j++) {
			if (Cache->sb[j].sb_valid_bit) {
				for (k=0 ; k < Cache->config.pref_m ; k++) {
					if (Cache->sb[j].sb_buf[k].valid_bit == 1 && Cache->sb[j].sb_buf[k].memblock_addr == Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr) {
						Cache->sb[j].sb_buf[k].valid_bit = 0;
						Cache->sb[j].sb_buf[k].tag = 0;
						Cache->sb[j].sb_buf[k].memblock_addr = 0;
					}
				}
			}
		}
		}

		/* Check for a Hit in the first block of all stream buffers. */
		sb_hit = 0;
		for (j = 0 ; j < Cache->config.pref_n ; j++) {
			if (Cache->sb[j].sb_buf[0].valid_bit == 1 && Cache->sb[j].sb_buf[0].memblock_addr == memblock_addr) {
				sb_hit = 1;
				break;
			}
		}

		if (sb_hit == 0) { ;
			if (next_Cache) {
				if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_writebacks += 1;
					retblock = handle_write_request(next_Cache, -1, Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr, NULL, 0);
				}
			} else {
				if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_writebacks += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_writebacks += 1;
				}
			}

			if (Cache->config.pref_n) {

				/* Request has missed in Cache as well as in first entry of all Stream Buffers. */
#ifdef DEBUG_OP
				printf("L%c-SB miss, ", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
				sb_max_lru_counter_val = Cache->config.pref_n - 1;
				sb_max_lru_counter_val_index = 0;
				for (j=0 ; j < Cache->config.pref_n ; j++) {
					if (Cache->sb[j].lru_counter == sb_max_lru_counter_val) {
						sb_max_lru_counter_val_index = j;
						break;
					}
				}
#ifdef DEBUG_OP
				printf("select LRU buffer #%d, ", sb_max_lru_counter_val_index);

				printf("update LRU\n");
#endif
				for (j=0 ; j < Cache->config.pref_n ; j++) {
					if (j != sb_max_lru_counter_val_index)
						Cache->sb[j].lru_counter += 1;
				}
			}

			/* Fetch the cache block from next level of memory hierarchy. */
			if (next_Cache) {
				sim_res.L2_reads_not_L1_prefetch += 1;
				retblock = handle_read_request(next_Cache, -1, mem_addr, NULL, 0);
				copy_block(&Cache->sets[index].blocks[max_lru_counter_val_index], &retblock);
			}

			Cache->sets[index].blocks[max_lru_counter_val_index].valid_bit = 1;
			Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit = 0;
			Cache->sets[index].blocks[max_lru_counter_val_index].tag = tag;
			Cache->sets[index].blocks[max_lru_counter_val_index].lru_counter = 0;
			Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr = memblock_addr;

			for (j=0 ; j < Cache->config.assoc ; j++) {
				if (max_lru_counter_val_index != j && Cache->sets[index].blocks[j].valid_bit == 1)
					Cache->sets[index].blocks[j].lru_counter += 1;
			}

			if (is_prefetch == 1 && Cache->config.cache_level == L2_LEVEL)
				sim_res.L2_read_miss_L1_prefetch += 1;
			else if (is_prefetch == 0 && Cache->config.cache_level == L2_LEVEL)
				sim_res.L2_read_misses_not_L1_prefetch += 1;

			done = 1;
#ifdef DEBUG_OP
			printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
			copy_block(&retblock, &Cache->sets[index].blocks[max_lru_counter_val_index]);

			if (Cache->config.cache_level == L1_LEVEL)
				sim_res.L1_read_misses += 1;

			if (Cache->config.pref_n) {
				/* Prefetch X+1 to X+M blocks in LRU selected Stream Buffer. */
				tmp_memblock_addr = memblock_addr;
				tmp_tag = 0;
				Cache->sb[sb_max_lru_counter_val_index].sb_valid_bit = 1;
				Cache->sb[sb_max_lru_counter_val_index].lru_counter = 0;
				for (j=0 ; j < Cache->config.pref_m ; j++) {
					tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
					tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
#ifdef DEBUG_OP
					printf("L%c-SB #%d prefetch %x\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_max_lru_counter_val_index, tmp_memblock_addr);
#endif
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_prefetches += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_prefetches += 1;

					if (next_Cache) {
						sim_res.L2_read_L1_prefetch += 1;
						tmp_retblock = handle_read_request(next_Cache, -1, tmp_memblock_addr, NULL, 1);
						copy_block(&Cache->sb[sb_max_lru_counter_val_index].sb_buf[j], &tmp_retblock);
					}
                               	        Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].valid_bit = 1;
					Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].tag = tmp_tag;
                                       	Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].memblock_addr = tmp_memblock_addr;
				}
			}
		} else {;
			/* Request missed in Cache but hit in first entry of one of the Stream Buffers. */
			sb_hit = j;

#ifdef DEBUG_OP	
			printf("L%c-SB #%d hit, pop first entry, update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_hit);
#endif

			if (next_Cache) {
				if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_writebacks += 1;
					retblock = handle_write_request(next_Cache, -1, Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr, NULL, 0);
				}
			} else {
				if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_writebacks += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_writebacks += 1;
				}
			}

			sb_old_lru_counter = Cache->sb[sb_hit].lru_counter;
			for (j=0 ; j < Cache->config.pref_n ; j++) {
				if (sb_hit != j && Cache->sb[j].lru_counter < sb_old_lru_counter) 
					Cache->sb[j].lru_counter += 1;
			}
			Cache->sb[sb_hit].lru_counter = 0;

			copy_block(&Cache->sets[index].blocks[max_lru_counter_val_index], &Cache->sb[sb_hit].sb_buf[0]);
			Cache->sets[index].blocks[max_lru_counter_val_index].valid_bit = 1;
			Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit = 0;
			Cache->sets[index].blocks[max_lru_counter_val_index].tag = tag;
			Cache->sets[index].blocks[max_lru_counter_val_index].lru_counter = 0;
			Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr = memblock_addr;
			for (j=0 ; j < Cache->config.assoc ; j++) {
				if (max_lru_counter_val_index != j && Cache->sets[index].blocks[j].valid_bit == 1)
					Cache->sets[index].blocks[j].lru_counter += 1;
			}


#ifdef DEBUG_OP
			printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
			copy_block(&retblock, &Cache->sets[index].blocks[max_lru_counter_val_index]);

			/*if (Cache->config.cache_level == L1_LEVEL)
				sim_res.L1_read_misses += 1;*/

			tmp_memblock_addr = memblock_addr;
			tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
			tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);

			for (j = 1 ; j < Cache->config.pref_m ; j++) {
				copy_block(&Cache->sb[sb_hit].sb_buf[j-1], &Cache->sb[sb_hit].sb_buf[j]);

				Cache->sb[sb_hit].sb_buf[j-1].valid_bit = Cache->sb[sb_hit].sb_buf[j].valid_bit;
				//Cache->sb[sb_hit].sb_buf[j-1].valid_bit = 1;
				Cache->sb[sb_hit].sb_buf[j-1].tag = tmp_tag;
				Cache->sb[sb_hit].sb_buf[j-1].memblock_addr = tmp_memblock_addr;
				tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
				tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
						
			}

#ifdef DEBUG_OP
			printf("L%c-SB #%d prefetch %x\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_hit, tmp_memblock_addr);
#endif
			if (Cache->config.cache_level == L1_LEVEL)
				sim_res.L1_prefetches += 1;
			else if (Cache->config.cache_level == L2_LEVEL)
				sim_res.L2_prefetches += 1;

			if (next_Cache) {
				sim_res.L2_read_L1_prefetch += 1;
				tmp_retblock = handle_read_request(next_Cache, -1, tmp_memblock_addr, NULL, 1);
				copy_block(&Cache->sb[sb_hit].sb_buf[j-1], &tmp_retblock);
			}
			Cache->sb[sb_hit].sb_buf[j-1].valid_bit = 1;
			Cache->sb[sb_hit].sb_buf[j-1].tag = tmp_tag;
			Cache->sb[sb_hit].sb_buf[j-1].memblock_addr = tmp_memblock_addr;
		
			/*if (is_prefetch == 1 && Cache->config.cache_level == L2_LEVEL)
				sim_res.L2_read_miss_L1_prefetch += 1;
			else if (is_prefetch == 0 && Cache->config.cache_level == L2_LEVEL)
				sim_res.L2_read_misses_not_L1_prefetch += 1;*/

			done = 1;
		}
	}

	return retblock;
}

/* cache_t *Cache: This argument specifies the Cache that is going to handle this request.
 * opnum: The number of this memory operation
 * mem_addr: The address at which the write request is made.
 * cache_t *next_Cache: If this argument is NULL, it means the next level is Main Memory, otherwise the L2_cache is passed.
 * */
cache_block_t handle_write_request(cache_t *Cache, int opnum, unsigned int mem_addr, cache_t *next_Cache, int prefetch)
{
	unsigned int tag;
	unsigned int index;
	unsigned int tmp;
	unsigned int memblock_addr;

	int i, j, k;
	int old_lru_counter, done=0;
	int max_lru_counter_val, max_lru_counter_val_index;

	int sb_hit=0, sb_old_lru_counter=0;
        int sb_max_lru_counter_val=0, sb_max_lru_counter_val_index=0;

	cache_block_t retblock;

        cache_block_t tmp_retblock;
        int tmp_memblock_addr=0, tmp_tag=0;

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

	if (opnum != -1) { ;
#ifdef DEBUG_OP
		printf("----------------------------------------\n");
		printf("# %d : write %x\n", opnum, mem_addr);
#endif
	}
#ifdef DEBUG_OP
	printf("L%c Write : %x  (tag %x, index %d)\n", Cache->config.cache_level==L1_LEVEL ? '1': '2', memblock_addr, tag, index);
#endif

	if (Cache->config.cache_level == L1_LEVEL)
		sim_res.L1_writes += 1;
	else if (Cache->config.cache_level == L2_LEVEL)
		sim_res.L2_writes += 1;

	for (i=0 ; i < Cache->config.assoc ; i++) {
		if (Cache->sets[index].blocks[i].valid_bit == 1 && Cache->sets[index].blocks[i].tag == tag) {
			/* Cache hit! */
#ifdef DEBUG_OP
			printf("L%c hit\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
			old_lru_counter = Cache->sets[index].blocks[i].lru_counter;
			for (j=0 ; j < Cache->config.assoc ; j++) {
				if (i != j && Cache->sets[index].blocks[j].valid_bit == 1 && Cache->sets[index].blocks[j].lru_counter < old_lru_counter) 
					Cache->sets[index].blocks[j].lru_counter += 1;
			}
			Cache->sets[index].blocks[i].lru_counter = 0;
			Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
			Cache->sets[index].blocks[i].dirty_bit = 1;
#ifdef DEBUG_OP
			printf("L%c update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
			printf("L%c set dirty\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
			done = 1;
			copy_block(&retblock, &Cache->sets[index].blocks[i]);
			return retblock;
		}
	}

	if (done == 0) {
		for (i=0 ; i < Cache->config.assoc ; i++) {
			if (Cache->sets[index].blocks[i].valid_bit == 0) {
				/* Cache Miss - The first empty block in the required set is found. */
#ifdef DEBUG_OP
				printf("L%c miss\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
				printf("L%c victim: none\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
                                /* Check for a Hit in the first block of all stream buffers. */
                                sb_hit = 0;
                                for (j = 0 ; j < Cache->config.pref_n ; j++) {
                                        if (Cache->sb[j].sb_buf[0].valid_bit == 1 && Cache->sb[j].sb_buf[0].memblock_addr == memblock_addr) {
                                                sb_hit = 1;
                                                break;
                                        }
                                }

                                if (sb_hit == 0) { ;
                                        if (Cache->config.pref_n) {
                                                /* Request has missed in Cache as well as in first entry of all Stream Buffers. */
#ifdef DEBUG_OP
                                                printf("L%c-SB miss, ", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
                                                sb_max_lru_counter_val = Cache->config.pref_n - 1;
                                                sb_max_lru_counter_val_index = 0;
                                                for (j=0 ; j < Cache->config.pref_n ; j++) {
                                                        if (Cache->sb[j].lru_counter == sb_max_lru_counter_val) {
                                                                sb_max_lru_counter_val_index = j;
								break;
                                                        }
                                                }
#ifdef DEBUG_OP
                                                printf("select LRU buffer #%d, ", sb_max_lru_counter_val_index);

                                                printf("update LRU\n");
#endif
                                                for (j=0 ; j < Cache->config.pref_n ; j++) {
                                                        if (j != sb_max_lru_counter_val_index)
                                                                Cache->sb[j].lru_counter += 1;
                                                }
                                        }

					/* Fetch the block from the next level of the memory hierarchy. */
	                                if (next_Cache) {
						sim_res.L2_reads_not_L1_prefetch += 1;
        	                                retblock = handle_read_request(next_Cache, -1, mem_addr, NULL, 0);
                	                        copy_block(&Cache->sets[index].blocks[i], &retblock);
                        	        }
					Cache->sets[index].blocks[i].valid_bit = 1;
					Cache->sets[index].blocks[i].dirty_bit = 1;
					Cache->sets[index].blocks[i].tag = tag;
					Cache->sets[index].blocks[i].lru_counter = 0;
					Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
					for (j=0 ; j < Cache->config.assoc ; j++) {
						if (i != j && Cache->sets[index].blocks[j].valid_bit == 1)
							Cache->sets[index].blocks[j].lru_counter += 1;
					}
					done = 1;
#ifdef DEBUG_OP
					printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
					copy_block(&retblock, &Cache->sets[index].blocks[i]);

					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_write_misses += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_write_misses += 1;

					if (Cache->config.pref_n) {
						/* Prefetch X+1 to X+M blocks in LRU selected Stream Buffer. */
						tmp_memblock_addr = memblock_addr;
						tmp_tag = 0;
						Cache->sb[sb_max_lru_counter_val_index].sb_valid_bit = 1;
						Cache->sb[sb_max_lru_counter_val_index].lru_counter = 0;
						for (j=0 ; j < Cache->config.pref_m ; j++) {
							tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
							tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
#ifdef DEBUG_OP
							printf("L%c-SB #%d prefetch %x\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_max_lru_counter_val_index, tmp_memblock_addr);
#endif
							if (Cache->config.cache_level == L1_LEVEL)
								sim_res.L1_prefetches += 1;
							else if (Cache->config.cache_level == L2_LEVEL)
								sim_res.L2_prefetches += 1;

							if (next_Cache) {
								sim_res.L2_read_L1_prefetch += 1;
								tmp_retblock = handle_read_request(next_Cache, -1, tmp_memblock_addr, NULL, 1);
								copy_block(&Cache->sb[sb_max_lru_counter_val_index].sb_buf[j], &tmp_retblock);
							}
							Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].valid_bit = 1;
							Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].tag = tmp_tag;
							Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].memblock_addr = tmp_memblock_addr;
                                                }
                                        }

#ifdef DEBUG_OP
					printf("L%c set dirty\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
					break;
				} else { ;
					/* Request missed in Cache but hit in first entry of one of the Stream Buffers. */
					sb_hit = j;
#ifdef DEBUG_OP	
					printf("L%c-SB #%d hit, pop first entry, update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_hit);
#endif
					sb_old_lru_counter = Cache->sb[sb_hit].lru_counter;
					for (j=0 ; j < Cache->config.pref_n ; j++) {
						if (sb_hit != j && Cache->sb[j].lru_counter < sb_old_lru_counter) 
							Cache->sb[j].lru_counter += 1;
					}
					Cache->sb[sb_hit].lru_counter = 0;

                                        copy_block(&Cache->sets[index].blocks[i], &Cache->sb[sb_hit].sb_buf[0]);
					Cache->sets[index].blocks[i].valid_bit = 1;
					Cache->sets[index].blocks[i].dirty_bit = 1;
					Cache->sets[index].blocks[i].tag = tag;
					Cache->sets[index].blocks[i].lru_counter = 0;
					Cache->sets[index].blocks[i].memblock_addr = memblock_addr;
					for (j=0 ; j < Cache->config.assoc ; j++) {
						if (i != j && Cache->sets[index].blocks[j].valid_bit == 1)
							Cache->sets[index].blocks[j].lru_counter += 1;
					}

#ifdef DEBUG_OP
					printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif

					/* if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_write_misses += 1; */

					tmp_memblock_addr = memblock_addr;
					tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
					tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
					for (j = 1 ; j < Cache->config.pref_m ; j++) {
						copy_block(&Cache->sb[sb_hit].sb_buf[j-1], &Cache->sb[sb_hit].sb_buf[j]);
						Cache->sb[sb_hit].sb_buf[j-1].valid_bit = Cache->sb[sb_hit].sb_buf[j].valid_bit;
						//Cache->sb[sb_hit].sb_buf[j-1].valid_bit = 1;
						Cache->sb[sb_hit].sb_buf[j-1].tag = tmp_tag;
						Cache->sb[sb_hit].sb_buf[j-1].memblock_addr = tmp_memblock_addr;
						
						tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
						tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);

					}
					
#ifdef DEBUG_OP
					printf("L%c-SB #%d prefetch %x\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_hit, tmp_memblock_addr);
#endif

					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_prefetches += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_prefetches += 1;

					if (next_Cache) {
						sim_res.L2_read_L1_prefetch += 1;
						tmp_retblock = handle_read_request(next_Cache, -1, tmp_memblock_addr, NULL, 1);
						copy_block(&Cache->sb[sb_hit].sb_buf[j-1], &tmp_retblock);
					}
					Cache->sb[sb_hit].sb_buf[j-1].valid_bit = 1;
					Cache->sb[sb_hit].sb_buf[j-1].tag = tmp_tag;
					Cache->sb[sb_hit].sb_buf[j-1].memblock_addr = tmp_memblock_addr;
					done = 1;
#ifdef DEBUG_OP
					printf("L%c set dirty\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
					copy_block(&retblock, &Cache->sets[index].blocks[i]);
					break;
				}

			}
		}
	}

	if (done == 0) {
		/* No Empty block found in the Set. Also no existing block in the set matches the required block address.
		*  Therefore, a victim block must be chosen and be replaced.
		*/
		max_lru_counter_val = 0; 
		max_lru_counter_val_index = 0;
		for (i=0 ; i < Cache->config.assoc ; i++) {
			if (Cache->sets[index].blocks[i].valid_bit == 1 && Cache->sets[index].blocks[i].lru_counter > max_lru_counter_val) {
				max_lru_counter_val = Cache->sets[index].blocks[i].lru_counter;
				max_lru_counter_val_index = i;
			}
		}

#ifdef DEBUG_OP
		printf("L%c miss\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
		printf("L%c victim: %x (tag %x, index %d, %s)\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr, Cache->sets[index].blocks[max_lru_counter_val_index].tag, index, Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit ? "dirty": "clean");
#endif

		if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
		for (j=0 ; j < Cache->config.pref_n ; j++) {
			if (Cache->sb[j].sb_valid_bit) {
				for (k=0 ; k < Cache->config.pref_m ; k++) {
					if (Cache->sb[j].sb_buf[k].valid_bit == 1 && Cache->sb[j].sb_buf[k].memblock_addr == Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr) {
						Cache->sb[j].sb_buf[k].valid_bit = 0;
						Cache->sb[j].sb_buf[k].tag = 0;
						Cache->sb[j].sb_buf[k].memblock_addr = 0;
					}
				}
			}
		}
		}

                /* Check for a Hit in the first block of all stream buffers. */
                sb_hit = 0;
                for (j = 0 ; j < Cache->config.pref_n ; j++) {
                        if (Cache->sb[j].sb_buf[0].valid_bit == 1 && Cache->sb[j].sb_buf[0].memblock_addr == memblock_addr) {
                                sb_hit = 1;
                                break;
                        }
                }

                if (sb_hit == 0) { ;
			if (next_Cache) {
				if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_writebacks += 1;

					retblock = handle_write_request(next_Cache, -1, Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr, NULL, 0);
				}
			} else {
				if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_writebacks += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_writebacks += 1;
				}
			}

                        if (Cache->config.pref_n) {
                                /* Request has missed in Cache as well as in first entry of all Stream Buffers. */
#ifdef DEBUG_OP
                                printf("L%c-SB miss, ", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
                                sb_max_lru_counter_val = Cache->config.pref_n - 1;
                                sb_max_lru_counter_val_index = 0;
                                for (j=0 ; j < Cache->config.pref_n ; j++) {
                                        if (Cache->sb[j].lru_counter == sb_max_lru_counter_val) {
                                                sb_max_lru_counter_val_index = j;
						break;
                                        }
                                }
#ifdef DEBUG_OP
                                printf("select LRU buffer #%d, ", sb_max_lru_counter_val_index);

                                printf("update LRU\n");
#endif
                                for (j=0 ; j < Cache->config.pref_n ; j++) {
                                        if (j != sb_max_lru_counter_val_index)
                                                Cache->sb[j].lru_counter += 1;
                                        }
                        }

                        /* Fetch the cache block from next level of memory hierarchy. */
                        if (next_Cache) {
				sim_res.L2_reads_not_L1_prefetch += 1;
                                retblock = handle_read_request(next_Cache, -1, mem_addr, NULL, 0);
                                copy_block(&Cache->sets[index].blocks[max_lru_counter_val_index], &retblock);
                        }

                        Cache->sets[index].blocks[max_lru_counter_val_index].valid_bit = 1;
                        Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit = 1;
                        Cache->sets[index].blocks[max_lru_counter_val_index].tag = tag;
                        Cache->sets[index].blocks[max_lru_counter_val_index].lru_counter = 0;
                        Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr = memblock_addr;

                        for (j=0 ; j < Cache->config.assoc ; j++) {
                                if (max_lru_counter_val_index != j && Cache->sets[index].blocks[j].valid_bit == 1)
                                        Cache->sets[index].blocks[j].lru_counter += 1;
                        }
                        done = 1;
#ifdef DEBUG_OP
                        printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
                        copy_block(&retblock, &Cache->sets[index].blocks[max_lru_counter_val_index]);

                        if (Cache->config.cache_level == L1_LEVEL)
                                sim_res.L1_write_misses += 1;
			else if (Cache->config.cache_level == L2_LEVEL)
				sim_res.L2_write_misses += 1;

                        if (Cache->config.pref_n) {
                                /* Prefetch X+1 to X+M blocks in LRU selected Stream Buffer. */
                                tmp_memblock_addr = memblock_addr;
                                tmp_tag = 0;
                                Cache->sb[sb_max_lru_counter_val_index].sb_valid_bit = 1;
                                Cache->sb[sb_max_lru_counter_val_index].lru_counter = 0;
                                for (j=0 ; j < Cache->config.pref_m ; j++) {
                                        tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
                                        tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
#ifdef DEBUG_OP
                                        printf("L%c-SB #%d prefetch %x\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_max_lru_counter_val_index, tmp_memblock_addr);
#endif
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_prefetches += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_prefetches += 1;

                                        if (next_Cache) {
						sim_res.L2_read_L1_prefetch += 1;
                                                tmp_retblock = handle_read_request(next_Cache, -1, tmp_memblock_addr, NULL, 1);
                                                copy_block(&Cache->sb[sb_max_lru_counter_val_index].sb_buf[j], &tmp_retblock);
                                        }
                                        Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].valid_bit = 1;
                                        Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].tag = tmp_tag;
                                        Cache->sb[sb_max_lru_counter_val_index].sb_buf[j].memblock_addr = tmp_memblock_addr;
                                }
                        }

#ifdef DEBUG_OP
			printf("L%c set dirty\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
		} else { ;
                       /* Request missed in Cache but hit in first entry of one of the Stream Buffers. */
                        sb_hit = j;
	
#ifdef DEBUG_OP
			printf("L%c-SB #%d hit, pop first entry, update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_hit);
#endif
			if (next_Cache) {
				if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_writebacks += 1;
					retblock = handle_write_request(next_Cache, -1, Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr, NULL, 0);
				}
			} else {
				if (Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit) {
					if (Cache->config.cache_level == L1_LEVEL)
						sim_res.L1_writebacks += 1;
					else if (Cache->config.cache_level == L2_LEVEL)
						sim_res.L2_writebacks += 1;
				}
			}

			sb_old_lru_counter = Cache->sb[sb_hit].lru_counter;
			for (j=0 ; j < Cache->config.pref_n ; j++) {
				if (sb_hit != j && Cache->sb[j].lru_counter < sb_old_lru_counter) 
					Cache->sb[j].lru_counter += 1;
			}
			Cache->sb[sb_hit].lru_counter = 0;

                        copy_block(&Cache->sets[index].blocks[max_lru_counter_val_index], &Cache->sb[sb_hit].sb_buf[0]);
                        Cache->sets[index].blocks[max_lru_counter_val_index].valid_bit = 1;
                        Cache->sets[index].blocks[max_lru_counter_val_index].dirty_bit = 1;
                        Cache->sets[index].blocks[max_lru_counter_val_index].tag = tag;
                        Cache->sets[index].blocks[max_lru_counter_val_index].lru_counter = 0;
                        Cache->sets[index].blocks[max_lru_counter_val_index].memblock_addr = memblock_addr;
                        for (j=0 ; j < Cache->config.assoc ; j++) {
                                if (max_lru_counter_val_index != j && Cache->sets[index].blocks[j].valid_bit == 1)
                                        Cache->sets[index].blocks[j].lru_counter += 1;
                        }
                        done = 1;
#ifdef DEBUG_OP
                        printf("L%c Update LRU\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif
                        copy_block(&retblock, &Cache->sets[index].blocks[max_lru_counter_val_index]);

                        /* if (Cache->config.cache_level == L1_LEVEL)
                                sim_res.L1_write_misses += 1; */

			tmp_memblock_addr = memblock_addr;
			tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
                        tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);

                        for (j = 1 ; j < Cache->config.pref_m ; j++) {
                                copy_block(&Cache->sb[sb_hit].sb_buf[j-1], &Cache->sb[sb_hit].sb_buf[j]);
				Cache->sb[sb_hit].sb_buf[j-1].valid_bit = Cache->sb[sb_hit].sb_buf[j].valid_bit;
				//Cache->sb[sb_hit].sb_buf[j-1].valid_bit = 1;
				Cache->sb[sb_hit].sb_buf[j-1].tag = tmp_tag;
				Cache->sb[sb_hit].sb_buf[j-1].memblock_addr = tmp_memblock_addr;

                                tmp_memblock_addr = (((tmp_memblock_addr >> Cache->config.num_blockoffset_bits) + 0x1 ) << Cache->config.num_blockoffset_bits);
                                tmp_tag = (tmp_memblock_addr >> (Cache->config.num_blockoffset_bits+Cache->config.num_index_bits)) & ~(~0 << Cache->config.num_tag_bits);
			}

#ifdef DEBUG_OP
			printf("L%c-SB #%d prefetch %x\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2', sb_hit, tmp_memblock_addr);
#endif

			if (Cache->config.cache_level == L1_LEVEL)
				sim_res.L1_prefetches += 1;
			else if (Cache->config.cache_level == L2_LEVEL)
				sim_res.L2_prefetches += 1;

                        if (next_Cache) {
				sim_res.L2_read_L1_prefetch += 1;
                                tmp_retblock = handle_read_request(next_Cache, -1, tmp_memblock_addr, NULL, 1);
                                copy_block(&Cache->sb[sb_hit].sb_buf[j-1], &tmp_retblock);
                        }
                        Cache->sb[sb_hit].sb_buf[j-1].valid_bit = 1;
                        Cache->sb[sb_hit].sb_buf[j-1].tag = tmp_tag;
                        Cache->sb[sb_hit].sb_buf[j-1].memblock_addr = tmp_memblock_addr;

#ifdef DEBUG_OP
			printf("L%c set dirty\n", Cache->config.cache_level==L1_LEVEL ? '1' : '2');
#endif

			done = 1;
                }

	}

	return retblock;
}

