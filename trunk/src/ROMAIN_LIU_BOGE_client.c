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

    char *message_client = malloc(50*sizeof(char));
    int lc;
    char *message_reponse = malloc(100*sizeof(char));
	 char *pseudo = malloc(50*sizeof(char));

	 printf("connexion avec le serveur...");
    while (connect(sock, (struct sockaddr*)&adr, (socklen_t)sizeof(struct sockaddr_in))<0){
    	printf("...");
	 }
	printf("\n");
   bool quitter = false;

	printf("******BIENVENUE******\n");
	printf("saisissez votre login de joueur svp!\n");
	printf("message client > ");
	scanf("%s",pseudo);
	write(sock, pseudo, strlen(pseudo));

   while(! quitter)  
	{
	

	printf("Entrez votre choix : quit: Quitter , duel : VS Joueur , solo : VS Systeme\n");
	printf("message client > ");
	scanf("%s",message_client);

	if (strcmp(message_client,"quit") == 0) {
		write(sock, message_client, strlen(message_client));		
		quitter = true;
	}

	// verification de la saisie
	if ((strcmp(message_client,"duel") != 0)&&(strcmp(message_client,"solo") != 0)&&(strcmp(message_client,"quit") != 0)) {
	    printf("> erreur de saisie\n");
	}
	
		
	else
	{
		 write(sock, message_client, strlen(message_client));	
	    lc = read (sock, message_reponse, strlen(message_reponse));
	    printf("serveur %d> %s\n",lc,message_reponse);
	}
    }

close(sock);
return 0;
}


