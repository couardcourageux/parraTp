#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a, b)               ((a) < (b) ? (a):(b))
#define BLOCK_LOW(id, p, n)     ((id)*(n) / (p))
#define BLOCK_HIGH(id, p, n)    (BLOCK_LOW((id)+1, p, n) - 1)
#define BLOCK_SIZE(id, p, n)    (BLOCK_HIGH((id), p, n) - BLOCK_LOW(id, p, n))
#define BLOCK_OWNER(idx, p, n)  ((((p)*(idx)+1)-1) / (n))

#define ARE_TWINS(a, b)         ((a) + 2 == (b))

void show(int* tab, int size) {
    printf("\n---------------------------\n");
    printf("[");
    int i;
    for (i = 0; i < size; i++) {
        printf(" %d ", tab[i]);
    }
    printf("]\n--------------------------\n");
}

int main(int argc, char* argv[]) {

    int i;
    int id; /*rang du proc*/
    int p;  /*nb de proc*/
    int n;  /*limit on where to search*/

    int low_value;
    int high_value;
    int size;
    int proc0_size;

    
    int index;
    int prime;

    int first;
    int last;

    int count;
    int global_count = 0;

    int premierIndice = -1;
    int twinCount;

    int last_from_prev_block = -30;
    int first_prime;
    int last_prime;

    MPI_Win window;

    /*
            [...]
    */

    
    // MPI_Comm_size(MPI_COMM_WORLD, &p);

    MPI_Init(&argc, &argv);
    MPI_Barrier(MPI_COMM_WORLD);

    

    double elapsed_time = -MPI_Wtime();
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);


    // /*lets create the window*/
    // MPI_Win_create(&last_prime, sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &window);
    // MPI_Win_fence(0, window);


    if (argc != 2) {
        if (!id) printf("command line: %s <n>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    n = atoi(argv[1]);
    low_value = 2 + BLOCK_LOW(id, p, n-1);
    high_value = 2 + BLOCK_HIGH(id, p, n-1);
    size = BLOCK_SIZE(id, p, n-1);
    proc0_size = (n-1)/p;

    if ((2 + proc0_size) < (int) sqrt((double) n)) {
        if (!id) printf("too many processes\n");
        MPI_Finalize();
        exit(1);
    }

    int* marked =  malloc(sizeof(int) * size);
    if (marked == NULL) {
        if (!id) printf("cannot allocate\n");
        MPI_Finalize();
        exit(1);
    }

    for (i = 0; i < size; i++) marked[i] = 0;
    // if (!id) index = 0;
    index = 0;
    prime = 2;

    while (prime * prime <= n) {
        if (prime * prime > low_value) first = prime * prime - low_value;
        else {
            if (!(low_value % prime)) first = 0;
            else first = prime - (low_value % prime);
        }

        for (i = first; i < size; i += prime) {marked[i] = 1;}


        if (!id) {
            while (marked[++index]);
            prime = index + 2;
        }

        MPI_Bcast(&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
    } /*old version with a do before, maybe this is trigerring a dephase in the code*/

    printf("id:%d low_val: %d, high_val: %d\n", id, low_value, high_value);
    count = 0;
    for (i = 0; i < size; i++) {
        if (!marked[i]){
             count++;            
        }
    }

    if (count) {
        twinCount = 0;
        int* premiers = malloc(sizeof(int) * count);
        
        premierIndice = 0;
        for (i = 0; i < size; i++) {
            if (!marked[i]){
                premiers[premierIndice] = low_value + i;
                premierIndice++;            
            }
        }

        // show(premiers, count);
        first_prime = premiers[0];
        last_prime = premiers[count - 1];


        // MPI_Win_fence(0, window);
        /*lets create the window*/
        MPI_Win_create(&last_prime, sizeof(int), sizeof(int), MPI_INFO_NULL, MPI_COMM_WORLD, &window);
        MPI_Win_fence(0, window);

        if (id) {
            MPI_Get(&last_from_prev_block, 1, MPI_INT, id - 1, 0, 1, MPI_INT, window);
        }

        MPI_Win_fence(0, window);


        MPI_Win_free(&window);


        if (ARE_TWINS(last_from_prev_block, first_prime)) {
            twinCount++;
        }
        for (i = 0; i < count - 1; i++) {
            if (ARE_TWINS(premiers[i], premiers[i+1])) {
                twinCount++;
            }
        }
    }
    
    MPI_Reduce(&twinCount, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    elapsed_time += PMPI_Wtime();
    if (!id) {
        printf("%d are less than equal or equal to %d\n", global_count, n);
        printf("Total elapsed time: %10.6f\n", elapsed_time);
    }
    fflush(stdout);

    // MPI_Win_free(&window);
    MPI_Finalize();

    return 0;
}