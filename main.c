//
//  main.c
//  minishell
//
//  Created by Etienne Dayre on 13/11/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define DELIMITATION " \n"  //Retour a la ligne

//1.-----------------------Lire entree-------------------------
char *lire_ligne(void)
{
  int indice = 0;
  char *buffer = malloc(sizeof(char) * 1024);
  int a;

  while (1) {
    // Lire une ligne
    a = getchar();

    // Si retour a la ligne, derniere valeur à null
    if (a == '\n') {
      buffer[indice] = '\0';
      return buffer;
    } else {
      buffer[indice] = a;
    }
      //Incrementation de l indice dans le tableau
      indice++;

  }
}

//2.----- Construction de argv (tableau de chaine) -----------
char **construction_tableau_chaine(char *line, char**envp)
{
    int buffersize = 64, indice = 0;
    char **elements = malloc(buffersize * sizeof(char*)); //Allocation mémoire
    char *element;
    
    //Premier decoupage
    element = strtok(line, DELIMITATION);
    
    //Parcours
    while (element != NULL) {
        elements[indice] = element;
        indice++;

        //Deuxieme découpage
        element = strtok(NULL, DELIMITATION);
    }

    elements[indice] = NULL;

    return elements;
}

//3.---------Executer entree + Lancement minishell------------
int executer_ligne(char **args, char **envp)
{
    char  *repertoire_absolu;
    char  *repertoire;
    char  *repertoire_destination;
    char buffer[1024];
    
    if (args[0] == NULL) {
        //Si aucun argument en entrée
        return 1;
    }
    
    //Si la commande est cd
    if (!strcmp(args[0], "cd")){

        repertoire_absolu = getcwd(buffer, sizeof(buffer));
        
        //Concaténation de chaines de caracteres
        repertoire = strcat(repertoire_absolu, "/");
        repertoire_destination = strcat(repertoire, args[1]);

        chdir(repertoire_destination);
        
    } else {    //Pour tout autre commande, autre de cd
        //Lancement du minishell
        pid_t pid, wpid;
        int status;
        
        pid = fork();
        if (pid == 0) {
            // Processus fils
            if (execve(args[0], args, envp) == -1) { //Remplacement image processus appelant
                
                perror("Error execve");
            }
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            // Erreur lors du fork()
            perror("Error");
        } else {
            // Processus pere
            do {
            
            //Attente du processus fils
            wpid = waitpid(pid, &status, WUNTRACED);
                
            } while (!WIFEXITED(status) && !WIFSIGNALED(status)); //Renvoie true si fils terminée&& terminé a cause signal
        }
    }
    return 1;
 
}

//------------------------ Main ------------------------------
int main(int argc, char **argv, char *envp[]) {

    //Etapes du mini shell
    //1. Lire la ligne en entrée
    //2. Transformer ligne en tableau de chaine grace a strtok()
    //3. Executer la commande
    char *chaine;
    char **args;
    int execute;
    
    do {
        
        //---------- Affichage dernier dossier courant------------
        char str_destination[1024];

        char dossier_courant[1024] = { 0 };
        char dossier_courant_char = '\0';
        int  index = 0;
        int  dernier_slash_index = 0;

        getcwd(dossier_courant, 1024);

        while ((dossier_courant_char = dossier_courant[index]) != '\0') {
            if (dossier_courant_char == '/') {
                dernier_slash_index = index;
            }

            index++;
        }

        strcpy(str_destination, dossier_courant + dernier_slash_index + 1);
        //--------------------------------------------------------
        
        printf("%s > ", str_destination);
        chaine = lire_ligne();
        args = construction_tableau_chaine(chaine, envp);
        execute = executer_ligne(args, envp);

        //Libération de la mémoire
        free(chaine);
        free(args);
    } while (execute);

    return EXIT_SUCCESS;

}
