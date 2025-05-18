#include <allegro.h>
#include <time.h>
#include <stdio.h>
#include "niv2.h"
#include "niv3.h"
#include "commun.h"  // contient toutes les fonctions utiles



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
    int quitVolontaire = 0, fin_victoire = 0;
    int kill = 0;
    int bouclier = 0;



    // Init Allegro
    initialisation_allegro();

    // Création back‐buffer
    buffer = create_bitmap(LARGEUR, HAUTEUR);
    if (!buffer) {
        allegro_message("Erreur : imposs. créer buffer");
        allegro_exit();
        return EXIT_FAILURE;
    }

    //Chargement menu + bonus
    fondmenu = charger_bitmap_avec_verification(FOND_MENU_FILE);

    while (1) {
        // Pseudo + menu principal
        menu_saisie_pseudo(pseudo, buffer, fondmenu);
        int choix = menu_principal(pseudo, buffer, fondmenu);
        if (choix == 2) break;

        // Chargement sprites jeu
        arriereplan           = charger_bitmap_avec_verification("arriereplan2.bmp");
        sprite_perso          = charger_bitmap_avec_verification("personnage1.bmp");
        sprite_missile        = charger_bitmap_avec_verification("missile.bmp");
        sprite_ennemi         = charger_bitmap_avec_verification("ennemiNiv3.bmp");
        sprite_vie            = charger_bitmap_avec_verification("vie.bmp");
        sprite_missile_ennemi = charger_bitmap_avec_verification("missile2.bmp");
        sprite_boss           = charger_bitmap_avec_verification("bossFinal.bmp");
        sprite_meteorite      = charger_bitmap_avec_verification("meteorite.bmp");
        sprite_arme           = charger_bitmap_avec_verification("ArmeNiv3Bonus.bmp");


        // Initialisations de base
        initialiser_personnage(&perso, 100, HAUTEUR/2, 80, 80, 7, sprite_perso);
        initialiser_vies(&vies, sprite_vie);
        if (choix == 1) {
            load_game(pseudo, &temps_jeu, &perso, &vies);
        } else {
            temps_jeu = TEMPS_LIMITE - 30 * 60;
            delai_en = boss_apparu = boss_vaincu = 0;

            frequence_en = 60;  // Un ennemi toutes les 60 frames
        }


        retour_menu_collision = 0;
        initialiser_ennemis(sprite_ennemi);
        initialiser_missiles();
        initialiser_missiles_ennemis();

        // Boucle jeu
        int playing = 1;
        while (playing && !key[KEY_ESC] && vies.total_vies > 0) {
            if (key[KEY_Y]) { //raccourci niv2
                lancer_niv2();
            }
            if (key[KEY_U]) {
                int res = lancer_niv3();
                if (res == 1) {
                    break;
                }
            }

            // Pause
            if (key[KEY_P]) {
                while (key[KEY_P]) rest(10);
                int act = menu_pause(pseudo, &perso, &vies, &temps_jeu, buffer);
                if (act == PAUSE_QUITTER) { quitVolontaire = 1; playing = 0; break; }
                if (act == PAUSE_RETURN_MENU) { playing = 0; break; }
            }

            // Incrémentation temps
            temps_jeu++;
            if ((temps_jeu - dernier_checkpoint) >= CHECKPOINT_INTERVAL) {
                save_game(pseudo, &temps_jeu, &perso, &vies);
                dernier_checkpoint = temps_jeu;
            }

            if (temps_jeu >= TEMPS_LIMITE && !boss_apparu) {
                playing = 0;
                lancer_niv2();
                continue;
            }



            // Apparition boss principal
            if (temps_jeu >= TEMPS_LIMITE && !boss_apparu) {
                boss.actif   = 1;
                boss_apparu  = 1;
                // Supprime tous les ennemis ordinaires à l’apparition du boss
                for (int i = 0; i < MAX_ENNEMIS; i++)
                    ennemis[i].actif = 0;
            }


            // Déplacement et tir joueur
            deplacer_personnage(&perso);
            if (key[KEY_SPACE] && delai_tir <= 0) {
                tirer_missile(&perso, mh);
                delai_tir = cadence_tir;
            }

            if (delai_tir > 0) delai_tir--;

            // Scrolling background
            bg_x -= vitesse_bg;
            if (bg_x <= -arriereplan->w) bg_x += arriereplan->w;

            // IA Boss principal
            if (boss.actif) {
                deplacer_boss(&boss);

                // Tirs
                if (--boss.delai_tir <= 0 && boss.missiles_tires < MAX_MISSILES_BOSS) {
                    tirer_missile_boss(&boss, me_h);
                    boss.delai_tir     = boss.cadence_tir;
                    boss.missiles_tires++;
                }
            }


            if (!boss_apparu && ++delai_en >= frequence_en) {
                creer_ennemi();
                delai_en = 0;
                if (frequence_en > 15) frequence_en--; // difficulté progressive
            }


            // Mises à jour générales
            mettre_a_jour_missiles();
            mettre_a_jour_missiles_ennemis();
            mettre_a_jour_ennemis();

            // Tirs ennemis normaux
            for (int i = 0; i < MAX_ENNEMIS; i++) {
                if (ennemis[i].actif) {
                    if (--ennemis[i].delai_tir <= 0) {
                        tirer_missile_ennemi(&ennemis[i], me_h);
                        ennemis[i].delai_tir = ennemis[i].cadence_tir;
                        ennemis[i].missiles_tires++;
                    }
                }
            }

            mettre_a_jour_invincibilite(&vies);

            // Collisions missiles avec ennemis et boss principal
            detecter_collisions_missiles_ennemis(mw, mh, &kill, &bouclier);

            if (boss.actif) {
                detecter_collisions_missiles_boss(&boss, mw, mh);
                if (boss.pv <= 0) boss.actif = 0;
            }


            // Collisions ennemis_joueur
            detecter_collisions_joueur_missiles_ennemis(&perso, &vies, me_w, me_h, &bouclier);
            detecter_collisions_joueur_ennemis(&perso, &vies);



            // Rendu
            clear_bitmap(buffer);

            //affichage background
            blit(arriereplan, buffer,
                 0, 0,
                 bg_x, 0,
                 arriereplan->w, HAUTEUR);

            blit(arriereplan, buffer,
                 0, 0,
                 (bg_x -4) + arriereplan ->w, 0, // enlever barre centrale noire
                 arriereplan->w, HAUTEUR);

            dessiner_ennemis(buffer);

            afficher_personnage(&perso, buffer, &vies);
            dessiner_missiles(sprite_missile, buffer, mw, mh);
            dessiner_missiles_ennemis(sprite_missile_ennemi, buffer, me_w, me_h);
            afficher_vies(&vies, buffer);

            int secondes_restantes = (TEMPS_LIMITE - temps_jeu) / 60;
            if (secondes_restantes < 0) secondes_restantes = 0;  // sécurité

            textprintf_ex(buffer, font, LARGEUR/2 - 60, 10,
                makecol(255, 255, 255), -1,
                "Temps: %d sec", secondes_restantes);


            blit(buffer, screen, 0,0, 0,0, LARGEUR, HAUTEUR);
            rest(16);
        }

        // Destruction bitmaps de la partie
        destroy_bitmap(arriereplan);
        destroy_bitmap(sprite_perso);
        destroy_bitmap(sprite_missile);
        destroy_bitmap(sprite_ennemi);
        destroy_bitmap(sprite_vie);
        destroy_bitmap(sprite_missile_ennemi);
        destroy_bitmap(sprite_boss);
        destroy_bitmap(sprite_meteorite);
    }

    // Nettoyage final
    destroy_bitmap(buffer);
    destroy_bitmap(fondmenu);
    allegro_exit();
    return 0;
}
END_OF_MAIN();
