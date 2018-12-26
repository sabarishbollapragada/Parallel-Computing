#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

pthread_mutex_t m;
int nbthreads;
void* f(void* x){
//uncomment mutex to prevent pre-emption
//pthread_mutex_lock(&m);
std::cout<<"I am thread "<< *(int*) x<<" of "<<nbthreads<<std::endl;
//pthread_mutex_unlock(&m);
return NULL;
}
int main (int argc, char* argv[]) {

 if (argc < 2) {
    std::cerr<<"usage: "<<argv[0]<<" <nbthreads>"<<std::endl;
    return -1;
  }
  pthread_mutex_init(&m,NULL);
  int a=atoi(argv[1]);
  nbthreads=a;
  int v[a];
  pthread_t x[a];
  for(int i=0;i<a;i++){
  v[i]=i;
  pthread_create(&x[i],NULL,f,&v[i]);
  }
  for(int i=0;i<a;i++){
  pthread_join(x[i],NULL);
  }
  pthread_mutex_destroy(&m);

  
  return 0;
}
