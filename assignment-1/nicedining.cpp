#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

std::mutex out;

void philosopher(int n, std::mutex *left, std::mutex *right)
{
  while (true)
    {
      out.lock();
      std::cout << "Philosopher " << n << " is thinking." << std::endl;
      out.unlock();

      left->lock();
      out.lock();
      std::cout << "Philosopher " << n << " picked up her left fork." << std::endl;
      out.unlock();
      
      // try locking the right fork and if not posible drop the left one and wait so others can eat
      if(right->try_lock()){ 
          out.lock();
          std::cout << "Philosopher " << n << " picked up her right fork." << std::endl;
          out.unlock();
          
          
          out.lock();
          std::cout << "Philosopher " << n << " is eating." << std::endl;
          out.unlock();

          out.lock();
          std::cout << "Philosopher " << n << " is putting down her forks" << std::endl;
          out.unlock();
          right->unlock();
          left->unlock();

          // make the thread sleep after eating to allow others to eat
          this_thread::sleep_for(chrono::milliseconds(100)); 

      }else{
          out.lock();
          cout<<"Philosopher " << n << " Could't get right fork so he is droping left" << std::endl;
          out.unlock();
          left->unlock();
          // make the thread sleep for a bit when unable to eat and allow others to eat
          this_thread::sleep_for(chrono::milliseconds(200)); 
      }

    }
}

void usage(char *program)
{
  std::cout << "Usage: " << program << " N  (where 2<=N<=10)" << std::endl;
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc != 2)
    {
      usage(argv[0]);
    }

  // philosophers = argv[1]
  int philosophers;
  try
    {
      philosophers = std::stoi(argv[1]);
    }
  catch (const std::exception&)
    {
      usage(argv[0]);
    }
  if (philosophers < 2 || philosophers > 10)
    {
      usage(argv[0]);
    }

  // forks
  std::mutex *forks = new std::mutex[philosophers];

  // philosophers
  std::thread *ph = new std::thread[philosophers];
  for (int i=0; i<philosophers; ++i)
    {
      int left = i;
      int right = (i == 0 ? philosophers : i) - 1;
      ph[i] = std::thread(philosopher, i, &forks[left], &forks[right]);
    }

  ph[0].join();
  delete[] forks;
  delete[] ph;

  return 0;
}
