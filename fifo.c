//compilar con gcc "gcc server.c -o server"
//ejecutar "./server"
// escribir en el chrome "localhost:8032"
// escribir en el chrome "localhost:8032/index.html"

#include<netinet/in.h>    
#include<stdio.h>    
#include<stdlib.h>    
#include<sys/socket.h>    
#include<sys/stat.h>    
#include<sys/types.h>    
#include<unistd.h>  
#include<string.h>  


//funcion para saber el largo de un archivo
int fsize(FILE *fp){
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET);
    return sz;
}


    
int main() {  
	// se crean los sockets  
   	int create_socket, new_socket;    
   	socklen_t addrlen;    
   	int bufsize = 1024;  
	int filesize = 100;  
   	char *buffer = malloc(bufsize); 
	char *filerequest = malloc(filesize);       

 	//se crea el socket que se encarga de escuchar
   	if ((create_socket = socket(AF_INET, SOCK_STREAM, 0)) > 0)
   	{
   		printf("The socket was created\n");
   	}
    
   	struct sockaddr_in address;
   	address.sin_family = AF_INET;    
   	address.sin_addr.s_addr = INADDR_ANY;    
   	address.sin_port = htons(8032);    
 
	//se asocia el socket de escucha con el puerto
   	if (bind(create_socket, (struct sockaddr *) &address, sizeof(address)) == 0)
   	{
   		printf("Binding Socket\n");
   	}
        
	//se esperan llamadas
   	while (1) {  
      	if (listen(create_socket, 10) < 0) {    
         	perror("server: listen");    
         	exit(1);    
    	}    
    	//se acceptan las llamadas entrantes y se manda a otro socket a atender la peticion
		addrlen = sizeof(struct sockaddr_in);
      	if ((new_socket = accept(create_socket, (struct sockaddr *) &address, &addrlen)) < 0) {    
         	perror("server: accept");    
         	exit(1);    
      	}    
    
      	if (new_socket > 0){    
         	printf("The Client is connected...\n");
      	}
		//se leen los parametros del cliente para ver que pide
      	recv(new_socket, buffer, bufsize, 0);    
      	printf("%s\n", buffer);   
	
		//se parsea el nombre del archivo que quiere
		char *dest = strstr(buffer, "HTTP/1.1");
		int pos;
		pos = dest - buffer ;
		strncpy(filerequest, buffer + 5, pos - 5);
		char *dnuevo = strstr(filerequest, " ");
		int largo;
		largo = dnuevo - filerequest;
	 	char *namefile = malloc(largo);
		strncpy(namefile, filerequest, largo);
		strcpy(filerequest, "0000000000000000000000000000000000000000000000");
 
		printf("Archivo = %s \n", namefile ); 


		//se intenta abrir el archivo que quiere
		FILE *f = fopen(namefile, "r");
		
		//si el archivo no existe le envia una pagina de error
		if (f == NULL)
		{
	  		write(new_socket, "HTTP/1.1 200 OK\n", 16);
		    write(new_socket, "Content-length: 46\n", 19);
		    write(new_socket, "Content-Type: text/html\n\n", 25);
		    write(new_socket, "<html><body><H1>File Error!</H1></body></html>",46);  
		    close(new_socket); 
		}
		else
		{
			//si el archivo existe le envia el archivo
			int size = fsize(f);
			char c; 
			do
			{
				c = fgetc(f);
				if( feof(f) )
				{
					break ;
				}
				write(new_socket, &c, 1);
				size=size-1;
				printf("largo = %d \n", size );
			}while(size > 0);


			fclose(f);
			close(new_socket); 
		}
   	}    
	
	close(create_socket);    
	return 0;    
}



