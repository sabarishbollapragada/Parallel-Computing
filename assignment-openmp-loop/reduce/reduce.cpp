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
  void generateReduceData (int* arr, size_t n);
#ifdef __cplusplus
}
#endif


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
  
  if (argc < 5) {
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads> <scheduling> <granularity>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  int nbthreads = atoi(argv[2]);
  string policy=argv[3];
  int granularity= atoi(argv[4]);
  int * arr = new int [n];
  float sum=0;
  omp_sched_t type=omp_sched_static;
  generateReduceData (arr, atoi(argv[1]));
  if(strcmp(argv[3],"static")==0){
  	type=omp_sched_static;
  }
  else if(strcmp(argv[3],"dynamic")==0){
  	type=omp_sched_dynamic;
  }
  omp_set_num_threads(nbthreads);
  omp_set_schedule(type,granularity);
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  //insert reduction code here
  #pragma omp parallel for reduction(+:sum) schedule(runtime)
  	for(int i=0;i<n;i++){
  		sum+=arr[i];
  	}
  	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  	std::chrono::duration<double> elapsed_seconds = end-start;
  	cout<<sum<<endl;
  	cerr<<elapsed_seconds.count()<<endl;
  
  delete[] arr;

  return 0;
}
