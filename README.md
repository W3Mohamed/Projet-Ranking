# Projet-Ranking

Projet M1 du module Methode de Ranking consacre au calcul du PageRank a partir d'une chaine de Markov de matrice `G`.

## Description

Ce projet implemente une variante du calcul de PageRank basee sur deux vecteurs bornes :

- `∇[j] = min_i G[i,j]`
- `∆[j] = max_i G[i,j]`

On construit ensuite deux suites de vecteurs lignes `X(k)` et `Y(k)` qui encadrent la distribution stationnaire `x` :

- pour tout `k` et tout `i`, `X(k)[i] <= x[i] <= Y(k)[i]`

L'algorithme iteratif est :

1. `X(0) <- ∇`
2. `Y(0) <- ∆`
3. Repeter jusqu'a convergence :
	- `X(k+1) = max(X(k), X(k)G + ∇(1 - ||X(k)||1))`
	- `Y(k+1) = min(Y(k), Y(k)G + ∆(1 - ||Y(k)||1))`
	- arret quand `||X(k) - Y(k)||1 < ε`

## Objectifs

- Proposer une version efficace en memoire et en temps de calcul.
- Comparer experimentalement cette methode avec la methode des puissances.
- Etudier l'impact des parametres `ε` (precision d'arret) et `α` (facteur d'amortissement) sur la convergence et la qualite des resultats.

## Donnees

Les graphes de test sont disponibles dans le dossier `matrix/`.
Chaque fichier contient le nombre de sommets, le nombre d'arcs, puis les liens sortants et leurs poids.
