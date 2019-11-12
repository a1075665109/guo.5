#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define maxLine 10000
// structure for resource descriptor
struct rd{
	int resources[20];		
	int avaliable[20];
	int shared[4];
};

// structure for for processes to keep tract of all resources allocated, max claim, and needed.
struct prm{
	int maxClaim[20];
	int allocated[20];
	int needed[20];
        int pid;
};

// clock in shared memory
struct clock{
        unsigned int sec;
        unsigned int nano_sec;
};


// variable the clock, pcb, maxtime and the output file.
struct clock* clk;
struct prm* prm;
char *outputFile;
int maxTime;
struct rd* rd;
int maxTime;

// function for printing the allocated graph




// alarm handler that kills all processes
void alarmHandler(int sig){
        FILE *fp;
        fp = fopen(outputFile,"a");
        fprintf(fp,"End of program, all processes terminated after max",maxTime);
        fclose(fp);
        int i=0;
        while(i <18){
                if(prm[i].pid>0){
                        kill(prm[i].pid,SIGTERM);
                }
                i=i+1;
        }
        printf("\nProgram terminated after %d seconds, review results in logFile\n",maxTime);
        kill(getpid(),SIGTERM);
}

int main(int argc, char *argv[]){
	// indicate is verbose is on or off
	int vflag = 0;
	int opt;
	outputFile = "logfile";
	maxTime =5;
	// sorts through command line options
	while((opt = getopt(argc,argv,":hv"))!=-1){
                switch(opt){
                        case 'h':
                                printf("\n-h is used for listing the available command line arguments\n");
                                printf("the command line options are -h, -t and -v\n");
				printf("use -t command followed by an argument to change the max run time\n");
                                printf("use -v command to turn on verbose\n");
                                printf("\nprogram terminated\n\n");
                                return 0;
                        case 'v':
                                vflag = 1;
                                break;
                        case 't':
				maxTime = atoi(optarg);
				break;
			case '?':
                                printf("\n Invalid arguments, please use option -h for help, program terminated\n");
                                return 0;
                }
        }

	alarm(maxTime);
	// initiate process resource management;
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

	// initiate the resource descriptor.
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

	// shared memory of clock
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
        clk->sec=0;
        clk->nano_sec=0;

	//initiating the process resource descriptor.
	int i = 0;
        while(i< 18){
        	int j =0;
		while (j<20){
			prm[i].maxClaim[j]=0;
			prm[i].allocated[j]=0;
			prm[i].needed[j]=0;
			prm[i].pid=-1;
			j = j+1;
		}
		i = i+1;
	}

	// random seed;
	time_t t;
        srand((unsigned)time(&t));
	// initializing the total amount of resources.
	i=0;
	while(i<20){
		rd->resources[i] = rand()%21;
		i = i+1;
	}

	// set up the shared memories.
	i =0;
	while(i<4){
                rd->shared[i] = 0;
        	i = i+1;
	}
	i=0;
	while(i<4){
		rd->shared[i] = rand()%21;
		i = i+1;
	}	











	return 0;


}
