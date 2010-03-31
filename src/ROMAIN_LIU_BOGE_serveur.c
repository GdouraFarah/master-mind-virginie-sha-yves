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




int ia(int fd_client){
	return EXIT_SUCCESS;
}

int duel(int fd_client, char* adversaire, int* tube, char* role){
	return EXIT_SUCCESS;
}


void lire_entree(){
  /*TODO*/
}


// procedure charge de s'occuper d'un client en particulier
void fils(int fd_client, int* tube_pub){
	int lc;
	int tube_priv[2];
	struct sockaddr adr_src;
	socklen_t lg_adr_cli;
	
	char* message = malloc(50*sizeof(char));
	//char* message_reponse = malloc(100*sizeof(char));
	char* pseudo_adv = malloc(15*sizeof(char));
	char* pseudo = malloc(15*sizeof(char));
	char* no_tel;
	char* role;
	char* role_adv;
	
	
	// recuperation du pseudo
	lc = recvfrom (fd_client, pseudo, 15, 0, &adr_src, &lg_adr_cli);
	pseudo[lc] = '\0';
	
	// reception du mode de jeu
	lc = recvfrom (fd_client, message, 50, 0, &adr_src, &lg_adr_cli);
	message[lc] = '\0';
	
	// prise en comptede la demande de fermeture
	if (strcmp(message,"exit") == 0)
		exit(0);
	
	// jeu solo
	else if (strcmp(message,"solo") == 0)
		ia(fd_client);
	
	// jeu en duel
	else if (strcmp(message,"duel") == 0) {
	
		// on regarde s'il y a un joueur de disponible
		if (read(tube_pub[0], message, 50) == 0) {	// aucun joueur disponible
			// on cree un tube prive
			if (pipe(tube_priv) != 0) {
				perror("pipe");
				exit(-1);
			}
			// preparation du "no de tel"
			sprintf(no_tel, "%i-%i", tube_priv[0], tube_priv[1]);
		
			// ecriture du no prive dans le tube publique
			write(tube_pub[1], no_tel, strlen(no_tel));
		
			// on signal au joueur qu'il va devoir patienter
			// et on lui demande ses preferences de jeu
			// (envoi d'un pseudo vide)
			write(fd_client, "", strlen(""));
		
			// on recupere les preferences de jeu,
			// et on attend un autre joueur
			lc = recvfrom (fd_client, message, 50, 0, &adr_src, &lg_adr_cli);
			message[lc] = '\0';
			if (strcmp(message,"chercheur") == 0) {	// chercheur
				sprintf(role, "%s", message);
				sprintf(role_adv, "colleur");
			} else {
				sprintf(role, "colleur");
				sprintf(role_adv, "%s", message);
			}
			read(tube_priv[0], pseudo_adv, 15);
			
			// on lui envoie notre pseudo en echange, puis son role
			write(tube_priv[1], pseudo, strlen(pseudo));
			write(tube_priv[1], role_adv, strlen(role_adv));
			
			// on lance le programme de jeu
			duel(fd_client, pseudo_adv, tube_priv, role);
		}
		
		// il y a un joueur disponible
		else {
			// on recupere le no du tube prive
			tube_priv[0] = atoi(strtok(message, "-"));
			tube_priv[1] = atoi(strtok(NULL, "-"));
			
			// on lui envoie notre pseudo
			write(tube_priv[1], pseudo, strlen(pseudo));
			
			// on recupere le sien
			read(tube_priv[0], pseudo_adv, 15);
			
			// on recupere notre role
			role = malloc(10*sizeof(char));
			read(tube_priv[0], role, 10);
			
			// on lance le programme de jeu
			duel(fd_client, pseudo_adv, tube_priv, role);
		}
	}
	
	// fermeture du fils
	exit(0);
}



int main(int argc, char *argv[])
{
	int sock;
	int fd_client;
	
	// tube permettant aux fils de s'echanger des no de tel :) pour jouer ensemble
	int tube[2];
   
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
	
	// creation du tube de discution entre les fils
	if (pipe(tube) != 0) {
		perror("pipe");
		exit(-5);
	}
	
	// programme d'ecoute du serveur
	while(1){
		// Connexion d'un client
	    if ( (fd_client = accept(sock, &addr_client, &lg_adresse_client)) ) {
	    	// Creation d'un fils pour s'occuper du client
			int pid = fork();
			if (pid == 0)	// il s'agit du fils
				fils(fd_client, tube);
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
