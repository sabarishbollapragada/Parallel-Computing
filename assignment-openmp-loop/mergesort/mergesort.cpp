#include <chrono>
#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <omp.h>

using namespace std;


#ifdef __cplusplus
extern "C" {
#endif

  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);

#ifdef __cplusplus
}
#endif

void merge(int arr[], int t[] , int n, int start, int mid, int last)
{
  int s1 = start,s2=start;
  int m = mid+1;
  while(s1 <= mid && m<=last){
      if(arr[s1] < arr[m]){
          t[s2] = arr[s1];
          s2++;
          s1++;
        }
      else{
          t[s2] = arr[m];
          s2++;
          m++;
        }
  }
  while(s1 <= mid){
      t[s2] = arr[s1];
      s2++;
      s1++;
  }
  for(int i = start; i <= last; i++){
      arr[i] = t[i];
  }
}

int main (int argc, char* argv[]) {
  if (argc < 3) {
    std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }
  int n = atoi(argv[1]);
  int nbthreads = atoi(argv[2]);
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
  int * arr = new int [n];
  int *temp =new int[n];
  generateMergeSortData (arr, atoi(argv[1]));
  omp_set_num_threads(nbthreads);
  for(int i=0; i<n; i++){
      temp[i] = arr[i];
  }
std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();
int i=1;
  while(i<n)
  {
   #pragma omp parallel for schedule(static)
   for (int start=0; start<n-1; start=start+(2*i))
   {
       int mid,end;
       mid = min(start+i-1, n-1);
       end = min(start+(2*i)-1, n-1);
       merge(arr, temp, n, start, mid, end);
   }
   i=2*i;
  }
std::chrono::time_point<std::chrono::system_clock> endtime = std::chrono::system_clock::now();
std::chrono::duration<double> elapsed_seconds = endtime-starttime;
cerr<<elapsed_seconds.count();
  checkMergeSortResult (arr, atoi(argv[1]));
  delete[] arr;
  return 0;
}
