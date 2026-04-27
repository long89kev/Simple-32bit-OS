# Simple 32-bit OS Simulation

This project is a simplified 32-bit operating system simulation, primarily designed for educational purposes in the Operating Systems course (CO2018) at Ho Chi Minh City University of Technology (VNU-HCM).

## Key Features

The simulation covers the following core OS components:

* **CPU Simulator**: Executes basic process instructions read from input files, including calculations (`calc`), memory allocation (`alloc`), deallocation (`free`), memory access (`read`/`write`), and system calls (`syscall`).
* **Process Scheduling**:
    * Implements a **Multi-Level Queue (MLQ)** scheduling algorithm.
    * Manages process states (Ready, Running, etc.) through various queue structures.
    * Supports multiple CPUs running concurrently using POSIX threads.
* **Memory Management**:
    * Simulates **Paging** mechanisms and Virtual Memory Area (VMA) management.
    * Includes a physical memory (RAM) model and secondary storage for Swap space.
    * Implements Page Table mapping and tracking for each process.
* **System Calls**: Provides an interface for essential kernel services defined in a syscall table:
    * `sys_listsyscall`: Lists available system calls.
    * `sys_memmap`: Handles memory mapping operations.
    * `sys_killall`: Terminates processes by name.

## Directory Structure

* `src/`: Contains C source files for the CPU, memory management, scheduler, loader, and system calls.
* `include/`: Contains header files defining system configurations, data structures, and APIs.
* `input/`: 
    * System configuration files that initialize the number of CPUs, time slots, and memory sizes (e.g., `os_1_mlq_paging`).
    * `input/proc/`: Script files defining the logic and instructions for simulated processes.
* `output/`: Contains log files showing execution traces, scheduling results, and memory dumps.
* `Makefile`: Build script for automating compilation using `gcc`.

## Getting Started

### 1. Prerequisites
Ensure you have `gcc` and `make` installed on your Linux/Unix environment. The project uses the `pthread` library for multi-threading.

### 2. Compilation
To compile the entire project, run:
```bash
make all
```
To compile only the main OS module:
```bash
make os
```
This will create an `obj/` directory for object files and generate the `os` executable.

### 3. Running the Simulation
Run the OS executable by providing a configuration file from the `input/` directory:
```bash
./os <config_file_name>
```
Example for running a paging simulation with MLQ:
```bash
./os os_1_mlq_paging
```

### 4. Cleaning Up
To remove compiled object files and executables:
```bash
make clean
```
