/* types */

typedef struct coor_s {
  int x;
  int y;
} coor;

typedef struct kase_s {
  /* indique ce qui est sur la case
   * (f:fourmi,F:fourmi avec du sucre,n:nid,s:sucre ou r:rien) */
  char quoi;
  /* valeur entre 0 et 255 de la pheromone de sucre */
  unsigned char ph_sucre;
  /* valeur entre 0 et 1 de la pheromone de nid */
  float ph_nid;
  /* quantite de sucre */
  unsigned char sucre;
  /* vrai si et seulement si la fourmi
   * qui y est a deja ete deplacee
   */
  int fait;
} kase;

typedef struct couleur_s {
  unsigned char r, v, b;
} couleur;



/* fonctions non statiques */
couleur pixcouleur(int i, int j);
