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

// le client joue contre le serveur
int ia(int fd_client){
	char message[50];
	int combinaison[5];
	int lc,i,resultat,tours_restants;
	char succes[6]={'R','R','R','R','R','\0'};
	int control = 0;

	// on envoie au serveur la combinaison
	do{
		// on demande au client sa tentative de combinaison
		printf("client> combinaison?\n");
		for(i=0;i<5;i++){
			scanf("%d",&resultat);
			combinaison[i] = resultat;
		}
		write(fd_client,combinaison,sizeof(combinaison));

		// on attend la reponse du serveur
		lc = read(fd_client,message,sizeof(message));
		printf("serveur> %s \n",message);
		write(fd_client,&control,sizeof(control));
		lc = read(fd_client,&tours_restants,sizeof(tours_restants));
		printf("serveur> il vous reste %d essaies \n",tours_restants);

	}while ((strcmp(message,succes) != 0) && (tours_restants != 0));
	
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

    char message_client[50];
    int lc;
	 char pseudo[50];
	 char message_reponse[100];
	 printf("connexion avec le serveur...");
    while (connect(sock, (struct sockaddr*)&adr, (socklen_t)sizeof(struct sockaddr_in))<0){
	 }
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


   while(! quitter)  
	{
	

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
		}
		
    }

close(sock);
return 0;
}


