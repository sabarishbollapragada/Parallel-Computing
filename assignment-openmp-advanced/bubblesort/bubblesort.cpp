#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <omp.h>
#include <chrono>
using namespace std;
#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (const int* arr, size_t n);

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
  
  if (argc < 3) { std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n= atoi(argv[1]);
  int nbthreads = atoi(argv[2]);
  // get arr data
  int * arr = new int [n];
  generateMergeSortData (arr, n);
  //int n;
  omp_set_num_threads(nbthreads);
  std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();

  //insert sorting code here.
  for(int i=0;i<n;i++)
   {
     if(i%2 == 0){
       #pragma omp parallel for
       for(int j = 0;j<=n-2;j+=2)
       {
         if(arr[j]>arr[j+1]){
           int temp = arr[j];
           arr[j] = arr[j+1];
           arr[j+1] = temp;
        }
      }
       
     }
     else{
       #pragma omp parallel for
       for(int j=1;j<=n-2;j+=2)
       {
        if(arr[j]>arr[j+1]){
           int temp = arr[j];
           arr[j] = arr[j+1];
           arr[j+1] = temp;
        }              
      }
    }
  
}
  std::chrono::time_point<std::chrono::system_clock> endtime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = endtime-starttime;
  cerr<<elapsed_seconds.count()<<endl; 
  
  checkMergeSortResult (arr, n);
  
  delete[] arr;

  return 0;
}
