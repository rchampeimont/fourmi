/*
 * I, Raphael Champeimont, the author of this program,
 * hereby release it into the public domain.
 * This applies worldwide.
 * 
 * In case this is not legally possible:
 * I grant anyone the right to use this work for any purpose,
 * without any conditions, unless such conditions are required by law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "SDL.h"
#include "SDL_image.h"

/* pour une liste chainee de surfaces */
typedef struct sl_s {
  SDL_Surface *s;
  struct sl_s *next;
} sl;

int ee(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    /* wait for next event */
    switch (event.type) {
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
	printf("Sortie sur demande\n");
	exit(0);
	break;
      case SDLK_RETURN:
      case SDLK_KP_ENTER:
	return 1;
	break;
      default:
	break;
      }
      break;
    case SDL_QUIT:
      printf("Sortie apres reception d'un SDL_QUIT\n");
      exit(0);
      break;
    default:
      break;
    }    
  }
  return 0;
}

int main(int argc, char **argv) {
  sl first = { NULL, NULL };
  sl *cur = NULL;
  FILE *f = NULL;
  char nom[32] = "";
  SDL_Surface *screen = NULL;
  unsigned long t = 0;
  const char *app = "lecture des fichiers PPM";

  /* init SDL */
  printf("Initilisation de SDL...\n");
  if (SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "Echec de SDL_Init()\n");
    exit(1);
  }
  atexit(SDL_Quit);

  if (!(screen = SDL_SetVideoMode(400, 400, 32,
	  SDL_HWSURFACE | SDL_DOUBLEBUF))) {
    fprintf(stderr, "%s\n", SDL_GetError());
    exit(1);
  }

  SDL_WM_SetCaption(app, app);

  /* on noircit la fenetre */
  SDL_FillRect(screen, NULL,
      SDL_MapRGB(screen->format, 0x0, 0x0, 0x0));
  SDL_Flip(screen);
  
  /* chargement */
  printf("Chargement des fichiers :\n");
  cur = &first;
  for (t = 0; t <= 1000000000UL; t++) {
    sprintf(nom, "%08lx.ppm", t);
    /* test de l'existence du fichier */
    f = fopen(nom, "rb");
    if (!f) {
      putchar('\n');
      printf("N'a pas pu ouvrir %s, arret du chargement.\n",
	  nom);
    } else {
      fclose(f);
      f = NULL;
      putchar('.');
      fflush(stdout);
    }

    cur->s = IMG_Load(nom);
    if (!cur->s) {
      printf("Erreur lors de l'ouverture du fichier %s\n",
	  nom);
      goto lire;
    }

    /* fichier suivant */
    cur->next = malloc(sizeof(sl));
    if (!cur->next) {
      fprintf(stderr,
	  "Erreur fatale : echec de malloc()\n");
      exit(1);
    }
    cur = cur->next;
    cur->s = NULL;
    cur->next = NULL;
  }

lire:

  /* on blanchit la fenetre */
  SDL_FillRect(screen, NULL,
      SDL_MapRGB(screen->format, 0x0, 0xff, 0x0));
  SDL_Flip(screen);

  for (;;) {

    printf("Appuyez sur ENTREE quand vous voulez lire.\n");

    while (!ee()) {
      SDL_Delay(100);
    }

    /* lire */
    printf("Affichage :\n");
    
    for (cur = &first; cur; cur = cur->next) {
      if (!cur->s) {
	continue;
      }
      
      /* on affiche l'image */
      SDL_BlitSurface(cur->s, NULL, screen, NULL);
      SDL_Flip(screen);
      putchar('.');
      fflush(stdout);

      /* on permet a l'utilisateur de quitter avec echap */
      ee();

      /* on attend 300ms */
      SDL_Delay(300);
    }
    putchar('\n');

    printf("Fin.\n");

  }

  return 0;
}
