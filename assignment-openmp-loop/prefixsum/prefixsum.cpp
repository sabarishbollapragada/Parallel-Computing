#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
using namespace std;


#ifdef __cplusplus
extern "C" {
#endif
  void generatePrefixSumData (int* arr, size_t n);
  void checkPrefixSumResult (int* arr, size_t n);
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
  
  if (argc < 3) {
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  
  int n = atoi(argv[1]);
  int nbthreads=atoi(argv[2]);

  int * arr = new int [n];
  generatePrefixSumData (arr, n);

  int * pr = new int [n+1];
  int * temp = new int [n/nbthreads];

  //insert prefix sum code here
  int num_of_threads=0;
  omp_set_num_threads(nbthreads);
  std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();
  #pragma omp parallel
   {
  int start=0,end=0;
  int psum=0;
  if(omp_get_thread_num()==omp_get_num_threads()-1){
    start=omp_get_thread_num()*(n/omp_get_num_threads());
    end=n;
  }
  else{
        start=omp_get_thread_num()*(n/omp_get_num_threads());
        end=(omp_get_thread_num()*(n/omp_get_num_threads()))+(n/omp_get_num_threads());
  }
  for(int i=start;i<end;i++){
      psum+=arr[i];
      pr[i+1]=psum;  
    }
    #pragma omp critical
    temp[omp_get_thread_num()]=psum;
    #pragma omp barrier
    int updated_psum=0;
    for(int i=0;i<omp_get_thread_num();i++){
        updated_psum+=temp[i];
    }
    for(int i=start;i<end;i++){
      pr[i+1]+=updated_psum;
    }
}
  std::chrono::time_point<std::chrono::system_clock> endtime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endtime-starttime;
  cerr<<elapsed_seconds.count()<<endl;
  checkPrefixSumResult(pr, n);

  delete[] arr;

  return 0;
}
