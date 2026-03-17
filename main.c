#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>

// --- STRUCTURES COMMUNES ---
typedef struct {
    int u, v;
    float val;
} Edge;

typedef struct {
    int N, m;
    Edge *links;
} SparseMatrix;

// Construit une matrice de transition dense (row-major) NxN initialisee a 0.
// Si un arc (u,v) existe, on met sa valeur; sinon la case reste a 0.
// Attention: pour N grand, une matrice dense peut etre enorme en memoire.
static float *sparse_to_dense_transition_matrix(const SparseMatrix P) {
    if (P.N <= 0) return NULL;

    size_t n = (size_t)P.N;
    if (n > 0 && n > (SIZE_MAX / n)) return NULL;     // overflow N*N
    size_t nn = n * n;
    if (nn > (SIZE_MAX / sizeof(float))) return NULL; // overflow bytes

    float *G = (float *)calloc(nn, sizeof(float));
    if (G == NULL) return NULL;

    for (int k = 0; k < P.m; k++) {
        int u = P.links[k].u;
        int v = P.links[k].v;
        if (u < 0 || u >= P.N || v < 0 || v >= P.N) continue;
        G[(size_t)u * (size_t)P.N + (size_t)v] = P.links[k].val;
    }

    return G;
}

static void print_dense_matrix(const float *G, int N) {
    if (G == NULL || N <= 0) return;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%7.3f", G[(size_t)i * (size_t)N + (size_t)j]);
        }
        printf("\n");
    }
}

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
    int parse_error = 0;
    for (int ligne = 0; ligne < P.N; ligne++) {
        int source, nb_liens;

        if (fscanf(f, "%d %d", &source, &nb_liens) != 2) {
            parse_error = 1;
            break;
        }

        for (int j = 0; j < nb_liens; j++) {
            int dest;
            float poids;

            // Si on ne peut pas lire une paire (dest,poids), c'est une erreur de format,
            // pas un "arc manquant". Les arcs manquants seront a 0 dans la matrice dense.
            if (fscanf(f, "%d %f", &dest, &poids) != 2) {
                parse_error = 1;
                break;
            }

            if (idx < P.m) {
                // fichier: sommets numerotes en 1..N ; stockage: 0..N-1
                int u = source - 1;
                int v = dest - 1;
                if (u >= 0 && u < P.N && v >= 0 && v < P.N) {
                    P.links[idx].u = u;
                    P.links[idx].v = v;
                    P.links[idx].val = poids;
                    idx++;
                }
            }
        }

        if (parse_error) break;
    }

    // On garde le nombre d'arcs vraiment lus
    P.m = idx;

    if (parse_error) {
        printf("Erreur: format invalide (lecture interrompue) dans %s\n", filename);
    }

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
    //lire la matrice 
    SparseMatrix p = read_file("matrix/8.txt");
    printf("N = %d, m = %d\n", p.N, p.m);

    // Matrice de transition complete: valeur si l'arc existe, 0 sinon.
    float *G = sparse_to_dense_transition_matrix(p);
    if (G == NULL) {
        printf("Erreur: impossible de construire la matrice dense (memoire insuffisante ?)\n");
    } else {
        printf("\nMatrice de transition dense (%d x %d):\n", p.N, p.N);
        print_dense_matrix(G, p.N);
        free(G);
    }

    free(p.links); 

    
    // 2. Initialiser X = Nabla, Y = Delta (Partenaire A)
    // 3. Boucle while (Norme L1 > epsilon) (Partenaire B)
    // 4. Comparer avec la m�thode des puissances (Ensemble)
    return 0;
}
