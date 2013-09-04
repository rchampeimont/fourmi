#include "fourmi.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#ifdef VERSION_SDL
#include "sfourmi.h"
#endif

#ifdef __MINGW32__
#undef main /* We don't want SDL to override our main() */
#endif

/* grille */
static kase grille[20][20];



/* FONCTIONS DE BASE */

/* retourne a modulo b,
   avec la definition mathematique du modulo :
   -1 modulo 20 = 19   (et non -1 % 20 = -1)
   (le modulo est dans [0,b[ ) */
static int modulo(int a, int b) {
  return ((a %= b) < 0 ? a + b : a);
}

/* minimum de 2 entiers */
static int intmin(int a, int b) {
  return (a < b ? a : b);
}

/* retourne un pointeur vers la case de coordonnees (a,b)
 * dans le tableau grille */
static kase* kor(int a, int b){
  return(&grille[modulo(a,20)][modulo(b,20)]);
}

/* FIN DES FONCTIONS DE BASE */




/* FONCTIONS BOOLEENNES */

/* retourne VRAI si une case contient une fourmi qui
 * cherche du sucre FAUX sinon
 */
static int cherche(int a, int b) {
  return (kor(a,b)->quoi == 'f');
}

/* retourne VRAI si une case contient une fourmi qui
 * rentre au nid FAUX sinon
 */
static int rentre(int a, int b) {
  return (kor(a,b)->quoi == 'F');
}

/* retourne VRAI si et seulement si la case (a,b)
 * contient une fourmi
 */
static int fourmi(int a, int b) {
  return (kor(a,b)->quoi == 'F' || kor(a,b)->quoi == 'f');
}

/* retourne VRAI si une case contient du sucre FAUX sinon */
static int sucre(int a, int b) {
  return (kor(a,b)->quoi == 's');
}

/* retourne VRAI si une case est un element du nid FAUX sinon */
static int nid(int a, int b) {
  return (kor(a,b)->quoi == 'n');
}

/* retourne VRAI si une case ne contient rien FAUX sinon */
static int vide(int a, int b) {
  return (kor(a,b)->quoi == 'r');
}

/* retourne VRAI si et seulement si une case contient une pheromone
 * de sucre (valeur non nulle)
 */
static int sur_une_piste(int a, int b) {
  return kor(a,b)->ph_sucre;
}

/* retourne VRAI si la case de coordonnees (c,d) est une
 * case vide adjacente a celle de coordonnees (a,b)
 * dont la pheromone de nid est egale a la plus faible
 * parmi les cases adjacentes a (a,b)
 */
static int plus_loin_nid(int a, int b, int c, int d) {
  int i,j;
  float min = 2.0;
  /* la case ou la pheromone est la plus faible */
  coor k = { 0, 0 };

  /* Voir si elle est vide */
  if (!vide(c,d)) {
    return 0;
  }

  for (i=-1; i<=1; i++) {
    for (j=-1; j<=1; j++) {
      if (!i && !j) {
	continue;
      }

      /* on ignore la case si elle est non vide */
      if (!vide(a+i, b+j)) {
	continue;
      }

      if (kor(a+i,b+j)->ph_nid < min) {
	k.x = a+i;
	k.y = b+j;
	min = kor(a+i,b+j)->ph_nid;
      }
    }
  }

  /* on a maintenant k, la case ou la pheromone est la
   * plus faible, on regarde si k = (c,d)
   */
  return (kor(c,d)->ph_nid == min);
}



/* retourne VRAI si la case de coordonnees (c,d) est une
 * case vide adjacente a celle de coordonnees (a,b)
 * dont la pheromone de nid est egale a la plus forte
 * parmi les cases adjacentes a (a,b)
 */
static int plus_proche_nid(int a, int b, int c, int d) {
  int i,j;
  float max = -1.0;
  /* la case ou la pheromone est la plus forte */
  coor k = { 0, 0 };

  /* Voir si elle est vide */
  if (!vide(c,d)) {
    return 0;
  }

  for (i=-1; i<=1; i++) {
    for (j=-1; j<=1; j++) {
      if (!i && !j) {
	continue;
      }

      /* on ignore la case si elle est non vide */
      if (!vide(a+i, b+j)) {
	continue;
      }

      if ( kor(a+i,b+j)->ph_nid > max ) {
	k.x = a+i;
	k.y = b+j;
	max = kor(a+i,b+j)->ph_nid;
      }
    }
  }

  /* on a maintenant k, la case ou la pheromone est la
   * plus forte, on regarde si k = (c,d)
   */
  return (kor(c,d)->ph_nid == max);
}



/* FIN DES FONCTIONS BOOLEENNES */





/* FONCTIONS UTILES */

static coor aleatoire(int a, int b) {
  /* retourne une case vide et adjacente a celle dont les coordonnees
   * sont passees en parametre. S'il n'y en a pas, retourne les
   * coordonnees de la case passees en parametre */
  int i,j;
  int existe_vide = 0;
  coor c = { 0, 0 };
  coor r = { 0, 0 };
  int essais = 0;

  for (i=-1; i<=1; i++) {
    for (j=-1; j<=1; j++) {
      if (!i && !j) {
	continue;
      }
      if (vide(a+i,b+j)) {
	existe_vide = 1;
	c.x = a+i;
	c.y = b+j;
	goto trouve;
      }
    }
  }

  /* pas de case vide -> on retourne la case initiale */
  c.x = a;
  c.y = b;
  return c;

trouve:

  for (;;) {
    /* on cherche une case vide au hasard autour */
    r.x = a + rand()%3 - 1;
    r.y = b + rand()%3 - 1;
    essais++;
    if (essais > 15) {
      /* pas de chance -> on a toujours pas trouve
       * de case vide au hasard -> on retourne la case
       * vide trouvee au debut
       *
       * (note : cette precaution est prise au cas ou la fonction
       * rand() donne de mauvais resultats, et elle peut,
       * d'apres la norme ANSI C, est tres mauvaise)
       */
      return c;
    }

    /* case non vide -> on continue a chercher */
    if (!vide(r.x, r.y)) {
      continue;
    }

    /* case initiale -> on continue a chercher */
    if (r.x == a && r.y == b) {
      continue;
    }

    /* on a trouve une case au hasard -> on la retourne */
    return r;
  }
}

static void enleve_sucre(int a, int b) {
  if (!sucre(a,b)) {
    return;
  }

  /* on enleve le sucre */
  kor(a,b)->sucre--;

  /* plus de sucre ? */
  if (kor(a,b)->sucre <= 0) {
    kor(a,b)->quoi = 'r';
  }
}

/* deplace une fourmi de (a_1, b_1) vers (a_2, b_2)
 * si (a_2, b_2) est non vide, le programme s'arrete
 * sauf si (a_2, b_2) == (a_1, b_2)
 * cette fonction met le flag 'fait' a 1
 */
static void depf(int a_1, int b_1, int a_2, int b_2) {
  if (!fourmi(a_1, b_1)) {
    fprintf(stderr,
	"Erreur fatale : la case (%d,%d) ne contient pas de fourmi.\n",
	a_1, b_1);
    exit(1);
  }
  if (!vide(a_2, b_2)) {
    fprintf(stderr,
	"Erreur fatale : la case (%d,%d) est non vide.\n",
	a_2, b_2);
    exit(1);
  }
  kor(a_2, b_2)->quoi = kor(a_1, b_1)->quoi;
  kor(a_1, b_1)->quoi = 'r';
  kor(a_2, b_2)->fait = 1;
}

/* FIN DES FONCTIONS UTILES */





/* REGLES DE DEPLACEMENT */

/* Toutes ces fonctions retournent vrai si et seulement
 * si la regle en question a pu etre appliquee
 */

/* regle 1 */
/* S'il y a une fourmi sur la case (a,b), et qu'elle ne
 * transporte pas de sucre et qu'elle en trouve,
 * charge du sucre et pose une pheromone de sucre */
static int charge(int a, int b) {

  int i,j;

  if (!cherche(a,b)) {
    return 0;
  }

  for (i=-1; i<=1; i++) {
    for (j=-1; j<=1; j++) {
      /* on ignore la case (a,b) elle-meme */
      if (!i && !j) {
	continue;
      }
      if (sucre(a+i, b+j)) {
	kor(a,b)->quoi = 'F';
	enleve_sucre(a+i, b+j);
	kor(a,b)->ph_sucre = 255;
	return 1;
      }
    }
  }
  return 0;
}

/* regle 2 */
/* Si une fourmi est adjacente au nid et qu'elle porte
 * de la nourriture, la depose et pose une pheromone de
 * sucre
 */
static int depose(int a, int b) {
  int i, j;

  if (!rentre(a,b)) {
    return 0;
  }

  /* elle pose la pheromone */
  kor(a,b)->ph_sucre = 255;

  for (i=-1; i<=1; i++) {
    for (j=-1; j<=1; j++) {
      /* on ignore la case (a,b) elle-meme */
      if (!i && !j) {
	continue;
      }
      if (nid(a+i,b+j)) {
	kor(a,b)->quoi = 'f';
	return 1;
      }
    }
  }
  return 0;
}

/* regle 3 */
/* Si il y a une fourmi qui porte du sucre sur (a,b),
 * elle se dirige vers le nid tout en posant une
 * pheromone de sucre.
 */
static int achemine(int a, int b) {
  int i, j;

  if (!rentre(a,b)) {
    return 0;
  }

  /* elle pose la pheromone */
  kor(a,b)->ph_sucre = 255;

  /* on cherche ou elle va aller */
  /* Pour des raisons graphiques, on trouve plus interessant
   * qu'elles favorisent les directions nord, sud, est, ouest */
  for (i=-1; i<=1; i++) {

    /* on ignore la case (a,b) elle-meme */
    if (!i) {
      continue;
    }
    if (plus_proche_nid(a,b,a+i,b)) {
      depf(a, b, a+i, b);
      return 1;
    }
  }
  for (j=-1; j<=1; j++) {

    /* on ignore la case (a,b) elle-meme */
    if (!i) {
      continue;
    }
    if (plus_proche_nid(a,b,a,b+j)) {
      depf(a, b, a, b+j);
      return 1;
    }
  }
  i=-1;
  j=-1;
  if (plus_proche_nid(a,b,a+i,b+j)) {
    depf(a, b, a+i, b+j);
    return 1;
  }
  i=1;
  j=-1;
  if (plus_proche_nid(a,b,a+i,b+j)) {
    depf(a, b, a+i, b+j);
    return 1;
  }
  i=-1;
  j=1;
  if (plus_proche_nid(a,b,a+i,b+j)) {
    depf(a, b, a+i, b+j);
    return 1;
  }
  i=1;
  j=1;
  if (plus_proche_nid(a,b,a+i,b+j)) {
    depf(a, b, a+i, b+j);
    return 1;
  }

/* elle n'a pas trouve ou aller... */
return 0;
}

/* regles 4 */
/* Si la fourmi sur (a,b) est sur une piste,
 * alors on la deplace sur la piste (si possible)
 * la ou la pheromone de nid est la plus faible
 */
static int remonte(int a, int b) {
  int i,j;

  if (!cherche(a,b) || !sur_une_piste(a,b)) {
    return 0;
  }

  for (i=-1; i<=1; i++) {
    for (j=-1; j<=1; j++) {
      /* on ignore la case (a,b) elle-meme */
      if (!i && !j) {
	continue;
      }
      if (vide(a+i,b+j) &&
	  plus_loin_nid(a,b,a+i,b+j) &&
	  sur_une_piste(a+i,b+j)) {
	depf(a, b, a+i, b+j);
	return 1;
      }
    }
  }

  return 0;
}



/* regle 5 */
/* La fourmi sur (a,b) n'est pas sur une piste et cherche
 * du sucre, si une case adjacente est sur une piste
 * la fourmi va sur cette case
 */
static int retourve(int a, int b) {
  int i, j;

  if (!cherche(a,b)) {
    return 0;
  }

  for (i=-1; i<=1; i++) {
    for (j=-1; j<=1; j++) {
      if (!i && !j) {
	continue;
      }
      if (vide(a+i, b+j) && sur_une_piste(a+i, b+j)) {
	depf(a, b, a+i, b+j);
	return 1;
      }
    }
  }

  return 0;
}



/* regle 6 */
/* une fourmi se deplace aleatoirement sur une case adjacente vide
 * ou reste sur la meme case si elle ne peut pas bouger
 */
static int deplace(int a, int b) {
  coor c;

  if (!fourmi(a,b)) {
    return 0;
  }

  c = aleatoire(a,b);
  depf(a, b, c.x, c.y);

  return 1;

}


/* FIN DES REGLES DE DEPLACEMENT */




/* vide la grille */
static void vider(void) {
  int i,j;

  printf("Vidage de la grille...\n");

  for(i=0; i<=19; i++) {
    for(j=0; j<=19; j++) {
      grille[i][j].quoi = 'r';
      grille[i][j].ph_nid = 0;
      grille[i][j].ph_sucre = 0;
      grille[i][j].sucre = 0;
    }
  }
}


/* reinitialise la random seed
 * (pour utiliser au debut du main())
 */
static void inital(void) {
  unsigned int seed;
  seed = time(NULL);
  printf("Semance aleatoire : %u\n", seed);
  srand(seed);
}


static void placer(unsigned char nb_sucre) {
  int i, j, a;

  /* Met les fourmis */
  printf("Mise en place des fourmis...\n");
  for (i=8;i<=11;i++) {
    for (j=8;j<=11;j++) {
      kor(i,j)->quoi = 'f';
    }
  }

  /* Met le nid */
  printf("Mise en place du nid...\n");
  for (i=9;i<=10;i++) {
    for (j=9;j<=10;j++) {
      kor(i,j)->quoi = 'n';
    }
  }

  /* insere la pheromone nid */
  printf("Placement de la pheromone de nid...\n");
  for (i=-10; i<=9; i++) {
    for (j=-10; j<=9; j++) {
      kor(i,j)->ph_nid =
	(float)intmin(abs(i<0?i+1:i),abs(j<0?j+1:j)) / 10.0;
    }
  }

  /* met le sucre */
  printf("Mise en place du sucre...\n");
  /* on place plusieurs morceaux de sucre au hasard */
  a = rand() % 15 + 2;
  for (i=0; i<=3 ; i++) {
    for (j=0; j>=-1 ; j--) {
      kor(i+a,j)->quoi = 's';
      kor(i+a,j)->sucre = nb_sucre;
    }
  }
  a = rand() % 15 + 2;
  for (i=0; i>=-1 ; i--) {
    for (j=0; j<=1 ; j++) {
      kor(i,j+a)->quoi = 's';
      kor(i,j+a)->sucre = nb_sucre;
    }
  }
}


/* effectue un tour */
static void tour(void) {
  int i, j;
  kase *k = NULL;

  /* on remet a 0 les flags "fait" */
  for (i=0; i<20; i++) {
    for (j=0; j<20; j++) {
      kor(i,j)->fait = 0;
    }
  }

  /* on traite maintenant chaque case */
  for (i=0; i<20; i++) {
    for (j=0; j<20; j++) {
      k = kor(i,j);

      /* on diminue la pheromone de sucre */
      if (k->ph_sucre >= 5) {
	k->ph_sucre -= 5;
      } else if (k->ph_sucre) {
	k->ph_sucre = 0;
      }

      /* on ne fait rien si la case est deja traitee */
      if (kor(i,j)->fait) {
	continue;
      }

      /* on essaye chaque regle de 1 a 6
       * (priorite la + haute a la + faible)
       */
      if (!charge(i,j)) {
	if (!depose(i,j)) {
	  if (!achemine(i,j)) {
	    if (!remonte(i,j)) {
	      if (!retourve(i,j)) {
		deplace(i,j);
	      }
	    }
	  }
	}
      }

      /* fin de traitement de cette case */
    }
  }
}



/* i, j : coordonnees de la case sur l'image rendue
 * retour :
 * 0 = le plus interne
 * 3 = le plus externe
 */
static int divca(unsigned int i, unsigned int j) {
  i %= 20;
  j %= 20;

  if (i >= 7 && i <= 12 && j >= 7 && j <= 12) {
    return 0;
  }
  if (i >= 4 && i <= 15 && j >= 4 && j <= 15) {
    return 1;
  }
  if (i >= 2 && i <= 17 && j >= 2 && j <= 17) {
    return 2;
  }
  return 3;
}


/* retourne la couleur que doit avoir le pixel (i,j) */
couleur pixcouleur(int i, int j) {
  couleur c = { 0, 0, 0 };
  kase *k = NULL;
  int division;

  /* on obtient un pointeur vers la case
   * de la grille correspondante
   */
  k = kor(i/20, j/20);

  if (k->quoi == 'n') {
    c.r = 0;
    c.v = 0x99;
    c.b = 0;
  } else {
    division = divca(i,j);

    /* a defaut d'autre chose on met la pheronome de nid */
    c.r = 0;
    c.v = 0;
    c.b = (k->ph_nid * 100 + 100);

    if (division < 3) {
      if (k->quoi == 's') {
	c.r = k->sucre;
	c.v = (k->sucre / 2 + 128);
	c.b = (k->ph_nid * 100 + 100);
      } else {
	c.r = k->ph_sucre;
	c.v = 0;
	c.b = (k->ph_nid * 100 + 100);
      }
    }

    if (division < 2) {
      if (k->quoi == 'f' || k->quoi == 'F') {
	c.r = c.v = c.b = 0;
      }
    }

    if (division < 1) {
      if (k->quoi == 'F') {
	c.r = 0xff;
	c.v = 0x99;
	c.b = 0;
      }
    }



  }

  return c;
}

#ifndef VERSION_SDL
/* fait le rendu de la grille */
static void rendu(unsigned long t) {
  FILE *f = NULL;
  char nom[32];
  unsigned int i, j;
  couleur c;

  /* pas overflow-safe mais c'est pour etre compatible ANSI C */
  sprintf(nom, "%08lx.ppm", t);

  f = fopen(nom, "wb");
  if (!f) {
    fprintf(stderr, "N'a pas pu ouvrir %s.\n", nom);
    exit(1);
  }

  fprintf(f, "P3\n400 400 255\n");

  for (i=0; i<400; i++) {
    for (j=0; j<400; j++) {

      /* on obtient la couleur que doit avoir le pixel */
      c = pixcouleur(i,j);

      /* on ecrit le pixel dans le fichier */
      fprintf(f, "%u %u %u\n",
	  (unsigned int) c.r,
	  (unsigned int) c.v,
	  (unsigned int) c.b);
    }
  }

  fclose(f);


}
#endif

/* execute la simulation, avec n tours */
static void simulation(unsigned long n) {
  unsigned long i;
  int a, b;

  printf("Simulation de %lu tours...\n", n);

  for (i=0; i<n || !n; i++) {
    putchar('.');
    fflush(stdout);
    rendu(i);

    /* n est nul -> on regarde si il reste du surcre
     * sur la grille
     */
    if (!n) {
      for (a=0; a<=19; a++) {
	for (b=0; b<=19; b++) {
	  if (kor(a,b)->sucre ||
	      kor(a,b)->quoi == 'F' ||
	      kor(a,b)->ph_sucre) {
	    goto encore;
	  }
	}
      }
      /* plus de sucre -> on s'arrete */
      break;
    }

encore:
    tour();
  }

  printf("\n");
  printf("Fin de la simulation : %lu tours.\n", i);
}



int main(void) {
  unsigned long n = 0;
  unsigned int nb_sucre;

  /* init de la random seed */
  inital();

  /* init de la grille */
  vider();

  printf("Nombre de tours ( Si 0 : jusqu'a la fin ) : ");
  scanf("%lu", &n);

  printf("Au bout de combien de chargement un emplacement de sucre s'est-il vide ?\n");
  do {
    printf("Veuillez entrer une valeur dans [0,255] :\n");
    scanf("%u", &nb_sucre);
  } while (nb_sucre > 255 || nb_sucre < 0 );

  /* placer les objets de la grille */
  placer(nb_sucre);
  simulation(n);

  return 0;
}

