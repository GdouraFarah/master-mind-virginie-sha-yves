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
	printf("Hote non trouvé");
	exit(-1);
    }

    adr.sin_family = AF_INET;
    adr.sin_addr = *(struct in_addr*) (hostinfo->h_addr);
    adr.sin_port = htons(atoi(argv[2]));

    char *message_client = malloc(50*sizeof(char));
    int lc;
    char *message_reponse = malloc(100*sizeof(char));

    while (1){
	connect(sock, (struct sockaddr*)&adr, (socklen_t)sizeof(struct sockaddr_in));
    }
    bool quitter = false;
    while(!quitter)
    {
	printf("Entrez votre choix : (0: Quitter , 1 : VS Joueur , 2 : VS Systeme\n");
	printf("message client > ");
	scanf("%s",message_client);
	write(sock, message_client, strlen(message_client));
	if (strcmp(message_client,"0")) {
	    quitter = true;
	}
	else
	{
	    lc = recvfrom (sock, message_reponse, 50, 0,NULL,NULL);
	    printf("serveur > : %s\n",message_reponse);
	    //if (strcmp(message_reponse, ))
	}
    }

}


