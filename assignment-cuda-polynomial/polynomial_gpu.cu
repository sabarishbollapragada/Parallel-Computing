#include <iostream>
#include <chrono>


__global__ void polynomial_expansion (float* poly, int degree,
			   int n, float* array) {
  //TODO: Write code to use the GPU here!
  //code should write the output back to array
  int index = blockIdx.x * blockDim.x + threadIdx.x;
  if(index < n){
    float out = 0.;
    float xtothepowerof = 1.;
    for (int i=0; i<=degree; ++i) {
      out += xtothepowerof*poly[i];
      xtothepowerof *= array[index];
    }
    array[index] = out;

  }

}


int main (int argc, char* argv[]) {
  //TODO: add usage
  
  if (argc < 3) {
     std::cerr<<"usage: "<<argv[0]<<" n degree"<<std::endl;
     return -1;
  }

  int n = atoi(argv[1]); //TODO: atoi is an unsafe function
  int degree = atoi(argv[2]);
  int nbiter = 1;

  float* array = new float[n];
  float* poly = new float[degree+1];
  float* gArray;
  float* gPoly;
  for (int i=0; i<n; ++i)
    array[i] = 1.;

  for (int i=0; i<degree+1; ++i)
    poly[i] = 1.;


  std::chrono::time_point<std::chrono::system_clock> begin, end;
  begin = std::chrono::system_clock::now();

  cudaMallocManaged(&gArray, n*sizeof(float));
  cudaMallocManaged(&gPoly, (degree+1)*sizeof(float));

  cudaMemcpy(gArray,array,n*sizeof(float),cudaMemcpyHostToDevice);
  cudaMemcpy(gPoly,poly,(degree+1)*sizeof(float),cudaMemcpyHostToDevice);

  int blocks=n/256;
  if(n%256>0){
    blocks = n/256+1;
  }

  
  
  for (int iter = 0; iter<nbiter; ++iter)
    polynomial_expansion<<<blocks,256>>> (gPoly, degree, n, gArray);

  

  cudaMemcpy(array,gArray,n*sizeof(float),cudaMemcpyDeviceToHost);
  cudaFree(gArray);
  cudaFree(gPoly);

  end = std::chrono::system_clock::now();
  std::chrono::duration<double> totaltime = (end-begin)/nbiter;

  {
    bool correct = true;
    int ind;
    for (int i=0; i< n; ++i) {
      if (fabs(array[i]-(degree+1))>0.01) {
        correct = false;
	ind = i;
      }
    }
    if (!correct)
      std::cerr<<"Result is incorrect. In particular array["<<ind<<"] should be "<<degree+1<<" not "<< array[ind]<<std::endl;
  }
  

  std::cerr<<array[0]<<std::endl;
  std::cout<<n<<" "<<degree<<" "<<totaltime.count()<<std::endl;

  delete[] array;
  delete[] poly;

  return 0;
}
