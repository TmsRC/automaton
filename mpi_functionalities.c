#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "automaton.h"


MPI_Comm world;
MPI_Comm cartesian_top;

MPI_Datatype CELL_VECTOR; // vector for sending subdomains into the main domain
MPI_Datatype TRANSVERSE_HALO; // vector halos transversal to the direction of memory contiguity

int size,rank;



/*
 * Initialization and finalization of MPI
 */

void initialize_mpi( ) {

  MPI_Init(NULL, NULL);
  world = MPI_COMM_WORLD;

  MPI_Comm_size(MPI_COMM_WORLD,&size);
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

//  prepare_tests(size,rank);

}


void finalize_mpi() {

  MPI_Finalize();
}



/*
 * Functions for requesting MPI information
 */

int main_process() {

  return rank==0;
}

int get_rank() {
  // used for debugging exclusively
  return rank;
}

double get_time() {

  MPI_Barrier(world);
  return MPI_Wtime();
}

int get_nprocs() {

  return size;
}


void get_local_coordinates(int *coords) {

  get_proc_coordinates(coords,rank);
}


void get_proc_coordinates(int *coords, int r) {

  MPI_Cart_coords(cartesian_top,r,2,coords);
}

void calculate_neighbors(int axis, int *backward_neigh, int *forward_neigh) {

  MPI_Cart_shift(cartesian_top,axis,1,backward_neigh,forward_neigh);
}



/*
 * Functions for domain initialization
 */

void define_halo_vector(int LX, int LY) {

  MPI_Type_vector(LX,1,LY+2,MPI_INT,&TRANSVERSE_HALO);
}

void create_topology(int *dims, int *boundary_conditions) {

  MPI_Dims_create(size,2,dims);
  MPI_Cart_create(MPI_COMM_WORLD,2,dims,boundary_conditions,0,&cartesian_top);
}


void init_topology(int *dims, int *lengths, int L, int *base_lengths) {

  int boundary_conditions[2];
  boundary_conditions[0] = 0;
  boundary_conditions[1] = 1;


  create_topology(dims,boundary_conditions);
  int NX = dims[0];
  int NY = dims[1];


  int coords[2];
  get_local_coordinates(&coords[0]);
  int coordX = coords[0];
  int coordY = coords[1];

  int LX = L/NX;
  int LY = L/NY;

  base_lengths[0] = LX;
  base_lengths[1] = LY;


  LX = L/NX + (coordX==(NX-1))*(L%NX); // if it's an edge process, adds the remainder cells from the uneven division
  LY = L/NY + (coordY==(NY-1))*(L%NY);

  lengths[0] = LX;
  lengths[1] = LY;
  define_halo_vector(LX,LY);

}



/*
 * Simple MPI communications
 */

void broadcast_domain(int** allcell, int L) {

  MPI_Bcast(&allcell[0][0],L*L,MPI_INT,0,MPI_COMM_WORLD);
}


void reduce_ncells(int *ncell, int *num_living) {

  MPI_Allreduce(ncell,num_living,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
}


void reduce_number(int *nlocal, int *ntotal) {

  MPI_Allreduce(nlocal,ntotal,1,MPI_INT,MPI_SUM,MPI_COMM_WORLD);
}


void reduce_matrix(int** allcell, int L) {
  MPI_Reduce(&allcell[0][0],&allcell[0][0],L*L,MPI_INT,MPI_SUM,0,world);
}


/*
 * Complex MPI communications
 */

void halo_swap_direct(int** cell, int LX, int LY) {
      // swaps halos in the direction along memory contiguity

      int forward_neigh, backward_neigh;
      calculate_neighbors(0,&backward_neigh,&forward_neigh);

      MPI_Request request_for;
      MPI_Status status;
      MPI_Issend(&cell[LX][1],LY,MPI_INT,forward_neigh,0,world,&request_for); //swap forward
      MPI_Recv(&cell[0][1],LY,MPI_INT,backward_neigh,0,world,&status); //receive backward
      MPI_Wait(&request_for,&status);

      MPI_Request request_back;
      MPI_Issend(&cell[1][1],LY,MPI_INT,backward_neigh,0,world,&request_back); // swap backward
      MPI_Recv(&cell[LX+1][1],LY,MPI_INT,forward_neigh,0,world,&status); //receive forward
      MPI_Wait(&request_back,&status);

}

void halo_swap_transverse(int** cell, int LX, int LY) {
      // swaps halos in the direction transverse to memory contiguity

      int forward_neigh, backward_neigh;
      calculate_neighbors(1,&backward_neigh,&forward_neigh);

      MPI_Request request_for;
      MPI_Status status;
      MPI_Issend(&cell[1][LY],1,TRANSVERSE_HALO,forward_neigh,0,world,&request_for); //swap forward
      MPI_Recv(&cell[1][0],1,TRANSVERSE_HALO,backward_neigh,0,world,&status); //receive backward
      MPI_Wait(&request_for,&status);

      MPI_Request request_back;
      MPI_Issend(&cell[1][1],1,TRANSVERSE_HALO,backward_neigh,0,world,&request_back); // swap backward
      MPI_Recv(&cell[1][LY+1],1,TRANSVERSE_HALO,forward_neigh,0,world,&status); //receive forward
      MPI_Wait(&request_back,&status);

}


void gather_full_domain(int** allcell, int L, int LX, int LY, int base_LX, int base_LY, int NX, int NY) {

  // Gather operation using vectors. Unutilized.

  MPI_Type_vector(LX,LY,L,MPI_INT,&CELL_VECTOR);

  int coords[2];
  int coordX, coordY;
  get_local_coordinates(&coords[0]);
  coordX = coords[0];
  coordY = coords[1];

  int my_i = coordX*base_LX;
  int my_j = coordY*base_LY;

  MPI_Status status;

  if(rank!=0 && rank<size) MPI_Ssend(&allcell[my_i][my_j],1,CELL_VECTOR,0,0,world);

  if(rank==0) {

    for(int r=1; r<size; r++) {

      get_proc_coordinates(&coords[0],r);
      coordX = coords[0];
      coordY = coords[1];

      my_i = coords[0]*base_LX;
      my_j = coords[1]*base_LY;

      LX = L/NX + (coordX==(NX-1))*(L%NX);
      LY = L/NY + (coordY==(NY-1))*(L%NY);

      MPI_Type_vector(LX,LY,L,MPI_INT,&CELL_VECTOR); // needs to create a new vector every time to account for differing sizes of cell in edge processes

      MPI_Recv(&allcell[my_i][my_j],1,CELL_VECTOR,r,0,world,&status);

    }
  }
}
