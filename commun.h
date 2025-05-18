#ifndef COMMUN_H
#define COMMUN_H

#include <allegro.h>

// Constantes
#define FOND_MENU_FILE       "fondmenu.bmp"
#define LARGEUR              1024
#define HAUTEUR              768
#define MAX_MISSILES         20
#define MAX_MISSILES_ENNEMIS 30
#define MAX_ENNEMIS          10
#define MAX_METEO            2
#define MAX_MISSILES_BOSS    50
#define POINTS_VIE_PAR_VIE   10
#define NOMBRE_VIES          3
#define TEMPS_LIMITE         (60 * 60)
#define MAX_PSEUDO           15
#define CHECKPOINT_INTERVAL  (60 * 30)

#define PAUSE_REPRENDRE      0
#define PAUSE_RETURN_MENU    1
#define PAUSE_QUITTER        2

// Structures
typedef struct {
    int x, y, w, h, vitesse;
    BITMAP *sprite;
} Personnage;

typedef struct {
    int x, y, actif;
} Missile;

typedef struct {
    int x, y, actif;
    int vitesse;
    int vx, vy;
} MissileEnnemi;

typedef struct {
    int x, y, w, h, vitesse, actif;
    int delai_tir, cadence_tir, missiles_tires;
    int pv;
    int a_ete_actif;
    BITMAP *sprite;
} Ennemi;

typedef struct {
    int x, y, w, h;
    int pv, actif, delai_tir, cadence_tir, missiles_tires;
    BITMAP *sprite;
    int start_x, start_y, phase;
} Boss;

typedef struct {
    int total_vies, points_vie_actuels;
    int invincible, duree_invincibilite;
    BITMAP *sprite_vie;
} Vies;

typedef struct {
    int x, y, w, h, actif;
} Meteore;

// Variables globales externes
extern Missile missiles[MAX_MISSILES];
extern MissileEnnemi missiles_ennemis[MAX_MISSILES_ENNEMIS];
extern Ennemi ennemis[MAX_ENNEMIS];
extern Boss boss;
extern Meteore meteores[MAX_METEO];
extern BITMAP *sprite_arme;
extern int arme_state, arme_x, arme_y;
extern int delai_en, frequence_en;
extern int boss_apparu, boss_vaincu;
extern int retour_menu_collision;
extern int dernier_checkpoint;
extern int arme_state;
extern int arme_x;
extern int arme_y;
extern BITMAP *sprite_arme;
extern BITMAP *sprite_perso2;


// Fonctions
void initialisation_allegro(void);
BITMAP *charger_bitmap_avec_verification(const char *f);
void initialiser_personnage(Personnage*, int, int, int, int, int, BITMAP*);
void initialiser_vies(Vies*, BITMAP*);
void initialiser_ennemis(BITMAP*);
void initialiser_missiles(void);
void initialiser_missiles_ennemis(void);
void initialiser_boss(Boss*, BITMAP*, int, int);
void deplacer_personnage(Personnage*);
void deplacer_boss(Boss*);
void deplacer_mini_boss(Boss*, int);
void tirer_missile(Personnage*, int);
void tirer_triplet(Personnage*, int);
void tirer_missile_ennemi(Ennemi*, int);
void tirer_missile_boss(Boss*, int);
void mettre_a_jour_missiles(void);
void mettre_a_jour_missiles_ennemis(void);
void mettre_a_jour_ennemis(void);
void mettre_a_jour_invincibilite(Vies*);
void detecter_collisions_missiles_ennemis(int mw, int mh, int *kill, int *bouclier);
void detecter_collisions_missiles_boss(Boss*, int, int);
void detecter_collisions_joueur_missiles_ennemis(Personnage *p, Vies *v, int mw, int mh, int *bouclier);
void detecter_collisions_joueur_ennemis(Personnage*, Vies*);
void detecter_collisions_joueur_boss(Personnage*, Boss*, Vies*);
void afficher_personnage(Personnage*, BITMAP*, Vies*);
void afficher_boss(Boss*, BITMAP*);
void afficher_vies(Vies*, BITMAP*);
void afficher_vies_boss(Boss*, BITMAP*, BITMAP*);
void dessiner_missiles(BITMAP*, BITMAP*, int, int);
void dessiner_missiles_ennemis(BITMAP*, BITMAP*, int, int);
void dessiner_ennemis(BITMAP*);
void creer_ennemi(void);
void deplacer_arme_bonus(void);
void detecter_collision_arme_bonus(Personnage*);
void afficher_arme_bonus(BITMAP*);
void menu_saisie_pseudo(char*, BITMAP*, BITMAP*);
int menu_principal(const char*, BITMAP*, BITMAP*);
int menu_pause(const char*, Personnage*, Vies*, int*, BITMAP*);
void save_game(const char*, int*, Personnage*, Vies*);
void load_game(const char*, int*, Personnage*, Vies*);
int lancer_niveau_info2(void);

#endif // COMMUN_H
