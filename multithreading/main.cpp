#include <iostream>
#include <Windows.h>
#include <fstream>
#include <ctime>
#include <cstdlib>  
#include <climits> 

using namespace std;

int tasks, workers;
int* durations;
int** ability;
int timeLimit;
int* assign;
int* sum_work;
int* results;
int* Buffer_results;
int* Tid;

DWORD WINAPI child(LPVOID param) {

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int numCPUs = sysinfo.dwNumberOfProcessors;

	DWORD threadId = GetCurrentThreadId();

	srand(static_cast<unsigned int>(time(0)) + threadId);
	int bestDiff = INT_MAX;
	int maxAnswer = INT_MIN;
	int minAnswer = INT_MAX;

	int* bestAssignments = new int[tasks];
	int* totalsum = new int[workers];
	int randWorker;

	time_t startTime = time(NULL);
	while (time(NULL) - startTime < timeLimit) {
		int* sum = new int[workers];
		int* assign_worker = new int[tasks];

		for (int i = 0; i < workers; i++) {
			sum[i] = 0;
		}

		for (int i = 0; i < tasks; i++) {
			do {
				randWorker = rand() % workers;
			} while (ability[i][randWorker] == 0);
			sum[randWorker] += durations[i];
			assign_worker[i] = randWorker;
		}

		int max_value = INT_MIN;
		int min_value = INT_MAX;
		for (int i = 0; i < workers; i++) {
			if (sum[i] > max_value) max_value = sum[i];
			if (sum[i] < min_value) min_value = sum[i];
		}
		int diff = max_value - min_value;

		if (diff < bestDiff) {
			bestDiff = diff;
			maxAnswer = max_value;
			minAnswer = min_value;
			for (int i = 0; i < tasks; i++) {
				bestAssignments[i] = assign_worker[i];
			}
			for (int i = 0; i < workers; i++) {
				totalsum[i] = sum[i];
			}
		}
		delete[] sum;
		delete[] assign_worker;
	}

	int id;
	bool found = false;
	for (int i = 0; i < numCPUs; i++) {

		if (Tid[i] == threadId) {
			id = i;
			found = true;
			break;
		}
		else if (i == numCPUs - 1)
			i = -1;
	}
	if (!found) {
		cerr << "Process ID not found it!" << endl;
		return 1;
	}

	for (int i = 0; i < tasks; i++) {
		assign[i + (tasks * id)] = bestAssignments[i];
	}
	for (int i = 0; i < workers; i++) {
		sum_work[i + (workers * id)] = totalsum[i];
	}
	results[id] = bestDiff;
	Buffer_results[id * 2] = maxAnswer;
	Buffer_results[(id * 2) + 1] = minAnswer;

	delete[] bestAssignments;
	delete[] totalsum;

	return 0;
}

int main() {
	cout << "Enter time limit in seconds: ";
	cin >> timeLimit;

	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	int numCPUs = sysinfo.dwNumberOfProcessors;

	ifstream file("input.txt");
	if (!file) {
		cout << "Cannot open file!" << endl;
		return 1;
	}

	file >> tasks >> workers;

	durations = new int[tasks];
	for (int i = 0; i < tasks; i++) {
		file >> durations[i];
	}

	ability = new int* [tasks];
	for (int i = 0; i < tasks; i++) {
		ability[i] = new int[workers];
		for (int j = 0; j < workers; j++) {
			file >> ability[i][j];
		}
	}
	file.close();

	assign = new int[tasks * numCPUs];
	sum_work = new int[workers * numCPUs];
	results = new int[numCPUs];
	Buffer_results = new int[numCPUs * 2];

	Tid = new int[numCPUs];
	HANDLE* threads = new HANDLE[numCPUs];
	for (int i = 0; i < numCPUs; i++) {
		threads[i] = CreateThread(NULL, 0, child, NULL, 0, NULL);
		Tid[i] = GetThreadId(threads[i]);
	}
	for (int i = 0; i < numCPUs; i++) {
		WaitForSingleObject(threads[i], INFINITE);
		CloseHandle(threads[i]);
	}

	int bestDiff = INT_MAX;
	int child1 = -1;

	for (int m = 0; m < numCPUs; m++) {
		cout << "child " << m + 1 << endl;
		for (int i = 0; i < tasks; i++) {
			cout << "work " << i + 1 << " assigned to worker " << assign[i + (m * tasks)] + 1 << endl;
		}
		for (int i = 0; i < workers; i++) {
			cout << "worker " << i + 1 << " --> " << sum_work[i + (m * workers)] << endl;
		}

		int max_value = Buffer_results[m * 2];
		int min_value = Buffer_results[(m * 2) + 1];
		cout << "\tmax = " << max_value << endl;
		cout << "\tmin = " << min_value << endl;
		cout << "------------------" << endl;
		cout << "max - min = " << max_value - min_value << endl;
		cout << "_______________________________" << endl;

		if (results[m] < bestDiff) {
			bestDiff = results[m];
			child1 = m + 1;
		}
	}
	cout << "\nBest Overall Answer:" << "\nMax - Min = " << bestDiff << " for child " << child1 << endl;

	delete[] durations;
	delete[] ability;
	delete[] assign;
	delete[] sum_work;
	delete[] results;
	delete[] Buffer_results;
	delete[] threads;

	return 0;
}