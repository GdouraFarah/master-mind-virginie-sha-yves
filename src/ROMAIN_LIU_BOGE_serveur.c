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


// procedure charge de s'occuper d'un client en particulier
void fils(int fd_client){
	bool quitter = false;
	int lc;
	char* message = malloc(50*sizeof(char));
	char* message_reponse = malloc(100*sizeof(char));
	struct sockaddr adr_src;
	socklen_t lg_adr_cli;

	while(!quitter) {
		// attente d'un message du client
		lc = recvfrom (fd_client, message, 50, 0, &adr_src, &lg_adr_cli);
		message[lc] = '\0';
		
		// prise en comptede la demande de fermeture
		if (strcmp(message,"exit") == 0 )
			quitter = true;
		
		// traitement du message
		else {
			printf("message client : %s\n",message);
			*message_reponse = '\0';
			strcat(message_reponse,"bien reçu ");
			strcat(message_reponse,message);
			write(fd_client, message_reponse, strlen(message_reponse));
	    } 
	}
	
	// fermeture du fils
	exit(0);
}


void lire_entree(){
  /*TODO*/
}



int main(int argc, char *argv[])
{
	int sock;
	int fd_client;
   
	struct sockaddr_in adr;
	struct sockaddr addr_client;
	socklen_t lg_adresse_client;
	struct hostent *hostinfo = NULL;
   
	// vérification de la syntaxe de la commande
	if (argc != 2){
  		printf("Usage : %s numero-port\n", argv[0]);
    	exit(-1);
	}
  
	// creation du socket d'ecoute des connexions
	if ((sock=socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("Probleme socket\n");
		exit(-2);
	}
	  
	// recuperation des informations de l'hote
	hostinfo = gethostbyname("localhost");
	
	// preparation de l'adresse de la socket
	adr.sin_family = AF_INET;
	adr.sin_port = htons(atoi(argv[1]));
	adr.sin_addr = *(struct in_addr*)(hostinfo -> h_addr);
	
	// liaison de la socket
	if (bind(sock,(struct sockaddr *)&adr,sizeof(adr))  == -1){
		perror("Probleme bind fbind.sock\n");
		exit(-3);
	}
	
	// lancement du serveur
	if (listen(sock,10) == -1) {
		perror("Impossible d'écouter sur ce port");
		exit(-4);
	}
	printf("serveur en ecoute\n");
	
	// programme d'ecoute du serveur
	while(1){
		// Connexion d'un client
	    if ( (fd_client = accept(sock, &addr_client, &lg_adresse_client)) ) {
	    	// Creation d'un fils pour s'occuper du client
			int pid = fork();
			if (pid == 0)	// il s'agit du fils
				fils(fd_client);
			else if (pid == -1)	// le fils n'a pas pu etre cree
				perror("impossible de prendre en charge ce client");
		}
	}
	
	// attente de la fin des fils
	while (wait(0) != -1);
	
	// fermeture du socket
	close(sock);
	
	return 0;
}
