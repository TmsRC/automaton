#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "automaton.h"
#include "arralloc.h"

double start_time;
double end_time;

int num_living;
int num_changes;
int *living_register;
int *changes_register;

int L;
int printfreq;
int maxstep;

int test_mode; // flag for test mode. 1 for correctness, 2 for performance



// I NEED TO DO SOMETHING ABOUT THIS:


void prepare_tests(int test_mode_input, int L_input, int maxstep_input, int printfreq_input) {

  test_mode = test_mode_input;
  L = L_input;

  maxstep = maxstep_input; // check for invalid maxstep
  printfreq = 500;

  if(printfreq_input > maxstep) { printfreq = 500;
    if(maxstep > 1500) {
      if(main_process() && !test_mode) printf("Invalid printfreq to maxstep ratio. Setting default printfreq = 500\n");
    }
    else {
      printfreq = maxstep;//10;
      if(main_process() && !test_mode) printf("Invalid printfreq to maxstep ratio. Setting default 10 total prints\n");
    }
  }
  else {

    printfreq = printfreq_input;
  }

   int size = maxstep;
   living_register  = (int *) arralloc(sizeof(int), 1, size);
   changes_register = (int *) arralloc(sizeof(int), 1, size);

}

void start_timer() {
  start_time = get_time();
}

double register_elapsed_time() {
  end_time = get_time();
  return end_time - start_time;
}

void report_elapsed_time() {

  if(main_process() && !test_mode) printf("Elapsed time: %.8f seconds\n",end_time-start_time);
}

void register_nprocs() {
  if(main_process() && !test_mode) printf("Running on %d processes\n",get_nprocs());
}

void register_simulation_parameters(int L, double rho, int seed,int maxstep) {
  if(main_process() && !test_mode) printf("automaton: L = %d, rho = %f, seed = %d, maxstep = %d\n", L, rho, seed, maxstep);
}

void register_initial_state(double rho,int ncell,int L) {
  if(main_process() && !test_mode) printf("automaton: rho = %f, live cells = %d, actual density = %f\n", rho, ncell, ((double) ncell)/((double) L*L) );
}


void register_topology(int NX, int NY) {
  if(main_process() && !test_mode) printf("Created topology: (%d,%d)\n",NX,NY);
}


void register_stats(int *ncell, int *changes,int step) {

  num_living = 0;
  num_changes = 0;

  reduce_number(ncell,&num_living);
  reduce_number(changes,&num_changes);

  *ncell = num_living;
  *changes = num_changes;

  living_register[step-1] = num_living;
  changes_register[step-1] = num_changes;

//  if (!test_mode && step % printfreq == 0 && main_process()) { // It should be better to append to list and print later. Perhaps change branch and use longer list?
//      printf("automaton --- Step %d; # of changes: %d; living cells: %d\n", step, num_changes, num_living);
//  }

}

void report_steps(int step) {

  if(main_process() && !test_mode) printf("Number of steps: %d\n",step-1);
}

void report_stats(int steps) {

  double total_time = end_time - start_time;
  int step = printfreq;
  int counter = 0;

  if(main_process() && test_mode==1) printf("%s,%s,%s,%s,%s\n","NPROC","L","step","num_changes","num_living");

  while(main_process() && step<=steps) {

    num_changes = changes_register[step-1];
    num_living = living_register[step-1];
    if(!test_mode) printf("automaton --- Step %d; # of changes: %d; living cells: %d\n", step, num_changes, num_living);
    if(test_mode==1) printf("%d,%d,%d,%d,%d\n",get_nprocs(),L,step,num_changes,num_living);
    step += printfreq;
    counter ++;
  }

  if(main_process() && test_mode==2) printf("%d,%d,%.6f\n",get_nprocs(),L,total_time/(steps-1)*1000); // time per step in milliseconds

}
