#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>



int main(int argc, char *argv[]){

        // random number seed
        	srand(time(NULL) ^ getpid());
        
        
