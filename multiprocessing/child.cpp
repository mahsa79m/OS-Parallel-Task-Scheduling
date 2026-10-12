#include <windows.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <limits.h>
using namespace std;

void getMinMax(int* totalsum, int workers, int& min_val, int& max_val) {
	min_val = INT_MAX;
	max_val = INT_MIN;

	for (int i = 0; i < workers; i++) {
		if (totalsum[i] < min_val) {
			min_val = totalsum[i];
		}
		if (totalsum[i] > max_val) {
			max_val = totalsum[i];
		}
	}
}
int main() {

	TCHAR Name1[] = TEXT("task");
	HANDLE MapObject1 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, Name1);
	if (MapObject1 == NULL) {
		cerr << "Failed to open shared memory1!" << endl;
		return 1;
	}
	int* Buffer_task = (int*)MapViewOfFile(MapObject1, FILE_MAP_ALL_ACCESS, 0, 0, 4 * sizeof(int));
	if (Buffer_task == NULL) {
		CloseHandle(MapObject1);
		cerr << "MapViewOfFile1 failed" << endl;
		return 1;
	}

	int tasks = Buffer_task[0];
	int workers = Buffer_task[1];
	int timeLimit = Buffer_task[2];
	int numCPUs = Buffer_task[3];
	
	int* durations = new int[tasks];
	TCHAR Name2[] = TEXT("durations");
	HANDLE MapObject2 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, Name2);
	if (MapObject2 == NULL) {
		cerr << "Failed to open shared memory2!" << endl;
		return 1;
	}
	int* Buffer_durations = (int*)MapViewOfFile(MapObject2, FILE_MAP_ALL_ACCESS, 0, 0, tasks * sizeof(int));
	if (Buffer_durations == NULL) {
		CloseHandle(MapObject2);
		cerr << "MapViewOfFile2 failed" << endl;
		return 1;
	}

	for (int i = 0; i < tasks; i++) {
		durations[i] = Buffer_durations[i];
	}
	
	int** ability = new int* [tasks];
	for (int i = 0; i < tasks; i++) {
		ability[i] = new int[workers];
	}
	TCHAR Name3[] = TEXT("ability");
	HANDLE MapObject3 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, Name3);
	if (MapObject3 == NULL) {
		cerr << "Failed to open shared memory3!" << endl;
		return 1;
	}
	int* Buffer_ability = (int*)MapViewOfFile(MapObject3, FILE_MAP_ALL_ACCESS, 0, 0, (tasks * workers) * sizeof(int));
	if (Buffer_ability == NULL) {
		CloseHandle(MapObject3);
		cerr << "MapViewOfFile3 failed" << endl;
		return 1;
	}

	for (int i = 0; i < tasks; ++i) {
		for (int j = 0; j < workers; ++j) {
			ability[i][j] = Buffer_ability[i * workers + j];
		}
	}
	
	const int totalSize = (tasks * numCPUs + 3 * numCPUs);
	TCHAR Name4[] = TEXT("results");
	HANDLE MapObject4 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, Name4);
	if (MapObject4 == NULL) {
		cerr << "Failed to open shared memory4" << endl;
		return 1;
	}
	int* Buffer_results = (int*)MapViewOfFile(MapObject4, FILE_MAP_ALL_ACCESS, 0, 0, totalSize * sizeof(int));
	if (Buffer_results == NULL) {
		CloseHandle(MapObject4);
		cerr << "MapViewOfFile4 failed" << endl;
		return 1;
	}

	TCHAR Name5[] = TEXT("cpu");
	HANDLE MapObject5 = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, Name5);
	if (MapObject5 == NULL) {
		cerr << "Failed to open shared memory5" << endl;
		return 1;
	}
	int* Buffer_cpu = (int*)MapViewOfFile(MapObject5, FILE_MAP_ALL_ACCESS, 0, 0, numCPUs * sizeof(int));
	if (Buffer_cpu == NULL) {
		CloseHandle(MapObject5);
		cerr << " MapViewOfFile5 failed" << endl;
		return 1;
	}

	int myPid = GetCurrentProcessId();
	int id;
	bool found = false;
	for (int i = 0; i < numCPUs; i++) {

		if (Buffer_cpu[i] == myPid) {
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

	srand(time(NULL) + GetCurrentProcessId());
	time_t startTime = time(0);
	int bestDiff = INT_MAX;
	int* bestAssignments = new int[tasks];
	int* totalsum = new int[workers];
	int* assign = new int[tasks];
	int randWorker;
	int maxAnswer = INT_MIN;
	int minAnswer = INT_MAX;
	while (difftime(time(0), startTime) < timeLimit) {
		for (int i = 0; i < workers; i++)
			totalsum[i] = 0;

		for (int i = 0; i < tasks; i++) {
			do {
				randWorker = rand() % workers;
			} while (ability[i][randWorker] == 0);
			totalsum[randWorker] += durations[i];
			assign[i] = randWorker;
		}
		int min_value = totalsum[0], max_value = totalsum[0];

		for (int i = 1; i < workers; i++) {
			if (totalsum[i] > max_value) max_value = totalsum[i];
			if (totalsum[i] < min_value) min_value = totalsum[i];
		}


		int diff = max_value - min_value;

		if (diff < bestDiff) {
			bestDiff = diff;
			maxAnswer = max_value;
			minAnswer = min_value;
			for (int i = 0; i < tasks; i++) {
				bestAssignments[i] = assign[i];
			}
		}
	}
	for (int i = 0; i < tasks; i++) {
		Buffer_results[i + (tasks * id)] = bestAssignments[i];
	}

	Buffer_results[(numCPUs * tasks) + id*3] = bestDiff;
	Buffer_results[(numCPUs * tasks) + id*3 + 1] = maxAnswer;
	Buffer_results[(numCPUs * tasks) + id*3 + 2] = minAnswer;


	delete[] durations;
	delete[] ability;
	delete[] totalsum;
	delete[] assign;
	delete[] bestAssignments;

	UnmapViewOfFile(Buffer_task);
	UnmapViewOfFile(Buffer_durations);
	UnmapViewOfFile(Buffer_ability);
	UnmapViewOfFile(Buffer_results);
	return 0;
}