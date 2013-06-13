#include<iostream>
#include<sstream>
#include<vector>
#include<algorithm>
#include<functional>
#include<cstdlib>
#include<ctime>
#include<mpi.h>
#define TAG 1

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

  size /= com_size;

  // initialize ary with random numbers
  srand((unsigned)time(NULL)+(unsigned)com_rank);
  vector<int> m;
  vector<int> tmp;
  vector<int> next;
  for(size_t i=0; i<size; i++)m.push_back(rand());

  // sort
  for(int division_unit = com_size; division_unit > 1;){
    int pivot;
    tmp.clear();
    for(int i=0;i<m.size()/10;i++)tmp.push_back(m[(int)((rand()/((double)RAND_MAX+1.0)) * m.size())]);
    sort(tmp.begin(), tmp.end());
    pivot = tmp[size/20];
    
    if(com_rank%division_unit == 0){
      for(int j = com_rank+1; j<com_rank+division_unit; j++)
	MPI::COMM_WORLD.Isend(&(*m.begin()), 1, MPI::INT, j, TAG);
    }else{
      MPI::COMM_WORLD.Recv(&pivot, 1, MPI::INT, (com_rank/division_unit)*division_unit, TAG);
    }
    vector<int>::iterator greater_half = partition(m.begin(), m.end(), bind2nd(less<int>(), pivot));
    size_t lthalf_datasize = greater_half - m.begin();
    size_t gthalf_datasize = m.size() - lthalf_datasize;
    vector<int>::iterator lesser_half = m.begin();
    division_unit /= 2;
    int receive_datasize;
    if((com_rank/division_unit)%2 == 0){
      MPI::COMM_WORLD.Recv(&receive_datasize, 1, MPI::INT, com_rank+division_unit, TAG);
      next.clear();
      next.resize(receive_datasize);
      MPI::COMM_WORLD.Recv(&(*next.begin()), receive_datasize, MPI::INT, com_rank+division_unit, TAG);
      
      MPI::COMM_WORLD.Isend(&gthalf_datasize, 1, MPI::INT, com_rank+division_unit, TAG);
      MPI::COMM_WORLD.Isend(&(*greater_half), gthalf_datasize, MPI::INT, com_rank+division_unit, TAG);
      
      next.insert(next.end(), m.begin(), m.begin() + lthalf_datasize);
    }else{
      MPI::COMM_WORLD.Isend(&lthalf_datasize, 1, MPI::INT, com_rank-division_unit, TAG);
      MPI::COMM_WORLD.Isend(&(*lesser_half), lthalf_datasize, MPI::INT, com_rank-division_unit, TAG);
      
      MPI::COMM_WORLD.Recv(&receive_datasize, 1, MPI::INT, com_rank-division_unit, TAG);
      next.clear();
      next.resize(receive_datasize);
      MPI::COMM_WORLD.Recv(&(*next.begin()), receive_datasize, MPI::INT, com_rank-division_unit, TAG);
      
      next.insert(next.end(), greater_half, m.end());
    }
    m.swap(next);
  }
  sort(m.begin(), m.end());

  // print
  int signal=0;
  if(com_rank == 0){
    for(size_t i=0; i<m.size(); i++)cout << m[i] << endl;
    // for(size_t i=0; i<m.size(); i++)cout << "[0]" << m[i] << endl;
    if(com_rank != com_last)MPI::COMM_WORLD.Send(&signal, 1, MPI::INT, com_rank+1, TAG);
  }else{
    MPI::COMM_WORLD.Recv(&signal, 1, MPI::INT, com_rank-1, TAG);
    for(size_t i=0; i<m.size(); i++)cout << m[i] << endl;
    // for(size_t i=0; i<m.size(); i++)cout << "[" << com_rank << "]" << m[i] << endl;
    if(com_rank != com_last)MPI::COMM_WORLD.Send(&signal, 1, MPI::INT, com_rank+1, TAG);
  }

  MPI::Finalize();

}
