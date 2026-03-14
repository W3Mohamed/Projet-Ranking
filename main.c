#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// --- STRUCTURES COMMUNES ---
typedef struct {
    int u, v;
    float val;
} Edge;

typedef struct {
    int N, m;
    Edge *links;
} SparseMatrix;

// --- PARTENAIRE A : GESTION DES DONN�ES ---

SparseMatrix read_file(const char *filename) {
     // � faire : Lire N, m, puis boucler pour lire chaque ligne
    // Attention au format : Source | Nb_liens | Dest1 | Poids1 ...
    SparseMatrix P;
    P.N = 0;        // nombre de noeuds
    P.m = 0;        // nombre d'arcs (liens)
    P.links = NULL; // tableau d'arcs

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Erreur: impossible d'ouvrir %s\n", filename);
        return P;
    }

    if (fscanf(f, "%d", &P.N) != 1 || fscanf(f, "%d", &P.m) != 1) {
        printf("Erreur: format invalide dans %s\n", filename);
        fclose(f);
        P.N = 0;
        P.m = 0;
        return P;
    }

    P.links = (Edge *)malloc(P.m * sizeof(Edge));
    if (P.links == NULL) {
        printf("Erreur: memoire insuffisante\n");
        fclose(f);
        P.N = 0;
        P.m = 0;
        return P;
    }

    int idx = 0; // position actuelle dans P.links[]
    for (int ligne = 0; ligne < P.N; ligne++) {
        int source, nb_liens;

        if (fscanf(f, "%d %d", &source, &nb_liens) != 2) {
            break;
        }

        for (int j = 0; j < nb_liens; j++) {
            int dest;
            float poids;

            if (fscanf(f, "%d %f", &dest, &poids) != 2) {
                break;
            }

            if (idx < P.m) {
                P.links[idx].u = source - 1; // indexation 0..N-1
                P.links[idx].v = dest - 1;
                P.links[idx].val = poids;
                idx++;
            }
        }
    }

    // On garde le nombre d'arcs vraiment lus
    P.m = idx;

    fclose(f);
    return P;
}

float* calculer_nabla(SparseMatrix P, float alpha) {
    // � faire : Trouver le min de chaque colonne de G
}

float* calculer_delta(SparseMatrix P, float alpha) {
    // � faire : Trouver le max de chaque colonne de G
}

// --- PARTENAIRE B : ALGORITHME ET CALCUL ---
float norme_L1(float *V1, float *V2, int N) {
    // � faire : Somme des abs(V1[i] - V2[i])
}

void iteration_nabla_delta(SparseMatrix P, float *X, float *Y, float alpha) {
    // � faire : Appliquer les formules (a) et (b) du projet
}

// --- MAIN (TRAVAIL ENSEMBLE) ---
int main() {
    // 1. Charger la matrice (Partenaire A)
    // 2. Initialiser X = Nabla, Y = Delta (Partenaire A)
    // 3. Boucle while (Norme L1 > epsilon) (Partenaire B)
    // 4. Comparer avec la m�thode des puissances (Ensemble)
    return 0;
}
