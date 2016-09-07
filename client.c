#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h> 
#include <stdlib.h> 

#include <time.h>
#include <pthread.h>

void *Client (void *k);
void CreateRequests();

char *host, *filename;
int port, nThreads, nCycles;


int main (int argc, char *argv[])
{
    assert(argc == 6);
    
    host = argv[1];
    port = atoi(argv[2]);
    filename = argv[3];
    nThreads = atoi(argv[4]);
    nCycles = atoi(argv[5]);

    //printf("%i\n",port);

    //Create log file with each server transcurred time
    FILE *log = fopen("ServerLog.txt", "a");
    fprintf(log, "%s %s %i %s %i %i\n", "Request = Client", host, port, filename, nThreads, nCycles);

    //In FIFO, it needs "get /<filename> HTTP/1.1"
    if (port == 8031 || port == 8032 || port == 8033 || port == 8034 || port == 8035){
    	char *append[strlen(filename) + 14];
    	
    	strcat(append, "GET /");
    	strcat(append, filename);
    	strcat(append, " HTTP/1.1");

    	strcpy(filename, append);
    }

    //fclose(log);

    //Start time
    clock_t start = clock();
    //Do request
    CreateRequests();
    //Stop timer
	clock_t end = clock();
	long double diff = (long double)(end - start) / CLOCKS_PER_SEC * 100;
	//Print in log file
	fprintf(log, "Time taken %Lf seconds %Lf milliseconds\n\n\n", diff, diff * 1000);
	//Colse file
	fclose(log);

    return 0;
}

void CreateRequests()
{

	pthread_t *tid = malloc( nThreads * sizeof(pthread_t) );

	for(int i=0; i<nThreads; i++ ) 
    	pthread_create( &tid[i], NULL, Client, NULL );

    for(int i=0; i<nThreads; i++ ) 
    	pthread_join( tid[i], NULL );
}
    
void *Client(void *k)
{
	for (int i = 0; i < (int)nCycles; i++)
	{	
		printf("Request\n\n");
		int r, s;
	    struct sockaddr_in sin;
	    char buf[1024];
	    
	    /* Setup the socket */
	    s = socket (AF_INET, SOCK_STREAM, 0);
	    
	    /* Make the connection */
	    bzero (&sin, sizeof (sin));
	    sin.sin_family = AF_INET;
	    sin.sin_port = htons (port);
	    inet_aton (host, &sin.sin_addr);
	    connect (s, (struct sockaddr *) &sin, sizeof (sin));
	    
	    /* Write the filename */
	    write (s, filename, strlen (filename));
	    write (s, "\n", 1);
	    
	    /* Send the bytes that come back to stdout */
	    while ((r = read (s, buf, sizeof (buf))) > 0){
	        //write (1, buf, r);
	    }
	    
	    /* Finish out */
	    close (s);
	}
}