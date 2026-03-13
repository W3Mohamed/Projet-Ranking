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

// --- PARTENAIRE A : GESTION DES DONNÉES ---
SparseMatrix read_file(const char *filename) {
    // À faire : Lire N, m, puis boucler pour lire chaque ligne
    // Attention au format : Source | Nb_liens | Dest1 | Poids1 ...
}

float* calculer_nabla(SparseMatrix P, float alpha) {
    // À faire : Trouver le min de chaque colonne de G
}

float* calculer_delta(SparseMatrix P, float alpha) {
    // À faire : Trouver le max de chaque colonne de G
}

// --- PARTENAIRE B : ALGORITHME ET CALCUL ---
float norme_L1(float *V1, float *V2, int N) {
    // À faire : Somme des abs(V1[i] - V2[i])
}

void iteration_nabla_delta(SparseMatrix P, float *X, float *Y, float alpha) {
    // À faire : Appliquer les formules (a) et (b) du projet
}

// --- MAIN (TRAVAIL ENSEMBLE) ---
int main() {
    // 1. Charger la matrice (Partenaire A)
    // 2. Initialiser X = Nabla, Y = Delta (Partenaire A)
    // 3. Boucle while (Norme L1 > epsilon) (Partenaire B)
    // 4. Comparer avec la méthode des puissances (Ensemble)
    return 0;
}
