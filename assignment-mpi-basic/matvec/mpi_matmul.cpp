#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
using namespace std;


long which_row,which_column,submatrixsize;
int row_root,col_root;
float genA (int row, int col) {
  if (row > col)
    return 1.;
  else
    return 0.;
}

float genx0 (int i) {
  return 1.;
}


void checkx (int iter, long i, float xval) {
  if (iter == 1) {
    float shouldbe = i;
    if (fabs(xval/shouldbe) > 1.01 || fabs(xval/shouldbe) < .99 )
      std::cout<<"incorrect : x["<<i<<"] at iteration "<<iter<<" should be "<<shouldbe<<" not "<<xval<<std::endl;
  }

  if (iter == 2) {
    float shouldbe =(i-1)*i/2;
    if (fabs(xval/shouldbe) > 1.01 || fabs(xval/shouldbe) < .99)
      std::cout<<"incorrect : x["<<i<<"] at iteration "<<iter<<" should be "<<shouldbe<<" not "<<xval<<std::endl;
  }
}

//perform dense y=Ax on an n \times n matrix
void matmul(float*A, float*x, float*y, long n, MPI_Comm row_comm) {
  for (long row = 0; row<n; ++row) {
    float sum = 0;
    
    for (long col = 0; col<n; ++col) {
      //sum += x[col] *A[row][col]
      sum += x[col] * A[row*n+col];
    }

    y[row] = sum;
  }
  //MPI_Reduce(y,reducedy,submatrixsize,MPI_FLOAT,MPI_SUM,row_root,row_comm);
  MPI_Reduce(y,x,submatrixsize,MPI_FLOAT,MPI_SUM,row_root,row_comm);
  /*if(which_column==which_row){
    
    for(int i=0;i<submatrixsize;i++){
          //cout<<reducedy[i]<<endl;
          y[i]=reducedy[i];
        }
      
      //y=reducedy;
  }*/
}

int main (int argc, char*argv[]) {

  if (argc < 3) {
    std::cout<<"usage: "<<argv[0]<<" <n> <iteration>"<<std::endl;
  }
  MPI_Init(&argc,&argv);
  int rank,nbprocess;

  bool check = true;
  
  long n = atol(argv[1]);

  long iter = atol(argv[2]);

  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nbprocess);


  MPI_Comm row_comm,col_comm;

  
  long partitionsize=sqrt(nbprocess);//  square root of nbprocesses
  submatrixsize=n/partitionsize; //calculating the size of sub-square matrix
  which_row=rank/partitionsize;
  which_column=rank%partitionsize;
  long submatrix_row_begin = which_row * submatrixsize;
  long submatrix_row_end = submatrix_row_begin + submatrixsize;
  long submatrix_column_begin = which_column * submatrixsize;
  long submatrix_column_end = submatrix_column_begin + submatrixsize;

  MPI_Comm_split(MPI_COMM_WORLD,which_row,rank,&row_comm);
  MPI_Comm_split(MPI_COMM_WORLD,which_column,rank,&col_comm);
  row_root=which_row;
  col_root=which_column;
  

  //initialize data
  float* localA = new float[submatrixsize*submatrixsize];

  for (long row = submatrix_row_begin; row<submatrix_row_end; row++) {
    for (long col=submatrix_column_begin; col<submatrix_column_end; col++) {
      localA[(row-submatrix_row_begin)*submatrixsize+(col-submatrix_column_begin)] = genA(row, col);
    }
  }

  // for (long row = 0; row<n; row++) {
  //   for (long col=0; col<n; col++) {
  //     std::cout<<A[row*n+col]<<" ";
  //   }
  //   std::cout<<std::endl;
  // }

  float* localx = new float[submatrixsize];

  for (long i=submatrix_row_begin; i<submatrix_row_end; ++i)
    localx[i-submatrix_row_begin] = genx0(i);

  // for (long i=0; i<n; ++i)
  //   std::cout<<x[i]<<" ";
  // std::cout<<std::endl;
  
  float* localy = new float[submatrixsize];
  //float* reducedlocaly = new float[submatrixsize];

  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  
  for (int it = 0; it<iter; ++it) {
  
    matmul(localA, localx, localy, submatrixsize, row_comm);

   /* {
      float*t = localx;
      localx=localy;
      localy=t;
    }*/

    // std::cout<<"x["<<it+1<<"]: ";
    // for (long i=0; i<n; ++i)
    //   std::cout<<x[i]<<" ";
    // std::cout<<std::endl;
    if(which_row==which_column){
    if (check)
      for (long i = submatrix_row_begin; i<submatrix_row_end; ++i)
	     checkx (it+1, i, localx[i-submatrix_row_begin]);
   }
   

    // if(which_row==which_column)
        MPI_Bcast(localx,submatrixsize,MPI_FLOAT,col_root,col_comm);
  }

  MPI_Finalize();
  
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();

  std::chrono::duration<double> elapsed_seconds = end-start;

  std::cerr<<elapsed_seconds.count()<<std::endl;
  
  
  delete[] localA;
  delete[] localx;
  delete[] localy;
  //delete[] reducedlocaly;
  
  return 0;
}
