#include <stdio.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <string.h>
#include <sys/types.h>
#include <thread>

#define READ_END 0
#define WRITE_END 1



using namespace std;
// TODO: add error checks
// TODO: add accurate comments

class worker {
    public:
        int readfd[2];
        int writefd[2];
        int pid;
        worker() {
            readfd[0] = -1;
            readfd[1] = -1;
            writefd[0] = -1;
            writefd[1] = -1;
        }
};


class workerType {
    public:
        int worker_type;
        int worker_count;
};

class Job {
	public:
		int job_type;
		int job_duration;
};

vector<thread> workerThreads;
workerType workers[5];
vector<worker> workerProcesses[5];
fd_set readfds;
int totalWorkerCount = 0;
atomic<bool> stopThread(false);



void CollectWorkerData(){
   
    int indWorker = 0;
    char line[100];
    while(fgets(line, sizeof(line), stdin) != NULL) {
        int worker_type;
        int worker_count;
        sscanf(line, "%d %d", &worker_type, &worker_count);
        workerProcesses[worker_type-1].resize(worker_count);
        workers[indWorker].worker_count = worker_count;
        workers[indWorker].worker_type = worker_type;
        indWorker++;
        
        totalWorkerCount += worker_count;
        if(indWorker == 5) break;
    }

    
}


void WorkerProcess(int worker_type, int worker_id) {
	
	// Close repective ends of the pipes in the parent process
	close(workerProcesses[worker_type][worker_id].readfd[WRITE_END]);
    close(workerProcesses[worker_type][worker_id].writefd[READ_END]);
	
	int readfd = workerProcesses[worker_type][worker_id].writefd[WRITE_END];
	int writefd = workerProcesses[worker_type][worker_id].readfd[READ_END];
	
	while(1) {
		char message[128];
		
		cout<<"Waiting for job by: "<<worker_type<<" id: "<<worker_id<<endl;
		
		ssize_t bytesRead = read(readfd, message, sizeof(message) - 1);
		
		cout<<"job received by: "<<worker_type<<" id: "<<worker_id<<endl;

		int job_duration = atoi(message);
		
		sleep(job_duration);
		
		cout<<"sleep completed by: "<<worker_type<<" id: "<<worker_id<<endl;
		
		char response[10] = "1";
		
		write(writefd,response,strlen(response));
		
		cout<<"job sent by: "<<worker_type<<" id: "<<worker_id<<endl;
	}
	
	close(readfd);
    close(writefd);
    
}


void available_worker() {
	while(!stopThread) {
		FD_ZERO(&readfds);
		
		int max_fd = -1;
		
		for(auto workerProcess : workerProcesses) {
			for(auto w : workerProcess) {
				int fd = w.readfd[READ_END];
				FD_SET(fd, &readfds);
			    if (fd > max_fd) max_fd = fd; // Track the max FD value
			}
		}
		

		struct timeval timeout;
		timeout.tv_sec = 1;   // 1 seconds timeout
		timeout.tv_usec = 0; 

		int ret = select(max_fd + 1, &readfds, NULL, NULL, &timeout);

		if (ret == -1) {
			perror("select failed");
		} else if (ret == 0) {
			std::cout << "Timeout occurred! No data available.\n";
		}
	}

}

void sendJobToWorker(Job job) {
    ////check for available workers.......
        int worker_id = -1;
        while(worker_id == -1) {
            for(int  w = 0 ;w < workers[job.job_type - 1].worker_count ;w++) {
                if(FD_ISSET(workerProcesses[job.job_type - 1][w].readfd[READ_END], &readfds)) {
                    worker_id = w;
                    break;
                }
            }
        } 
		
		int job_type = job.job_type;
        int job_duration = job.job_duration;
		char message[128];
		sprintf(message,"%d",job_duration);
		
		int readfd = workerProcesses[job_type - 1][worker_id].writefd[READ_END];
		int writefd = workerProcesses[job_type - 1][worker_id].readfd[WRITE_END];
		cout<<"job sent to: "<<job_type<<" id: "<<worker_id<<endl;
		write(writefd,message,strlen(message));
		
		char response[10];
		
		ssize_t bytesRead = read(readfd, response, sizeof(response) - 1);
		if(strcmp(response,"1") == 0){
			cout<<"job completed by: "<<job_type<<" id: "<<worker_id<<endl;
		} else {
			cout<<"job failed by: "<<job_type<<" id: "<<worker_id<<endl;
		}
}

void MainProcess() {
    printf("Main Process\n");
	
	thread t(available_worker);
    
	
    char line[100];
    while(fgets(line, sizeof(line), stdin) != NULL) {
        int job_type;
        int job_duration; // in seconds
        sscanf(line, "%d %d", &job_type, &job_duration);
		
		Job job;
		job.job_type = job_type;
		job.job_duration = job_duration;
		thread workerInstance(sendJobToWorker,job);
        workerThreads.push_back(move(workerInstance));
	}


    int process = 0;

    while(process < totalWorkerCount){
        for(auto workerProcess : workerProcesses) {
			for(auto w : workerProcess) {
				kill(w.pid, SIGTERM);
                process++;
                if(process == totalWorkerCount) {
                    break;
                }
			}
		}
    }



		
    stopThread = true;
    t.join();
    for(auto &workerThread : workerThreads) {
        if(workerThread.joinable()) {
            workerThread.join();
        }
    }
    for(int wtype = 0; wtype < 5; wtype++) {
        for (int j = 0; j < workers[wtype].worker_count; j++) {
            //kill(workerProcesses[wtype][j].pid, SIGTERM);
            close(workerProcesses[wtype][j].readfd[WRITE_END]);
            close(workerProcesses[wtype][j].writefd[READ_END]);
        }
    }
		
    
}

void startWorkerProcess() {

    for(int wtype = 0; wtype < 5; wtype++) {
        if(workers[wtype].worker_count <= 0) continue;
        for (int j = 0; j < workers[wtype].worker_count; j++) {
			
			
            // Create pipes for each worker process
            pipe(workerProcesses[wtype][j].readfd);
            pipe(workerProcesses[wtype][j].writefd);

            pid_t pid ;
            if ((pid = fork()) == 0) {
                WorkerProcess(wtype, j);
                return;
            } else if(pid < 0) {
                perror("Fork failed");
                return;
            }
            workerProcesses[wtype][j].pid = pid;
            // Close repective ends of the pipes in the parent process
            close(workerProcesses[wtype][j].readfd[READ_END]);
            close(workerProcesses[wtype][j].writefd[WRITE_END]); 
        }
    }

    MainProcess();

}


int main() {


    // step 1.1 Collecting worker data

    CollectWorkerData();

    // step 1.2 and 2.1 Starting worker processes creating pipes

    
    startWorkerProcess();
    
    

    return 0;
}

// 1 1
// 2 3
// 3 1
// 4 1
// 5 1
// 1 3
// 2 5
// 1 2
// 4 7
// 3 1
// 5 1
// 1 5