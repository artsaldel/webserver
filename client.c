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

int client (char *host, int port, char *filename);

int main (int argc, char *argv[])
{
    char *host, *filename;
    int port, r;
    
    assert(argc == 4);
    
    host = argv[1];
    port = atoi(argv[2]);
    filename = argv[3];

    //printf("%i\n",port);

    //In FIFO, it needs "get /<filename> HTTP/1.1"
    if (port == 8031 || port == 8033){
    	char *append[strlen(filename) + 14];
    	
    	strcat(append, "GET /");
    	strcat(append, filename);
    	strcat(append, " HTTP/1.1");

    	strcpy(filename, append);
    }

    printf("%s\n",filename);
    
    r = client (host, port, filename);
    return r;
}
    
int client (char *host, int port, char *filename)
{
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
        write (1, buf, r);
    }
    
    /* Finish out */
    close (s);
    return 0;
}