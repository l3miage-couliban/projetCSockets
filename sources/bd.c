#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <float.h>

#include "../headers/bd.h"
#include "../headers/dependencies.h"

extern char *bd;


/* Fonction qui retourne un train satisfaisant le critère
 * ou premier train possible à partir de l'horaire de depart demandé
 */
void trouver_train(char *ville_depart, char *ville_arrivee, char *heure_depart, char *result)
{
    FILE *file = fopen(bd, "r");

    char line[100];
    char result_[100];

    //critère de recherche d'un train: trajet(ville_depart;ville_arrive)
    char trajet[100];
    sprintf(trajet, "%s;%s", ville_depart, ville_arrivee);

    // critere de recherche d'un train: trajet;heure_depart
    char trajet_hd[200];
    sprintf(trajet_hd, "%s;%s", trajet, heure_depart);

    //conversion de l'heure de départ en décimale
    char heure[7];
    replace(heure_depart, ':', '.', heure);
    float h_depart = atof(heure);

    float max = FLT_MAX;
    while (fgets(line, sizeof(line), file)) {
        if(!(strstr(line, trajet) == NULL)) {
            /*
            * Si la ligne ne correspond pas spécifiquement au trajet (heure départ compris)
            * mais les villes de départ et d'arrivée y sont
            */
            char **tab = calloc(8, 50);
            split_request(line, tab);

            //heure du train de la ligne
            char heure_t[7];
            replace(tab[3], ':', '.', heure_t);
            float heure_train = atof(heure_t);

            float diff_heure = heure_train - h_depart;

            if (diff_heure >= 0)
            {
                if (diff_heure < max)
                {
                    max = diff_heure;
                    strcpy(result_, line);
                }
            }
            free(tab);
        }
        
    }

    fclose(file);
    strcpy(result, result_);
}

void trouver_trains(struct trouver_trains_params params) {
    FILE *file = fopen(bd, "r");

    char *lines;
    lines = calloc(5000, sizeof(char));

    //critère de recherche d'un train: trajet(ville_depart;ville_arrive)
    char trajet[100];
    sprintf(trajet, "%s;%s", params.ville_depart, params.ville_arrivee);

    //conversion de l'heure de départ en décimale
    char hd_0[7];
    replace(params.depart_interval_0, ':', '.', hd_0);
    float h_depart_0 = atof(hd_0);

    char hd_1[7];
    replace(params.depart_interval_1, ':', '.', hd_1);
    float h_depart_1 = atof(hd_1);

    // lecture de la base de donnée ligne par ligne
    char line[100];
    while (fgets(line, sizeof(line), file)) {

        if(!(strstr(line, trajet) == NULL)) {
            /*
            * Si la ligne contient le critère du trajet
            */
            char **tab = calloc(8, 50);
            split_request(line, tab);


            //heure du train de la ligne
            char hd_t[7];
            replace(tab[3], ':', '.', hd_t);
            float heure_train = atof(hd_t);

            if (h_depart_0 <= heure_train && heure_train <= h_depart_1)
            {
                
                strcat(lines, line);
                strcat(lines, "#");
            }

            free(tab);
            
        }
        
    }

    strcpy(params.result, lines);
    fclose(file);
    free(lines);
}

void trouver_train_par_trajet(struct trouver_trains_par_trajet params) {
    FILE *file = fopen(bd, "r");

    char *lines;
    lines = calloc(5000, sizeof(char));

    //critère de recherche d'un train: trajet(ville_depart;ville_arrive)
    char trajet[strlen(params.ville_depart)+strlen(params.ville_arrivee)+1];

    sprintf(trajet, "%s;%s", params.ville_depart, params.ville_arrivee);

    //lecture de la base de donnée ligne par ligne
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if(!(strstr(line, trajet) == NULL)) {
            /*
            * Si la ligne contient le critère du trajet
            */
            strcat(lines, line);
            strcat(lines, "#");
        }
        
    }

    strcpy(params.result, lines);
    fclose(file);
    free(lines);
}