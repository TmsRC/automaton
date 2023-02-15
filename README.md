# 2D decomposed automaton

This repository contains the files for the 2D decomposed automaton simulation for the MPP course 22-23.

The automaton in question is a simple 2D grid where cells can be either ``dead" or ``alive"; the simulation progresses by changing the state of each cell based on the state of their neighbors at each iteration. If the number of living neighbors in the vicinity of a cell (including the cell itself) is 2, 4 or 5, the cell will be alive in the next iteration, otherwise, it will be dead. This automaton is implemented on a square grid with size $L$, and its initial state is assigned randomly given a starting "density" $\rho$ of living cells. The 2D parallel decomposition divides the grid along both directions and assigns one chunk to every process assigned to the program.

Sample graphical output for the simulation is found in `cell.pbm` (not rendered on GitHub). Readable output is found in `interactive_results_sample`. The `errors/` and `results/` directories contain the corresponding files obtained from executing in test mode on the Cirrus supercomputer. using the scripts provided. 

All files are already compiled, but they can be recompiled again using the command `make` with the given Makefile. No additional steps need to be taken.

To run the program in serial, it is only necessary to call `./src/automaton <seed>` with the desired seed (the results in the repository correspond to a seed of 1234). To run in parallel, one should use `mpirun -n <P> ./automaton <seed>` with the desired number of processes `P`. In both cases, additional flags can be added to change parameters or alter the behavior of the program. This flags must be added after the seed.

The flags are:

- `-l`: allows the user to change the length of the domain L. The value (integer) should be typed immediatlely after the flag. *Default: 768*
- `-r`: allows the user to change the initial living cell density. The value (real) should be typed immediatlely after the flag. *Default: 0.49*
- `-x`: allows the user to *propose* a value of NX for the parallel decomposition (how many processes along x). The value (integer) should be typed immediatlely after the flag. *Default behavior is optimal value find by program*
- `-y`: allow the user to *propose* a value of NY for the parallel decomposition. The value (integer) should be typed immediatlely after the flag. *Default behavior is optimal value find by program*
- `-s`: allows the user to set the maximum number of steps for the simulation. The value (integer) should be typed immediatlely after the flag. *Default: 10L*
- `-f`: allows the user to set the print frequency for live stats of the program. The value (integer) should be typed immediatlely after the flag. Print frequency can be overriden if the value is not compatible with the actual number of steps. *Default: 500*
- `-t`: allows the user to run the program in `test_mode`. The value {0,1,2} should be added after the flag. 0 means not running in test mode, the code prints in human readable form. 1 means testing for correctness (outputs stats for correctness testing in tabular form). 2 means testing for performance, the only output is number of processes, domain length and execution time per step. *Default: 0*

The program is distributed among the following files:

- `automaton.c`: main file
- `functions.c`: arithmetic and logical functions used in the main file.
- `mpi_functionalities.c`: file containing all mpi message-passing functions.
- `tests.c`: file containing functions to print for user reading and for tests.
- `arralloc.c`: file containing the functions to generate dynamic arrays.
- `cellio.c`: file containing functionality to print final result to .pbm file.
- `unirand.c`: file for generating random numbers

All these files, including header files for `automaton` and `arralloc`, are contained in the main directory of the repository.

Execution on Cirrus compute nodes can be performed using the provided slurm job files (main directory).
