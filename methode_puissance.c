#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <float.h>

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


// � faire : Lire N, m, puis boucler pour lire chaque ligne
// -------------------------------------------------------------------------
// Lecture d'une matrice creuse P depuis un fichier texte
// Format attendu (comme dans tes fichiers matrix/8.txt, G101.txt, ...):
//   N
//   m
//   puis N lignes de la forme :
//     source  nb_liens  dest1  poids1  dest2  poids2  ...
// Les sommets sont numerotes de 1 a N dans le fichier.
SparseMatrix lire_matrice_creuse(const char *filename) {
    SparseMatrix P;
    P.N = 0;
    P.m = 0;
    P.links = NULL;

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

    int idx = 0;
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
            if (fscanf(f, "%d %f", &dest, &poids) != 2) {
                parse_error = 1;
                break;
            }

            if (idx < P.m) {
                int u = source - 1; // 0..N-1
                int v = dest - 1;
                if (u >= 0 && u < P.N && v >= 0 && v < P.N) {
                    P.links[idx].u = u;
                    P.links[idx].v = v;
                    P.links[idx].val = poids; // on suppose deja normalise
                    idx++;
                }
            }
        }

        if (parse_error) break;
    }
    if (P.m != idx){
        parse_error = 1; // nombre reel d'arcs lus
    }
     

    if (parse_error) {
        printf("Erreur: format invalide (lecture interrompue)ou bien fichier n'est pas complet dans %s\n", filename);
    }

    fclose(f);
    return P;
}



//condtruir le vecteur F :  f[i] = 1 si le sommet i est pendant (aucun lien sortant)
//                          f[i] = 0 sinon
int* construir_vecteur_F(SparseMatrix P) {
    int *F = (int *)calloc(P.N, sizeof(int));
    if (F == NULL) return NULL;

    // Initialiser le vecteur F à 1 (tous les sommets sont initialement considérés comme pendants)
    for (int i = 0; i < P.N; i++) {
        F[i] = 1;
    }

    // Parcourir tous les arcs et marquer les sommets qui ont des liens sortants
    for (int k = 0; k < P.m; k++) {
        F[P.links[k].u] = 0; // Le sommet source a un lien sortant
    }

    return F;
}


// -------------------------------------------------------------------------
// Construit une matrice dense G (N x N) a partir de la matrice creuse P.
// Ici on construit directement la matrice de Google :
//   G = alpha * P + alpha*(1/N) * f e^T + (1-alpha)*(1/N) * e e^T
// avec f le vecteur des sommets pendants (donne par construir_vecteur_F).
// G est stockee en "row-major" : G[i*N + j] represente l'entree G[i,j].
float* construire_matrice_dense_G(SparseMatrix P, float alpha) {
    // Verifier que la taille est valide
    if (P.N <= 0) return NULL;

    int N = P.N;

    // Allouer la matrice dense G (N x N)
    float *G = (float *)malloc(N * N * sizeof(float));
    if (G == NULL) return NULL;

    // Construire le vecteur f (noeuds pendants)
    int *F = construir_vecteur_F(P);
    if (F == NULL) {
        free(G);
        return NULL;
    }

    // Calculer 1/N une seule fois
    float un_sur_N = 1.0f / (float)N;

    // Etape 1 : initialiser G avec les termes
    //   alpha*(1/N) * f e^T  +  (1-alpha)*(1/N) * e e^T
    // On remplit ligne par ligne.
    for (int i = 0; i < N; i++) {
        int est_pendant = (F[i] == 1); // 1 si f[i] = 1, sinon 0

        // Partie provenant du vecteur f sur la ligne i
        float apport_f = alpha * (float)est_pendant * un_sur_N;

        // Partie teleportation (1-alpha)/N, la meme pour tout le monde
        float apport_teleport = (1.0f - alpha) * un_sur_N;

        for (int j = 0; j < N; j++) {
            // Pour chaque colonne j, la valeur de base est :
            //   alpha*(1/N)*f[i]  +  (1-alpha)*(1/N)
            G[i * N + j] = apport_f + apport_teleport;
        }
    }

    // Etape 2 : ajouter la partie alpha * P
    // Pour chaque arc (u -> v) avec probabilite P.links[k].val,
    // on ajoute alpha * P[u,v] a G[u,v].
    for (int k = 0; k < P.m; k++) {
        int u = P.links[k].u;
        int v = P.links[k].v;

        if (u < 0 || u >= N || v < 0 || v >= N) {
            continue;
        }

        float p_uv = P.links[k].val; // probabilite de u vers v dans P
        G[u * N + v] += alpha * p_uv;
    }

    // On n'a plus besoin de F
    free(F);

    return G;
}

// -------------------------------------------------------------------------
// Methode de puissance pour trouver le vecteur de PageRank pi tel que:
//   pi = pi * G
// G est une matrice (N x N) stochastique en lignes.
// Retourne un tableau de taille N (a liberer avec free) ou NULL en cas d'erreur.
float* methode_puissance_pagerank(const float *G, int N, int max_iter, float tol, int *nb_iter_effectif) {
    if (G == NULL || N <= 0 || max_iter <= 0 || tol <= 0.0f) {
        return NULL;
    }

    float *x = (float *)malloc(N * sizeof(float));
    float *x_new = (float *)malloc(N * sizeof(float));
    if (x == NULL || x_new == NULL) {
        free(x);
        free(x_new);
        return NULL;
    }

    // Initialisation uniforme
    for (int i = 0; i < N; i++) {
        x[i] = 1.0f / (float)N;
    }

    int iter;
    for (iter = 0; iter < max_iter; iter++) {
        // x_new = x * G
        for (int j = 0; j < N; j++) {
            double s = 0.0;
            for (int i = 0; i < N; i++) {
                s += (double)x[i] * (double)G[i * N + j];
            }
            x_new[j] = (float)s;
        }

        // Normalisation pour limiter les erreurs numeriques
        double somme = 0.0;
        for (int i = 0; i < N; i++) {
            somme += (double)x_new[i];
        }
        if (somme <= DBL_MIN) {
            free(x);
            free(x_new);
            return NULL;
        }
        for (int i = 0; i < N; i++) {
            x_new[i] = (float)((double)x_new[i] / somme);
        }
        printf("Iteration %d : x_new = [", iter);
        for (int i = 0; i < N; i++) {
            printf(" %f", x_new[i]);
        }
        printf(" ]\n");

        // Critere d'arret en norme L1
        double diff = 0.0;
        for (int i = 0; i < N; i++) {
            diff += fabs((double)x_new[i] - (double)x[i]);
        }

        for (int i = 0; i < N; i++) {
            x[i] = x_new[i];
        }
        printf("Iteration %d : diff = %e\n", iter, diff);

        if (diff < (double)tol) {
            iter++;
            break;
        }
    }

    free(x_new);

    if (nb_iter_effectif != NULL) {
        *nb_iter_effectif = iter;
    }

    return x;
}



// --- MAIN (TRAVAIL ENSEMBLE) ---
int main() {
    float alpha = 0.85f; // Facteur de d�croissance pour PageRank
    int max_iter = 1000;
    float tol = 1e-7f;

    // 1. Charger la matrice (Partenaire A)
    //lire la matrice 

        SparseMatrix p = lire_matrice_creuse("matrix/8.txt");
        printf("N = %d, m = %d\n", p.N, p.m);

    // Matrice de transition complete

        float *G = construire_matrice_dense_G(p, alpha);
        if (G == NULL) {
            printf("Erreur: impossible de construire la matrice dense (memoire insuffisante ?)\n");
        } else {
            printf("\nMatrice de transition dense (%d x %d):\n", p.N, p.N);

            int nb_iter = 0;
            float *pagerank = methode_puissance_pagerank(G, p.N, max_iter, tol, &nb_iter);
            if (pagerank == NULL) {
                printf("Erreur: echec de la methode de puissance.\n");
            } else {
                printf("\nPageRank (methode de puissance), iterations = %d:\n", nb_iter);
                for (int i = 0; i < p.N; i++) {
                    printf("Noeud %d : %.10f\n", i + 1, pagerank[i]);
                }
                free(pagerank);
            }

            free(G);
        }
        
   
  
    
    
    
	// Liberer toute la memoire
       
        free(p.links); 


    
    
    return 0;
}
