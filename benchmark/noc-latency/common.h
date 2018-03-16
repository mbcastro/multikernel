#define KB 1024
#define MB 1024 * KB

// maximum size of a message
#define MAX_BUFFER_SIZE 1 * MB + MB / 2

// number of executions of the benchmark
#define NB_EXEC 30

typedef struct {
  int cluster_id;
} rqueue_msg_t;

void init_buffer(char *buffer, int size);
void fill_buffer(char *buffer, int size, int cluster_id);
