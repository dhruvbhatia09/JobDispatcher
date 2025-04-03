#include <stdio.h>
#include <unistd.h>
#include <string.h>

// TODO: add error checks

struct worker {
    int worker_type;
    int readfd[2];
    int writefd[2];
};

int max_fd = 0;

struct workerType {
    int worker_type;
    int worker_count;
};
struct workerType workers[5];
struct worker **workerProcesses;



int CollectWorkerData(){
    int maxWorker = 0;
    int indWorker = 0;
    char line[100];
    while(fgets(line, sizeof(line), stdin) != NULL) {
        sscanf(line, "%d %d", &workers[indWorker].worker_type, &workers[indWorker].worker_count);
        indWorker++;
        maxWorker += workers[indWorker].worker_count;

        if(indWorker == 5) break;
    }

    return maxWorker;
}


void WorkerProcess(int worker_type, int worker_count) {
    printf("Worker Type: %d, Worker Count: %d\n", worker_type, worker_count);
}

void MainProcess() {
    printf("Main Process\n");
    // Main process logic here
}

void startWorkerProcess() {

    for(int wtype = 0; wtype < 5; wtype++) {
        if(workers[wtype].worker_count <= 0) continue;
        for (int j = 0; j < workers[wtype].worker_count; j++) {
            pipe(workerProcesses[]);
            pid_t pid ;
            if ((pid = fork()) == 0) {
                WorkerProcess(workers[wtype].worker_type, workers[wtype].worker_count);
                return;
            } else if(pid < 0) {
                perror("Fork failed");
                return;
            }
        }
    }

    MainProcess();

}


int main() {


    // step 1.1 Collecting worker data

    int maxWorker = CollectWorkerData();

    // step 1.2 Starting worker processes

    workerProcesses = (struct worker **)malloc(maxWorker*sizeof(struct worker));
    if (workerProcesses == NULL) {
        perror("Failed to allocate memory for worker processes");
        return 1;
    }

    startWorkerProcess();
    
    

    return 0;
}