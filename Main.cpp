#include <stdio.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include <string.h>
#include <sys/types.h>
#include <thread>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1



using namespace std;


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
fd_set writefds;
int totalWorkerCount = 0;
vector<Job> jobs;
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
        workers[indWorker].worker_type = worker_type-1;
        indWorker++;
        
        totalWorkerCount += worker_count;
        if(indWorker == 5) break;
    }
	
	

    
}


void WorkerProcess(int worker_type, int worker_id, int readfd,int writefd) {
	while(1) {
		char message[128];
		
		
		ssize_t bytesRead = read(readfd, message, sizeof(message) - 1);
		
		if (bytesRead == -1) {
			perror("Read failed");
			// return;
		} else if (bytesRead == 0) {
			std::cerr << "Pipe closed.\n";
		} else {
			message[bytesRead] = '\0';  // Null-terminate string
		}

		

		int job_duration = atoi(message);
		sleep(job_duration);
		
		
		char response[10] = "1";
		
		write(writefd,response,strlen(response));
		
	}
	
	close(readfd);
    close(writefd);
    
}


void available_worker() {
	
		FD_ZERO(&writefds);
		
		int max_fd = -1;
		
		for(auto workerProcess : workerProcesses) {
			for(auto w : workerProcess) {
				int fd = w.readfd[WRITE_END];
                if (fd == -1) continue; // Skip if the file descriptor is invalid
                if (fcntl(fd, F_GETFD) == -1) {
                    continue;  // Skip closed FDs
                }
				FD_SET(fd, &writefds);
			    if (fd > max_fd) max_fd = fd; // Track the max FD value
			}
		}
		
		

        int ret = select(max_fd + 1, NULL, &writefds, NULL, NULL);

		
        if (ret < 0) {
            std::cerr << "select() failed: " << strerror(errno) << std::endl;
        }
}


void sendJobToWorker(int ind) {
    ////check for available workers.......
        int worker_id = -1;
		Job job = jobs[ind];
		
       
        available_worker();
        while(worker_id == -1) {
            
            for(int  w = 0 ;w < workers[job.job_type - 1].worker_count ;w++) {
                if(FD_ISSET(workerProcesses[job.job_type - 1][w].readfd[WRITE_END], &writefds)) {
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
		write(writefd,message,strlen(message));
		
		char response[10];
		
		ssize_t bytesRead = read(readfd, response, sizeof(response) - 1);
		if(strcmp(response,"1") == 0){
			cout<<endl;
			cout<<"MAIN: job completed by: "<<job_type<<" and "<<worker_id + 1<<"th worker"<<endl;
		} else {
			cout<<endl;
			cout<<"MAIN: job failed by: "<<job_type<<" and "<<worker_id + 1<<"th worker"<<endl;
		}
}

void MainProcess() {
	
	for(int wtype = 0; wtype < 5; wtype++) {
        for (int j = 0; j < workers[wtype].worker_count; j++) {
            close(workerProcesses[wtype][j].readfd[READ_END]);
            close(workerProcesses[wtype][j].writefd[WRITE_END]); 
        }
    }
	
	
    char line[100];
    while(fgets(line, sizeof(line), stdin) != NULL) {
        int job_type;
        int job_duration; // in seconds
        sscanf(line, "%d %d", &job_type, &job_duration);
		
		Job job;
		job.job_type = job_type;
		job.job_duration = job_duration;
		jobs.push_back(job);
		int ind = jobs.size() - 1;
		thread workerInstance(sendJobToWorker,ind);
        workerThreads.push_back(move(workerInstance));
	}


    
		
    
    for(auto &workerThread : workerThreads) {
        if(workerThread.joinable()) {
            workerThread.join();
        }
    }
    
    for(int wtype = 0; wtype < 5; wtype++) {
        for (int j = 0; j < workers[wtype].worker_count; j++) {
            
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
            if(pipe(workerProcesses[wtype][j].readfd) == -1){
				cout<<"pipe creation failed."<<endl;
			}
            if(pipe(workerProcesses[wtype][j].writefd) == -1){
				cout<<"pipe creation failed."<<endl;
			}
			int childread = workerProcesses[wtype][j].readfd[READ_END];
			int childwrite = workerProcesses[wtype][j].writefd[WRITE_END];
            pid_t pid ;
            if ((pid = fork()) == 0) {
				close(workerProcesses[wtype][j].readfd[WRITE_END]);
				close(workerProcesses[wtype][j].writefd[READ_END]);
                WorkerProcess(wtype, j,childread,childwrite);
                return;
            } else if(pid < 0) {
                perror("Fork failed");
                return;
            }
            workerProcesses[wtype][j].pid = pid;
           
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
