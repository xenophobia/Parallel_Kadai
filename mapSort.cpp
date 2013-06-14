#include<iostream>
#include<sstream>
#include<algorithm>
#include<cstdlib>
#include<ctime>
#include<cstring>
#include<mpi.h>
#define TAG_0 1000
using namespace std;

int main(int argc, char *argv[]){
  if(argc < 1)exit(1);
  stringstream ss;
  size_t size;
  size_t block_size;
  ss << argv[1]; ss >> size;

  MPI::Init();
  const int com_size = MPI::COMM_WORLD.Get_size(); // assume that com_size is a power of two
  const int com_rank = MPI::COMM_WORLD.Get_rank();
  const int com_last = com_size - 1;
  block_size = (size_t)com_size * 2;
  int *m;
  int *key;
  int *mp_partial;
  int *mp;
  m = new int[size];
  key = new int[block_size];
  mp_partial = new int[block_size];
  mp = new int[block_size*com_size];
  size /= com_size;

  // initialize ary with random numbers
  srand((unsigned)time(NULL)+(unsigned)com_rank);
  for(size_t i=0; i<size; i++)m[i] = rand();

  // gather keys & define sections
  MPI::COMM_WORLD.Gather(m, 2, MPI::INT, key, 2, MPI::INT, 0);
  sort(key, key+block_size);
  MPI::COMM_WORLD.Bcast(key, block_size, MPI::INT, 0);
  
  // count
  int s;
  for(int i=0;i<size;i++){
    for(s=0;s<block_size;){
      if(m[i]<key[s++]){
	break;
      }
    }
    mp_partial[s-1]++;
  }
  
  // Gather each partial maps and make map
  MPI::COMM_WORLD.Gather(mp_partial, block_size, MPI::INT, mp, block_size, MPI::INT, 0);
  int t = 0;
  for(size_t i=0; i<block_size; i++){
    for(size_t j=0; j<com_size; j++){
      int k = i+block_size*j;
      int tmp = t;
      t += mp[k];
      mp[k] = tmp;
    }
  }
  MPI::COMM_WORLD.Bcast(mp, block_size*com_size, MPI::INT, 0);

  // print
  int signal=0;
  if(com_rank == 0){
    cout << "keys:" << endl;
    for(size_t i=0; i<block_size; i++)cout << key[i] << endl;
    cout << "map:" << endl;
    for(size_t i=0; i<block_size; i++){
      for(size_t j=0; j<com_size; j++){
	cout << "[" << (i+block_size*j) << "]" << mp[i+block_size*j] << "\t";
      }
      cout<<endl;
    }
    for(size_t i=0; i<size; i++)cout << "[0]" << m[i] << endl;
    if(com_rank != com_last)MPI::COMM_WORLD.Send(&signal, 1, MPI::INT, com_rank+1, TAG_0);
  }else{
    MPI::COMM_WORLD.Recv(&signal, 1, MPI::INT, com_rank-1, TAG_0);
    for(size_t i=0; i<size; i++)cout << "[" << com_rank << "]" << m[i] << endl;
    if(com_rank != com_last)MPI::COMM_WORLD.Send(&signal, 1, MPI::INT, com_rank+1, TAG_0);
  }

  MPI::Finalize();
  delete [] m;
  delete [] key;
  delete [] mp_partial;
  delete [] mp;
}
