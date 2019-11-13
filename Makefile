all: oss

clean:
	-rm oss logfile

dt:
	gcc -o oss.c oss 
