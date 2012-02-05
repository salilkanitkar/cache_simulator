#define TRUE 0
#define FALSE 1

#define MAX_FILENAME_LEN 100
#define MAX_TRACESTR_LEN 16

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

