#include "stdio.h"
#include "cargs.h"
#include "cthread.h"
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define TEST_CTHREAD

#ifdef TEST_CARGS
#define ARGS \
  START_FN(test,,) \
    START_FN(test1,,) \
      REQ("test1_int", int, test1_int) \
    END_FN(test1) \
    START_FN(test2,,) \
      OPT("-t2", "--test2", int, test2_int, 1) \
    END_FN(test2) \
    REQ("testing_double", int, testdouble) \
    REQ("testing_double2", int, test1double) \
    OPT("-o", "--opt", int, i, 1) \
    OPT("-opt", "--optional", int, j, 2) \
    FLG("-j", "", int, jobs, 1) \
    FLG("-Wall", "", int, wall, 0) \
    TXT("-h", "--help", "help") \
    TXT("-help", "---help", "another help") \
  END_FN(test) \
  START_FN(cool_function,,) \
    OPT("-c", "--cool", int, a, 3) \
  END_FN(cool_function) \
  START_FN(,,) \
    REQ("default int", int, d) \
    HLP("-h", "--help", "%%subfunctions%%") \
  END_FN(,,,) 

#include "cargs.h"

void ARG_HANDLER(test1,) {
  printf("test1");
}
void ARG_HANDLER(test2,) {
  printf("test2");
}
void ARG_HANDLER(cool_function,) {
  printf("cool_function");
}

void ARG_HANDLER(test,) {
  struct s_test *arg = args;
  printf("%d %d %d %d", arg->testdouble, arg->test1double, arg->j, arg->jobs);
}
void ARG_HANDLER(,) {
  printf("default callback");
}
#endif // TEST_CARGS

#ifdef TEST_CTHREAD

thread_fn_t thread_fn(int* arg) {
  printf("thread started, %d\n", *arg);
  fflush(stdout);
}

#endif // TEST_CTHREAD

int main(int argc, char** argv) {

#ifdef TEST_CARGS
  if (!cargs_parse_args(argc, argv)) {
    return 0;
  }
#endif // TEST_CARGS

#ifdef TEST_CTHREAD

  thread_pool_t *pool = thread_pool_create(16); 
  thread_pool_resize(pool, 32);

  int a = 42;
  thread_t t = {
    .fn = (thread_fn_t)thread_fn,
    .arg = &a,
    .pool = pool
  };
  if (thread_start(&t) != 0) {
    exit(1);
  }
  
  if (thread_start_attr(&t, (thread_attr_t){
    .stacksize = 8 * 1024 * 1024,
    .guardsize = 4 * 1024
  }) != 0) {

  }
  
#endif // TEST_CTHREAD
       
  sleep(1);

  return 1;
}
