#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <netdb.h>
#include <float.h>

#include "../headers/socket.h"
#include "../headers/dependencies.h"
#include "../headers/bd.h"

int child_group;
int socket_desc;
extern int port;

int ecouter(struct sockaddr_in p)
{
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    p.sin_family = AF_INET;
    p.sin_addr.s_addr = htonl(INADDR_ANY);
    p.sin_port = htons(port);

    int bind_ = bind(socket_desc, (struct sockaddr *)&p, sizeof(struct sockaddr_in));

    if (bind_ != 0)
    {
        perror("bind"),
        exit(0);
    }

    int listen_ = listen(socket_desc, 10);

    if (listen_ == -1){
        perror("listen");
        exit(0);
    } else {
        printf("Serveur en marche ...\n");
    }

    return socket_desc;
}

int accepter(struct sockaddr_in client_addr, int serv_descr)
{
    int len = sizeof(client_addr);

    while (1)
    {
        int service_socket_desc = accept(socket_desc, (struct sockaddr *)&client_addr, &len);

        switch (fork())
        {
        case -1:
            perror("fork");
            exit(1);
        case 0:
            fils(service_socket_desc);
        default:
            break;
        }
    }

    close(serv_descr);
}

void fils(int server_desc)
{
    child_group = getpgrp();
    printf("child group: %d\n", child_group);
    printf("Connexion client n°%d\n", server_desc);
    communication(server_desc);
    printf("Deconnexion client n°%d\n", server_desc);
    close(server_desc);
    exit(0);
}

void communication(int sock_desc)
{
    int req_len;
    char *request;
    char *result;

    int exit = 0;

    while (exit != 1)
    {
        // attend la taille du resultat
        if (read(sock_desc, &req_len, sizeof(int)) == -1) {
            perror("read");
        }

        printf("taille requête: %d\n", req_len);

        request = calloc(req_len, sizeof(char));

        // lit le message envoyé par le client
        int lu = read(sock_desc, request, req_len);

        if (lu == -1)
        {
            perror("\nechec de lecture");
        }
        else
        {
            printf("=> requête client n°%d: %s\n", sock_desc, request);
        }

        //si le message est exit, la communication s'arrête et le socket se ferme
        if (strncmp("exit", request, 4) == 0) {
            exit = 1;
        } else { // sinon écrit au client

            result = calloc(1000, sizeof(char));

            // operation sur la requete envoyé
            operation(request, result);

            int tmp = strlen(result);

            //envoi de la taille de la reponse
            write(sock_desc, &tmp, sizeof(int));

            //envoi la réponse
            int ecrit = write(sock_desc, result, strlen(result));
            if (ecrit == -1)
            {
                perror("echec d'écriture");
            }

            free(request);
            free(result);
        }
    }
}

void operation(char *req, char* result)
{
    char **tab;

    tab = calloc(10, 50);

    split_request(req, tab);

    //Récupérer le numéro de la requête
    char num[3];
    sprintf(num, "%s", tab[0]);
    int num_req = atoi(num);

    switch (num_req)
    {
    case 1:
    {
        /* ville_depart;ville_arrivee;heure_depart */
        char *ville_depart = tab[1];
        char *ville_arrivee = tab[2];
        char *heure_depart = tab[3];

        trouver_train(ville_depart, ville_arrivee, heure_depart, result);
        break;
    }
    case 2:
    {
        /* ville_depart;ville_arrivee;depart_intervalle(depart_1;depart_2)*/

        struct trouver_trains_params params = {
            tab[1], // ville depart
            tab[2], // ville arrivee
            tab[3], // heure depart 0
            tab[4], // heure depart 1
            result
        };
        
        trouver_trains(params);
        strcpy(result, params.result);
        
        break;
    }
    case 3:
    {
        /* ville_depart;ville_arrivee
         * Affiche la liste de tous les trains puis
         * 1. Selectionne le train le meilleur prix
         * 2. Ou le train au temps optimum
         */
        struct trouver_trains_par_trajet params_ = {
            tab[1], // ville depart
            tab[2], // ville arrivee
            result
        };

        trouver_train_par_trajet(params_);
        strcpy(result, params_.result);
        break;
    }
    default:
        break;
    }

    free(tab);
}