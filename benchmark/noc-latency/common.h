#define KB (1024)
#define MB (1024*KB)
#define GB (1024*KB)

#define milli (1.0/1000)
#define micro (milli/1000)
#define nano  (micro/1000)

#define MIN_BUFFER_SIZE (1*KB)
#define MAX_BUFFER_SIZE (512*KB)

// number of executions of the benchmark
#define NITERATIONS 1537

typedef struct {
  int cluster_id;
} rqueue_msg_t;

void init_buffer(char *buffer, int size);
void fill_buffer(char *buffer, int size, int cluster_id);
