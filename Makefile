CFLAGS=-I /home/arturo/Escritorio/WebServer
OBJETOS=fifo.c 
FUENTES=fifo.c

MiEjecutable: $(OBJETOS) 
	gcc $(OBJETOS) -o fifo 
	./fifo

depend: 
	makedepend $(CFLGAS) $(FUENTES)
