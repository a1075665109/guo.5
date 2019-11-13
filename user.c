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
	int maxChild;
};

struct clock* clk;
struct prm* prm;
char *outputFile = "logfile";
struct rd* rd;
int maxTime = 3;
int childSpot;
FILE *fp;


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


void alarmHandler(int sig){
	fp = fopen(outputFile,"a");
        fprintf(fp,"Process %d terminated, released resources: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",getpid(),
			prm[childSpot].allocated[0],prm[childSpot].allocated[1],prm[childSpot].allocated[2],prm[childSpot].allocated[3],
			prm[childSpot].allocated[4],prm[childSpot].allocated[5],prm[childSpot].allocated[6],prm[childSpot].allocated[7],
			prm[childSpot].allocated[8],prm[childSpot].allocated[9],prm[childSpot].allocated[10],prm[childSpot].allocated[11],
			prm[childSpot].allocated[12],prm[childSpot].allocated[13],prm[childSpot].allocated[14],prm[childSpot].allocated[15],
			prm[childSpot].allocated[16],prm[childSpot].allocated[17],prm[childSpot].allocated[18],prm[childSpot].allocated[19]);
	fclose(fp);
        int i=0;
        while(i <20){
		rd->available[i] = rd->available[i] + prm[childSpot].allocated[i];
		prm[childSpot].allocated[i]=0;
		prm[childSpot].needed[i] = 0;
		prm[childSpot].maxClaim[i] =0;	
      		prm[childSpot].pid=-1;
		i = i+1;
	}
	clk->maxChild =clk->maxChild +1;
        kill(getpid(),SIGTERM);
}





int main(int argc, char *argv[]){

	printf("A child process has started\n\n");
	srand(time(NULL) ^ getpid());
	
	fp =fopen("logfile","a");
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
		childSpot = locate(pid);

	int i =0;

	
	while(i<20){
		// randomly choosing a max Claim based on what is available
		int a = rand()%rd->available[i] +1;
		
		if(a>10){
			a=a/10;
		}
		prm[childSpot].maxClaim[i] = a;
		
			
		// randomly choose a allocated amount between 0 to the max claim
		prm[childSpot].allocated[i] = rand()%prm[childSpot].maxClaim[i];
		
		// update the available resource by decuting the allocated
		rd->available[i] = rd->available[i] - prm[childSpot].allocated[i];
	
		// find out the needed resource by subtracting the allocated from the max claim
		prm[childSpot].needed[i] = prm[childSpot].maxClaim[i] - prm[childSpot].allocated[i];
		i+=1;
		printTable();
	}
	
	alarm(maxTime);
	
	float b = rand()%10;
        float a = b/10;	
	int c=0;
	int d =0;
	while(c < 35){
		c+=1;	
		// sleep for the bound amount of time before requesting or releasing a resource
		sleep(a);
		
		

		// 80% chance of requesting and 20% chance of releasing
		int c = rand()%100;
		if(c >20){
			int need = 21;
			// pick a random resource out of the 20 resoures to request
			while(need == 21){
				i = rand()%20;
				if(prm[childSpot].needed[i] > 0){	
					need = i;
				}
			}
		
			// check to see if resource is granted
			int granted = 0;
			while (granted ==0){
				if(rd->available[need] >0){
					prm[childSpot].allocated[need] +=1;
					prm[childSpot].needed[need] -=1;
					rd->available[need] -=1;
					clk->grant +=1;
					fprintf(fp,"Granted one resource of resource type %d to process %d\n\n",need,getpid());
					clk->lines +=1;				
					granted =1;
					if(clk->grant>=5){
						printTable();
						clk->grant-=5;
					}
				}
			}	
		}else{
			// randomly release one resource from the ones this process is allocated
			int release =21;
			while(release == 21){
				i = rand()%20;
				if(prm[childSpot].allocated[i]>0){
					release = i;
				}
			}
			prm[childSpot].allocated[release]-=1;
			prm[childSpot].needed[release]+=1;
			rd->available[release]+=1;
			fprintf(fp,"Released one resource of resource type %d from process %d\n\n",release,getpid());		
			d +=1;
			fclose(fp);
			if(d>=3){
				printTable();
			}
			fp = fopen("logfile","a");
			d-=3;
		}	



	}
	return 0;
}        
