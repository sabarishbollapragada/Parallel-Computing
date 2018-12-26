#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <omp.h>

using namespace std;
#ifdef __cplusplus
extern "C" {
#endif

  void generateReduceData (int* arr, size_t n);

#ifdef __cplusplus
}
#endif
int n,nbthreads;
int partialSum(int i,int j);
int * arr;
int findSum(int i,int j, int N) 
{ 
    int m,a,b,partial_sum;
    if(j-i+1<=n/(2*nbthreads)){
      partial_sum=partialSum(i,j);
      return partial_sum;
    }
    m=(i+j)/2;
    #pragma omp task shared(a)
    a=findSum(i,m,N);
    #pragma omp task shared(b)
    b=findSum(m+1,j,N);
    #pragma omp taskwait
    return a+b;

} 
int partialSum(int i,int j){
  int psum=0;
  //When using recursive method use: x<=j
  //When using for loop method use : x<j
  for(int x=i;x<=j;x++){
    psum+=arr[x];
  }
  return psum;
}
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
    std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  n = atoi(argv[1]);
  //int * arr = new int [n];
  
  nbthreads=atoi(argv[2]);
  arr = (int*)malloc(n*sizeof(int));
  generateReduceData (arr, atoi(argv[1]));
  int result=0;
  int m=50;
  int * temp=new int[m];
  //insert reduction code here

  omp_set_num_threads(nbthreads);

  std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();
  //Using recursion to define tasks
  #pragma omp parallel
  {
  #pragma omp single
    result=findSum(0,n-1, n);
  }
  //Using for loop to define tasks
  /*#pragma omp parallel
  {
    
    #pragma omp single
    {
    //#pragma omp for 
    for(int i=0;i<omp_get_num_threads();i++){
      if(n%omp_get_num_threads()>0 && i==omp_get_num_threads()-1){
        #pragma omp task
        temp[i]=partialSum(i*(n/omp_get_num_threads()),n);
      }
      else{
      #pragma omp task
      temp[i]=partialSum(i*(n/omp_get_num_threads()),i*(n/omp_get_num_threads())+n/omp_get_num_threads());
    }
    }
    }
  
  
  #pragma omp single
  {
    for(int i=0;i<omp_get_num_threads();i++){
      result+=temp[i];
    }
  }
  }*/

  std::chrono::time_point<std::chrono::system_clock> endtime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endtime-starttime;
  cerr<<elapsed_seconds.count()<<endl; 
  cout<<result<<endl;
  delete[] arr;

  return 0;
}
