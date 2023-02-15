#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <mpi.h>

#include "automaton.h"
#include "arralloc.h"

/*
 * Simple serial program to simulate a 2D cellular automaton
 */


//int maxstep;

int main(int argc, char *argv[])
{


  initialize_mpi();


  /*
   * Simulation parameters and parameter parsing
   */


  int seed;
  double rho = 0.49;
  int L = 768;
  int NX=0;
  int NY=0;

  int maxstep = 10*L;
  int printfreq = 500; // defaults in the header
  int test_mode = 0;

  int opt;

  while((opt = getopt(argc, argv, ":l:r:s:f:x:y:t:")) != -1) {
    switch(opt)
    {
      case 'l':
        L = atoi(optarg);
        maxstep = 10*L;
        break;
      case 'r':
        rho = atof(optarg);
        break;
      case 's':
        maxstep = atoi(optarg);
        break;
      case 'f':
        printfreq = atoi(optarg);
        break;
      case 'x':
        NX = atoi(optarg);
        break;
      case 'y':
        NY = atoi(optarg);
        break;
      case 't': // test mode, print in a tabular form
        test_mode = atoi(optarg);
        break;
      case ':':
        if(main_process()) printf("option needs a value: %c\n",optopt);
        break;
      case '?':
        if(main_process()) printf("Flag not recognized: %c\n", optopt);
        break;
      }
  }

  if(optind>=argc) {
      if(main_process()) printf("Usage: automaton <seed>\n");
      finalize_mpi();
      return 1;
  }

  seed = atoi(argv[optind]); // Set the random number seed and initialise the generator


  for(int index=optind+1; index < argc; index++){
    if(main_process()) printf("Received arguments without flag after first argument (seed must be first argument)\n");
  }

  if(NX!=0 && get_nprocs()%NX!=0) {
    if(main_process()) printf("Invalid value for NX given total number of processes. Value will be ignored.\n");
    NX = 0;
  }
  if(NY!=0 && get_nprocs()%NY!=0) {
    if(main_process()) printf("Invalid value for NY given total number of processes. Value will be ignored.\n");
    NY = 0;
  }
  if(NX!=0 && NY!=0 && (NX*NY)!=get_nprocs()) {
    if(main_process()) printf("The provided values of NX and NY are inconsistent with the number of processes. Values will be ignored.\n");
    NX = 0;
    NY = 0;
  }


  /*
   * Initial setup of auxiliary processes
   */

  set_io(L);
//  prepare_tests(maxstep,printfreq);
  prepare_tests(test_mode,L,maxstep,printfreq);

  register_simulation_parameters(L,rho,seed,maxstep);

  if(main_process()) rinit(seed); // Initialize RNG



  /*
   *  Create the domain and subdomains for the simulation
   */


  int LX,LY,base_LX,base_LY; // lengths of the local cell arrays for each process. The base lengths correspond to non-edge processes (see README)

  int dims[2] = {NX,NY};
  int lengths[2];
  int base_lengths[2];

  init_topology(dims,lengths,L,base_lengths); // Creates the cartesian topology that subdivides the domain amongst processes

  NX = dims[0];
  NY = dims[1];
  LX = lengths[0];
  LY = lengths[1];
  base_LX = base_lengths[0];
  base_LY = base_lengths[1];

  register_topology(NX,NY);


  int **cell, **neigh;
  int **allcell;

  cell  = (int **) arralloc(sizeof(int), 2, LX+2, LY+2);
  neigh = (int **) arralloc(sizeof(int), 2, LX+2, LY+2);

  allcell = (int **) arralloc(sizeof(int), 2, L, L);

  if (NULL == cell || NULL == neigh || NULL == allcell) {
    printf("automaton: array allocation failed\n");
    return 1;
  }



  /*
   *  Initialise full domain with the fraction of filled cells equal to rho
   */

  int ncell;

  initialize_fullDomain(allcell, rho, &ncell, L);

  register_initial_state(rho,ncell,L);



  /*
   * Initialise the cell array: copy the LxL array allcell to the
   * centre of cell, and set the halo values to zero.
   */


  int coords[2];
  get_local_coordinates(&coords[0]);
  int coordX = coords[0];
  int coordY = coords[1];


  initialize_subdomain(cell,allcell,coordX,coordY,LX,LY,base_LX,base_LY);

  register_nprocs(); // registers the number of processors to file for ease of anlaysis



  /*
   * Execute the simulation
   */


  int step, changes;

  int in_range = 1; // termination condition. 1 if in range 2/3 to 3/2. 0 if not
  double domain_size = (double)(L*L);
  double initial_ncells = rho* domain_size;
  double low_lim = initial_ncells*(2.0/3.0);
  double high_lim = initial_ncells*(3.0/2.0);


  start_timer();

  for (step = 1; step <= maxstep && in_range; step++)
  {


    halo_swap_direct(cell,LX,LY);
    halo_swap_transverse(cell,LX,LY);

    verify_survival(cell,neigh, LX,LY); // checks the condition for whether each cell should live or die

    ncell = 0;
    changes = 0;

    update_survivors(cell,neigh,&ncell,&changes,LX,LY); // updates cell array

    register_stats(&ncell,&changes,step); // Registers the number of living cells to file for correctness testing


    if((double)ncell<low_lim || (double)ncell>high_lim) { // termination conditions

      in_range=0;
    }
//    in_range = !( (double)ncell<low_lim || (double)ncell>high_lim ); // changes flag when ncell falls outside of 2/3 to 3/2 range

  }

  register_elapsed_time();



  reset_fullDomain(allcell, L);

  insert_into_fullDomain(cell,allcell,coordX,coordY, LX,LY, base_LX, base_LY); // Copy the centre of cell, excluding the halos, into allcell

  reduce_matrix(allcell,L);



  report_stats(step);

  report_steps(step);

  report_elapsed_time();


  /*
   *  Write the cells to the file "cell.ppm", displaying the two
   */

  if(main_process()) {
    cellwritedynamic("cell.pbm", allcell,L);
  }

  finalize_mpi();
  return 0;

}
