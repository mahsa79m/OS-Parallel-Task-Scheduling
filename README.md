# Parallel Task Scheduling in C++

A C++ task scheduling project that explores workload balancing through randomized task allocation using multithreading and multiprocessing.

The project includes two implementations: a multithreaded version using the Windows API and a multiprocess version using shared memory for inter-process communication.

## Features

- Randomized assignment of tasks to eligible workers
- Workload calculation for each worker
- Search for balanced task allocation
- Multithreading using Windows API
- Multiprocessing using parent and child processes
- Shared memory communication between processes
- Time-limited search for task assignments

## Technologies Used

- C++
- Windows API
- Multithreading (`CreateThread`)
- Multiprocessing (`CreateProcess`)
- Shared Memory
- File I/O

## Project Structure

```text
OS-Parallel-Task-Scheduling/
├── multithreading/
│   └── main.cpp
├── multiprocessing/
│   ├── parent.cpp
│   └── child.cpp
└── README.md
```

## How It Works

The program reads task durations and worker eligibility information from an input file.

Each task is assigned to a worker who is eligible to perform it. The program evaluates randomized assignments and calculates the total workload of each worker.

The scheduling objective is to reduce the difference between the maximum and minimum worker workloads.

### Multithreading Implementation

The multithreaded version uses Windows API threads to explore task assignments within a single process.

The implementation uses `CreateThread` to create threads that perform randomized scheduling.

### Multiprocessing Implementation

The multiprocessing version consists of two programs:

- **Parent:** Reads the input data, creates shared memory regions, and launches child processes.
- **Child:** Accesses the shared memory and evaluates randomized task assignments.

The implementation uses Windows API functions, including `CreateProcess`, `CreateFileMapping`, and `OpenFileMapping`.

## Running the Project

**Requirements**

- Windows operating system
- C++ compiler with Windows API support
- Visual Studio or a compatible development environment

**Multithreading**

1. Compile `multithreading/main.cpp`.
2. Place the required `input.txt` file in the working directory.
3. Run the executable.

**Multiprocessing**

1. Compile `multiprocessing/child.cpp` as `child.exe`.
2. Compile `multiprocessing/parent.cpp`.
3. Place `child.exe` and the required `input.txt` file in the working directory.
4. Run the parent executable.

## Notes

The project uses randomized scheduling to search for balanced task assignments. It does not implement an exact optimization algorithm.

The original scheduling algorithms and thread/process management approaches are preserved.
