#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#include "automaton.h"


void initialize_fullDomain(int** allcell, double rho, int *ncell, int L) {

  double r;

  *ncell = 0;
  if(main_process() ) {
    for (int i=0; i < L; i++)
    {
       for (int j=0; j < L; j++)
       {
          r=uni();

          if(r < rho)
          {
             allcell[i][j] = 1;
             (*ncell)++; // do not remove parentheses
          }
          else
          {
             allcell[i][j] = 0;
          }
       }
    }
  }

  broadcast_domain(allcell, L);

}


void initialize_subdomain(int** cell, int** allcell, int coordX, int coordY, int LX, int LY, int base_LX, int base_LY) {

  int i,j;

  for (i=1; i <= LX; i++) {

    int my_i = coordX*base_LX + i;

    for (j=1; j <= LY; j++) {

      int my_j = coordY*base_LY + j;
      cell[i][j] = allcell[my_i-1][my_j-1];
    }

  }


  for (i=0; i <= LX+1; i++) {
    // zero the bottom and top halos
    cell[i][0]    = 0;
    cell[i][LY+1] = 0;
  }

  for (j=0; j <= LY+1; j++) {
    // zero the left and right halos
    cell[0][j]    = 0;
    cell[LX+1][j] = 0;
  }

}

void insert_into_fullDomain(int** cell, int** allcell, int coordX, int coordY, int LX, int LY, int base_LX, int base_LY) {

  for (int i=1; i<=LX; i++) {

      int my_i = coordX*base_LX + i;

      for (int j=1; j<=LY; j++) {
          int my_j = coordY*base_LY + j;
          allcell[my_i-1][my_j-1] = cell[i][j];
      }
  }
}


void verify_survival(int** cell, int** neigh, int LX, int LY) {

  for (int i=1; i<=LX; i++) {
    for (int j=1; j<=LY; j++) {

       // Set neigh[i][j] to be the sum of cell[i][j] plus its four nearest neighbors
       neigh[i][j] =   cell[i][j]
                     + cell[i][j-1]
                     + cell[i][j+1]
                     + cell[i-1][j]
                     + cell[i+1][j];
    }
  }

}


void update_survivors(int** cell, int** neigh, int *ncell, int *changes, int LX, int LY) {

   int old;
//   int survives;

   for (int i=1; i<=LX; i++) {
     for (int j=1; j<=LY; j++) {


        // Update based on number of neighbours

//        survives = neigh[i][j] == 2 || neigh[i][j] == 4 || neigh[i][j] == 5;
//
        old = cell[i][j];
//
//        cell[i][j] = survives; // automaton condition with no branching
//        (*ncell) += survives;
//
//        (*changes) += (old!=survives); // adds 1 if the old value is different from the new one, 0 if not

        if (neigh[i][j] == 2 || neigh[i][j] == 4 || neigh[i][j] == 5) {
          cell[i][j] = 1;
          (*ncell)++; // do not remove parentheses
        }
        else {
          cell[i][j] = 0;
        }

        if(old!=cell[i][j]) {
          (*changes) ++;
        }

     }
   }

}


void reset_fullDomain(int** allcell, int L) {

  for (int i=0; i < L; i++) {
    for (int j=0; j < L; j++) {
      allcell[i][j] = 0;
    }
  }
}
