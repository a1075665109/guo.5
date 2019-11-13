#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

// max amount of lines and nano-sec to sec ratio
#define maxLine 10000
#define maxTimeNS 1000000000

// structure for resource descriptor
struct rd{
	int resources[20];		
	int available[20];
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
	int lines;
	int grant;
	int maxChild;
};


// variable the clock, prm, maxtime, the output file,rd.
struct clock* clk;
struct prm* prm;
char *outputFile;
struct rd* rd;
int maxTime;


// function for printing the allocated graph
void printTable(){
	FILE *fp;
	fp = fopen(outputFile,"a");
	int i =0;
	while (i<18){
		fprintf(fp,"Process %d: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",prm[i].pid,
			prm[i].allocated[0],prm[i].allocated[1],prm[i].allocated[2],prm[i].allocated[3],
			prm[i].allocated[4],prm[i].allocated[5],prm[i].allocated[6],prm[i].allocated[7],
			prm[i].allocated[8],prm[i].allocated[9],prm[i].allocated[10],prm[i].allocated[11],
			prm[i].allocated[12],prm[i].allocated[13],prm[i].allocated[14],prm[i].allocated[15],
			prm[i].allocated[16],prm[i].allocated[17],prm[i].allocated[18],prm[i].allocated[19]);
		i = i+1;
		clk->lines = clk->lines + 1;
	}	
	fprintf(fp,"\n");
	clk->lines +=1;
	fclose(fp);
	return;
}	

void printAllResource(){
	FILE *fp;
        fp = fopen(outputFile,"a");
        
        fprintf(fp,"Total resources : %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n\n",
			rd->resources[0],rd->resources[1],rd->resources[2],rd->resources[3],rd->resources[4],
 			rd->resources[5],rd->resources[6],rd->resources[7],rd->resources[8],rd->resources[9],
			rd->resources[10],rd->resources[11],rd->resources[12],rd->resources[13],rd->resources[14],
			rd->resources[15],rd->resources[16],rd->resources[17],rd->resources[18],rd->resources[19]);
        clk->lines = clk->lines + 1;
        fclose(fp);
        return;
}

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
	
	clk->lines=0;
        clk->sec=0;
        clk->nano_sec=0;
	clk->maxChild = 18;

	//initiating the process resource descriptor.
	int i = 0;
        while(i< 18){
        	int j =0;
		while (j<20){
			prm[i].maxClaim[j]=0;
			prm[i].allocated[j]=0;
			prm[i].needed[j]=0;
			j = j+1;
		}
		prm[i].pid=-1;
		i = i+1;
	}

	// random seed;
	time_t t;
        srand((unsigned)time(&t));
	// initializing the total amount of resources.
	i=0;
	while(i<20){
		rd->resources[i] = rand()%50+1;
		rd->available[i] = rd->resources[i];
		i = i+1;
	}

	// set up the shared memories.
	i =0;
	i=0;
	while(i<4){
		rd->shared[i] = rand()%21;
		i = i+1;
	}	
	// print all the resources to the output file
	printAllResource();

	int picked =0;
	unsigned int s;
        unsigned int ns;
	// oss running infinitely until signal handles max run time
	while(1){
		/*
		if(clk->grant >=20){
			printTable();
			clk->grant = clk->grant-20;
		}
		*/

		// if a child process has yet been picked , pick a time interval to fork
		if(picked == 0){
                        s = clk->sec;
                        ns = clk->nano_sec + rand()%(10000000000)+1;
                        if(ns>= maxTimeNS){
                                s = s+1;
                                ns = ns-maxTimeNS;
                        }
                        picked = 1;
                }	
		if(clk->lines >= 10000){
			alarm(0);
		}

		// clock continues to run, every loop increments the clock by 250000000 nano-seconds

                clk->nano_sec = clk->nano_sec +250000000;
                if(clk->nano_sec >= maxTimeNS){
                        clk->sec = clk->sec +1;
                        clk->nano_sec =0;
                }
		
		//if the amount of time interval has passed and it is time to start another process
                if(clk->sec >= s && picked == 1){
                        if(clk->nano_sec >= ns){
				picked = 0;		
				if(clk->maxChild>0){
					int i =0;
                                        while(i<18){
                                                if(prm[i].pid == -1){
                                                        break;
                                                }
                                                i+=1;
                                        }
                                        clk->maxChild = clk->maxChild -1;
					int child_pid = fork();
					if(child_pid <=0){
						execvp("./user",NULL);		
						exit(0);
                                        }else{
						prm[i].pid=child_pid;
					}
				}
			}
		}
	}

	return 0;
}
