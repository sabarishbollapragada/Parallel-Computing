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

  void generateLCS(char* X, int m, char* Y, int n);
  void checkLCS(char* X, int m, char* Y, int n, int result);

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

  if (argc < 4) { std::cerr<<"usage: "<<argv[0]<<" <m> <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int m = atoi(argv[1]);
  int n = atoi(argv[2]);
  int nbthreads=atoi(argv[3]);
  // get string data 
  char *X = new char[m];
  char *Y = new char[n];
  int **arr=new int*[m+1];
  for(int i=0;i<m+1;i++){
    arr[i]=new int[n+1];
  }
  generateLCS(X, m, Y, n);
  omp_set_num_threads(nbthreads);
  
  int am=m+1,an=n+1;

  //insert LCS code here.
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  #pragma omp parallel for
  for(int i =0;i<an;i++)
  arr[0][i] = 0;

  #pragma omp parallel for
  for(int i=0;i<am;i++)
  arr[i][0] = 0;
  for(int i=2;i<am+an-1;i++){
    if(i<=an-1){
      int j= i <= am ? i : am;
      #pragma omp parallel for
      for(int k=1;k<j;k++){
        if(X[k-1]==Y[i-k-1]){
          arr[k][i-k]=arr[k-1][i-k-1]+1;
        }
        else{
          arr[k][i-k]= std::max(arr[k-1][i-k], arr[k][i-k-1]);
        }
        
      }
    }
    else{
      int lower=i-(an-1);
      int j= ((am-1)-lower) < an-1 ? am-lower : an-1;
      #pragma omp parallel for
      for(int k=j;k>0;k--){
        if(X[lower+(k-1)-1]==Y[(an-1)-(k-1)-1]){
          arr[lower+(k-1)][(an-1)-(k-1)]=arr[lower+(k-1)-1][(an-1)-(k-1)-1]+1;
        }
        else{
          arr[lower+(k-1)][(an-1)-(k-1)]= std::max(arr[lower+(k-1)-1][(an-1)-(k-1)], arr[lower+(k-1)][(an-1)-(k-1)-1]);
        }
        
      }
    }
  }
  //int result = -1; // length of common subsequence
  
  std::chrono::time_point<std::chrono::system_clock> endtime = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = endtime-start;

  cerr<<elapsed_seconds.count()<<endl; 
  checkLCS(X, m, Y, n, arr[m][n]);
  for(int i = 0; i < m+1; ++i)
  free(arr[i]);
    
  free(arr);

  return 0;
}
