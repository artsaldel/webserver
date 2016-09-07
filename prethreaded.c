#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <assert.h>
      
#include <sys/socket.h>     
#include <sys/types.h>    
#include <unistd.h>  

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"

struct http 
{
   char *uri;
   char *body;
   enum 
   {
      GET, PUT, DELETE
   } method;
   enum 
   {
      OK, NOT_FOUND
   } status;
};

//funcion para saber el largo de un archivo
int fsize(FILE *fp){
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET);
    return sz;
}

static char const *http_status[2] = {"200 OK", "404 NOT FOUND"};

//Here goes the port number
static char const PORT[6] = "8039";

char ROOT[1024];
int sock_fd;

void start_server();
void *eloop(int* slot);
void respond(int, struct http);
void cleanup(int);

struct http parse_request(int);
char* concat(char*, char*);

//Here are the amount of workers
int totalWorkers;



int main(int argc, char *argv[]) 
{
	assert(argc == 2);

	totalWorkers = atoi(argv[1]);

	int worker;
	struct sockaddr_in addr_in;

	if (getcwd(ROOT, sizeof(ROOT)) == NULL) 
	{
 	perror("Could not determine currect directory");
   	exit(EXIT_FAILURE);
	}

	start_server();


   pthread_t *tid = malloc( totalWorkers * sizeof(pthread_t) );

	for (int worker_count = 0; worker_count < totalWorkers; worker_count++) 
	{
   	worker = pthread_create(&tid[worker_count], NULL, eloop, worker_count);
   	// Failed to thread worker
   	if (worker < 0) 
   	{  
      	perror("Threading failed");
   	} 
      else if (worker == 0) 
      {  
         printf("New thread in slot %d\n", worker_count);
         
      }
	}

   for(int worker_count = 0; worker_count < totalWorkers; worker_count++)
   {
      pthread_join( tid[worker_count], NULL );
      _Exit(EXIT_SUCCESS);
   }

	// Wait for every children processes to die
	while (waitpid(-1, NULL, 0) > 0);

	close(sock_fd);
	return 0;
}


void start_server() 
{
   	struct addrinfo hints, *res, *r;

   	// See getaddrinfo(2) for more info
   	hints.ai_family = AF_INET;
   	hints.ai_socktype = SOCK_STREAM;
   	hints.ai_flags = AI_PASSIVE;

   	// Get addrinfo for socket creation and binding
   	if (getaddrinfo(NULL, PORT, &hints, &res) != 0) 
   	{
     	perror("Error with getaddrinfo");
     	exit(EXIT_FAILURE);
   	}

   	// Try to create and bind socket with on of the addrinfo
   	for (r = res; r != NULL; r = r->ai_next) 
   	{
      	sock_fd = socket(r->ai_family, r->ai_socktype, 0);
      	if (sock_fd < 0)
         	continue;  // Could not create socket
      	if (bind(sock_fd, r->ai_addr, r->ai_addrlen) == 0)
         	break;  // Socket bound!
   	}

   	if (r != NULL) 
   	{
      	printf("Socket created and bound!\n");
   	} 
   	else 
   	{
      	perror("Could not create or bind socket");
      	exit(EXIT_FAILURE);
   	}

   	freeaddrinfo(res);

   	// Listen for incoming connections
   	// listen()
   	if (listen(sock_fd, totalWorkers) < 0) 
   	{
      	perror("Error while trying to listen");
      	exit(EXIT_FAILURE);
   	}
}


void *eloop(int *slot) 
{
	struct sockaddr_in addr_in;
	socklen_t addrlen = sizeof(addr_in);

	for(;;) 
	{
   	int csock_fd = accept(sock_fd, (struct sockaddr *) &addr_in, &addrlen);

   	if (csock_fd < 0) 
   	{
      	perror("Could not accept incoming request");
   	} 
   	else 
   	{
      	printf(COLOR_YELLOW "Worker %d: incoming request!" COLOR_RESET "\n", slot);
   	}

   	respond(csock_fd, parse_request(csock_fd));
   	cleanup(csock_fd);
	}
}


struct http parse_request(int csock_fd) 
{
   	int buffer_size = 1024;
   	char *buffer = malloc(buffer_size);
   	struct http request;
   	struct stat file_info;

   	recv(csock_fd, buffer, buffer_size, 0);

   	char *http_method;
   	char *http_uri;

   	http_method = strtok(buffer, " ");
   	request.uri = strtok(NULL, " ");

   	if (strcmp(http_method, "GET") == 0) 
   	{
      	request.method = GET;
   	} 
   	else if (strcmp(http_method, "PUT") == 0) 
   	{
      	request.method = PUT;
   	} 
   	else if (strcmp(http_method, "DELETE") == 0) 
   	{
      	request.method = DELETE;
   	}

   	printf("  Method: %s\n", http_method);
   	printf("  URI: %s\n", request.uri);
   	printf("  Path: %s%s\n", ROOT, request.uri);

   	return request;
}


void respond(int csock_fd, struct http request) 
{
   struct http response;
   struct stat file_info;
   char hstatus[32];
   char hlength[20];

   if (lstat(concat(ROOT, request.uri), &file_info) == 0) 
   {
      response.status = OK;
      response.body = "<html><body><h1>Yay!</h1></body></html>";
      printf(COLOR_GREEN "200 OK" COLOR_RESET "\n");
   } 
   else 
   {
      response.status = NOT_FOUND;
      response.body = "<html><body><h1>File not found</h1></body></html>";
      printf(COLOR_RED "404 NOT FOUND" COLOR_RESET "\n");
   }

   snprintf(hstatus, sizeof(hstatus), "HTTP/1.1 %s\n", http_status[response.status]);
   snprintf(hlength, sizeof(hlength), "Content-length: %d\n", strlen(response.body));

   if (response.status == OK) 
   {
      	FILE *f = fopen(concat(ROOT, request.uri), "r");
      	int size  = fsize(f);
      	char c;
      	do
      	{
      		c = fgetc(f);
      		if (feof(f))
      			break;
      		write(csock_fd, &c, 1);
      		size--;
      	}while(size > 0);

      	fclose(f);
   	}

   	// Respond to client, writing to socket descriptors
   	write(csock_fd, hstatus, sizeof(hstatus));
   	write(csock_fd, hlength, sizeof(hlength));
   	write(csock_fd, "Content-Type: text/html\n\n", 25);
   	write(csock_fd, response.body, strlen(response.body));

}

void cleanup(int csock_fd) 
{
   // Using SHUT_RDWR instead of SHUT_WR to disallow transmission and reception
   shutdown(csock_fd, SHUT_RDWR);
   close(csock_fd);
}


char* concat(char *s1, char *s2) 
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1);
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}