#include<iostream>
#include<sstream>
#include<vector>
#include<algorithm>
#include<functional>
#include<cstdlib>
#include<cmath>
#include<ctime>
#include<mpi.h>
#define TAG_0 1
#define TAG_1 1000
#define TAG_2 2000

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
  for(size_t i=0; i<size; i++)m.push_back(rand());

  // sort
  for(int division_unit = com_size; division_unit > 1;){
    int pivot;
    tmp.clear();
    for(int i=0;i<sqrt(m.size());i++)tmp.push_back(m[(int)((rand()/((double)RAND_MAX+1.0)) * m.size())]);
    sort(tmp.begin(), tmp.end());
    pivot = tmp[sqrt(m.size())/2];
    
    if(com_rank%division_unit == 0){
      for(int j = com_rank+1; j<com_rank+division_unit; j++)
	MPI::COMM_WORLD.Isend(&(*m.begin()), 1, MPI::INT, j, TAG_0);
    }else{
      MPI::COMM_WORLD.Recv(&pivot, 1, MPI::INT, (com_rank/division_unit)*division_unit, TAG_0);
    }

    division_unit /= 2;
    int receive_datasize;
    if((com_rank/division_unit)%2 == 0){
      vector<int>::iterator senddata = partition(m.begin(), m.end(), bind2nd(less<int>(), pivot));
      size_t send_datasize = m.size() - (senddata - m.begin());
      int send_to = com_rank+division_unit;

      MPI::COMM_WORLD.Isend(&send_datasize, 1, MPI::INT, send_to, TAG_0);
      MPI::Request req = MPI::COMM_WORLD.Isend(&(*senddata), send_datasize, MPI::INT, send_to, TAG_1);

      MPI::COMM_WORLD.Recv(&receive_datasize, 1, MPI::INT, send_to, TAG_0);
      tmp.clear();
      tmp.resize(receive_datasize);
      MPI::COMM_WORLD.Recv(&(*tmp.begin()), receive_datasize, MPI::INT, send_to, TAG_2);
      m.resize(m.size()-send_datasize);
      req.Wait();
      
      m.insert(m.end(), tmp.begin(), tmp.end());
    }else{
      vector<int>::iterator senddata = partition(m.begin(), m.end(), bind2nd(greater_equal<int>(), pivot));
      size_t send_datasize = m.size() - (senddata - m.begin());
      int send_to = com_rank-division_unit;
     
      MPI::COMM_WORLD.Isend(&send_datasize, 1, MPI::INT, send_to, TAG_0);
      MPI::Request req = MPI::COMM_WORLD.Isend(&(*senddata), send_datasize, MPI::INT, send_to, TAG_2);

      MPI::COMM_WORLD.Recv(&receive_datasize, 1, MPI::INT, send_to, TAG_0);
      tmp.clear();
      tmp.resize(receive_datasize);
      MPI::COMM_WORLD.Recv(&(*tmp.begin()), receive_datasize, MPI::INT, send_to, TAG_1);
      m.resize(m.size()-send_datasize);
      req.Wait();
      
      m.insert(m.end(), tmp.begin(), tmp.end());
    }
  }
  sort(m.begin(), m.end());

  // print
  int signal=0;
  if(com_rank == 0){
    for(size_t i=0; i<m.size(); i++)cout << m[i] << endl;
    // for(size_t i=0; i<m.size(); i++)cout << "[0]" << m[i] << endl;
    if(com_rank != com_last)MPI::COMM_WORLD.Send(&signal, 1, MPI::INT, com_rank+1, TAG_0);
  }else{
    MPI::COMM_WORLD.Recv(&signal, 1, MPI::INT, com_rank-1, TAG_0);
    for(size_t i=0; i<m.size(); i++)cout << m[i] << endl;
    // for(size_t i=0; i<m.size(); i++)cout << "[" << com_rank << "]" << m[i] << endl;
    if(com_rank != com_last)MPI::COMM_WORLD.Send(&signal, 1, MPI::INT, com_rank+1, TAG_0);
  }

  MPI::Finalize();

}
