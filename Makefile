all: oss user

clean:
	-rm oss logfile user

dt:
	gcc -o oss.c oss 
	gcc -o user.c user
