/*
 *  Main header file for percolation exercise.
 */



/*
 *  Prototypes for supplied functions
 */


//void initialize_fullDomain(int allcell[L][L], double rho, int *ncell);
//void initialize_subdomain(int cell[LX+2][LY+2], int allcell[L][L], int coordX, int coordY);
//void verify_survival(int cell[LX+2][LY+2], int neigh[LX+2][LY+2]);
//void update_survivors(int cell[LX+2][LY+2], int neigh[LX+2][LY+2], int *ncell);
//void insert_into_fullDomain(int cell[LX+2][LY+2], int allcell[L][L], int coordX, int coordY);


void initialize_fullDomain(int** allcell, double rho, int *ncell, int L);
void initialize_subdomain(int** cell, int** allcell, int coordX, int coordY, int LX, int LY, int base_LX, int base_LY);
void verify_survival(int** cell, int** neigh, int LX, int LY);
void update_survivors(int** cell, int** neigh, int *ncell, int *changes, int LX, int LY);
void insert_into_fullDomain(int** cell, int** allcell, int coordX, int coordY, int LX, int LY, int base_LX, int base_LY);
void reset_fullDomain(int** allcell, int L);


/*
 *  Visualisation
 */

//void cellwrite(char *cellfile, int cell[L][L]);
void cellwritedynamic(char *cellfile, int **cell, int l);
void set_io(int L);

/*
 *  Random numbers
 */

void rinit(int ijkl);
float uni(void);


/*
 *  MPI functionalities
 */


void register_bases(int LX, int LY);

void initialize_mpi(void);
void finalize_mpi(void);

double get_time(void);
int main_process(void);
int get_nprocs(void);
int get_rank(void);

void create_topology(int *dims, int *boundary_conditions);
void define_halo_vector(int LX, int LY);

void get_local_coordinates(int *coords);
void get_proc_coordinates(int *coords, int rank);
void calculate_neighbors(int axis, int *backward_neigh, int *forward_neigh);

void reduce_ncells(int *ncell, int *num_living);
void reduce_number(int *nlocal, int *ntotal);

void broadcast_domain(int** cell, int L);
void halo_swap_direct(int** cell, int LX, int LY);
void halo_swap_transverse(int** cell, int LX, int LY);
void gather_full_domain(int** allcell, int L, int LX, int LY,int base_LX, int base_LY, int NX, int NY);


void init_topology(int *ndims, int *lengths, int L, int *base_lengths);
void reduce_matrix(int** allcell, int L);


/*
 *  Tests
 */

//void prepare_tests(int size, int rank);
void prepare_tests(int test_mode, int L, int maxstep_input, int printfreq_input);

void start_timer(void);
double register_elapsed_time(void);

void register_simulation_parameters(int L, double rho, int seed,int maxstep);
void register_initial_state(double rho,int ncell,int L);
void register_topology(int NX, int NY);

void register_nprocs(void);
void register_stats(int *ncell, int *changes,int step);

void report_steps(int step);
void report_elapsed_time(void);
void report_stats(int steps);
