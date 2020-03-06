#include "game.h"
#include "engine.h"
#include "../cubeconflict/cubedef.h"

int message1, message2, message3, ctfmessage1, ctfmessage2, ctfmessage3, ctfmessage4, ctfmessage5, ctfmessage6;
int message_streak1;

string strclassetueur, straptitudevictime;

int decal_message = 0;
bool need_message1, need_message2;

namespace game
{
    void rendermessage(string message, int textsize = 100, float pos = 8.8f, int decal = 0)
    {
        int tw = text_width(message);
        float tsz = 0.04f*min(screenw, screenh)/textsize,
              tx = 0.5f*(screenw - tw*tsz), ty = screenh - 0.075f*pos*min(screenw, screenh+decal) - textsize*tsz;
        pushhudmatrix();
        hudmatrix.translate(tx, ty, 0);
        hudmatrix.scale(tsz, tsz, 1);
        flushhudmatrix();
        draw_text(message, 0, 0);
        pophudmatrix();
    }

    void drawmessages(int killstreak, string str_pseudovictime, int n_aptitudevictime, string str_pseudoacteur, int n_killstreakacteur, float killdistance)
    {
        decal_message = 0, need_message1 = true, need_message2 = true;

        if(totalmillis-message1<=2500)
        {
            string streakmsg;

            switch(killstreak)
            {
                case 3: formatstring(streakmsg, "Triplette ! \fc(x%d)", killstreak); break;
                case 5: formatstring(streakmsg, "Pentaplette ! \fc(x%d)", killstreak); break;
                case 10: formatstring(streakmsg, "D�caplette ! \fc(x%d)", killstreak); break;
                case 20: formatstring(streakmsg, "Eicoplette ! \fc(x%d)", killstreak); break;
                case 30: formatstring(streakmsg, "Triaconplette ! \fc(x%d)", killstreak); break;
                default : need_message1 = false;
            }

            if(need_message1) {rendermessage(streakmsg, 85, 8.8f, decal_message); decal_message -= 45;}
        }

        if(totalmillis-message2<=2500)
        {
            string killmsg;

            formatstring(killmsg, "Tu as tu� \fc%s \f7! (%s � %.1fm)", str_pseudovictime, aptitudes[n_aptitudevictime].apt_nom, killdistance);
            rendermessage(killmsg, 100, 8.8f, decal_message);
            decal_message -= 40;
        }

        if(totalmillis-message3<=2500)
        {
            string infomsg;

            switch(n_killstreakacteur)
            {
                case 3: formatstring(infomsg, "\fc%s\f7 est chaud ! (Triplette)", str_pseudoacteur); break;
                case 5: formatstring(infomsg, "\fc%s\f7 domine ! (Pentaplette)", str_pseudoacteur); break;
                case 10: formatstring(infomsg, "\fc%s\f7 est inarr�table ! (D�caplette)", str_pseudoacteur); break;
                case 20: formatstring(infomsg, "\fc%s\f7 est invincible ! (Eicoplette)", str_pseudoacteur); break;
                case 30: formatstring(infomsg, "\fc%s\f7 est un Dieu ! (Triaconplette)", str_pseudoacteur); break;
                default: need_message2 = false;
            }

            if(need_message2) {rendermessage(infomsg, 100, 8.8f, decal_message); decal_message -= 40;}
        }

        if(m_ctf)
        {
            string infomsg;
            if(totalmillis-ctfmessage1<=3000) {formatstring(infomsg, "\f9Notre �quipe a marqu� un point !"); rendermessage(infomsg, 100, 8.8f, decal_message); decal_message -= 40;}
            if(totalmillis-ctfmessage2<=3000) {formatstring(infomsg, "\f3L'�quipe ennemie a marqu� un point."); rendermessage(infomsg, 100, 8.8f, decal_message); decal_message -= 40;}
            if(totalmillis-ctfmessage3<=3000) {formatstring(infomsg, "\f9Notre �quipe a r�cup�r� son drapeau !"); rendermessage(infomsg, 100, 8.8f, decal_message); decal_message -= 40;}
            if(totalmillis-ctfmessage4<=3000) {formatstring(infomsg, "\f3L'�quipe ennemie a r�cup�r� son drapeau."); rendermessage(infomsg, 100, 8.8f, decal_message); decal_message -= 40;}
            if(totalmillis-ctfmessage5<=3000) {formatstring(infomsg, "\f9Notre �quipe a vol� le drapeau ennemi !"); rendermessage(infomsg, 100, 8.8f, decal_message); decal_message -= 40;}
            if(totalmillis-ctfmessage6<=3000) {formatstring(infomsg, "\f3L'�quipe ennemie a vol� notre drapeau !"); rendermessage(infomsg, 100, 8.8f, decal_message); decal_message -= 40;}
        }


        if(player1->state==CS_DEAD)
        {
            string killedbymsg, withmsg, waitmsg;
            if(suicided) formatstring(killedbymsg, "Tu t'es suicid� !");
            else formatstring(killedbymsg, "Tu� par %s (%s)", str_pseudotueur, aptitudes[n_aptitudetueur].apt_nom);

            rendermessage(killedbymsg, 65, 1.5f, 0);
            formatstring(withmsg, "Avec %s", str_armetueur);
            rendermessage(withmsg, 95, 1.5f, -360);

            int wait = cmode ? cmode->respawnwait(player1) : (lastmillis < player1->lastpain + 1000) ? 1 : 0 ;
            if(wait>0) formatstring(waitmsg, "Respawn possible dans %d seconde%s", wait, wait<=1?"":"s");
            else formatstring(waitmsg, "Appuie n'importe o� pour revivre ! ");
            rendermessage(waitmsg, 95, 1.5f, -580);
        }

        if(player1->state==CS_SPECTATOR)
        {
            string spectatormsg, color;

            gameent *f = followingplayer();

            if(f)
            {
                f->state!=CS_DEAD ? formatstring(color, "\f7") : formatstring(color, "\fg") ;

                if(f->privilege)
                {
                    f->privilege>=PRIV_ADMIN ? formatstring(color, "\fc") : formatstring(color, "\f7");
                    if(f->state==CS_DEAD) formatstring(color, "\fg") ;
                }
                formatstring(spectatormsg, "Spectateur : %s%s", color, colorname(f));
            }
            else
            {
                formatstring(spectatormsg, "Spectateur : Cam�ra libre");
            }
            rendermessage(spectatormsg, 75, 2.0f);
        }
    }

    void gameplayhud(int w, int h)
    {
        gameent *d = hudplayer();
        if(d->state==CS_EDITING || d->state==CS_SPECTATOR) return;
        else
        {
            pushhudscale(h/1800.0f);
            if(cmode) cmode->drawhud(d, w, h);
            pophudmatrix();
        }

        if(zoom && crosshairsize >= 31) {crosshairsize -= 3; if(crosshairsize<31) crosshairsize = 31;}
        else if (crosshairsize<40) crosshairsize += 3;
        zoomfov = guns[player1->gunselect].maxzoomfov;

        if((player1->gunselect == GUN_SKS  || player1->gunselect == GUN_SV98 || player1->gunselect==GUN_ARBALETE || player1->gunselect==GUN_S_CAMPOUZE || player1->gunselect==GUN_S_ROQUETTES) && zoom == 1)
        {
            gle::colorf(crosshairalpha, crosshairalpha, crosshairalpha, crosshairalpha);

            if(player1->gunselect==GUN_S_ROQUETTES) settexture("media/interface/hud/viseurA.png");
            if(player1->gunselect==GUN_SKS) settexture("media/interface/hud/viseurC.png");
            else settexture("media/interface/hud/viseurB.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(player1->ragemillis>0)
        {
            if(player1->ragemillis>1000) gle::colorf(1, 1, 1, 1);
            else gle::colorf(player1->ragemillis/1000.0f, player1->ragemillis/1000.0f, player1->ragemillis/1000.0f, player1->ragemillis/1000.0f);

            settexture("media/interface/hud/ragescreen.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(d->aptisort3 && d->aptitude==APT_MAGICIEN)
        {
            gle::colorf(1, 1, 1, 0.7f);

            settexture("media/interface/hud/mageprotectionscreen.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(d->aptisort2 && d->aptitude==APT_PHYSICIEN)
        {
            gle::colorf(0.3, 0.6, 1, 0.7f);

            settexture("media/interface/hud/mageprotectionscreen.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        if(player1->health<500)
        {
            gle::colorf((-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f, (-(player1->health)+700)/1000.0f);
            settexture("media/interface/hud/damage.png");
            bgquad(0, 0, w, h);

            gle::colorf(1, 1, 1, 1);
        }

        pushhudmatrix();

        //////////////////////////////////////// RENDU DES IMAGES ////////////////////////////////////////

        if(d->state==CS_DEAD || player1->state==CS_SPECTATOR)
        {
            if(d->state==CS_DEAD)
            {
                settexture("media/interface/hud/mort.png");
                bgquad(15, h-130, 115, 115);
            }
            pophudmatrix();
            return;
        }

        if(player1->gunselect == GUN_CAC349 || player1->gunselect == GUN_CACFLEAU || player1->gunselect == GUN_CACMARTEAU || player1->gunselect == GUN_CACMASTER) settexture("media/interface/hud/epee.png");
        else settexture("media/interface/hud/balle.png");
        bgquad(w-130, h-130, 115, 115);

        settexture("media/interface/hud/coeur.png");
        bgquad(15, h-130, 115, 115);

        if(d->armour)
        {
            switch(d->armourtype)
            {
                case A_BLUE: settexture("media/interface/hud/bouclier_bois.png"); break;
                case A_GREEN: settexture("media/interface/hud/bouclier_fer.png"); break;
                case A_YELLOW: settexture("media/interface/hud/bouclier_or.png"); break;
                case A_MAGNET: settexture("media/interface/hud/bouclier_magnetique.png"); break;
            }
            bgquad(250, h-130, 115, 115);
        }

        int decal_icon = 0;

        if(player1->crouching && player1->aptitude==APT_CAMPEUR)
        {
            settexture("media/interface/hud/campeur.png");
            bgquad(15, h-260, 115, 115);
            decal_icon += 130;
        }

        if(player1->aptitude==APT_MAGICIEN || player1->aptitude==APT_PHYSICIEN || player1->aptitude==APT_PRETRE)
        {
            settexture("media/interface/hud/mana.png");
            bgquad(15, h-260, 115, 115);
            decal_icon += 130;
        }

        if(d->aptitude==APT_MAGICIEN || d->aptitude==APT_PHYSICIEN || d->aptitude==APT_PRETRE)
        {
            float positionsorts = 0.5f*(w - 100);
            int neededdata = 0;
            switch(d->aptitude) {case APT_PHYSICIEN: neededdata++; break; case APT_PRETRE: neededdata+=2;}

            if(d->aptisort1) gle::colorf(2, 2, 2, 1);
            else if(d->mana<sorts[neededdata].mana1 || !d->sort1pret) gle::colorf(0.2, 0.2, 0.2, 1);
            else gle::colorf(1, 1, 1, 1);
            settexture(sorts[neededdata].tex1, 3);
            bgquad(positionsorts-85, h-114, 100, 100);
            gle::colorf(1, 1, 1, 1);

            if(d->aptisort2) gle::colorf(2, 2, 2, 1);
            else if(d->mana<sorts[neededdata].mana2 || !d->sort2pret) gle::colorf(0.2, 0.2, 0.2, 1);
            else gle::colorf(1, 1, 1, 1);
            settexture(sorts[neededdata].tex2, 3);
            bgquad(positionsorts, h-114, 100, 100);
            gle::colorf(1, 1, 1, 1);

            if(d->aptisort3) gle::colorf(2, 2, 2, 1);
            else if(d->mana<sorts[neededdata].mana3 || !d->sort3pret) gle::colorf(0.2, 0.2, 0.2, 1);
            else gle::colorf(1, 1, 1, 1);
            settexture(sorts[neededdata].tex3, 3);
            bgquad(positionsorts+85, h-114, 100, 100);
            gle::colorf(1, 1, 1, 1);
        }

        if(player1->ragemillis){settexture("media/interface/hud/rage.png"); bgquad(15, h-260, 115, 115); decal_icon += 130;}
        if(player1->steromillis){settexture("media/interface/hud/steros.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->epomillis){settexture("media/interface/hud/epo.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->champimillis){settexture("media/interface/hud/champis.png"); bgquad(15, h-260-decal_icon, 115, 115); decal_icon += 130;}
        if(player1->jointmillis){settexture("media/interface/hud/joint.png"); bgquad(15, h-260-decal_icon, 115, 115);}

        float lxbarvide = 0.5f*(w - 966), lxbarpleine = 0.5f*(w - 954);

        settexture("media/interface/hud/barrexppleine.png", 3);
        bgquad(lxbarpleine, h-19, (pourcents + 1)*954.0f, 19);

        settexture("media/interface/hud/barrexpvide.png", 3);
        bgquad(lxbarvide, h-29, 966, 40);

        //////////////////////////////////////// RENDU DES NOMBRES ////////////////////////////////////////

        int decal_number = 0;

        switch(player1->gunselect)
        {
            case GUN_S_CAMPOUZE: case GUN_S_GAU8: case GUN_S_NUKE: case GUN_S_ROQUETTES: draw_textf("%d", (d->ammo[d->gunselect] > 99 ? w-227 : d->ammo[d->gunselect] > 9 ? w-196 : w-166), h-103, d->ammo[d->gunselect]); break;
            default:
            {
                if(m_random)
                {
                    settexture("media/interface/hud/inf.png"); bgquad(w-227, h-130, 115, 115);
                }
                else draw_textf("%d", (d->ammo[d->gunselect] > 99 ? w-227 : d->ammo[d->gunselect] > 9 ? w-196 : w-166), h-103, d->ammo[d->gunselect]);
            }
        }

        draw_textf("%d", 135, h-103, d->health < 9 ? 1 : d->health/10);
        if(d->armour > 0) draw_textf("%d", 370, h-103, d->armour < 9 ? 1 : d->armour/10);


        if(player1->aptitude==APT_MAGICIEN || player1->aptitude==APT_PHYSICIEN || player1->aptitude==APT_PRETRE) {draw_textf("%d", 135, h-233-decal_number, d->mana); decal_number +=130;}
        if(player1->crouching && player1->aptitude==9) decal_number +=130;
        if(player1->ragemillis) {draw_textf("%d", 135, h-233-decal_number, d->ragemillis/1000); decal_number +=130;}
        if(player1->steromillis) {draw_textf("%d", 135, h-233-decal_number, d->steromillis/1000); decal_number +=130;}
        if(player1->epomillis) {draw_textf("%d", 135, h-233-decal_number, d->epomillis/1000); decal_number +=130;}
        if(player1->champimillis) {draw_textf("%d", 135, h-233-decal_number, d->champimillis/1000); decal_number +=130;}
        if(player1->jointmillis) {draw_textf("%d", 135, h-233-decal_number, d->jointmillis/1000); decal_number +=130;}

        defformatstring(infobarrexp, "%d/%d XP - LVL %d", neededxp-(needxp-ccxp), neededxp, cclvl);

        int tw = text_width(infobarrexp);
        float tsz = 0.4f,
              tx = 0.5f*(w - tw*tsz), ty = h - 22;
        pushhudmatrix();
        hudmatrix.translate(tx, ty, 0);
        hudmatrix.scale(tsz, tsz, 1);
        flushhudmatrix();

        draw_text(infobarrexp, 0, 0);
        pophudmatrix();


        pophudmatrix();

    }
}

// Code des autres modes de jeux � remettre

    /*

    }
    */
