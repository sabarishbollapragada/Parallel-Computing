#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <mpi.h>
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

  
int main (int argc, char* argv[]) {
  
  if (argc < 6) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity>"<<std::endl;
    return -1;
  }
  MPI_Init(&argc,&argv);
  int i,functionid=atoi(argv[1]),intensity=atoi(argv[5]);
  int rank,size;
  float a=atof(argv[2]),b=atof(argv[3]);
  int n=atoi(argv[4]);
  float z=(b-a)/n;
  float result=0;
  	std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();
  	
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);
	float psum=0,start,end;
	if(rank==size-1){
		start=rank*(n/size);
		end=n;
	}
	else{
		start=rank*(n/size);
		end=(rank+1)*(n/size);
	}
	for(int i=start;i<end;i++){
		psum+=(*func_ptr_calculatef[functionid-1])((a+(i+.5)*(z)),intensity);
	}
	//MPI_Reduce(&psum,&result,1,MPI_FLOAT,MPI_SUM,0,MPI_COMM_WORLD);
	MPI_Reduce(&psum,&result,1,MPI_FLOAT,MPI_SUM,0,MPI_COMM_WORLD);
	MPI_Finalize();
	std::chrono::time_point<std::chrono::system_clock> endtime = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = endtime-starttime;
	if(rank==0){
		//psum=result;
		std::cout<<result*z<<endl;
		std::cerr<<elapsed_seconds.count()<<endl;
	}
  return 0;
}
