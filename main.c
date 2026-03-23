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


// � faire : Lire N, m, puis boucler pour lire chaque ligne
SparseMatrix read_file(const char *filename) {
     
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

float* calculer_nabla(SparseMatrix P) {
    // Trouver le minimum de chaque colonne de G
    
    // Etape 1: Convertir la matrice creuse en matrice dense
    float *G = sparse_to_dense_transition_matrix(P);
    if (G == NULL) return NULL;
    
    // Etape 2: Allouer de la memoire pour les resultats (un minimum par colonne)
    float *nabla = (float *)malloc(P.N * sizeof(float));
    if (nabla == NULL) {
        free(G);
        return NULL;
    }
    
    // Etape 3: Pour chaque colonne j, trouver le minimum
    for (int j = 0; j < P.N; j++) {
        // Initialiser avec la premiere valeur de la colonne
        float min_val = G[0 * P.N + j];
        
        // Parcourir tous les elements de la colonne j (lignes 0 a N-1)
        for (int i = 0; i < P.N; i++) {
            float val = G[i * P.N + j];  // acces element [i][j]
            if (val < min_val) {
                min_val = val;  // trouver plus petit
            }
        }
        
        // Stocker le minimum dans le resultat
        nabla[j] = min_val;
    }
    
    // Liberer la memoire de la matrice dense
    free(G);
    
    return nabla;
}

float* calculer_delta(SparseMatrix P) {
    // Trouver le maximum de chaque colonne de G
    
    // Etape 1: Convertir la matrice creuse en matrice dense
    float *G = sparse_to_dense_transition_matrix(P);
    if (G == NULL) return NULL;

    // Etape 2: Allouer de la memoire pour les resultats (un maximum par colonne)
    float *delta = (float *)malloc(P.N * sizeof(float));
    if (delta == NULL) {
        free(G);
        return NULL;
    }
    // Etape 3: Pour chaque colonne j, trouver le maximum
    for (int j = 0; j < P.N; j++) {
        // Initialiser avec la premiere valeur de la colonne
        float max_val = G[0 * P.N + j];
        
        // Parcourir tous les elements de la colonne j (lignes 0 a N-1)
        for (int i = 0; i < P.N; i++) {
            float val = G[i * P.N + j];  // acces element [i][j]
            if (val > max_val) {
                max_val = val;  // trouver plus grand
            }
        }
        
        // Stocker le maximum dans le resultat
        delta[j] = max_val;
    }
    
    // Liberer la memoire de la matrice dense
    free(G);
    return delta;
}

// --- PARTENAIRE B : ALGORITHME ET CALCUL ---
float norme_L1(float *V1, float *V2, int N) {
    float somme = 0.0;
    for (int i = 0; i < N; i++) {
        somme += fabsf(V1[i] - V2[i]); // Utilise fabsf pour les float
    }
    return somme;
}

void multiplier_vecteur_sparse(SparseMatrix P, float *V_in, float *V_out, float alpha) {
    int N = P.N;
    float terme_constant = (1.0 - alpha) / N; // Simplification du saut al�atoire

    // Initialiser V_out avec le terme constant
    for (int i = 0; i < N; i++) V_out[i] = terme_constant;

    // Ajouter la partie alpha * V_in * P
    for (int k = 0; k < P.m; k++) {
        V_out[P.links[k].v] += alpha * V_in[P.links[k].u] * P.links[k].val;
    }
}

void iteration_nabla_delta(SparseMatrix P, float *X, float *Y, float *nabla, float *delta, float alpha) {
    // � faire : Appliquer les formules (a) et (b) du projet
    int N = P.N;
    float *X_next = malloc(N * sizeof(float));
    float *Y_next = malloc(N * sizeof(float));

    // Calculer ||X||1 et ||Y||1 (la somme des �l�ments)
    float normeX = 0, normeY = 0;
    for(int i=0; i<N; i++) {
        normeX += X[i];
        normeY += Y[i];
    }

    // Calculer X*G et Y*G (en utilisant ta fonction de multiplication)
    float *XG = malloc(N * sizeof(float));
    float *YG = malloc(N * sizeof(float));
    multiplier_vecteur_sparse(P, X, XG, alpha);
    multiplier_vecteur_sparse(P, Y, YG, alpha);

    // Appliquer les formules (a) et (b)
    for (int i = 0; i < N; i++) {
        // X(k+1) = max(X(k), X(k)G + nabla(1 - ||X||1))
        float valX = XG[i] + nabla[i] * (1.0 - normeX);
        X[i] = (X[i] > valX) ? X[i] : valX;

        // Y(k+1) = min(Y(k), Y(k)G + delta(1 - ||Y||1))
        float valY = YG[i] + delta[i] * (1.0 - normeY);
        Y[i] = (Y[i] < valY) ? Y[i] : valY;
    }

    free(X_next); free(Y_next); free(XG); free(YG);
}

// --- MAIN (TRAVAIL ENSEMBLE) ---
int main() {
    // 1. Charger la matrice (Partenaire A)
    //lire la matrice 

        SparseMatrix p = read_file("matrix/G101.txt");
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
        
    // 2. Initialiser X = Nabla, Y = Delta (Partenaire A)
    
    // Etape 2.1: Calculer nabla (minimum de chaque colonne)
        float *nabla = calculer_nabla(p);
        if (nabla == NULL) {
            printf("Erreur: impossible de calculer nabla\n");
            free(p.links);
            return 1;
        }else {
            printf("\nNabla (minimum de chaque colonne):\n");
            for (int i = 0; i < p.N; i++) {
                printf("%7.3f ", nabla[i]);
            }
            printf("\n");
        }
        
    // Etape 2.2: Calculer delta (maximum de chaque colonne)
        float *delta = calculer_delta(p);
        if (delta == NULL) {
            printf("Erreur: impossible de calculer delta\n");
            free(nabla);
            free(p.links);
            return 1;
        }else {
            printf("\nDelta (maximum de chaque colonne):\n");
            for (int i = 0; i < p.N; i++) {
                printf("%7.3f ", delta[i]);
            }
            printf("\n");
        }
        
    // X et Y pointent directement vers nabla et delta
    // Pas besoin d'allouer et copier !
       /* float *X = nabla;
        float *Y = delta;
        
        printf("X = nabla, Y = delta initialises\n"); */
    
    // 3. Boucle while (Norme L1 > epsilon) (Partenaire B)
        /*float epsilon = 0.000001;
        int iter = 0;
        while (norme_L1(X, Y, p.N) > epsilon && iter < 1000) {
            iteration_nabla_delta(p, X, Y, nabla, delta, 0.85);
            
            
                printf("Iteration %d : erreur = %f\n", iter, norme_L1(X, Y, p.N));
            
            iter++;
        }
        printf("Convergence atteinte en %d iterations !\n", iter); */
	
	// Liberer toute la memoire
	// X et Y pointent vers nabla et delta, donc pas besoin de free(X) et free(Y)
        free(nabla);
        free(delta);
        free(p.links); 


    // 4. Comparer avec la m�thode des puissances (Ensemble)
    
    return 0;
}
