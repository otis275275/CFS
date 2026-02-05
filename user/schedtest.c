#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
spin(int n)
{
  int i;
  int j = 0;
  for(i = 0; i < n; i++) {
    j += i;
  }
  // printf("Spin done: %d\n", j);
}

int
main(int argc, char *argv[])
{
  if(argc < 2){
    fprintf(2, "Usage: schedtest [0|1] (0=RR, 1=CFS)\n");
    exit(1);
  }

  int policy = atoi(argv[1]);
  if (set_sched_policy(policy) < 0) {
      fprintf(2, "Error setting policy\n");
      exit(1);
  }

  printf("\n=== STARTING SCHEDULER TEST (%s) ===\n", policy == 0 ? "Round Robin" : "CFS");
  printf("Creating 3 CPU-bound processes to visualize scheduling...\n\n");
  
  // Create 3 children
  // Child 1: Normal (Nice 0)
  if (fork() == 0) {
      set_priority(0, 0); // Self, nice 0
      // printf("Child Normal (PID %d) started (Nice 0)\n", getpid());
      spin(100000000);
      exit(0);
  }
  
  // Child 2: High Priority (Nice -10)
  if (fork() == 0) {
      set_priority(0, -10); // Self, nice -10
      // printf("Child HighPrio (PID %d) started (Nice -10)\n", getpid());
      spin(100000000);
      exit(0);
  }

  // Child 3: Low Priority (Nice 10)
  if (fork() == 0) {
      set_priority(0, 10); // Self, nice 10
      // printf("Child LowPrio (PID %d) started (Nice 10)\n", getpid());
      spin(100000000);
      exit(0);
  }

  // Parent also spins? No, parent just waits to be clean.
  // Wait for all 3
  for(int i = 0; i < 3; i++) {
    wait(0);
  }

  printf("\n=== TEST FINISHED ===\n");
  exit(0);
}
