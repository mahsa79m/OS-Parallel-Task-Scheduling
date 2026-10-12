#include <iostream>
#include <fstream>
#include <windows.h>

using namespace std;

int main() {

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int numCPUs = sysinfo.dwNumberOfProcessors;
    ifstream file("input.txt");
    if (!file) {
        cout << "Can not open file!" << endl;
        return 1;
    }
    int tasks, workers;
    file >> tasks >> workers;

    int* durations = new int[tasks];
    for (int i = 0; i < tasks; ++i) {
        file >> durations[i];
    }

    int** ability = new int* [tasks];
    for (int i = 0; i < tasks; i++) {
        ability[i] = new int[workers];
    }
    for (int i = 0; i < tasks; ++i) {
        for (int j = 0; j < workers; ++j) {
            file >> ability[i][j];
        }
    }
    file.close();
    cout << "Enter time limit in seconds: ";
    int timeLimit;
    cin >> timeLimit;
    TCHAR Name1[] = TEXT("task");
    HANDLE MapObject1 = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4 * sizeof(int), Name1);
    if (MapObject1 == NULL) {
        cerr << "Error: CreateFileMapping1 failed" << endl;
        return 1;
    }
    int* Buffer_task = (int*)MapViewOfFile(MapObject1, FILE_MAP_ALL_ACCESS, 0, 0, 4 * sizeof(int));
    if (Buffer_task == NULL) {
        CloseHandle(MapObject1);
        cout << "Error: Buffer task failed" << endl;
        return 1;
    }

    Buffer_task[0] = tasks;
    Buffer_task[1] = workers;
    Buffer_task[2] = timeLimit;
    Buffer_task[3] = numCPUs;
    TCHAR Name2[] = TEXT("durations");
    HANDLE MapObject2 = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, tasks * sizeof(int), Name2);
    if (MapObject2 == NULL) {
        cerr << "Error: CreateFileMapping2 failed" << endl;
        return 1;
    }
    int* Buffer_durations = (int*)MapViewOfFile(MapObject2, FILE_MAP_ALL_ACCESS, 0, 0, tasks * sizeof(int));
    if (Buffer_durations == NULL) {
        CloseHandle(MapObject2);
        cerr << "Error: MapViewOfFile2 failed" << endl;
        return 1;

    }

    for (int i = 0; i < tasks; i++) {
        Buffer_durations[i] = durations[i];
    }

    TCHAR Name3[] = TEXT("ability");
    HANDLE MapObject3 = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (tasks * workers) * sizeof(int), Name3);
    if (MapObject3 == NULL) {
        cerr << "Error: CreateFileMapping3 failed" << endl;
        return 1;
    }
    int* Buffer_ability = (int*)MapViewOfFile(MapObject3, FILE_MAP_ALL_ACCESS, 0, 0, (tasks * workers) * sizeof(int));
    if (Buffer_ability == NULL) {
        CloseHandle(MapObject3);
        cerr << "Error: Buffer ability failed" << endl;
        return 1;
    }

    for (int i = 0; i < tasks; ++i) {
        for (int j = 0; j < workers; ++j) {
            Buffer_ability[i * workers + j] = ability[i][j];
        }
    }

    const int totalSize = (tasks * numCPUs + 3 * numCPUs);//max,min,max-min
    TCHAR Name4[] = TEXT("results");
    HANDLE MapObject4 = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, totalSize * sizeof(int), Name4);
    if (MapObject4 == NULL) {
        cerr << "Error: CreateFileMapping4 failed" << endl;
        return 1;
    }
    int* Buffer_results = (int*)MapViewOfFile(MapObject4, FILE_MAP_ALL_ACCESS, 0, 0, totalSize * sizeof(int));
    if (Buffer_results == NULL) {
        CloseHandle(MapObject4);
        cerr << "Error: Buffer results failed" << endl;
        return 1;
    }

    TCHAR Name5[] = TEXT("cpu");
    HANDLE MapObject5 = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, numCPUs * sizeof(int), Name5);
    if (MapObject5 == NULL) {
        cerr << "Error: CreateFileMapping5 failed" << endl;
        return 1;
    }

    int* Buffer_cpu = (int*)MapViewOfFile(MapObject5, FILE_MAP_ALL_ACCESS, 0, 0, numCPUs * sizeof(int));
    if (Buffer_cpu == NULL) {
        CloseHandle(MapObject5);
        cerr << "Error: MapViewOfFile5 failed" << endl;
        return 1;
    }

    STARTUPINFO* S = new STARTUPINFO[numCPUs];
    PROCESS_INFORMATION* P = new PROCESS_INFORMATION[numCPUs];

    for (int i = 0; i < numCPUs; i++) {
        ZeroMemory(&S[i], sizeof(S[i]));
        S[i].cb = sizeof(S[i]);
        ZeroMemory(&P[i], sizeof(P[i]));
        bool create = CreateProcess(TEXT("child.exe"), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &S[i], &P[i]);
        if (!create) {
            cerr << "Error: Failed to create process!" << endl;
            return 1;
        }
        Buffer_cpu[i] = P[i].dwProcessId;   
    }

    for (int i = 0; i < numCPUs; i++) {
        WaitForSingleObject(P[i].hProcess, INFINITE);
        CloseHandle(P[i].hProcess);
        CloseHandle(P[i].hThread);
    }

    int* assign = new int[tasks];
    int bestDiff = INT_MAX;
    int* maxAnswer = new int[numCPUs];
    int* minAnswer = new int[numCPUs];
  
    for (int i = 0; i < numCPUs; i++) {
        for (int j = 0; j < tasks; j++) {
            cout << "Task " << j + 1 << " assigned to worker " << Buffer_results[j + (tasks * i)] + 1 << endl;
        }
        cout << "Worker " << i + 1 << " - Best Difference: " << Buffer_results[(numCPUs * tasks) + i*3] << endl;
        cout << "Worker " << i + 1 << " - Max Time: " << Buffer_results[(numCPUs * tasks)  + i*3 + 1] << endl;
        cout << "Worker " << i + 1 << " - Min Time: " << Buffer_results[(numCPUs * tasks)  + i*3 + 2] << endl;

        if (Buffer_results[(numCPUs * tasks) + i * 3] < bestDiff)
            bestDiff = Buffer_results[(numCPUs * tasks) + i * 3];
    }

    cout << "\nBest Overall Answer (Minimum Difference): " << bestDiff<<endl;

    delete[] S;
    delete[] P;
    delete[] durations;
    delete[] ability;

    UnmapViewOfFile(Buffer_task);
    UnmapViewOfFile(Buffer_durations);
    UnmapViewOfFile(Buffer_ability);
    UnmapViewOfFile(Buffer_results);
    UnmapViewOfFile(Buffer_cpu);
  

    CloseHandle(MapObject1);
    CloseHandle(MapObject2);
    CloseHandle(MapObject3);
    CloseHandle(MapObject4);
    CloseHandle(MapObject5);

    return 0;
}
