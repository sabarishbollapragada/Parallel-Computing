#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <chrono>
using namespace std;
#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif
float (*func_ptr_calculatef[])(float,int)={f1,f2,f3,f4};
int main (int argc, char* argv[]) {
  //forces openmp to create the threads beforehand

#pragma omp parallel
  {
    int fd = open (argv[0], O_RDONLY);
    if (fd != -1) {
      close (fd);
    }
    else {
      std::cerr<<"something is amiss"<<std::endl;
    }
  }
  
  if (argc < 9) {
    std::cerr<<"Usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <scheduling> <granularity>"<<std::endl;
    return -1;
  }
  int n,granularity;
  float sum=0;
  int i,functionid=atoi(argv[1]),intensity=atoi(argv[5]),nbthreads=atoi(argv[6]);
  granularity=atoi(argv[8]);
  float a=atof(argv[2]),b=atof(argv[3]);
  string policy=argv[7];
  n=atoi(argv[4]);
  float z=(b-a)/n;
  omp_sched_t type=omp_sched_static;
  if(strcmp(argv[7],"static")==0){
    type=omp_sched_static;
  }
  else if(strcmp(argv[7],"dynamic")==0){
    type=omp_sched_dynamic;
  }
  omp_set_num_threads(nbthreads);
  omp_set_schedule(type,granularity);
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  //insert reduction code here
  #pragma omp parallel for reduction(+:sum) schedule(runtime)
    for(int i=0;i<n;i++){
      sum+=(*func_ptr_calculatef[functionid-1])((a+(i+.5)*(z)),intensity);
    }
    float result=z*sum;
    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    cout<<result<<endl;
    cerr<<elapsed_seconds.count()<<endl;
  //insert code here

  return 0;
}
