#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <cmath>
using namespace std;
using namespace std::chrono; 
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
pthread_mutex_t m;
float sum=0;
struct integral{
  float a;
  float b;
  int intensity;
  float z;
  int nbthreads_low;
  int nbthreads_high;
  int fid;
  };

void* iteration(void* integral1){
	float var=0;
	struct integral *temp=(struct integral *)integral1;
	for(int i=temp->nbthreads_low;i<temp->nbthreads_high;i++){
			var=(*func_ptr_calculatef[temp->fid-1])((temp->a+(i+.5)*(temp->z)),temp->intensity);
			pthread_mutex_lock(&m);
			sum=sum+var;
			pthread_mutex_unlock(&m);
		}
}
void* thread(void* integral1){
	float var=0;
	struct integral *temp=(struct integral *)integral1;
	for(int i=temp->nbthreads_low;i<temp->nbthreads_high;i++){
			var+=(*func_ptr_calculatef[temp->fid-1])((temp->a+(i+.5)*(temp->z)),temp->intensity);
		}
		pthread_mutex_lock(&m);
		sum=sum+var;
		pthread_mutex_unlock(&m);
}
int main (int argc, char* argv[]) {

  if (argc < 8) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync>"<<std::endl;
    return -1;
  }
  
  int i,functionid=atoi(argv[1]),n=atoi(argv[4]),intensity=atoi(argv[5]),nbthreads=atoi(argv[6]);
  float a=atof(argv[2]),b=atof(argv[3]);
  string sync=argv[7];
  float result;
  float z=(b-a)/n;
  struct integral integral1[nbthreads];
  pthread_t static_threads[nbthreads];
  pthread_mutex_init(&m,NULL);
  
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
	if(sync=="iteration"){	
			for(i=0;i<nbthreads;i++){
			if(n%nbthreads>0 && i==nbthreads-1){
				integral1[i].a=a;integral1[i].b=b;integral1[i].intensity=intensity;integral1[i].z=z;
				integral1[i].fid=functionid;integral1[i].nbthreads_low=i*(n/nbthreads);integral1[i].nbthreads_high=n;
				pthread_create(&static_threads[i],NULL,iteration,&integral1[i]);
				break;
			}
			integral1[i].a=a;integral1[i].b=b;integral1[i].intensity=intensity;integral1[i].z=z;integral1[i].fid=functionid;
			integral1[i].nbthreads_low=i*(n/nbthreads);integral1[i].nbthreads_high=i*(n/nbthreads)+(n/nbthreads);
			pthread_create(&static_threads[i],NULL,iteration,&integral1[i]);
		}
}
		
 		else if(sync=="thread"){
			for(i=0;i<nbthreads;i++){
			if(n%nbthreads>0 && i==nbthreads-1){
				integral1[i].a=a;integral1[i].b=b;integral1[i].intensity=intensity;integral1[i].z=z;
				integral1[i].fid=functionid;integral1[i].nbthreads_low=i*(n/nbthreads);integral1[i].nbthreads_high=n;
				pthread_create(&static_threads[i],NULL,thread,&integral1[i]);
				break;
			}
			integral1[i].a=a;integral1[i].b=b;integral1[i].intensity=intensity;integral1[i].z=z;integral1[i].fid=functionid;
			integral1[i].nbthreads_low=i*(n/nbthreads);integral1[i].nbthreads_high=i*(n/nbthreads)+(n/nbthreads);
			pthread_create(&static_threads[i],NULL,thread,&integral1[i]);
		}
}
		
  for(int i=0;i<nbthreads;i++){
  pthread_join(static_threads[i],NULL);
  }
  result=z*sum;
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  pthread_mutex_destroy(&m);
  std::cerr<<elapsed_seconds.count()<<std::endl;
  std::cout << result <<std::endl;
   
  
  return 0;
}

