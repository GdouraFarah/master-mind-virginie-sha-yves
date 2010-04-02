#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdbool.h>
#include <regex.h>

#define CHAINE_MAX 100


// le client joue contre le serveur
int duel(int fd_client){
	int lc, resultat;
	char message[CHAINE_MAX];
	
	lc = read(fd_client,message,sizeof(message));
	message[lc] = '\0';
	
	// demande de selection des preference de jeu
	if (strcmp(message, "") == 0) {
		printf("Préférez vous commencer à chercher (1) ou à proposer un code (2) ?\n");
		scanf("%d",&resultat);
		while (resultat != 1 && resultat != 2){
			printf("\nVous devez entrer '1' ou '2' suivant vos préférences\n");
			scanf("%d",&resultat);
		}
		
		// on envois le role au serveur
		if (resultat == 1) {
			write(fd_client, "chercheur", 10);
		} else {
			write(fd_client, "colleur", 8);
		}
	}
	
	return EXIT_SUCCESS;
}

// le client joue contre le serveur
int ia(int fd_client){
	char message[CHAINE_MAX];
	int combinaison[5];
	char resultat[CHAINE_MAX];
	int lc, i, res, tours_restants = 12;
	char succes[6]={'R','R','R','R','R','\0'};
	int control = 0;
	regex_t regex_essai;
	char* chaine_regex = "^[:digit:]{5}$";


	// on prepare la regex de control du format
	if (regcomp (&regex_essai, chaine_regex, REG_NOSUB | REG_EXTENDED) == 0) {
	
		// on initialise la variable message
		sprintf(message, "VVVVV");
		
		// on envoie au serveur la combinaison
		do{
			// on demande au client sa tentative de combinaison
			printf("client> combinaison?\n");
			scanf("%s", resultat);
			// format correct
			// la regex ne trouve jamais de correspondance :( retour aux anciennes methodes
			// res = regexec(&regex_essai, resultat, 0, NULL, 0);
			res = REG_NOMATCH; i = 0;
			while (i < 5 && resultat[i] >= '0' && resultat[i] <= '9' ) {
				i++;
			}
			if (i == 5 && resultat[5] == '\0')
				res = 0;
			
			if ( res == 0 ) {
				// stockage de la combinaison pour envoyer
				for(i = 0 ; i < 5 ; i++) {  
					combinaison[i] = resultat[i] - '0';
				}
				
				write(fd_client,combinaison,sizeof(combinaison));

				// on attend la reponse du serveur
				lc = read(fd_client, message, sizeof(message));
				printf("serveur> %s \n", message);
				
				write(fd_client, &control, sizeof(control));
				lc = read(fd_client,&tours_restants,sizeof(tours_restants));
				printf("serveur> il vous reste %d essais \n",tours_restants);
				
			}
			if (res == REG_NOMATCH)	// format de code incorrect
				printf("erreur de format du message : le code doit être une suite de 5 chiffres.\n");
			if (res == REG_ENOSYS)
				printf("l'utilisation de regex n'est pas supporte sur ce systeme.\n");
			
		}while ((strcmp(message,succes) != 0) && (tours_restants != 0));
	
	} else {	// echec lors de la construction de la regex
		printf("erreur lors de la création de la regex de control des essais");
	}
	
	if((strcmp(message,succes) == 0)){
		printf("client> félicitation vous avez gagné\n");
	}else{
		printf("client> désolé vous avez perdu\n");
	}
	close(fd_client);
	return EXIT_SUCCESS;
}


int main (int argc, char** argv){
    
	struct hostent *hostinfo = NULL;
	struct sockaddr_in adr;
	int sock;

	if (argc != 3){
		printf("Usage : %s nom-serveur numero-port \n", argv[0]);
		exit(-1);
	}

	if ((sock=socket(AF_INET,SOCK_STREAM,0)) == -1) {
		perror("Probleme socket de message\n");
		exit(-2);
	}

	hostinfo = gethostbyname(argv[1]);
	if (hostinfo == NULL){
		printf("Hote non trouvé\n");
		exit(-1);
	}

	adr.sin_family = AF_INET;
	adr.sin_addr = *(struct in_addr*) (hostinfo->h_addr);
	adr.sin_port = htons(atoi(argv[2]));

	char message_client[CHAINE_MAX];
	int lc;
	char pseudo[CHAINE_MAX];
	char message_reponse[CHAINE_MAX];
	printf("connexion avec le serveur...");
	while (connect(sock, (struct sockaddr*)&adr, (socklen_t)sizeof(struct sockaddr_in))<0);
	printf("\n");
	bool quitter = false;

	printf("******BIENVENUE******\n\n");
	printf("saisissez votre login de joueur svp!\n");
	printf("client > ");
	scanf("%s",pseudo);
	write(sock, pseudo, strlen(pseudo));
	fflush(stdout);
	lc = read(sock, message_reponse, sizeof(message_reponse));
	printf("serveur> %s\n",message_reponse);


   while(! quitter) {
	
		printf("\nEntrez votre choix : quit: Quitter , duel : VS Joueur , solitaire : VS Systeme\n");
		printf("client > ");
		scanf("%s",message_client);

		if (strcmp(message_client,"quit") == 0) {
			write(sock, message_client, sizeof(message_client));		
			quitter = true;
		}

		// verification de la saisie
		if ((strcmp(message_client,"duel") != 0)&&(strcmp(message_client,"solitaire") != 0)&&(strcmp(message_client,"quit") != 0)) {
		    printf("> erreur de saisie\n");
		}
		
		if (strcmp(message_client,"solitaire") == 0){
			write(sock, message_client, sizeof(message_client));			
			ia(sock);
		}

		if (strcmp(message_client,"duel") == 0){
			write(sock, message_client, sizeof(message_client));	
			printf("\ncertains problemes de devellopement et le manque de temps nous\n");
			printf("ont empeche de faire fonctionner cette partie. Les détails du\n");
			printf("probleme sont precise dans un commentaire du code serveur,\n");
			printf("qui est ecrit pratiquement en totalite pour cette fonctionnalite\n");		
			duel(sock);
		}
		
    }

	close(sock);
	
	return EXIT_SUCCESS;
}
