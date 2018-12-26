#include <mpi.h>
#include <unistd.h>
#include <iostream>

int main(int argc, char*argv[]) {
	int rank,size;
	char hostname[1024];
	gethostname(hostname, 1024);
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	printf("I am %d of %d running on %s\n",rank,size,hostname);

	MPI_Finalize();
	return 0;

}
