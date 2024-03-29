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
#include <regex.h>
#include <fcntl.h>
#include <time.h>



#define TAILLE_MAX_DONNEES  1024
#define CHAINE_MAX 100



// le client joue contre le serveur
int ia(int fd_client){
	int tours_restants = 12;
	int combinaison[5];
	int i,lc,j;
	bool quitter=false;
	int tentative[5];
	int verification = 0;
	char reponse_tentative[6];
	bool trouve;
	int control; 

	printf("combinaison a deviner par le client: ");
	// initialisation de la combinaison a deviner
	for(i=0;i < 5;i++){
		srand(i*time(NULL));
		combinaison[i]=rand()%10;
		printf(" %d ",combinaison[i]);
	}       
	printf("\n");

	//      le client essaie de deviner le code genere
	while ((tours_restants > 0) && (! quitter)){
		verification = 0;

		// on attend l'essaie du client
		lc = read(fd_client,tentative,sizeof(tentative));

		// verification de la tentative du client               
		for(i=0;i<5;i++){
			trouve = false;
			if(combinaison[i]==tentative[i]){
				reponse_tentative[i]='R';
				verification++; 
			}
			else {
				j=0;
				while ((j<5)&&(! trouve)){
					if((tentative[i] == combinaison[j])&&(i != j)){
						trouve = true;
						reponse_tentative[i]='B';
					} 
					j++;
				}
				if (! trouve){
					reponse_tentative[i]='V';
				}
			}
		}

		if(verification == 5){
			quitter = true;
		}
		tours_restants--;
		reponse_tentative[5]='\0';
		write(fd_client, reponse_tentative, sizeof(reponse_tentative));
		read(fd_client,&control,sizeof(control));
		write(fd_client,&tours_restants,sizeof(tours_restants));
	}

	return EXIT_SUCCESS;
}


// procedure de jeu contre un autre joueur
int duel(int fd_client, char* adversaire, int* tube, char* role){
	
	int lc, tours_restant = 12;
	bool termine = false;
	char chaine[CHAINE_MAX];
	char message[CHAINE_MAX];
	regex_t regex_essai;
	regex_t regex_notation;
	
		
	// on prepare la regex de control du format
	if (regcomp (&regex_essai, "^[0-9]{5}$", REG_NOSUB) == 0) {
	
		// client devant chercher une solution
		if (strcmp(role,"chercheur") == 0) {
			// on attend que le colleur ait choisit une combinaison
			read(tube[0], message, CHAINE_MAX);
			while (strcmp(message, "combinaison_ok") != 0)
				read(tube[0], message, CHAINE_MAX);
		
			// on informe le chercheur que la combinaison est choisit
			write(fd_client, message, strlen(message));
	
			// tant que le colleur ne dit pas que la combinaisons
			// est trouve, on transmet les messages en verifiant le format
			while ( !termine ) {
				lc = read (fd_client, message, CHAINE_MAX);	
				message[lc] = '\0';
			
				// format correct
				if (regexec(&regex_essai, message, 0, NULL, 0) == 0) {
				
					// transmition
					write(tube[1], message, strlen(message));
					tours_restant--;
					sleep(1);
				
					// attente reponse
					read(tube[0], message, CHAINE_MAX);
				
					// analyse de la reponse
					if (strcmp(message, "gagne") == 0 || tours_restant == 0) {
						termine = true;
					}
				
					// transmition
					write(fd_client, message, strlen(message));
					sleep(1);
				
				} else {
					write(fd_client, "erreur de format du message : le code doit être une suite de 5 chiffres", 72);
				}
			}
		
			// gagne
			if (strcmp(message, "gagne") == 0) {
				sprintf(chaine, "BRAVO ! vous avez trouve en %i essais", 12 - tours_restant);
				write(fd_client, chaine, strlen(chaine));
			}
		
			//nombre de tours max ecoule
			else {
				// recuperation du code
				read(tube[0], message, CHAINE_MAX);
				sprintf(chaine, "dommage :(, vous n'avez pas reussi a trouver %s", message);
				write(fd_client, chaine, strlen(chaine));
			}
		}
	
		// client devant proposer une combinaison
		else {
			char combinaison[CHAINE_MAX];
			
			// on demande la combinaison
			write(fd_client, "combinaison", 12);
			lc = read (fd_client, combinaison, CHAINE_MAX);	
			combinaison[lc] = '\0';
			
			// tant que le format est incorrect, on redemande
			while (regexec(&regex_essai, combinaison, 0, NULL, 0) != 0) {
				write(fd_client, "combinaison", 12);
				lc = read (fd_client, combinaison, CHAINE_MAX);	
				combinaison[lc] = '\0';
			}
			
			// on informe le chercheur que la combinaison est choisit
			write(tube[1], "combinaison_ok", 14);
			
			// on prepare la regex de control du format de notation
			if (regcomp (&regex_notation, "^[VBR]{5}$", REG_NOSUB) == 0) {
			
				// tant que le colleur ne dit pas que la combinaisons
				// est trouve, on transmet les messages en verifiant le format
				while ( !termine ) {
					read (tube[0], message, CHAINE_MAX);
					
					// transmition
					write(fd_client, message, strlen(message));
					tours_restant--;
					sleep(1);
					
					// attente reponse
					read(fd_client, message, CHAINE_MAX);
			
					// on redemande tant que le format est incorrect
					while (regexec(&regex_notation, message, 0, NULL, 0) != 0 && strcmp(message, "gagne") != 0) {
						write(fd_client, "erreur de format du message : le code doit être une suite de 5 caracteres apartenant a {V,B,R}", 95);
						sleep(1);
						read(fd_client, message, CHAINE_MAX);
					}
					
					// analyse de la reponse
					if (strcmp(message, "gagne") == 0 || tours_restant == 0) {
						termine = true;
					}
				
					// transmition
					write(tube[1], message, strlen(message));
					sleep(1);
				}
		
				// gagne
				if (strcmp(message, "gagne") == 0) {
					sprintf(chaine, "%s a trouve le code en %i essais", adversaire, 12 - tours_restant);
					write(fd_client, chaine, strlen(chaine));
				}
		
				//nombre de tours max ecoule
				else {
					// recuperation du code
					write(tube[1], combinaison, 6);
					sprintf(chaine, "BRAVO ! Vous avez reussi a coller %s", adversaire);
					write(fd_client, chaine, strlen(chaine));
				}
			} else {
				printf("erreur lors de la création de la regex de control des notations");
			}
	
			// on libere la memoire utilisee pour la regex
			regfree(&regex_notation);
		}
	} else {
		printf("erreur lors de la création de la regex de control des essais");
	}
	
	// on libere la memoire utilisee pour la regex
	regfree(&regex_essai);
	
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

	char message[CHAINE_MAX];
	char pseudo_adv[CHAINE_MAX];
	char pseudo[CHAINE_MAX];
	char no_tel[CHAINE_MAX];
	char role[CHAINE_MAX];
	char role_adv[CHAINE_MAX];
	
	// recuperation du pseudo
	lc = read (fd_client, pseudo, 15);	
	pseudo[lc] = '\0';		
	
	printf("joueur= %s\n",pseudo);
	
	char bienvenue[60];
	sprintf(bienvenue, "Bienvenue %s", pseudo);
	fflush(stdout);
	
	write(fd_client, bienvenue, sizeof(bienvenue)); 
	// reception du mode de jeu
	lc = read (fd_client,message, CHAINE_MAX);	
	message[lc] = '\0';
	
	// prise en compte de la demande de fermeture
	if (strcmp(message,"exit") == 0)
		exit(0);
	
	// jeu solo
	else if (strcmp(message,"solitaire") == 0){
		printf("mode de jeu en solitaire choisit par %s\n", pseudo);
		ia(fd_client);
	}
	// jeu en duel
	else if (strcmp(message,"duel") == 0) {
		printf("mode de jeu en duel choisit par %s\n", pseudo);
		
		/*
		 * Il semblerait que les tubes soit bloque jusqu'à ce qu'un 
		 * autre processus les ouvre dans le mode oppose ou qu'il ne
		 * soit pas possible de les utilisé dans les deux sens pour le
		 * meme processus. Nous ne disposont plus du temps necessaire
		 * a un changement de solution et laissons de cote le jeu en reseau.
		 */
		
		// On met une chaine vide dans le tube pour etre sur de recuperer qq chose
		write(tube_pub[1], " ", 2);printf("ecrit");
		
		// on regarde s'il y a un joueur de disponible
		read(tube_pub[0], message, CHAINE_MAX);
			printf("test");
		if (strcmp(message, " ") == 0) {	// aucun joueur disponible
			printf("aucun joueur dispo");
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
			lc = recvfrom (fd_client, message, CHAINE_MAX, 0, &adr_src, &lg_adr_cli);
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
			sleep(1);
			write(tube_priv[1], role_adv, strlen(role_adv));
			
			// on lance le programme de jeu
			duel(fd_client, pseudo_adv, tube_priv, role);
		}
		
		// il y a un joueur disponible
		else {
			printf("il y a un joueur dispo");
			// on recupere le no du tube prive
			tube_priv[0] = atoi(strtok(message, "-"));
			tube_priv[1] = atoi(strtok(NULL, "-"));
			
			// on lui envoie notre pseudo
			write(tube_priv[1], pseudo, strlen(pseudo));
			
			// on recupere le sien
			read(tube_priv[0], pseudo_adv, 15);
			
			// on recupere notre role
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
	socklen_t lg_adresse_client = sizeof(struct sockaddr_in);
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
		perror("Impossible d'écouter sur ce port\n");
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
		 fd_client = accept(sock, (struct sockaddr *) &addr_client, &lg_adresse_client); 
		 if ( fd_client != -1) {    	
			// Creation d'un fils pour s'occuper du client
			int pid = fork();
			if (pid == 0){	// il s'agit du fils
				printf("connexion avec le client établie..\n");
				fils(fd_client, tube);
			}
			if (pid == -1){	// le fils n'a pas pu etre cree
				perror("impossible de prendre en charge ce client\n");
			}
		}else{
			printf("erreur connexion avec le client...\n");
		}
	}
	
	// attente de la fin des fils
	while (wait(0) != -1);
	

	// fermeture du socket
	close(sock);
	
	return 0;
}
