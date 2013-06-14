#include<iostream>
#include<sstream>
#include<cstdlib>
#include<ctime>
#include<cstring>
#include<mpi.h>
using namespace std;

int main(int argc, char *argv[]){
  if(argc < 1)exit(1);
  stringstream ss;
  size_t size;
  ss << argv[1]; ss >> size;

  MPI::Init();
  const int com_size = MPI::COMM_WORLD.Get_size(); // assume that com_size is a power of two
  const int com_rank = MPI::COMM_WORLD.Get_rank();
  const int com_last = com_size - 1;
  int *m, *tmp;
  m = new int[size];
  tmp = new int[size];
  size /= com_size;
  MPI::Finalize();
}
