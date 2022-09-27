/* Spring 2021 CSCI 144

   David Andrade 

*/




#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <assert.h>
using namespace std;

//To compile on cygwin: g++ deadlock_project.cpp -o deadlock_project
//To run code:  ./deadlock_project.exe 2 1   The numbers are the input.
//Make sure in same directory and folder as cpp file

//Global variables
mutex myLock;
condition_variable cv;
int **max2d, **alloc2d, **request, **need;

//Do i need capacity ? or just available 
int* capacity, * available,* toBeAvailable;
int myThreads, myResources;
bool* finish;
thread* threadsArr;

bool isSafe() {
	//Initializing array's for isSafe function
	toBeAvailable = new int[myResources];
	finish = new bool[myResources];
	need = new int *[myThreads];

	for (int i = 0; i < myResources; i++) {
		toBeAvailable[i] = available[i];
	}

	for (int i = 0; i < myThreads; i++) {
		need[i] = new int[myThreads];
	}

	for (int i = 0; i < myThreads; i++) {
		for (int j = 0; j < myResources; j++) {
			need[i][j] = max2d[i][j] - alloc2d[i][j];
		}
	}

	for (int i = 0; i < myThreads; i++)
		finish[i] = false;
	
	int jid = 0;
	while (true) {
		bool condition = false;
		for (int j = 0; j < myThreads; j++) {
			bool canMeet = true;
			for (int i = 0; i < myResources; i++) {

				if (need[j][i] > toBeAvailable[i]) {
					canMeet = false;
					break;

				}
			}
			if(finish[j] == false && canMeet) {
				jid = j;
				condition = true;
				break;
			}
		
		}

		if (condition == false) {
			for (int j = 0; j < myThreads; j++) {
				if (finish[j] == false) {
					cout << "Unsafe" << endl;
					return false;
				}
			}
			cout << "isSafe" << endl;
			return true;
		}
		else {
			finish[jid] = true;
			for (int i = 0; i < myResources; i++) {
				toBeAvailable[i] = toBeAvailable[i] + alloc2d[jid][i];
			}
		}
		
	}

}

//Makes sure we dont hit deadlock, and uses isSafe to see that.
bool wouldBeSafe(int tid, int j) {
	bool result = false;
	//cout << "i am here wouldBeSafe" << endl;
	available[j]--;
	alloc2d[tid][j]++;
	if (isSafe()) {
		result = true;
	}

	available[j]++;
	alloc2d[tid][j]--;
	return result;
}

//function for requesting resources for a thread, if not safe
//then cv.wait for thread trying to request
void Request(int tid, int j) {
	std::unique_lock<std::mutex> mlock(myLock);
	myLock.try_lock();
	
	while (!wouldBeSafe(tid, j)) {
		cout << "Thread " << tid << " is suspended due to waiting condition" << endl;
		cv.wait(mlock);
	}

	alloc2d[tid][j]++;
	available[j]--;
	cout << "Resource granted to thread " << tid << endl;
	myLock.unlock();
}

void threadFun(int tid) {
	bool istrue = true;

	//Generate random request for tid for all resources
	//tRequest[tid][j] between 0 and max2d[tid][j]
	while (istrue) {

		for (int j = 0; j < myResources; j++) {
			request[tid][j] = rand() % max2d[tid][j];
		}

		for (int j = 0; j < myResources; j++) {
			for (int k = 0; k < request[tid][j]; k++) {
				cout << "Thread " << tid << " requesting resources" << endl;
				Request(tid, j);
				
			}

		}

		for (int j = 0; j < myResources; j++) {
			available[j] += alloc2d[tid][j];
			alloc2d[tid][j] = 0;
		}
		

		cv.notify_all();


		std::this_thread::sleep_for(std::chrono :: seconds(1));

		cout << "waiting" << endl;

	}

	//Allocation 2darray after a thread is granted all requested resources and then releases the resources.
	cout << endl;
	cout << "Allocation table after a thread is granted all requested resources" << endl;
	for (int i = 0; i < myThreads; i++) {
		for (int j = 0; j < myResources; j++) {
			cout << alloc2d[i][j] << "  ";

		}
		cout << endl;
	}

}

int main(int argc, char *argv[]) {
	myThreads = atoi(argv[1]);
	myResources = atoi(argv[2]);

	threadsArr = new thread[myThreads];
	capacity = new int[myResources];
	available = new int[myResources];

	alloc2d = new int *[myThreads];
	max2d = new int *[myThreads];
	request = new int *[myThreads];


	//Is this appropriate for ever M and N size ?
	for (int i = 0; i < myResources; i++) {
		capacity[i] = 9;
		available[i] = capacity[i];
	}
	
	
	//Initializing appropriate 2d array's
	for (int i = 0; i < myThreads; ++i) {
		max2d[i] = new int[myResources];
		alloc2d[i] = new int[myResources];
		request[i] = new int[myResources];
	}
		
	//populating the max2d array with a random max
	for (int i = 0; i < myThreads; i++) {
		for (int j = 0; j < myResources; j++) {
			alloc2d[i][j] = 0;
			request[i][j] = 0;


			 
			max2d[i][j] = rand() % 9 +1;
			//cout << max2d[i][j] << " ";
		}
		//cout << endl;
	}


	//Creating threads
	cout << "creating threads: " << endl;
	for (int tid = 0; tid < myThreads; tid++) {
		cout << "Thread " << tid << " created" << endl;
		threadsArr[tid] = thread(threadFun, tid);
	}

	for (int tid = 0; tid < myThreads; tid++) {
		threadsArr[tid].join();
	}


	//Test cygwin input
	//cout << "You entered " << myThreads << myResources;

	for (int i = 0; i < myThreads; ++i) {
		delete [] max2d[i];
		delete [] alloc2d[i];
		delete [] request[i];
		delete [] need[i];
	}
	delete [] max2d;
	delete [] alloc2d;
	delete [] request;
	delete [] capacity;
	delete [] available;
	delete [] threadsArr;
	delete [] need;
	delete [] toBeAvailable;
	delete [] finish;
	
	return 0;
}