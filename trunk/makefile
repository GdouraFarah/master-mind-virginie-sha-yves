# Variables relatives a la compilation :
CC              = gcc
DEBUG           = no
#
# Variables precisant l'arborescence des fichiers :
REP_EXE         = bin
REP_OBJ         = obj
REP_SRC         = src
#
# Variables definissant les fichiers a compiler :
EXEC            	= ROMAIN_LIU_BOGE
EXEC_DEP_CLIENT 	= $(addprefix $(REP_OBJ)/, $(EXEC)_client.o)
EXEC_DEP_SERVEUR	= $(addprefix $(REP_OBJ)/, $(EXEC)_serveur.o)
AVEC_HEADER     	= $(addprefix $(REP_OBJ)/, )
SANS_HEADER     	= $(addprefix $(REP_OBJ)/, $(EXEC)_client.o $(EXEC)_serveur.o)
#
#
ifeq ($(DEBUG),yes)
	CFLAGS=-Wall -g
else
	CFLAGS=-Wall
endif
#
#
#       Regle par defaut :
#
all : $(REP_EXE)/$(EXEC)_client $(REP_EXE)/$(EXEC)_serveur
	
#
# Regle principale
#       
#
$(REP_EXE)/$(EXEC)_serveur : $(EXEC_DEP_SERVEUR)
	$(CC) -o $(@) $^
#
$(REP_EXE)/$(EXEC)_client : $(EXEC_DEP_CLIENT)
	$(CC) -o $(@) $^
#
$(AVEC_HEADER) : $(REP_OBJ)/%.o : $(REP_SRC)/%.c $(REP_SRC)/%.h
	$(CC) -c $< $(CFLAGS) -o $(@)
#
$(SANS_HEADER) : $(REP_OBJ)/%.o : $(REP_SRC)/%.c
	$(CC) -c $< $(CFLAGS) -o $(@)
#
# on desactive la verification des dependances pour clean
.PHONY: clean
	
#
clean:
	rm -f $(REP_OBJ)/*.o $(REP_SRC)/*~ $(REP_EXE)/$(EXEC)_client $(REP_EXE)/$(EXEC)_serveur makefile~
#
