all: oss

clean:
	-rm oss user logFile

dt:
	gcc -o oss.c oss 
	gcc -o user.c user
