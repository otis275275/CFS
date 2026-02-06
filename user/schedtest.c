#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void spin(int iterations) {
  volatile int j = 0;
  for(int i = 0; i < iterations; i++) {
    j += i;
  }
}

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: schedtest [0|1] (0=RR, 1=CFS)\n");
    exit(1);
  }

  int policy = atoi(argv[1]);
  
  enable_sched_output(0);
  set_priority(0, -20);
  
  if (set_sched_policy(policy) < 0) {
      fprintf(2, "Error setting policy\n");
      exit(1);
  }
  
  printf("\n");
  printf("+----------------------------------------------------------+\n");
  if(policy == 0) {
    printf("|            SCHEDULER TEST: ROUND ROBIN                   |\n");
  } else {
    printf("|            SCHEDULER TEST: CFS                           |\n");
  }
  printf("+----------------------------------------------------------+\n\n");
  
  if(policy == 0) {
    printf("  Round Robin: Each process gets EQUAL time slices.\n");
    printf("  Expected: All PIDs appear equally (~33%% each).\n\n");
  } else {
    printf("  CFS: Lower vruntime = Higher priority = More CPU time.\n\n");
    printf("  +----------+----------+----------------+---------------+\n");
    printf("  |   Nice   |  Delta   |   vruntime     |   CPU Time    |\n");
    printf("  +----------+----------+----------------+---------------+\n");
    printf("  |   -10    |    5     |  Slow increase |   ~50%% MORE   |\n");
    printf("  |    0     |   10     |  Normal        |   ~33%%        |\n");
    printf("  |   +10    |   20     |  Fast increase |   ~17%% LESS   |\n");
    printf("  +----------+----------+----------------+---------------+\n\n");
  }

  int go[2];
  pipe(go);
  
  int pid1, pid2, pid3;
  
  pid1 = fork();
  if (pid1 == 0) {
      close(go[1]);
      set_priority(0, 0);
      char buf; read(go[0], &buf, 1);
      close(go[0]);
      spin(80000000);
      exit(0);
  }
  
  pid2 = fork();
  if (pid2 == 0) {
      close(go[1]);
      set_priority(0, -10);
      char buf; read(go[0], &buf, 1);
      close(go[0]);
      spin(80000000);
      exit(0);
  }

  pid3 = fork();
  if (pid3 == 0) {
      close(go[1]);
      set_priority(0, 10);
      char buf; read(go[0], &buf, 1);
      close(go[0]);
      spin(80000000);
      exit(0);
  }

  close(go[0]);
  sleep(2);

  printf("  Process Mapping:\n");
  printf("  +-------+--------+------------------+\n");
  printf("  |  PID  |  Nice  |     Priority     |\n");
  printf("  +-------+--------+------------------+\n");
  printf("  |   %d   |    0   |  NORMAL          |\n", pid1);
  printf("  |   %d   |  -10   |  HIGH (runs MORE)|\n", pid2);
  printf("  |   %d   |  +10   |  LOW (runs LESS) |\n", pid3);
  printf("  +-------+--------+------------------+\n\n");
  
  printf("  Scheduling Events:\n");
  printf("  ------------------------------------------------------------\n");
  
  enable_sched_output(1);
  write(go[1], "GGG", 3);
  close(go[1]);

  for(int i = 0; i < 3; i++) wait(0);
  
  enable_sched_output(0);

  printf("  ------------------------------------------------------------\n\n");
  printf("+----------------------------------------------------------+\n");
  printf("|                     TEST FINISHED                        |\n");
  printf("+----------------------------------------------------------+\n");
  exit(0);
}
