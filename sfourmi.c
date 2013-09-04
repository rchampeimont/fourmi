#include "sfourmi.h"
#include "fourmi.h"
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"

static SDL_Surface *screen = NULL;

static int pret = 0;

/* si l'utilisateur a appuye sur:
 * - escape => sortie du programme
 * - enter => retourne 1
 * - autre chose ou rien => retourne 0
 */
static int ee(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    /* wait for next event */
    switch (event.type) {
    case SDL_KEYDOWN:
      switch (event.key.keysym.sym) {
      case SDLK_ESCAPE:
	printf("Sortie sur demande.\n");
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
      printf("Sortie sur reception de SDL_QUIT.\n");
      exit(0);
      break;
    default:
      break;
    }    
  }
  return 0;
}


static void sinit(void) {
  if (pret) {
    return;
  }

  /* init SDL */
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

  SDL_WM_SetCaption("Rendu SDL", "Rendu SDL");

  /* on noircit la fenetre */
  SDL_FillRect(screen, NULL,
      SDL_MapRGB(screen->format, 0x0, 0x0, 0x0));
  SDL_Flip(screen);

  pret = 1;
}

void rendu(unsigned long t) {
  unsigned int i, j;
  couleur c;
  Uint16 sw;
  static int rapide = 0;

  if (!pret) {
    sinit();
  }

  sw = screen->pitch/4;

  if (SDL_MUSTLOCK(screen)) {
    if (SDL_LockSurface(screen)) {
      fprintf(stderr,
	  "Erreur fatale : %s\n",
	  SDL_GetError());
      exit(1);
    }
  }

  for (i=0; i<400; i++) {
    for (j=0; j<400; j++) {

      /* on obtient la couleur que doit avoir le pixel */
      c = pixcouleur(i,j);

      /* on ecrit le pixel sur la surface */
      *((Uint32*)screen->pixels + i*sw + j) =
	SDL_MapRGB(screen->format, c.r, c.v, c.b);
    }
  }

  if (SDL_MUSTLOCK(screen)) {
    SDL_UnlockSurface(screen);
  }

  /* on met a jour l'affichage */
  SDL_Flip(screen);

  /* donne la possibilite a l'utilisateur d'interrompre */
  if (ee()) {
    rapide = 1;
    printf("Mode rapide!\n");
  }

  /* delai */
  if (!rapide) {
    SDL_Delay(250);
  }
}
