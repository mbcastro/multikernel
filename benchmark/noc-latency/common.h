#define KB (1024)
#define MB (1024*KB)
#define GB (1024*KB)

#define milli (1.0/1000)
#define micro (milli/1000)
#define nano  (micro/1000)

#define MIN_BUFFER_SIZE (16*KB)
#define MAX_BUFFER_SIZE (1024*KB)

#define NR_DMA 4

// number of executions of the benchmark
#define NITERATIONS 5

void init_buffer(char *buffer, int size);
void fill_buffer(char *buffer, int size, int cluster_id);
