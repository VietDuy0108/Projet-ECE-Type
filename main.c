#include <allegro.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define FOND_MENU_FILE       "fondmenu.bmp"
#define LARGEUR              1024
#define HAUTEUR              768
#define MAX_MISSILES         20
#define MAX_MISSILES_ENNEMIS 30
#define MAX_ENNEMIS          10
#define POINTS_VIE_PAR_VIE   10
#define NOMBRE_VIES          3
#define TEMPS_LIMITE         (10 * 60)    /* 1 min 30 @ 60 FPS */
#define MAX_MISSILES_BOSS    50
#define MAX_PSEUDO           15
#define MAX_METEO 2    // nombre de météorites

/* retours menu_pause */
#define PAUSE_REPRENDRE      0
#define PAUSE_RETURN_MENU    1
#define PAUSE_QUITTER        2

/* Structures */
typedef struct {
    int x, y, w, h, vitesse;
    BITMAP *sprite;
} Personnage;

typedef struct {
    int x, y, actif;
} Missile;

typedef struct {
    int x, y, actif, vitesse;
} MissileEnnemi;

typedef struct {
    int x, y, w, h, vitesse, actif;
    int delai_tir, cadence_tir, missiles_tires;
    BITMAP *sprite;
} Ennemi;

/* Boss avec mouvement paramétrique */
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

/* Globals */
Missile        missiles[MAX_MISSILES];
MissileEnnemi  missiles_ennemis[MAX_MISSILES_ENNEMIS];
Ennemi         ennemis[MAX_ENNEMIS];
Boss           boss;
Meteore meteores[MAX_METEO];

/* Variables de progression à sauvegarder */
int delai_en, frequence_en;
int boss_apparu, boss_vaincu;
int retour_menu_collision = 0;

/* Prototypes */
void initialisation_allegro(void);
BITMAP *charger_bitmap_avec_verification(const char *f);
void initialiser_personnage(Personnage*,int,int,int,int,int,BITMAP*);
void initialiser_vies(Vies*,BITMAP*);
void initialiser_ennemis(BITMAP*);
void initialiser_missiles(void);
void initialiser_missiles_ennemis(void);
void initialiser_boss(Boss*,BITMAP*,int,int);
void deplacer_personnage(Personnage*);
void deplacer_boss(Boss*);
void tirer_missile(Personnage*,int);
void tirer_missile_ennemi(Ennemi*,int);
void tirer_missile_boss(Boss*,int);
void mettre_a_jour_missiles(void);
void mettre_a_jour_missiles_ennemis(void);
void mettre_a_jour_ennemis(void);
void mettre_a_jour_invincibilite(Vies*);
void detecter_collisions_missiles_ennemis(int,int);
void detecter_collisions_joueur_missiles_ennemis(Personnage*,Vies*,int,int);
void detecter_collisions_joueur_ennemis(Personnage*,Vies*);
void detecter_collisions_missiles_boss(Boss*,int,int);
void detecter_collisions_joueur_boss(Personnage*,Boss*,Vies*);
void dessiner_missiles(BITMAP*,BITMAP*,int,int);
void dessiner_missiles_ennemis(BITMAP*,BITMAP*,int,int);
void dessiner_ennemis(BITMAP*);
void afficher_personnage(Personnage*,BITMAP*,Vies*);
void afficher_boss(Boss*,BITMAP*);
void afficher_vies(Vies*,BITMAP*);
void afficher_vies_boss(Boss*,BITMAP*,BITMAP*);
void creer_ennemi(void);
void save_game(const char*, int*, Personnage*, Vies*);
void load_game(const char*, int*, Personnage*, Vies*);
void menu_saisie_pseudo(char*,BITMAP*,BITMAP*);
int  menu_principal(const char*,BITMAP*,BITMAP*);
int  menu_pause(const char*,Personnage*,Vies*,int*,BITMAP*);

/* Implementation */

void initialisation_allegro() {
    allegro_init();
    install_keyboard();
    install_mouse();
    srand(time(NULL));
    set_color_depth(desktop_color_depth());
    if (set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, LARGEUR, HAUTEUR, 0, 0) != 0)
        if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, LARGEUR, HAUTEUR, 0, 0) != 0) {
            allegro_message("Erreur gfx");
            allegro_exit();
            exit(EXIT_FAILURE);
        }
}

BITMAP* charger_bitmap_avec_verification(const char *f) {
    BITMAP *b = load_bitmap(f, NULL);
    if (!b) {
        allegro_message("Erreur load %s", f);
        allegro_exit();
        exit(EXIT_FAILURE);
    }
    return b;
}

void initialiser_personnage(Personnage *p, int x, int y, int w, int h, int v, BITMAP *s) {
    p->x = x; p->y = y; p->w = w; p->h = h; p->vitesse = v; p->sprite = s;
}

void initialiser_vies(Vies *v, BITMAP *s) {
    v->total_vies = NOMBRE_VIES;
    v->points_vie_actuels = POINTS_VIE_PAR_VIE;
    v->invincible = 0;
    v->duree_invincibilite = 0;
    v->sprite_vie = s;
}

void initialiser_ennemis(BITMAP *s) {
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        ennemis[i].actif = 0;
        ennemis[i].sprite = s;
        ennemis[i].w = 60;
        ennemis[i].h = 60;
        ennemis[i].vitesse = 3;
        ennemis[i].delai_tir = rand() % 60;
        ennemis[i].cadence_tir = 80 + rand() % 60;
        ennemis[i].missiles_tires = 0;
    }
}

void initialiser_missiles() {
    for (int i = 0; i < MAX_MISSILES; i++)
        missiles[i].actif = 0;
}

void initialiser_missiles_ennemis() {
    for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) {
        missiles_ennemis[i].actif = 0;
        missiles_ennemis[i].vitesse = 6;
    }
}

void initialiser_boss(Boss *b, BITMAP *s, int pw, int ph) {
    b->start_x = LARGEUR - 200;
    b->start_y = HAUTEUR/2 - (ph*3)/2;
    b->x = b->start_x;
    b->y = b->start_y;
    b->w = pw * 3;
    b->h = ph * 3;
    b->pv = 10;
    b->actif = 0;
    b->delai_tir = rand() % 30;
    b->cadence_tir = 30;
    b->missiles_tires = 0;
    b->sprite = s;
    b->phase = 0;
}

void deplacer_boss(Boss *b) {
    if (!b->actif) return;
    b->phase++;
    double t = b->phase * 0.05;
    int A = 100, B = 80;
    double a = 3.0, bf = 2.0, delta = M_PI/2;
    b->x = b->start_x + (int)(A * sin(a*t + delta));
    b->y = b->start_y + (int)(B * sin(bf*t));
    if (b->x < 0) b->x = 0;
    if (b->x > LARGEUR - b->w) b->x = LARGEUR - b->w;
    if (b->y < 0) b->y = 0;
    if (b->y > HAUTEUR - b->h) b->y = HAUTEUR - b->h;
}

void deplacer_personnage(Personnage *p) {
    if (key[KEY_UP])    p->y -= p->vitesse;
    if (key[KEY_DOWN])  p->y += p->vitesse;
    if (key[KEY_LEFT])  p->x -= p->vitesse;
    if (key[KEY_RIGHT]) p->x += p->vitesse;
    if (p->x < 0) p->x = 0;
    if (p->y < 0) p->y = 0;
    if (p->x > LARGEUR - p->w) p->x = LARGEUR - p->w;
    if (p->y > HAUTEUR - p->h) p->y = HAUTEUR - p->h;
}

void tirer_missile(Personnage *p, int mh) {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!missiles[i].actif) {
            missiles[i].x = p->x + p->w;
            missiles[i].y = p->y + p->h/2 - mh/2;
            missiles[i].actif = 1;
            break;
        }
    }
}

void tirer_missile_ennemi(Ennemi *e, int mh) {
    for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) {
        if (!missiles_ennemis[i].actif) {
            missiles_ennemis[i].x = e->x;
            missiles_ennemis[i].y = e->y + e->h/2 - mh/2;
            missiles_ennemis[i].actif = 1;
            missiles_ennemis[i].vitesse = 6;
            break;
        }
    }
}

void tirer_missile_boss(Boss *b, int mh) {
    for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) {
        if (!missiles_ennemis[i].actif) {
            missiles_ennemis[i].x = b->x;
            missiles_ennemis[i].y = b->y + b->h/2 - mh/2;
            missiles_ennemis[i].actif = 1;
            missiles_ennemis[i].vitesse = 8;
            break;
        }
    }
}

void mettre_a_jour_missiles() {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (missiles[i].actif && (missiles[i].x += 12) > LARGEUR)
            missiles[i].actif = 0;
    }
}

void mettre_a_jour_missiles_ennemis() {
    for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) {
        if (missiles_ennemis[i].actif) {
            missiles_ennemis[i].x -= missiles_ennemis[i].vitesse;
            if (missiles_ennemis[i].x < 0)
                missiles_ennemis[i].actif = 0;
        }
    }
}

void mettre_a_jour_ennemis() {
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        if (ennemis[i].actif) {
            ennemis[i].x -= ennemis[i].vitesse;
            if (ennemis[i].x + ennemis[i].w < 0)
                ennemis[i].actif = 0;
        }
    }
}

void mettre_a_jour_invincibilite(Vies *v) {
    if (v->invincible && --v->duree_invincibilite <= 0)
        v->invincible = 0;
}

void detecter_collisions_missiles_ennemis(int mw, int mh) {
    for (int i = 0; i < MAX_MISSILES; i++) if (missiles[i].actif)
        for (int j = 0; j < MAX_ENNEMIS; j++) if (ennemis[j].actif)
            if (missiles[i].x < ennemis[j].x + ennemis[j].w &&
                missiles[i].x+mw > ennemis[j].x &&
                missiles[i].y < ennemis[j].y+ennemis[j].h &&
                missiles[i].y+mh> ennemis[j].y)
            {
                missiles[i].actif = 0;
                ennemis[j].actif = 0;
            }
}

void detecter_collisions_joueur_missiles_ennemis(Personnage *p, Vies *v, int mw, int mh) {
    if (v->invincible) return;
    for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) if (missiles_ennemis[i].actif)
        if (p->x < missiles_ennemis[i].x+mw &&
            p->x+p->w>missiles_ennemis[i].x &&
            p->y < missiles_ennemis[i].y+mh &&
            p->y+p->h>missiles_ennemis[i].y)
        {
            missiles_ennemis[i].actif = 0;
            if (--v->points_vie_actuels <= 0) {
                if (--v->total_vies > 0)
                    v->points_vie_actuels = POINTS_VIE_PAR_VIE;
            }
            v->invincible = 1;
            v->duree_invincibilite = 60;
            break;
        }
}

void detecter_collisions_joueur_ennemis(Personnage *p, Vies *v) {
    if (v->invincible) return;
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        if (ennemis[i].actif) {
            if (p->x < ennemis[i].x + ennemis[i].w &&
                p->x + p->w > ennemis[i].x &&
                p->y < ennemis[i].y + ennemis[i].h &&
                p->y + p->h > ennemis[i].y) {
                // Au lieu de v->total_vies = 0 etc.
                retour_menu_collision = 1;
                return;
                }
        }
    }
}


void detecter_collisions_missiles_boss(Boss *b, int mw, int mh) {
    if (!b->actif) return;
    for (int i = 0; i < MAX_MISSILES; i++) if (missiles[i].actif)
        if (missiles[i].x < b->x+b->w &&
            missiles[i].x+mw > b->x &&
            missiles[i].y < b->y+b->h &&
            missiles[i].y+mh> b->y)
        {
            missiles[i].actif = 0;
            b->pv--;
            if (b->pv <= 0) b->actif = 0;
        }
}

void detecter_collisions_joueur_boss(Personnage *p, Boss *b, Vies *v) {
    if (!b->actif || v->invincible) return;
    if (p->x < b->x+b->w && p->x+p->w> b->x &&
        p->y < b->y+b->h && p->y+p->h> b->y)
    {
        v->total_vies = 0;
        v->points_vie_actuels = 0;
    }
}

void dessiner_missiles(BITMAP *s, BITMAP *buf, int mw, int mh) {
    for (int i = 0; i < MAX_MISSILES; i++) if (missiles[i].actif)
        masked_stretch_blit(s, buf, 0,0, s->w,s->h,
                            missiles[i].x, missiles[i].y,
                            mw, mh);
}

void dessiner_missiles_ennemis(BITMAP *s, BITMAP *buf, int mw, int mh) {
    for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) if (missiles_ennemis[i].actif)
        masked_stretch_blit(s, buf, 0,0, s->w,s->h,
                            missiles_ennemis[i].x, missiles_ennemis[i].y,
                            mw, mh);
}

void dessiner_ennemis(BITMAP *buf) {
    for (int i = 0; i < MAX_ENNEMIS; i++) if (ennemis[i].actif)
        masked_stretch_blit(ennemis[i].sprite, buf, 0,0,
                            ennemis[i].sprite->w, ennemis[i].sprite->h,
                            ennemis[i].x, ennemis[i].y,
                            ennemis[i].w, ennemis[i].h);
}

void afficher_personnage(Personnage *p, BITMAP *buf, Vies *v) {
    if (!v->invincible || (v->duree_invincibilite%10 < 5))
        masked_stretch_blit(p->sprite, buf, 0,0,
                            p->sprite->w, p->sprite->h,
                            p->x, p->y,
                            p->w, p->h);
}

void afficher_boss(Boss *b, BITMAP *buf) {
    if (!b->actif) return;
    masked_stretch_blit(b->sprite, buf, 0,0,
                        b->sprite->w, b->sprite->h,
                        b->x, b->y,
                        b->w, b->h);
}

void afficher_vies(Vies *v, BITMAP *buf) {
    int px = 10, py = 10, sz = 30, esp = sz+5;
    for (int i = 0; i < v->total_vies; i++)
        masked_stretch_blit(v->sprite_vie, buf, 0,0,
                            v->sprite_vie->w, v->sprite_vie->h,
                            px + i*esp, py,
                            sz, sz);
    textprintf_ex(buf, font, px, py+sz+5, makecol(255,255,255), -1,
                  "PV: %d/%d", v->points_vie_actuels, POINTS_VIE_PAR_VIE);
}

void afficher_vies_boss(Boss *b, BITMAP *sv, BITMAP *buf) {
    if (!b->actif) return;
    int px=LARGEUR-40, py=10, sz=20, esp=sz+5;
    for (int i = 0; i < b->pv; i++)
        masked_stretch_blit(sv, buf, 0,0, sv->w,sv->h,
                            px, py + i*esp,
                            sz, sz);
}

void creer_ennemi() {
    for (int i = 0; i < MAX_ENNEMIS; i++)
        if (!ennemis[i].actif) {
            ennemis[i].x = LARGEUR;
            ennemis[i].y = 50 + rand()%(HAUTEUR-150);
            ennemis[i].actif = 1;
            ennemis[i].delai_tir = rand()%60;
            ennemis[i].cadence_tir = 80 + rand()%60;
            ennemis[i].missiles_tires = 0;
            break;
        }
}

void save_game(const char *pseudo, int *temps_jeu,
               Personnage *p, Vies *v)
{
    char fn[64];
    sprintf(fn, "%s.sav", pseudo);
    FILE *f = fopen(fn, "w");
    if (!f) return;
    fprintf(f,
        "%d %d %d %d %d %d %d %d %d\n",
        *temps_jeu,
        p->x, p->y,
        v->total_vies, v->points_vie_actuels,
        delai_en, frequence_en,
        boss_apparu, boss_vaincu
    );
    fclose(f);
}

void load_game(const char *pseudo, int *temps_jeu,
               Personnage *p, Vies *v)
{
    char fn[64];
    sprintf(fn, "%s.sav", pseudo);
    FILE *f = fopen(fn, "r");
    if (!f) {
        *temps_jeu    = 0;
        p->x          = 100;  p->y = HAUTEUR/2;
        v->total_vies = NOMBRE_VIES;
        v->points_vie_actuels = POINTS_VIE_PAR_VIE;
        delai_en      = 0;
        frequence_en  = 30;
        boss_apparu   = 0;
        boss_vaincu   = 0;
        return;
    }
    fscanf(f,
        "%d %d %d %d %d %d %d %d %d\n",
        temps_jeu,
        &p->x, &p->y,
        &v->total_vies, &v->points_vie_actuels,
        &delai_en, &frequence_en,
        &boss_apparu, &boss_vaincu
    );
    fclose(f);
}

void menu_saisie_pseudo(char *pseudo, BITMAP *buf, BITMAP *fm) {
    int pos = 0;
    pseudo[0] = '\0';
    while (1) {
        if (keypressed()) {
            int k = readkey();
            int sc = k >> 8, a = k & 0xFF;
            if (a >= 32 && a <= 126 && pos < MAX_PSEUDO) {
                pseudo[pos++] = (char)a;
                pseudo[pos] = '\0';
            } else if (sc == KEY_BACKSPACE && pos > 0) {
                pseudo[--pos] = '\0';
            } else if (sc == KEY_ENTER) {
                break;
            }
        }
        blit(fm, buf, 0, 0, 0, 0, LARGEUR, HAUTEUR);
        textprintf_ex(buf, font,
            LARGEUR/2-200, HAUTEUR/2-20,
            makecol(255,255,255), -1,
            "Entrez votre pseudo (max %d):", MAX_PSEUDO);
        textprintf_ex(buf, font,
            LARGEUR/2-200, HAUTEUR/2+20,
            makecol(255,255,0), -1,
            "> %s_", pseudo);
        blit(buf, screen, 0, 0, 0, 0, LARGEUR, HAUTEUR);
        rest(16);
    }
}

int menu_principal(const char *pseudo, BITMAP *buf, BITMAP *fm) {
    const char *opt[3] = {"Nouvelle partie","Continuer","Quitter"};
    int choix = 0, n = 3;
    char fn[64];
    sprintf(fn, "%s.sav", pseudo);
    FILE *f = fopen(fn, "rb");
    int peut = (f != NULL);
    if (f) fclose(f);
    while (1) {
        if (keypressed()) {
            int k = readkey() >> 8;
            if (k == KEY_UP)
                do { choix = (choix + n - 1) % n; } while (choix == 1 && !peut);
            if (k == KEY_DOWN)
                do { choix = (choix + 1) % n; } while (choix == 1 && !peut);
            if (k == KEY_ENTER)
                return choix;
        }
        blit(fm, buf, 0,0, 0,0, LARGEUR, HAUTEUR);
        textprintf_centre_ex(buf, font,
            LARGEUR/2, HAUTEUR/2-80,
            makecol(255,255,255), -1,
            "Bienvenue %s !", pseudo);
        for (int i = 0; i < n; i++) {
            int y = HAUTEUR/2 + i*30;
            int col = (i == choix ? makecol(255,255,0) : makecol(200,200,200));
            if (i == 1 && !peut) col = makecol(100,100,100);
            textprintf_centre_ex(buf, font,
                LARGEUR/2, y, col, -1, opt[i]);
        }
        blit(buf, screen, 0,0, 0,0, LARGEUR, HAUTEUR);
        rest(16);
    }
}

int menu_pause(const char *pseudo, Personnage *p, Vies *v, int *temps_jeu, BITMAP *buf) {
    const char *opt[3] = {"Enregistrer","Retour menu","Quitter"};
    int choix = 0;
    while (1) {
        if (keypressed()) {
            int k = readkey() >> 8;
            if (k == KEY_UP) choix = (choix+2)%3;
            if (k == KEY_DOWN) choix = (choix+1)%3;
            if (k == KEY_ENTER) {
                if (choix == 0) {
                    save_game(pseudo, temps_jeu, p, v);
                } else if (choix == 1) {
                    return PAUSE_RETURN_MENU;
                } else {
                    return PAUSE_QUITTER;
                }
            }
        }
        blit(buf, screen, 0,0, 0,0, LARGEUR, HAUTEUR);
        rectfill(screen,
            LARGEUR/2-150, HAUTEUR/2-60,
            LARGEUR/2+150, HAUTEUR/2+60,
            makecol(0,0,0));
        textprintf_centre_ex(screen, font,
            LARGEUR/2, HAUTEUR/2-40,
            makecol(255,255,255), -1,
            "=== PAUSE ===");
        for (int i = 0; i < 3; i++) {
            int col = (i==choix ? makecol(255,255,0) : makecol(200,200,200));
            textprintf_centre_ex(screen, font,
                LARGEUR/2, HAUTEUR/2-10 + i*25,
                col, -1, opt[i]);
        }
        rest(16);
    }
}

void initialiser_meteores() {
    for (int i = 0; i < MAX_METEO; i++) {
        meteores[i].w = 100;  // largeur en pixels de ton sprite météore
        meteores[i].h =  80;  // hauteur
        // positions fixes ou aléatoires
        meteores[i].x = 200 + i * 150;
        meteores[i].y = 100 + (i%2)*200;
        meteores[i].actif = 1;
    }
}

void dessiner_meteores(BITMAP *sprite, BITMAP *buffer) {
    for (int i = 0; i < MAX_METEO; i++) {
        if (meteores[i].actif) {
            masked_stretch_blit(sprite, buffer,
                0,0, sprite->w,sprite->h,
                meteores[i].x, meteores[i].y,
                meteores[i].w, meteores[i].h);
        }
    }
}

void detecter_collisions_joueur_meteores(Personnage *p) {
    for (int i = 0; i < MAX_METEO; i++) {
        if (meteores[i].actif) {
            if (p->x < meteores[i].x + meteores[i].w &&
                p->x + p->w > meteores[i].x &&
                p->y < meteores[i].y + meteores[i].h &&
                p->y + p->h > meteores[i].y) {
                retour_menu_collision = 1;
                return;
                }
        }
    }
}

int main() {
    BITMAP *buffer, *fondmenu;
    BITMAP *arriereplan, *sprite_perso, *sprite_missile;
    BITMAP *sprite_ennemi, *sprite_vie, *sprite_missile_ennemi, *sprite_boss;
    BITMAP *sprite_meteorite;
    Personnage perso;
    Vies vies;
    char pseudo[MAX_PSEUDO+1];

    int bg_x = 0, vitesse_bg = 2;
    int mw = 30, mh = 15, me_w = 20, me_h = 10;
    int delai_tir = 0, cadence_tir = 10;
    int temps_jeu = 0;
    int quitVolontaire = 0, victoire = 0, fin_victoire = 0, retour_menu_collision = 0;

    /* 1) Init Allegro */
    initialisation_allegro();

    /* 2) Création du back‐buffer */
    buffer = create_bitmap(LARGEUR, HAUTEUR);
    if (!buffer) {
        allegro_message("Erreur : impossible de créer le buffer");
        allegro_exit();
        return EXIT_FAILURE;
    }

    /* Positions « monde » des météorites */
    const int base_meteo_x[MAX_METEO] = { 300, 800 };
    const int base_meteo_y[MAX_METEO] = { 150, 400 };

    int running = 1;
    while (running) {
        /* --- pseudo + menu principal --- */
        fondmenu = charger_bitmap_avec_verification(FOND_MENU_FILE);
        menu_saisie_pseudo(pseudo, buffer, fondmenu);
        int choix = menu_principal(pseudo, buffer, fondmenu);
        destroy_bitmap(fondmenu);
        if (choix == 2) break;  /* Quitter */

        /* --- chargement des sprites de jeu --- */
        arriereplan           = charger_bitmap_avec_verification("arriereplan1.bmp");
        sprite_perso          = charger_bitmap_avec_verification("personnage1.bmp");
        sprite_missile        = charger_bitmap_avec_verification("missile.bmp");
        sprite_ennemi         = charger_bitmap_avec_verification("ennemi.bmp");
        sprite_vie            = charger_bitmap_avec_verification("vie.bmp");
        sprite_missile_ennemi = charger_bitmap_avec_verification("missile2.bmp");
        sprite_boss           = charger_bitmap_avec_verification("BOSS1.bmp");
        sprite_meteorite      = charger_bitmap_avec_verification("meteorite.bmp");

        /* Initialise le joueur et ses vies */
        initialiser_personnage(&perso, 100, HAUTEUR/2, 80, 80, 7, sprite_perso);
        initialiser_vies(&vies, sprite_vie);

        /* Nouvelle partie ou continuer */
        if (choix == 1) {
            load_game(pseudo, &temps_jeu, &perso, &vies);
        } else {
            temps_jeu    = 0;
            delai_en     = 0;
            frequence_en = 30;
            boss_apparu  = 0;
            boss_vaincu  = 0;
        }
        retour_menu_collision = 0;

        /* Initialise ennemis, boss, missiles */
        initialiser_ennemis(sprite_ennemi);
        initialiser_boss(&boss, sprite_boss, perso.w, perso.h);
        initialiser_missiles();
        initialiser_missiles_ennemis();

        /* Taille à l’écran des météorites (10% du sprite) */
        float scale = 0.10f;
        int w_meteo = sprite_meteorite->w * scale;
        int h_meteo = sprite_meteorite->h * scale;

        /* Boucle de jeu */
        int playing = 1;
        while (playing && !key[KEY_ESC] && vies.total_vies > 0) {
            /* Pause */
            if (key[KEY_P]) {
                while (key[KEY_P]) rest(10);
                int act = menu_pause(pseudo, &perso, &vies, &temps_jeu, buffer);
                if (act == PAUSE_QUITTER) { quitVolontaire = 1; running = 0; playing = 0; break; }
                if (act == PAUSE_RETURN_MENU) { playing = 0; break; }
            }

            /* Compte le temps et apparition du boss */
            temps_jeu++;
            if (temps_jeu >= TEMPS_LIMITE && !boss_apparu) {
                boss.actif   = 1;
                boss_apparu  = 1;
            }

            /* Scrolling du fond */
            bg_x -= vitesse_bg;
            if (bg_x <= -arriereplan->w) bg_x += arriereplan->w;

            /* Déplacement et tir du joueur */
            deplacer_personnage(&perso);
            if (key[KEY_SPACE] && delai_tir <= 0) {
                tirer_missile(&perso, mh);
                delai_tir = cadence_tir;
            }
            if (delai_tir > 0) delai_tir--;

            /* Tirs boss / ennemis */
            if (boss.actif) {
                deplacer_boss(&boss);
                if (--boss.delai_tir <= 0 && boss.missiles_tires < MAX_MISSILES_BOSS) {
                    tirer_missile_boss(&boss, me_h);
                    boss.delai_tir     = boss.cadence_tir;
                    boss.missiles_tires++;
                }
            } else if (!boss_vaincu) {
                if (++delai_en >= frequence_en) {
                    creer_ennemi();
                    delai_en     = 0;
                    frequence_en = 30 + rand()%50;
                }
                for (int i = 0; i < MAX_ENNEMIS; i++) {
                    if (ennemis[i].actif && ennemis[i].missiles_tires < 3 && --ennemis[i].delai_tir <= 0) {
                        tirer_missile_ennemi(&ennemis[i], me_h);
                        ennemis[i].delai_tir     = ennemis[i].cadence_tir;
                        ennemis[i].missiles_tires++;
                    }
                }
            }

            /* Mises à jour des entités */
            mettre_a_jour_missiles();
            mettre_a_jour_missiles_ennemis();
            mettre_a_jour_ennemis();
            mettre_a_jour_invincibilite(&vies);

            /* Collisions missiles → ennemis */
            detecter_collisions_missiles_ennemis(mw, mh);

            /* Collisions missiles → boss (DÉGÂTS) */
            if (boss.actif) {
                detecter_collisions_missiles_boss(&boss, mw, mh);
                if (boss.pv <= 0) {
                    /* On a tué le boss */
                    boss.actif   = 0;
                    boss_vaincu  = 1;
                    victoire     = 1;
                    fin_victoire = 1;
                    playing      = 0;
                    running      = 0;
                    break;
                }
            }

            /* Collisions ennemis/missiles ennemis → joueur */
            detecter_collisions_joueur_missiles_ennemis(&perso, &vies, me_w, me_h);
            detecter_collisions_joueur_ennemis(&perso, &vies);
            if (boss.actif) detecter_collisions_joueur_boss(&perso, &boss, &vies);

            /* Collisions joueur ↔ météorites → retour menu */
            for (int i = 0; i < MAX_METEO; i++) {
                int mx = base_meteo_x[i] + bg_x;
                if (mx < -w_meteo) mx += arriereplan->w;
                if (perso.x < mx + w_meteo &&
                    perso.x + perso.w > mx &&
                    perso.y < base_meteo_y[i] + h_meteo &&
                    perso.y + perso.h > base_meteo_y[i]) {
                    retour_menu_collision = 1;
                    break;
                }
            }
            if (retour_menu_collision) { retour_menu_collision = 0; playing = 0; break; }

            /* --- Rendu --- */
            clear_bitmap(buffer);
            blit(arriereplan, buffer, 0,0, bg_x,0, arriereplan->w, HAUTEUR);
            blit(arriereplan, buffer, 0,0, bg_x+arriereplan->w,0, arriereplan->w, HAUTEUR);

            /* Dessin des météorites */
            for (int i = 0; i < MAX_METEO; i++) {
                int mx = base_meteo_x[i] + bg_x;
                if (mx < -w_meteo) mx += arriereplan->w;
                masked_stretch_blit(sprite_meteorite, buffer,
                    0,0, sprite_meteorite->w, sprite_meteorite->h,
                    mx, base_meteo_y[i],
                    w_meteo, h_meteo);
            }

            dessiner_ennemis(buffer);
            if (boss.actif)   afficher_boss(&boss, buffer);
            afficher_personnage(&perso, buffer, &vies);
            dessiner_missiles(sprite_missile, buffer, mw, mh);
            dessiner_missiles_ennemis(sprite_missile_ennemi, buffer, me_w, me_h);
            afficher_vies(&vies, buffer);
            if (boss.actif) afficher_vies_boss(&boss, sprite_vie, buffer);

            /* Texte temps / boss / victoire */
            if (!boss_apparu) {
                int tt = (TEMPS_LIMITE - temps_jeu) / 60;
                textprintf_ex(buffer, font, LARGEUR/2-50, 10,
                               makecol(255,255,0), -1,
                               "Temps: %02d:%02d", tt/60, tt%60);
            }
            else if (boss.actif) {
                textprintf_ex(buffer, font, LARGEUR/2-50, 10,
                               makecol(255,0,0), -1,
                               "BOSS!");
            }
            else if (boss_vaincu) {
                textprintf_ex(buffer, font, LARGEUR/2-80, 10,
                               makecol(0,255,0), -1,
                               "VICTOIRE!");
            }

            blit(buffer, screen, 0,0, 0,0, LARGEUR, HAUTEUR);
            rest(16);
        }

        /* Destruction des bitmaps de la partie */
        destroy_bitmap(arriereplan);
        destroy_bitmap(sprite_perso);
        destroy_bitmap(sprite_missile);
        destroy_bitmap(sprite_ennemi);
        destroy_bitmap(sprite_vie);
        destroy_bitmap(sprite_missile_ennemi);
        destroy_bitmap(sprite_boss);
        destroy_bitmap(sprite_meteorite);
    }

    /* 4) Écran final */
    clear_bitmap(buffer);
    if (fin_victoire) {
        textprintf_centre_ex(buffer, font,
            LARGEUR/2, HAUTEUR/2,
            makecol(0,255,0), -1,
            "VICTOIRE !");
    }

    blit(buffer, screen, 0,0, 0,0, LARGEUR, HAUTEUR);
    rest(3000);

    destroy_bitmap(buffer);
    allegro_exit();
    return 0;
}
END_OF_MAIN();
