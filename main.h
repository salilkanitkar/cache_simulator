#define TRUE 0
#define FALSE 1

#define MAX_FILENAME_LEN 100
#define MAX_TRACESTR_LEN 16

#define L1_LEVEL 1
#define L2_LEVEL 2

#define NUM_ADDR_BITS 32

extern int blockSize;

extern int L1_size;
extern int L1_assoc;
extern int L1_pref_n;
extern int L1_pref_m;

extern int L2_size;
extern int L2_assoc;
extern int L2_pref_n;
extern int L2_pref_m;

extern char trace_file[MAX_FILENAME_LEN];
extern FILE *fp_trace;
extern char trace_str[MAX_TRACESTR_LEN];

typedef struct _mem_op_t {
	char opcode;
	unsigned int addr;
}mem_op_t;

extern mem_op_t mem_op;

typedef struct _cache_config_t {
	int cache_level;

	int size;
	int assoc;
	int pref_n;
	int pref_m;

	int num_sets;
	int num_tag_bits;
	int num_index_bits;
	int num_blockoffset_bits;
}cache_config_t;

typedef struct _cache_block_t {
	int valid_bit;
	int dirty_bit;
	int lru_counter;
	unsigned int tag;
	unsigned int memblock_addr;
}cache_block_t;

typedef struct _cache_set_t {
	int assoc;
	cache_block_t *blocks;
}cache_set_t;

typedef struct _sb_t {
	int sb_valid_bit;
	int lru_counter;
	cache_block_t *sb_buf;
}sb_t;

typedef struct _cache_t {
	cache_config_t config;
	cache_set_t *sets;
	sb_t *sb;
}cache_t;

extern cache_t L1_cache;
extern cache_t L2_cache;

extern void set_cache_params(cache_t *);
extern void allocate_cache(cache_t *);
extern void print_cache(cache_t *);

typedef struct _sim_res_t {
	int L1_reads;
	int L1_read_misses;
	int L1_writes;
	int L1_write_misses;
	float L1_miss_rate;
	int L1_writebacks;
	int L1_prefetches;

	int L2_reads_not_L1_prefetch;
	int L2_read_misses_not_L1_prefetch;
	int L2_read_L1_prefetch;
	int L2_read_miss_L1_prefetch;
	int L2_writes;
	int L2_write_misses;
	float L2_miss_rate;
	int L2_writebacks;
	int L2_prefetches;

	unsigned int tot_mem_traffic;
}sim_res_t;

extern sim_res_t sim_res;

extern cache_block_t handle_read_request(cache_t *, int , unsigned int , cache_t *, int);
extern cache_block_t handle_write_request(cache_t *, int , unsigned int , cache_t *, int);

typedef struct _lru_sort_t {
	int index;
	int lru_counter;
}lru_sort_t;

