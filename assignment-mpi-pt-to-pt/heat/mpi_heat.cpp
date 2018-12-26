#include <mpi.h>
#include <math.h>
#include <iostream>
#include <chrono>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

  int check2DHeat(double** H, long n, long rank, long P, long k); //this assumes array of array and grid block decomposition
  void calculate_heat(double**, double**, double*,double*,double*,double*,int);

#ifdef __cplusplus
}
#endif

/***********************************************
 *         NOTES on check2DHeat.
 ***********************************************
 *         
 *  First of, I apologize its wonky. 
 *
 *  Email me ktibbett@uncc.edu with any issues/concerns with this. Dr. Saule or the other
 *    TA's are not familiar with how it works. 
 *
 * Params:
 *  n - is the same N from the command line, NOT the process's part of N
 *  P - the total amount of processes ie what MPI_Comm_size gives you.
 *  k - assumes n/2 > k-1 , otherwise may return false negatives.
 *
 *   
 * Disclaimer:
 ***
 *** Broken for P is 9. Gives false negatives, for me it was always
 ***  ranks 0, 3, 6. I have not found issues with 1, 4, or 16, and these
 ***  are what `make test` will use.
 ***
 *
 * Usage:
 *  When code is WRONG returns TRUE. Short example below
 *  if (check2DHeat(...)) {
 *    // oh no it is (maybe) wrong  
 *    std::cout<<"rank: "<<rank<<" is incorrect"<<std::endl;
 *  }
 *
 *
 *
 *  I suggest commenting this out when running the bench
 *
 *
 * - Kyle
 *
 *************/

long which_row,which_column,submatrixsize,submatrix_row_begin,submatrix_row_end,submatrix_column_begin,submatrix_column_end;

// Use similarily as the genA, genx from matmult assignment.
double genH0(long row, long col, long n) {
  double val = (double)(col == (n/2));
  return val;
}

void calculate_heat(double**current, double**next, double*up,double*down,double*left,double*right, int rank){

    for(long i=0;i<submatrixsize;i++){
      for(long j=0;j<submatrixsize;j++){
        if(i==0 && j==0){
          next[i][j]=(1/static_cast<double>(5))*(current[i+1][j]+current[i][j+1]+current[i][j]+left[i]+up[j]);
        }
        else if(i==0 && j==submatrixsize-1){
          next[i][j]=(1/static_cast<double>(5))*(current[i][j-1]+current[i][j+1]+current[i][j]+right[i]+up[j]);
        }
        else if(i==submatrixsize-1 && j==0){
          next[i][j]=(1/static_cast<double>(5))*(current[i][j+1]+current[i-1][j]+current[i][j]+left[i]+down[j]);
        }
        else if(i==submatrixsize-1 && j==submatrixsize-1){
          next[i][j]=(1/static_cast<double>(5))*(current[i][j-1]+current[i-1][j]+current[i][j]+right[i]+down[j]);
        }
        else if(i==0){
          next[i][j]=(1/static_cast<double>(5))*(current[i][j-1]+current[i+1][j]+current[i][j+1]+up[j]+current[i][j]);
        }
        else if(j==0){
          next[i][j]=(1/static_cast<double>(5))*(current[i][j+1]+current[i+1][j]+current[i-1][j]+left[i]+current[i][j]);
          //cout<<next[i][j]<<endl;
        }
        else if(i==submatrixsize-1){
          next[i][j]=(1/static_cast<double>(5))*(current[i][j-1]+current[i-1][j]+current[i][j+1]+down[j]+current[i][j]);
        }
        else if(j==submatrixsize-1){
          next[i][j]=(1/static_cast<double>(5))*(current[i][j-1]+current[i+1][j]+current[i-1][j]+right[i]+current[i][j]);
        }
        else{
          next[i][j]=(1/static_cast<double>(5))*(current[i-1][j]+current[i][j-1]+current[i][j]+current[i+1][j]+current[i][j+1]);
        }
      }
    }
}

int main(int argc, char* argv[]) {

  if (argc < 3) {
    std::cerr<<"usage: mpirun "<<argv[0]<<" <N> <K>"<<std::endl;
    return -1;
  }
  MPI_Init(&argc,&argv);
  int rank,nbprocess;

  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nbprocess);

  // declare and init command line params
  long N, K;
  N = atol(argv[1]);
  K = atol(argv[2]);
  long partitionsize=sqrt(nbprocess);//  square root of nbprocesses
  submatrixsize=N/partitionsize; //calculating the size of sub-square matrix
  which_row=rank/partitionsize;
  which_column=rank%partitionsize;
  submatrix_row_begin = which_row * submatrixsize;
  submatrix_row_end = submatrix_row_begin + submatrixsize;
  submatrix_column_begin = which_column * submatrixsize;
  submatrix_column_end = submatrix_column_begin + submatrixsize;

  string pos=(which_row==0 || which_column==0 || which_row==partitionsize-1 || which_column==partitionsize-1)?
  ((which_row==0 || which_row==partitionsize-1)?((which_row==0)?((which_column==0 || which_column==partitionsize-1)?
    ((which_column==0)?("topleft"):("topright")):("topedge")):((which_column==0 || which_column==partitionsize-1)?
    ((which_column==0)?("buttomleft"):("buttomright")):("buttomedge"))):((which_column==0)?("leftedge"):("rightedge"))):("core");

  double* up = new double[submatrixsize];
  double* down = new double[submatrixsize];
  double* left = new double[submatrixsize];
  double* right = new double[submatrixsize];
  double* sleft = new double[submatrixsize];
  double* sright = new double[submatrixsize];

  double** current=new double*[submatrixsize];
  double** next=new double*[submatrixsize];

  for(long i=0;i<submatrixsize;i++){
    current[i]=(double*)malloc(submatrixsize * sizeof(double));
    next[i]=(double*)malloc(submatrixsize * sizeof(double));
  }

  for (long i=submatrix_row_begin;i<submatrix_row_end;i++){
    for(long j=submatrix_column_begin;j<submatrix_column_end;j++){
      current[i-submatrix_row_begin][j-submatrix_column_begin]= genH0(i, j, N);
    }
  }
  MPI_Request* requests;
  MPI_Status* statuses;

  if(pos=="topleft" || pos=="topright" || pos == "buttomright" || pos=="buttomleft"){
    requests = new MPI_Request[4];
    statuses = new MPI_Status[4];
  }
  else if(pos=="topedge"|| pos=="buttomedge"|| pos=="leftedge" || pos=="rightedge"){
    requests = new MPI_Request[6];
    statuses = new MPI_Status[6];
  }
  else{
    requests = new MPI_Request[8];
    statuses = new MPI_Status[8];
  }
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  int count=0;
  if(nbprocess==1){
    for(int x=0;x<K;x++){
      for(long i = 0;i<submatrixsize;i++){
      left[i]=current[i][0];
      right[i]=current[i][submatrixsize-1];
      up[i]=current[0][i];
      down[i]=current[submatrixsize-1][i];
      }

      calculate_heat(current,next,up,down,left,right,rank);

      {
      double**t = current;
      current=next;
      next=t;
      }
      /*if(check2DHeat(next, N, rank, nbprocess, x)){
        cout<<"Incorrect for rank "<<rank<<endl;
      }*/
    }

  }
  else{
  for(int x=0;x<K;x++){
    count=0;
    for(long i = 0;i<submatrixsize;i++){
      sleft[i]=current[i][0];
      sright[i]=current[i][submatrixsize-1];
      left[i]=current[i][0];
      right[i]=current[i][submatrixsize-1];
      up[i]=current[0][i];
      down[i]=current[submatrixsize-1][i];
      }
      //Sending down
    if(pos=="core" || pos=="leftedge" || pos=="rightedge" || pos=="topedge" || pos=="topleft" ||pos=="topright"){
      MPI_Isend(current[submatrixsize-1], submatrixsize, MPI_DOUBLE, rank+partitionsize, 0, MPI_COMM_WORLD, &requests[count]);
      count++;
    }
    //Sending up
    if(pos=="core" || pos=="leftedge" || pos=="rightedge" || pos=="buttomedge" || pos=="buttomleft" ||pos=="buttomright"){
      MPI_Isend(current[0], submatrixsize, MPI_DOUBLE, rank-partitionsize, 0, MPI_COMM_WORLD, &requests[count]);
      count++;
    }
    //Sending left
    if(pos=="core" || pos=="buttomedge" || pos=="rightedge" || pos=="topedge" || pos=="buttomright" ||pos=="topright"){
      MPI_Isend(sleft, submatrixsize, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &requests[count]);
      count++;
    }
    //Sending right
    if(pos=="core" || pos=="leftedge" || pos=="buttomedge" || pos=="topedge" || pos=="topleft" ||pos=="buttomleft"){
      MPI_Isend(sright, submatrixsize, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &requests[count]);
      count++;
    }
    //Receiving up
    if(pos=="core" || pos=="leftedge" || pos=="rightedge" || pos=="buttomedge" || pos=="buttomleft" ||pos=="buttomright"){
      //cout<<"Receiving up matrix"<<endl;
      MPI_Irecv(up, submatrixsize, MPI_DOUBLE, rank-partitionsize, 0, MPI_COMM_WORLD, &requests[count]);
      count++;
    }
    //Receiving down
    if(pos=="core" || pos=="leftedge" || pos=="rightedge" || pos=="topedge" || pos=="topleft" ||pos=="topright"){
      MPI_Irecv(down, submatrixsize, MPI_DOUBLE, rank+partitionsize, 0, MPI_COMM_WORLD, &requests[count]);
      count++;
    }
    //Receiving left
    if(pos=="core" || pos=="buttomedge" || pos=="rightedge" || pos=="topedge" || pos=="buttomright" ||pos=="topright"){
      MPI_Irecv(left, submatrixsize, MPI_DOUBLE, rank-1, 0, MPI_COMM_WORLD, &requests[count]);
      count++;
    }
    //Receiving right
    if(pos=="core" || pos=="leftedge" || pos=="buttomedge" || pos=="topedge" || pos=="topleft" ||pos=="buttomleft"){
      MPI_Irecv(right, submatrixsize, MPI_DOUBLE, rank+1, 0, MPI_COMM_WORLD, &requests[count]);
      count++;
    }
    MPI_Waitall(count, requests, statuses);
    calculate_heat(current,next,up,down,left,right,rank);
    
    /*if(check2DHeat(current, N, rank, nbprocess, x)){
      cout<<"Incorrect for rank "<<rank<<endl;
    }*/
  
    {
      double**t = current;
      current=next;
      next=t;
    }
  }
}
std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
std::chrono::duration<double> elapsed_seconds = end-start;
MPI_Finalize();
if(rank==0){
  std::cerr<<elapsed_seconds.count()<<std::endl;
}
delete[] up;
delete[] down;
delete[] right;
delete[] left;
  for(int i=0;i<submatrixsize;i++){
    delete current[i];
    delete next[i];
  }
delete current;
delete next; 
  return 0;
}

