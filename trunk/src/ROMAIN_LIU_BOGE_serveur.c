#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <strings.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>



#define TAILLE_MAX_DONNEES  1024

void fils(int fd_client){
	bool quitter = false;
	int lc;
	char* message = malloc(50*sizeof(char));
	char *message_reponse = malloc(100*sizeof(char));

	while(!quitter) {
		lc = recvfrom (fd_client, message, 50, 0,NULL,NULL);
		message[lc] = '\0';
		if (strcmp(message,"exit") == 0 )
			quitter = true;
		else {
			printf("message client : %s\n",message);
			*message_reponse = '\0';
			strcat(message_reponse,"bien reçu ");
			strcat(message_reponse,message);
			write(fd_client, message_reponse, strlen(message_reponse));
	    } 
	}
	exit(0);
}

void lire_entree(){
  /*TODO*/
}

int main(int argc, char *argv[])
{
    int sock;
   
    struct sockaddr_in adr;
   
   if (argc != 2){
     printf("Usage : %s numero-port\n", argv[0]);
      exit(-1);
   }
   
  if ((sock=socket(AF_INET,SOCK_STREAM,0)) == -1){
      perror("Probleme socket\n");
      exit(-2);
  }
  
  struct hostent *hostinfo = NULL;
  hostinfo = gethostbyname("localhost");
 
  adr.sin_family = AF_INET;
  adr.sin_port = htons(atoi(argv[1]));
  adr.sin_addr = *(struct in_addr*)(hostinfo -> h_addr);
  
  //bcopy("fbind.sock",&adr.sin_addr,strlen("fbind.sock"));
  if (bind(sock,(struct sockaddr *)&adr,sizeof(adr))  == -1){
      perror("Probleme bind fbind.sock\n");
      exit(-3);
  }

  if (listen(sock,10) == -1) {
      perror("Impossible d'écouter sur ce port");
      exit(-4);
  }
  
  printf("serveur en ecoute\n");
 
  int fd_client;
  //int lc;
  struct sockaddr addr_client;
  socklen_t lg_adresse_client;
	while(1){
	    if ( (fd_client = accept(sock, &addr_client, &lg_adresse_client)) )
	    {	   
		int pid = fork();
		if (pid == 0){
		    fils(fd_client);
		} else if (pid == -1)
		    perror("impossible to launch the son !");
	    }
	}
	while (wait(0) != -1);
  close(sock);
  return 0;
}
