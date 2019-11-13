#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

struct rd{
        int resources[20];
        int available[20];
        int shared[4];
};

struct prm{
        int maxClaim[20];
        int allocated[20];
        int needed[20];
        int pid;
};


struct clock{
        unsigned int sec;
        unsigned int nano_sec;
        int lines;
        int grant;
};

struct clock* clk;
struct prm* prm;
char *outputFile = "logfile";
struct rd* rd;

int locate(int a){
        int i = 0;
        while (i<18){
                if(prm[i].pid == a){
                        return i;
                }
                i = i+1;
        }
        return 0;
}



int main(int argc, char *argv[]){

	// access process resource memory descripter in shared memory
	int prmid;
        int prmsize = 18 * sizeof(prm);
        prmid = shmget(0x1234,prmsize,0666|IPC_CREAT);
        if(prmid == -1){
                perror("Shared memory\n");
                return 0;
        }
        prm = shmat(prmid,NULL,0);
        if(prm == (void *)-1){
                perror("Shared memory attach\n");
                return 0;
        }

	// access total resource descripter shared memory
	int rdid;
        int rdsize = sizeof(rd);
        rdid = shmget(0x2234,rdsize,0666|IPC_CREAT);
        if(rdid == -1){
                perror("Shared memory\n");
                return 0;
        }
        rd = shmat(rdid,NULL,0);
        if(rd == (void *)-1){
                perror("Shared memory attach\n");
                return 0;
        }
	
	// access clock structure in shared memory
	int clockid;
        int clocksize = sizeof(clk);
        clockid = shmget(0x3234,clocksize,0666|IPC_CREAT);
        if(clockid == -1){
                perror("Shared memory\n");
                return 0;
        }
        clk = shmat(clockid,NULL,0);
        if(clk== (void *)-1){
                perror("Shared memory attach\n");
                return 0;
        }

	// get the id and then find the location of this process in the prm
		int pid = getpid();
		int childSpot = locate(pid);

	


	return 0;
}        
