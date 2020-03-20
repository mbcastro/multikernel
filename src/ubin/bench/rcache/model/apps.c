#include "headers/sha-data-60-60-config.h"
#include "benchmark.h"

static const int g_row[] = {row_sha};
static const int g_col[] = {col_sha};
static struct interval *g_pages_interval[] = {pages_interval_sha};
static unsigned *g_work[] = {work_sha};
static unsigned *g_pages_strike[] = {pages_strike_sha};

struct workload apps = {
.size = 1,
.row = g_row,
.col = g_col,
.pages_interval = g_pages_interval,
.work = g_work,
.pages_strike = g_pages_strike
};
