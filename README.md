# IPC Job Dispatcher

## Overview
This project is a screening assignment for the Software Development Engineer - Systems role. It implements a job dispatcher using Inter-Process Communication (IPC) with multiple worker processes. The main process distributes jobs to worker processes based on job type and manages communication efficiently using pipes and the `select` system call.

## Features
- **Worker Process Creation**: Spawns worker processes to handle specific job types.
- **Inter-Process Communication**: Uses pipes for communication between the dispatcher and worker processes.
- **Job Scheduling**: Assigns jobs dynamically based on worker availability.
- **Concurrency Management**: Uses `select` to handle multiple worker processes efficiently.

## Build and Run
### Requirements
- GCC (for C++) or Clang  
- Linux-based OS (for process management and `select` system call)

### Steps
1. Clone the repository:
   ```sh
   git clone <repository-url>
   cd <repository-folder>
   ```
2. Build the project:
   ```sh
   make build
   ```
3. Run the program:
   ```sh
   ./dispatcher < input.txt
   ```
   - The input file should contain worker configurations followed by job assignments.

## Input Format
1. **Worker Configuration** (First 5 lines):
   ```
   <worker_type> <worker_count>
   ```
2. **Job Input** (After worker config):
   ```
   <job_type> <job_duration>
   ```

## Example Input
```
1 1
2 3
3 1
4 1
5 1
1 3
2 5
1 2
4 7
3 1
5 1
1 5
```

## License
This project is for evaluation purposes only.

