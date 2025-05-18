
#include "commun.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <math.h>

Missile missiles[MAX_MISSILES];
MissileEnnemi missiles_ennemis[MAX_MISSILES_ENNEMIS];
Ennemi ennemis[MAX_ENNEMIS];
Boss boss;
Meteore meteores[MAX_METEO];
int delai_en, frequence_en;
int boss_apparu, boss_vaincu;
int retour_menu_collision = 0;
int dernier_checkpoint = 0;

int kill_count = 0;

int arme_state = 0;
int arme_x = 0;
int arme_y = 0;
BITMAP *sprite_arme = NULL;
BITMAP *sprite_perso2 = NULL;


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

        ennemis[i].actif           = 0;
        ennemis[i].a_ete_actif     = 0;
        ennemis[i].sprite          = s;
        ennemis[i].w               = 60;
        ennemis[i].h               = 60;
        ennemis[i].vitesse         = 3;
        ennemis[i].delai_tir       = rand() % 60;
        ennemis[i].cadence_tir     = 80 + rand() % 60;
        ennemis[i].missiles_tires  = 0;
        ennemis[i].pv              = 3;
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
        missiles_ennemis[i].vx = -6;
        missiles_ennemis[i].vy = 0;
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

void tirer_triplet(Personnage *p, int mh) {
    int offsets[3] = {- mh, 0, + mh};
    for (int k = 0; k < 3; k++) {
        for (int i = 0; i < MAX_MISSILES; i++) {
            if (!missiles[i].actif) {
                missiles[i].x = p->x + p->w;
                missiles[i].y = p->y + p->h/2 - mh/2 + offsets[k];
                missiles[i].actif = 1;
                break;
            }
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
    int patterns[3][2] = {
        {-8, 0}, {-7, -3}, {-7, 3}
    };
    for (int d = 0; d < 3; d++) {
        for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) {
            if (!missiles_ennemis[i].actif) {
                missiles_ennemis[i].x = b->x;
                missiles_ennemis[i].y = b->y + b->h / 2 - mh / 2;
                missiles_ennemis[i].vx = patterns[d][0];
                missiles_ennemis[i].vy = patterns[d][1];
                missiles_ennemis[i].actif = 1;
                break;
            }
        }
    }
}

void mettre_a_jour_missiles(void) {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (missiles[i].actif) {
            missiles[i].x += 10;
            if (missiles[i].x > LARGEUR)
                missiles[i].actif = 0;
        }
    }
}

void mettre_a_jour_missiles_ennemis() {
    for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) {
        if (missiles_ennemis[i].actif) {
            missiles_ennemis[i].x += missiles_ennemis[i].vx;
            missiles_ennemis[i].y += missiles_ennemis[i].vy;
            if (missiles_ennemis[i].x < 0 || missiles_ennemis[i].y < 0 || missiles_ennemis[i].y > HAUTEUR)
                missiles_ennemis[i].actif = 0;
        }
    }
}

void mettre_a_jour_ennemis(void) {
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        if (ennemis[i].actif) {
            ennemis[i].x -= ennemis[i].vitesse;
            if (ennemis[i].x < -ennemis[i].w)
                ennemis[i].actif = 0;
        }
    }
}

void mettre_a_jour_invincibilite(Vies *v) {
    if (v->invincible) {
        v->duree_invincibilite--;
        if (v->duree_invincibilite <= 0)
            v->invincible = 0;
    }
}

void detecter_collisions_missiles_ennemis(int mw, int mh, int *kill, int *bouclier) {
    for (int i = 0; i < MAX_MISSILES; i++) if (missiles[i].actif)
        for (int j = 0; j < MAX_ENNEMIS; j++) if (ennemis[j].actif)
            if (missiles[i].x < ennemis[j].x + ennemis[j].w &&
                missiles[i].x+mw > ennemis[j].x &&
                missiles[i].y < ennemis[j].y + ennemis[j].h &&
                missiles[i].y+mh > ennemis[j].y)
            {
                missiles[i].actif = 0;
                ennemis[j].pv--;            // Décrémentation de la vie
                if (ennemis[j].pv <= 0) {
                    (*kill)++;
                    if (*kill == 5) {
                        *bouclier = 5; // 5 points de bouclier
                        *kill = 0;
                    }
                    ennemis[j].actif = 0; // Mort après 3 tirs
                    kill_count += 1;
                }
                break;
            }
}

void detecter_collisions_joueur_missiles_ennemis(Personnage *p, Vies *v, int mw, int mh, int *bouclier) {
    if (v->invincible) return;
    for (int i = 0; i < MAX_MISSILES_ENNEMIS; i++) if (missiles_ennemis[i].actif)
        if (p->x < missiles_ennemis[i].x+mw &&
            p->x+p->w>missiles_ennemis[i].x &&
            p->y < missiles_ennemis[i].y+mh &&
            p->y+p->h>missiles_ennemis[i].y)
        {
            missiles_ennemis[i].actif = 0;
            if (*bouclier > 0) {
                (*bouclier)--;
            } else if (--v->points_vie_actuels <= 0){

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

                // Perte vie
                v->total_vies--;
                if (v->total_vies > 0)
                    v->points_vie_actuels = POINTS_VIE_PAR_VIE;
                else
                    v->points_vie_actuels = 0;

                v->invincible = 1;
                v->duree_invincibilite = 60;
                return;
                }
        }
    }
}


void detecter_collisions_missiles_boss(Boss *b, int mw, int mh) {
    for (int i = 0; i < MAX_MISSILES; i++) {
        if (!missiles[i].actif || !b->actif) continue;
        if (missiles[i].x < b->x + b->w &&
            missiles[i].x + mw > b->x &&
            missiles[i].y < b->y + b->h &&
            missiles[i].y + mh > b->y) {
            b->pv--;
            missiles[i].actif = 0;
        }
    }
}

void dessiner_ennemis(BITMAP *buf) {
    for (int i = 0; i < MAX_ENNEMIS; i++) if (ennemis[i].actif){
        // Sprite
        masked_stretch_blit(ennemis[i].sprite, buf, 0,0,
                            ennemis[i].sprite->w, ennemis[i].sprite->h,
                            ennemis[i].x, ennemis[i].y,
                            ennemis[i].w, ennemis[i].h);
        // Barre de vie
        int bx = ennemis[i].x;
        int by = ennemis[i].y - 6;
        int bw = ennemis[i].w;
        int h  = 4;
        int filled = (bw * ennemis[i].pv) / 3;
        rectfill(buf, bx, by, bx + bw,    by + h, makecol(100,0,0));
        rectfill(buf, bx, by, bx + filled,by + h, makecol(0,200,0));
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
        if (key[KEY_P]) {
            while (key[KEY_P]) rest(10);
            return PAUSE_REPRENDRE;
        }
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
void creer_ennemi() {
    for (int i = 0; i < MAX_ENNEMIS; i++) {
        if (!ennemis[i].actif) {
            ennemis[i].x             = LARGEUR;
            ennemis[i].y             = 50 + rand() % (HAUTEUR - 150);
            ennemis[i].actif         = 1;
            ennemis[i].a_ete_actif   = 1;
            ennemis[i].delai_tir     = rand() % 60;
            ennemis[i].cadence_tir   = 80 + rand() % 60;
            ennemis[i].missiles_tires= 0;
            ennemis[i].pv = 3;
            break;
        }
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
void detecter_collisions_joueur_boss(Personnage *p, Boss *b, Vies *v) {
    if (!b->actif || v->invincible) return;

    if (p->x < b->x + b->w && p->x + p->w > b->x &&
        p->y < b->y + b->h && p->y + p->h > b->y) {

        int degats = 5;
        while (degats-- > 0) {
            if (v->points_vie_actuels > 0) {
                v->points_vie_actuels--;
            } else if (v->total_vies > 0) {
                v->total_vies--;
                v->points_vie_actuels = POINTS_VIE_PAR_VIE;
            }
        }

        v->invincible = 1;
        v->duree_invincibilite = 60;
        }
}

void deplacer_mini_boss(Boss *b, int index) {
    if (!b->actif) return;
    b->phase++;
    double t = b->phase * 0.05;

    int A = 60;  // Amplitude horizontale plus faible pour mini-boss
    int B = (HAUTEUR - b->h) / 2;
    double a = 2.0 + index;
    double bf = 0.5 + 0.3 * index;

    // Horizontal : sinus autour de start_x
    b->x = b->start_x + (int)(A * sin(a * t));

    // Vertical : sinus autour du centre vertical
    b->y = (HAUTEUR - b->h)/2 + (int)(B * sin(bf * t));


    if (b->x < 0) b->x = 0;
    if (b->x > LARGEUR - b->w) b->x = LARGEUR - b->w;
    if (b->y < 0) b->y = 0;
    if (b->y > HAUTEUR - b->h) b->y = HAUTEUR - b->h;
}

void deplacer_boss(Boss *b) {
    if (!b->actif) return;
    b->phase++;
    double t = b->phase * 0.05;

    int A = 100;                         // Amplitude horizontale
    int B = (HAUTEUR - b->h) / 2;       // Amplitude verticale maximale
    double a = 3.0, bf = 1.0;
    double delta = M_PI / 2;

    // Horizontal : sinus autour de start_x
    b->x = b->start_x + (int)(A * sin(a * t + delta));

    // Vertical : sinus autour du centre vertical (pas start_y)
    b->y = (HAUTEUR - b->h) / 2 + (int)(B * sin(bf * t));


    if (b->x < 0) b->x = 0;
    if (b->x > LARGEUR - b->w) b->x = LARGEUR - b->w;
    if (b->y < 0) b->y = 0;
    if (b->y > HAUTEUR - b->h) b->y = HAUTEUR - b->h;
}
void detecter_collision_arme_bonus(Personnage *p) {
    if (arme_state != 1) return;
    if (p->x < arme_x + 50 &&
        p->x + p->w > arme_x &&
        p->y < arme_y + 50 &&
        p->y + p->h > arme_y) {
        arme_state = 2;  // Ramassée
        }
}
void deplacer_arme_bonus() {
    if (arme_state != 1) return;

    arme_x += (rand() % 7) - 3;
    arme_y += (rand() % 7) - 3;

    // Garder l’arme dans les limites de l’écran
    if (arme_x < 0) arme_x = 0;
    if (arme_y < 0) arme_y = 0;
    if (arme_x > LARGEUR - 50) arme_x = LARGEUR - 50;
    if (arme_y > HAUTEUR - 50) arme_y = HAUTEUR - 50;
}
void afficher_arme_bonus(BITMAP *buf) {
    if (arme_state == 1)
        masked_stretch_blit(sprite_arme, buf, 0, 0,
                            sprite_arme->w, sprite_arme->h,
                            arme_x, arme_y, 50, 50);
}

