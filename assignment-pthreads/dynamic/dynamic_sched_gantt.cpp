#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <chrono>
#include <cmath>
#include <iomanip>
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
pthread_mutex_t m1,m;
int n,inc=0,granularity;
float sum=0;
struct integral{
  float a;
  float b;
  int intensity;
  float z;
  int fid;
  string sync;
  time_point<std::chrono::system_clock> x;
  //double main_start_time;
  double thread_start_time;
  double thread_end_time;
  int id;
  };
int done=1;
int loopDone(){return done;}
void getNext(int* low,int* high){
	pthread_mutex_lock(&m1);
	if(inc>=n){
		done=0;
	}
	if(done==1 && inc+granularity<=n){
        *low=inc;
	*high=inc+granularity;inc=inc+granularity;}
	else{
		        *low=inc;
			*high=n;inc=n;
	}
	pthread_mutex_unlock(&m1);
}
void* computef(void* integral1){
		std::chrono::time_point<std::chrono::system_clock> thread_start = std::chrono::system_clock::now();

	float var=0;
	struct integral *temp=(struct integral *)integral1;
	std::chrono::duration<double> startofthread = thread_start-(temp->x);
	if(temp->sync=="iteration"){	
		while(loopDone()){
			int low=0,high=0;
			getNext(&low,&high);
			for(int i=low;i<high;i++){
				var=(*func_ptr_calculatef[temp->fid-1])((temp->a+(i+.5)*(temp->z)),temp->intensity);
				pthread_mutex_lock(&m);
				sum=sum+var;
				pthread_mutex_unlock(&m);
			}
		}
	}
 	else if(temp->sync=="thread"){
		while(loopDone()){
			int low=0,high=0;
			getNext(&low,&high);
			for(int i=low;i<high;i++){
				var+=(*func_ptr_calculatef[temp->fid-1])((temp->a+(i+.5)*(temp->z)),temp->intensity);
			}
		}
		pthread_mutex_lock(&m);
		sum=sum+var;
		pthread_mutex_unlock(&m);
	}
	else if(temp->sync=="chunk"){
		while(loopDone()){
			var=0;
			int low=0,high=0;
			getNext(&low,&high);
			for(int i=low;i<high;i++){
				var+=(*func_ptr_calculatef[temp->fid-1])((temp->a+(i+.5)*(temp->z)),temp->intensity);
			}
			pthread_mutex_lock(&m);
			sum=sum+var;
			pthread_mutex_unlock(&m);
		}
	}
		std::chrono::time_point<std::chrono::system_clock> thread_end = std::chrono::system_clock::now();
		std::chrono::duration<double> endofthread = thread_end-thread_start;
		temp->thread_start_time=startofthread.count();
		temp->thread_end_time=endofthread.count();

}
int main (int argc, char* argv[]) {
	std::chrono::time_point<std::chrono::system_clock> main_start = std::chrono::system_clock::now();
  if (argc < 9) {
    std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync> <granularity>"<<std::endl;
    return -1;
  }
  int i,functionid=atoi(argv[1]),intensity=atoi(argv[5]),nbthreads=atoi(argv[6]);
  granularity=atoi(argv[8]);
  float a=atof(argv[2]),b=atof(argv[3]);
  string sync=argv[7];
  float result=0;
  n=atoi(argv[4]);
  float z=(b-a)/n;
  struct integral integral1[nbthreads];
  //integral1.a=a;integral1.b=b;integral1.intensity=intensity;integral1.z=z;integral1.fid=functionid;
  //integral1.sync=sync;
  pthread_t static_threads[nbthreads];
  pthread_mutex_init(&m,NULL);
  pthread_mutex_init(&m1,NULL);
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  for(i=0;i<nbthreads;i++){
  	integral1[i].a=a;integral1[i].b=b;integral1[i].intensity=intensity;integral1[i].z=z;integral1[i].fid=functionid;
  	integral1[i].id=i;integral1[i].sync=sync;
  	integral1[i].x=main_start;
	pthread_create(&static_threads[i],NULL,computef,&integral1[i]);
	}
  for(int i=0;i<nbthreads;i++){
  	pthread_join(static_threads[i],NULL);
  	}
  result=z*sum;
  std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
  std::chrono::duration<double> elapsed_seconds = end-start;
  pthread_mutex_destroy(&m);
  pthread_mutex_destroy(&m1);
  for(int j=0;j<nbthreads;j++){
  	std::cout<<"Thread "<<integral1[j].id<<" has "<<"start time:"<<integral1[i].thread_start_time<<" execution time:"<<integral1[j].thread_end_time<<std::endl;
  }
  std::cerr<<elapsed_seconds.count()<<std::endl;
  std::cout << result <<std::endl;
  return 0;
}
