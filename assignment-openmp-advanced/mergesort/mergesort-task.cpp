#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
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
int n,nbthreads;
/*int * arr;
void merge_sort(int, int,int);
void merge_array(int, int, int, int,int);

void merge_sort(int i, int j,int n) {
    int m;

    if (i < j) {
        m = (i + j) / 2;
        #pragma omp task
        merge_sort(i, m,n);
        #pragma omp task
        merge_sort(m + 1, j,n);
        #pragma omp taskwait
        // Merging two arrays
        merge_array(i, m, m + 1, j,n);
    }
}

void merge_array(int a, int b, int c, int d,int n) {
    int * t=new int[n];
    int i = a, j = c, k = 0;

    while (i <= b && j <= d) {
        if (arr[i] < arr[j])
            t[k++] = arr[i++];
        else
            t[k++] = arr[j++];
    }

    //collect remaining elements 
    while (i <= b)
        t[k++] = arr[i++];

    while (j <= d)
        t[k++] = arr[j++];

    for (i = a, j = 0; i <= d; i++, j++)
        arr[i] = t[j];

    delete[] t;
}*/
void merge(int * arr, int l, int mid, int r) {

  if (l == r) return;
  if (r-l == 1) {
    if (arr[l] > arr[r]) {
      int temp = arr[l];
      arr[l] = arr[r];
      arr[r] = temp;
    }
    return;
  }

  int i, j, k;
  int n = mid - l;
  

  int *temp = new int[n];
  for (i=0; i<n; ++i)
    temp[i] = arr[l+i];

  i = 0;    // temp left half
  j = mid;  // right half
  k = l;    // write to 

  
  while (i<n && j<=r) {
     if (temp[i] <= arr[j] ) {
       arr[k++] = temp[i++];
     } else {
       arr[k++] = arr[j++];
     }
  }

 
  while (i<n) {
    arr[k++] = temp[i++];
  }

  delete[] temp;

}



void mergesort(int * arr, int l, int r) {

  if (l < r) {
    int mid = (l+r)/2;
    //#pragma omp task
    mergesort(arr, l, mid);
    //#pragma omp task
    mergesort(arr, mid+1, r);
    //#pragma omp taskwait
    merge(arr, l, mid+1, r);
  }

}
void mergesortl(int * arr, int l, int r) {

  //if (l < r) {
  if(r-l+1<=n/(nbthreads)){
      mergesort(arr,l,r);
      return ;
    }
    int mid = (l+r)/2;
    #pragma omp task
    mergesortl(arr, l, mid);
    #pragma omp task
    mergesortl(arr, mid+1, r);
    #pragma omp taskwait
    merge(arr, l, mid+1, r);
  //}

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
  
  if (argc < 3) { std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  n = atoi(argv[1]);
  
  // get arr data
  //int * arr = new int [n];
  nbthreads=atoi(argv[2]);
  //arr = (int*)malloc(n*sizeof(int));
  int * arr = new int [n];
  generateMergeSortData (arr, n);
  omp_set_num_threads(nbthreads);
  //insert sorting code here.
  std::chrono::time_point<std::chrono::system_clock> starttime = std::chrono::system_clock::now();
  #pragma omp parallel
  {
  #pragma omp single
    mergesortl(arr, 0, n-1);
  //merge_sort(0, n - 1,n);
  }

std::chrono::time_point<std::chrono::system_clock> endtime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = endtime-starttime;
  cerr<<elapsed_seconds.count()<<endl; 
  checkMergeSortResult (arr, n);
  
  delete[] arr;

  return 0;
}
