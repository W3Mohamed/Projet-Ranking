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


// Trouver le minimum de chaque colonne de G
// Calcule nabla[j] = min_i G[i,j] a partir d'une matrice dense G (N x N)
float* calculer_nabla_depuis_G(const float *G, int N) {
    if (G == NULL || N <= 0) return NULL;

    float *nabla = (float *)malloc(N * sizeof(float));
    if (nabla == NULL) return NULL;

    for (int j = 0; j < N; j++) {
        float min_val = G[0 * N + j];
        for (int i = 0; i < N; i++) {
            float val = G[i * N + j];
            if (val < min_val) {
                min_val = val;
            }
        }
        nabla[j] = min_val;
    }

    return nabla;
}



// Calcule delta[j] = max_i G[i,j] a partir d'une matrice dense G (N x N)
float* calculer_delta_depuis_G(const float *G, int N) {
    if (G == NULL || N <= 0) return NULL;

    float *delta = (float *)malloc(N * sizeof(float));
    if (delta == NULL) return NULL;

    for (int j = 0; j < N; j++) {
        float max_val = G[0 * N + j];
        for (int i = 0; i < N; i++) {
            float val = G[i * N + j];
            if (val > max_val) {
                max_val = val;
            }
        }
        delta[j] = max_val;
    }

    return delta;
}

// Fonctions pratiques : a partir de P, construire G puis calculer directement nabla ou delta
float* calculer_nabla_G(SparseMatrix P, float alpha) {
    float *G = construire_matrice_dense_G(P, alpha);
    if (G == NULL) return NULL;

    float *nabla = calculer_nabla_depuis_G(G, P.N);
    free(G);
    return nabla;
}

float* calculer_delta_G(SparseMatrix P, float alpha) {
    float *G = construire_matrice_dense_G(P, alpha);
    if (G == NULL) return NULL;

    float *delta = calculer_delta_depuis_G(G, P.N);
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
    float alpha = 0.85f; // Facteur de d�croissance pour PageRank

    // 1. Charger la matrice (Partenaire A)
    //lire la matrice 

        SparseMatrix p = lire_matrice_creuse("matrix/8.txt");
        printf("N = %d, m = %d\n", p.N, p.m);

    // Matrice de transition complete: valeur si l'arc existe, 0 sinon.

        float *G = construire_matrice_dense_G(p, alpha);
        if (G == NULL) {
            printf("Erreur: impossible de construire la matrice dense (memoire insuffisante ?)\n");
        } else {
            printf("\nMatrice de transition dense (%d x %d):\n", p.N, p.N);
            free(G);
        }
        
    // 2. Initialiser X = Nabla, Y = Delta (Partenaire A)
    
    // Etape 2.1: Calculer nabla (minimum de chaque colonne)
        float *nabla = calculer_nabla_G(p, alpha);
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
        float *delta = calculer_delta_G(p, alpha);
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
