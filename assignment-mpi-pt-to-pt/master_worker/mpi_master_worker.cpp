#include <mpi.h>
#include <iostream>
#include <chrono>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

float (*func_ptr_calculatef[])(float,int)={f1,f2,f3,f4};

#ifdef __cplusplus
}
#endif
int done=1,n,inc=0,granularity=1000,c=0;
void getNext(int* low,int* high,int* status){
  if(inc>=n){
    done=0;
  }
  if(done==1 && inc+granularity<=n){
        *low=inc;
    *high=inc+granularity;
    inc=inc+granularity;
    *status=done;
    c++;
  }
  else{
    *low=inc;
    *high=n;
    inc=n;
    *status=done;
    if(done==1)
      c++;
  }
}
int main (int argc, char* argv[]) {

  if (argc < 6) {
    std::cerr<<"usage: mpirun "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }
  std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();
  MPI_Init(&argc,&argv);
  int functionid=atoi(argv[1]),intensity=atoi(argv[5]);
  int rank,size;
  float a=atof(argv[2]),b=atof(argv[3]);
  n=atoi(argv[4]);
  float z=(b-a)/n;
  float result=0;
  int init_array[3];
  float psum=0;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  MPI_Comm_size(MPI_COMM_WORLD,&size);

  if(rank==0){
    int terminate=0;
    MPI_Status status;
      for(int i=1;i<size;i++){
        getNext(&init_array[0],&init_array[1],&init_array[2]);
        MPI_Send(init_array,3,MPI_INT,i,0,MPI_COMM_WORLD);
        if(init_array[2]==1){
          terminate++;
        }
      }
      while(terminate){
        MPI_Recv(&psum,1,MPI_FLOAT,MPI_ANY_SOURCE,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
        terminate=terminate-1;
        result+=psum;
        getNext(&init_array[0],&init_array[1],&init_array[2]);
        if(init_array[2]!=0){
          terminate+=1;
        }
        MPI_Send(init_array,3,MPI_INT,status.MPI_SOURCE,0,MPI_COMM_WORLD);
      }
      cout<<z*result<<endl;
  }
  else{
    float temp=0;
    MPI_Status status;
    while(1){
      MPI_Recv(init_array,3,MPI_INT,0,0,MPI_COMM_WORLD,&status);
      if(init_array[2]==0){
        break;
      }
      for(int i=init_array[0];i<init_array[1];i++){
        temp+=(*func_ptr_calculatef[functionid-1])((a+(i+.5)*(z)),intensity);
      }
    MPI_Send(&temp,1,MPI_FLOAT,0,0,MPI_COMM_WORLD);
    temp=0;
    }
  }
  MPI_Finalize();
  std::chrono::time_point<std::chrono::system_clock> endtime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = endtime-starttime;
  if(rank==0)
  std::cerr<<elapsed_seconds.count()<<std::endl;
  return 0;
}
