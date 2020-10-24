
#ifndef EXTRA_TYPES
#define EXTRA_TYPES
typedef int (*test_fp) (int, char **);

typedef struct
{
  char *name;
  char *suite;
  test_fp test_function;

} testentry_t;
#else
#endif


int run_testrunner(int argc, char **argv, testentry_t *entries,int entry_count);
void set_testrunner_default_timeout(int s);
void set_testrunner_timeout(int s);

//void do_randomized_test(int strategyToUse, int totalSize, float fillRatio, int minBlockSize, int maxBlockSize, int iterations);

