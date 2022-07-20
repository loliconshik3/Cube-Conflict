#include "game.h"
#include "engine.h"
#include "ccheader.h"
#include "customisation.h"
#include "stats.h"

float weapposside, weapposup, maxweapposside, maxweapposup, shieldside;
float maxshieldside = -15;
float crosshairalpha = 1;

VAR(UI_smiley, 0, 0, 9);
VAR(UI_cape, 0, 0, 13);
VAR(UI_tombe, 0, 0, 12);
VAR(UI_voix, 0, 0, 10);
VAR(UI_custtab, 0, 0, 3);
VAR(UI_showsteamnamebtn, 0, 0, 1);

namespace game
{
    vector<gameent *> bestplayers;
    vector<int> bestteams;

    VARP(ragdoll, 0, 1, 1);
    VARP(ragdollmillis, 0, 10000, 300000);
    VARP(ragdollfade, 0, 500, 5000);
    VARP(forceplayermodels, 0, 0, 1);
    VARP(hidedead, 0, 0, 1);

    extern int playermodel;

    vector<gameent *> ragdolls;

    void saveragdoll(gameent *d)
    {
        if(!d->ragdoll || !ragdollmillis || (!ragdollfade && lastmillis > d->lastpain + ragdollmillis)) return;
        gameent *r = new gameent(*d);
        r->lastupdate = ragdollfade && lastmillis > d->lastpain + max(ragdollmillis - ragdollfade, 0) ? lastmillis - max(ragdollmillis - ragdollfade, 0) : d->lastpain;
        r->edit = NULL;
        r->ai = NULL;
        if(d==player1) r->playermodel = playermodel;
        r->attackchan = -1;
        ragdolls.add(r);
        d->ragdoll = NULL;
    }

    void savetombe(gameent *d)
    {
        if(!ragdollmillis || (!ragdollfade && lastmillis > d->lastpain + ragdollmillis)) return;
        gameent *r = new gameent(*d);
        r->lastupdate = ragdollfade && lastmillis > d->lastpain + max(ragdollmillis - ragdollfade, 0) ? lastmillis - max(ragdollmillis - ragdollfade, 0) : d->lastpain;
        r->edit = NULL;
        r->ai = NULL;
        r->attackchan = -1;
        r->dansechan = -1;
        r->sortchan = -1;
        ragdolls.add(r);
        d->ragdoll = NULL;
    }

    void clearragdolls()
    {
        ragdolls.deletecontents();
    }

    void moveragdolls()
    {
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            if(lastmillis > d->lastupdate + ragdollmillis)
            {
                delete ragdolls.remove(i--);
                continue;
            }
            moveragdoll(d);
        }
    }

    static const int playercolors[] =
    {
        0xA12020,
        0xA15B28,
        0xB39D52,
        0x3E752F,
        0x3F748C,
        0x214C85,
        0xB3668C,
        0x523678,
        0xB3ADA3
    };

    static const int playercolorsazul[] =
    {
        0x27508A,
        0x3F748C,
        0x3B3B80,
        0x5364B5
    };

    static const int playercolorsrojo[] =
    {
        0xAC2C2A,
        0x992417,
        0x802438,
        0xA3435B
    };

    extern void changedplayercolor();
    VARFP(playercolor, 0, 4, sizeof(playercolors)/sizeof(playercolors[0])-1, changedplayercolor());
    VARFP(playercolorazul, 0, 0, sizeof(playercolorsazul)/sizeof(playercolorsazul[0])-1, changedplayercolor());
    VARFP(playercolorrojo, 0, 0, sizeof(playercolorsrojo)/sizeof(playercolorsrojo[0])-1, changedplayercolor());

    static const playermodelinfo playermodels[10] =
    {
        { { "smileys/hap/red", "smileys/hap", "smileys/hap/red" },                      { "hudgun", "hudgun", "hudgun" },   { "hap_red", "hap", "hap_red" }, true },
        { { "smileys/noel/red", "smileys/noel", "smileys/noel/red" },                   { "hudgun", "hudgun", "hudgun" },   { "noel_red", "noel", "noel_red" }, true },
        { { "smileys/malade/red", "smileys/malade", "smileys/malade/red" },             { "hudgun", "hudgun", "hudgun" },   { "malade_red", "malade", "malade_red" }, true },
        { { "smileys/content/red", "smileys/content", "smileys/content/red" },          { "hudgun", "hudgun", "hudgun" },   { "content_red", "content", "content_red" }, true },
        { { "smileys/colere/red", "smileys/colere", "smileys/colere/red" },             { "hudgun", "hudgun", "hudgun" },   { "colere_red", "colere", "colere_red" }, true },
        { { "smileys/sournois/red", "smileys/sournois", "smileys/sournois/red" },       { "hudgun", "hudgun", "hudgun" },   { "sournois_red", "sournois", "sournois_red" }, true },
        { { "smileys/fou/red", "smileys/fou", "smileys/fou/red" },                      { "hudgun", "hudgun", "hudgun" },   { "fou_red", "fou", "fou_red" }, true },
        { { "smileys/clindoeil/red", "smileys/clindoeil", "smileys/clindoeil/red" },    { "hudgun", "hudgun", "hudgun" },   { "clindoeil_red", "clindoeil", "clindoeil_red" }, true },
        { { "smileys/cool/red", "smileys/cool", "smileys/cool/red" },                   { "hudgun", "hudgun", "hudgun" },   { "cool_red", "cool", "cool_red" }, true },
        { { "smileys/bug/red", "smileys/bug", "smileys/bug/red" },                      { "hudgun", "hudgun", "hudgun" },   { "bug_red", "bug", "bug_red" }, true },
    };

    extern void changedplayermodel();
    VARFP(playermodel, 0, 0, sizeof(playermodels)/sizeof(playermodels[0])-1, changedplayermodel());

    int chooserandomtraits(int seed, int trait)
    {
        switch(trait)
        {
            case 0: return (seed&0xFFFF)%(sizeof(aptitudes)/sizeof(aptitudes[0])); //Classe
            case 1: return (seed&0xFFFF)%(sizeof(playermodels)/sizeof(playermodels[0])); //Smiley
            case 2: return (seed&0xFFFF)%(sizeof(customscapes)/sizeof(customscapes[0])); //Cape
            case 3: return (seed&0xFFFF)%(sizeof(customstombes)/sizeof(customstombes[0])); //Tombe
            case 4: return (seed&0xFFFF)%(sizeof(customsdance)/sizeof(customsdance[0])); //Voice
            default: return 0;
        }
    }

    const playermodelinfo *getplayermodelinfo(int n)
    {
        if(size_t(n) >= sizeof(playermodels)/sizeof(playermodels[0])) return NULL;
        return &playermodels[n];
    }

    const playermodelinfo &getplayermodelinfo(gameent *d)
    {
        const playermodelinfo *mdl = getplayermodelinfo(d==player1 || forceplayermodels ? playermodel : d->playermodel);
        if(!mdl) mdl = getplayermodelinfo(playermodel);
        return *mdl;
    }

    int getplayercolor(int team, int color)
    {
        #define GETPLAYERCOLOR(playercolors) \
            return playercolors[color%(sizeof(playercolors)/sizeof(playercolors[0]))];
        switch(team)
        {
            case 1: GETPLAYERCOLOR(playercolorsazul)
            case 2: GETPLAYERCOLOR(playercolorsrojo)
            default: GETPLAYERCOLOR(playercolors)
        }
    }

    ICOMMAND(getplayercolor, "ii", (int *color, int *team), intret(getplayercolor(*team, *color)));

    int getplayercolor(gameent *d, int team)
    {
        if(d==player1) switch(team)
        {
            case 1: return getplayercolor(1, playercolorazul);
            case 2: return getplayercolor(2, playercolorrojo);
            default: return getplayercolor(0, playercolor);
        }
        else return getplayercolor(team, (d->playercolor>>(5*team))&0x1F);
    }

    void changedplayermodel()
    {
        if(cust[SMI_HAP+playermodel]<= 0) {conoutf(CON_GAMEINFO, "\f3Vous ne poss�dez pas ce smiley !"); playsound(S_ERROR); player1->playermodel=0; return;}
        if(player1->clientnum < 0) player1->playermodel = playermodel;
        if(player1->ragdoll) cleanragdoll(player1);
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            if(!d->ragdoll) continue;
            if(!forceplayermodels)
            {
                const playermodelinfo *mdl = getplayermodelinfo(d->playermodel);
                if(mdl) continue;
            }
            cleanragdoll(d);
        }
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || !d->ragdoll) continue;
            if(!forceplayermodels)
            {
                const playermodelinfo *mdl = getplayermodelinfo(d->playermodel);
                if(mdl) continue;
            }
            cleanragdoll(d);
        }
    }

    void changedplayercolor()
    {
        if(player1->clientnum < 0) player1->playercolor = playercolor | (playercolorazul<<5) | (playercolorrojo<<10);
    }

    void syncplayer()
    {
        if(player1->playermodel != playermodel)
        {
            player1->playermodel = playermodel;
            addmsg(N_SWITCHMODEL, "ri", player1->playermodel);
        }

        int col = playercolor | (playercolorazul<<5) | (playercolorrojo<<10);
        if(player1->playercolor != col)
        {
            player1->playercolor = col;
            addmsg(N_SWITCHCOLOR, "ri", player1->playercolor);
        }

        addmsg(N_SENDCAPE, "ri", player1->customcape);
        addmsg(N_SENDTOMBE, "ri", player1->customtombe);
        addmsg(N_SENDDANSE, "ri", player1->customdanse);

    }

    void preloadplayermodel()
    {
        int loadshield = 1;
        loopi(5) //Preloading all shields
        {
            loopi(5)
            {
                preloadmodel(gfx::getshielddir(i, 20*(loadshield), false, true));
                preloadmodel(gfx::getshielddir(i, 20*(loadshield), true, true));
            }
            loadshield++;
        }

        loopi(14) //Preloading all capes
        {
            preloadmodel(customscapes[i].team1capedir);
            preloadmodel(customscapes[i].team2capedir);
        }

        loopi(15) preloadmodel(aptitudes[i].apt_tete); //Preloading all classe's hats
        loopi(13) preloadmodel(customstombes[i].tombedir); //Preloading all graves

        loopi(4) //Preloading all spy's disguisement
        {
            preloadmodel(costumes[i].entrainement_village_chateaux);
            preloadmodel(costumes[i].usine);
            preloadmodel(costumes[i].lune);
            preloadmodel(costumes[i].volcan);
        }

        preloadmodel("smileys/armureassistee"); //Preloading powered armor playermodel
        preloadmodel("smileys/armureassistee/red");
        preloadmodel("boosts/epo"); //Preloading boosts models
        preloadmodel("boosts/joint");
        preloadmodel("boosts/steros");
        preloadmodel("hudboost/joint");

        loopi(sizeof(playermodels)/sizeof(playermodels[0]))
        {
            const playermodelinfo *mdl = getplayermodelinfo(i);
            if(!mdl) break;
            if(i != playermodel && (!multiplayer(false) || forceplayermodels)) continue;
            if(m_teammode)
            {
                loopj(MAXTEAMS) preloadmodel(mdl->model[1+j]);
            }
            else preloadmodel(mdl->model[0]);
        }
    }

    int numanims() { return NUMANIMS; }

    void findanims(const char *pattern, vector<int> &anims)
    {
        loopi(sizeof(animnames)/sizeof(animnames[0])) if(matchanim(animnames[i], pattern)) anims.add(i);
    }

    VAR(animoverride, -1, 0, NUMANIMS-1);
    VAR(testanims, 0, 0, 1);
    VAR(testpitch, -90, 0, 90);

    VARFP(player1_cape, 0, 0, sizeof(customscapes)/sizeof(customscapes[0])-1,
    {
        if(cust[CAPE_CUBE+player1_cape]<= 0) {conoutf(CON_GAMEINFO, "\f3Vous ne poss�dez pas cette cape !"); playsound(S_ERROR); player1_cape=0; return;}
        addmsg(N_SENDCAPE, "ri", player1_cape);
        player1->customcape = player1_cape;
    });

    VARFP(player1_tombe, 0, 0, sizeof(customstombes)/sizeof(customstombes[0])-1,
    {
        if(cust[TOM_MERDE+player1_tombe]<= 0) {conoutf(CON_GAMEINFO, "\f3Vous ne poss�dez pas cette tombe !"); playsound(S_ERROR); player1_tombe=0; return;}
        addmsg(N_SENDTOMBE, "ri", player1_tombe);
        player1->customtombe = player1_tombe;
        if(player1->customtombe==10) unlockachievement(ACH_FUCKYOU);
    });

    void rendertombeplayer(gameent *d, float fade)
    {
         rendermodel(customstombes[d->customtombe].tombedir, ANIM_MAPMODEL|ANIM_LOOP, vec(d->o.x, d->o.y, d->o.z-16.0f), d->yaw, 0, 0, MDL_CULL_VFC|MDL_CULL_DIST|MDL_CULL_OCCLUDED, d, NULL, 0, 0, fade); //DEBUG
    }

    string costumemdlname, curmapname;
    bool updatestat = true;

    void renderplayer(gameent *d, const playermodelinfo &mdl, int color, int team, float fade, int flags = 0, bool mainpass = true)
    {
        int anim = ANIM_IDLE|ANIM_LOOP, lastaction = d->lastaction;

        float yaw = testanims && d==player1 ? 0 : d->yaw,
              pitch = testpitch && d==player1 ? testpitch : d->pitch;
        vec o = d->feetpos();
        int basetime = 0;
        if(animoverride) anim = (animoverride<0 ? ANIM_ALL : animoverride)|ANIM_LOOP;
        const char *mdlname = d->armourtype==A_ASSIST && d->armour>0 ? validteam(team) ? d->team==player1->team ? "smileys/armureassistee" : "smileys/armureassistee/red" : "smileys/armureassistee/red" : mdl.model[validteam(team) ? d->team==player1->team ? 1 : 2 : 0];

        if(intermission && updatestat && (validteam(team) ? bestteams.htfind(player1->team)>=0 : bestplayers.find(player1)>=0))
        {
            unlockachievement(ACH_WINNER);
            addstat(1, STAT_WINS);
            updatestat = false;
        }
        else updatestat = true;

        if(d->state==CS_DEAD && d->lastpain)
        {
            flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;

            if(d->tombepop<1.0f) d->tombepop += 3.f/nbfps;
            rendermodel(customstombes[d->customtombe].tombedir, ANIM_MAPMODEL|ANIM_LOOP, vec(d->o.x, d->o.y, d->o.z-16.0f), d->yaw, 0, 0, flags, NULL, NULL, 0, 0, d->tombepop, vec4(vec::hexcolor(color), 5));

            d->skeletonfade -= 3.f/nbfps;
            if(d->skeletonfade<0.066f) return;
            rendermodel("mapmodel/smileys/mort", ANIM_MAPMODEL, o, d->yaw+90, 0, 0, flags, NULL, NULL, 0, 0, d->skeletonfade);
            return;
        }

        //////////////////////////////////////////////////////////////////MODELES//////////////////////////////////////////////////////////////////

        modelattach a[10];
        int ai = 0;
        if(guns[d->gunselect].vwep)
        {
            int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
            if(lastaction && d->lastattack >= 0 && attacks[d->lastattack].gun==d->gunselect && lastmillis < lastaction+250)
            {
                vanim = attacks[d->lastattack].vwepanim;
                vtime = lastaction;
            }
            a[ai++] = modelattach("tag_weapon", guns[d->gunselect].vwep, vanim, vtime);
        }
        if(mainpass && !(flags&MDL_ONLYSHADOW))
        {
            d->muzzle = vec(-1, -1, -1);
            if(guns[d->gunselect].vwep) a[ai++] = modelattach("tag_muzzle", &d->muzzle);
            if(guns[d->gunselect].vwep) a[ai++] = modelattach("tag_balles", &d->balles);
        }

        ////////Boucliers////////
        if(d->armour && d->state == CS_ALIVE && camera1->o.dist(d->o))
        {
            a[ai++] = modelattach("tag_shield", gfx::getshielddir(d->armourtype, d->armour), ANIM_VWEP_IDLE|ANIM_LOOP, 0);
        }
        ////////Boosts////////
        if(d->jointmillis) a[ai++] = modelattach("tag_boost1", "boosts/joint", ANIM_VWEP_IDLE|ANIM_LOOP, 0);
        if(d->steromillis) a[ai++] = modelattach("tag_boost1", "boosts/steros", ANIM_VWEP_IDLE|ANIM_LOOP, 0);
        if(d->epomillis)   a[ai++] = modelattach("tag_boost2", "boosts/epo", ANIM_VWEP_IDLE|ANIM_LOOP, 0);

        a[ai++] = modelattach("tag_hat", aptitudes[d->aptitude].apt_tete, ANIM_VWEP_IDLE|ANIM_LOOP, 0);

        ////////Customisations////////
        if(d->customcape>=0 && d->customcape<=13)
        {
            a[ai++] = modelattach("tag_cape", d->team==player1->team ? customscapes[d->customcape].team1capedir : customscapes[d->customcape].team2capedir, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
        }

        //////////////////////////////////////////////////////////////////ANIMATIONS//////////////////////////////////////////////////////////////////
        if(d->state==CS_EDITING || d->state==CS_SPECTATOR) anim = ANIM_EDIT|ANIM_LOOP;
        else if(d->state==CS_LAGGED)                       anim = ANIM_LAG|ANIM_LOOP;

        if(intermission && d->state!=CS_DEAD)
        {
            anim = ANIM_LOSE|ANIM_LOOP;
            if(validteam(team) ? bestteams.htfind(team)>=0 : bestplayers.find(d)>=0) anim = ANIM_WIN|ANIM_LOOP;
        }
        else if(d->lasttaunt && lastmillis-d->lasttaunt<1000)
        {
            lastaction = d->lasttaunt;
            anim = ANIM_TAUNT|ANIM_LOOP;
        }
        else if(!intermission)
        {
            if(d->inwater && d->physstate<=PHYS_FALL)
            {
                anim |= (((game::allowmove(d) && (d->move || d->strafe)) || d->vel.z+d->falling.z>0 ? ANIM_SWIM : ANIM_SINK)|ANIM_LOOP)<<ANIM_SECONDARY;
                if(d->move && randomevent(0.16f*nbfps)) particle_splash(PART_EAU, d->armourtype==A_ASSIST ? 3 : 2, 120, d->o, 0x222222, 8.0f+rnd(d->armourtype==A_ASSIST ? 8 : 5), 150, 15);
            }
            else
            {
                static const int dirs[9] =
                {
                    ANIM_LEFT,  ANIM_FORWARD,   ANIM_RIGHT,
                    ANIM_LEFT,  0,              ANIM_RIGHT,
                    ANIM_LEFT,  ANIM_BACKWARD,  ANIM_RIGHT
                };
                int dir = dirs[(d->move+1)*3 + (d->strafe+1)];
                if(d->timeinair>50) anim |= ((ANIM_JUMP) | ANIM_END) << ANIM_SECONDARY;
                else if(dir && game::allowmove(d)) anim |= (dir | ANIM_LOOP) << ANIM_SECONDARY;

                if(d->move && d->physstate==PHYS_FLOOR && randomevent(0.16f*nbfps)) particle_splash(randomambience && (lookupmaterial(d->feetpos())==MAT_WATER || n_ambiance==4 || n_ambiance==8) ? PART_EAU : PART_SMOKE, d->armourtype==A_ASSIST ? 5 : 3, 120, d->feetpos(), randomambience && n_ambiance==4 ? 0x131313 : 0x333022, 6.0f+rnd(d->armourtype==A_ASSIST ? 10 : 5), 150, 15);
            }
            if(d->crouching && d->timeinair<50) anim |= (ANIM_CROUCH|ANIM_END)<<ANIM_SECONDARY;

            if((anim&ANIM_INDEX)==ANIM_IDLE && (anim>>ANIM_SECONDARY)&ANIM_INDEX) anim >>= ANIM_SECONDARY;
        }
        if(!((anim>>ANIM_SECONDARY)&ANIM_INDEX)) anim |= (ANIM_IDLE|ANIM_LOOP)<<ANIM_SECONDARY;

        if(d!=player1) flags |= MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY;
        if(d->type==ENT_PLAYER) flags |= MDL_FULLBRIGHT;
        else flags |= MDL_CULL_DIST;
        if(!mainpass) flags &= ~(MDL_FULLBRIGHT | MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY | MDL_CULL_DIST);
        float trans = d->state == CS_LAGGED ? 0.5f : 1.0f;
        if(d->aptisort2 && d->aptitude==APT_PHYSICIEN) trans = 0.f;
        else if(d->aptisort1 && d->aptitude==APT_MAGICIEN) trans = 0.7f;

        if(d->aptitude==APT_ESPION && d->aptisort1)
        {
            if(d!=hudplayer()) flags = NULL;

            vec doublepos = o;
            float posx = 25, posy = 25;
            switch(d->aptiseed)
            {
                case 1: posx=-25; posy=-25; break;
                case 2: posx=25; posy=-25; break;
                case 3: posx=-25; posy=25; break;
            }

            doublepos.add(vec(posx+(rnd(10)/20.f), posy+(rnd(10)/20.f), 0));

            rendermodel(mdlname, anim, doublepos, yaw, d->pitch>12 ? 12 : d->pitch<-25 ? -25 : pitch, 0, MDL_CULL_VFC | MDL_CULL_OCCLUDED | MDL_CULL_QUERY, d, a[0].tag ? a : NULL, basetime, 0, fade, vec4(vec::hexcolor(color), d==player1 ? 0.3f : trans));
        }

        if(d->aptitude==APT_ESPION && d->aptisort2)
        {
            switch(n_map)
            {
                case 0: case 1: case 4:
                    copystring(costumemdlname, costumes[d->aptiseed].entrainement_village_chateaux);
                    break;
                case 2: copystring(costumemdlname, costumes[d->aptiseed].usine); break;
                case 3: copystring(costumemdlname, costumes[d->aptiseed].lune); break;
                case 5: copystring(costumemdlname, costumes[d->aptiseed].volcan); break;
            }

            rendermodel(costumemdlname, anim, o, yaw, d->pitch>12 ? 12 : d->pitch<-25 ? -25 : pitch, 0, flags, d, NULL, basetime, 0, fade, vec4(vec::hexcolor(color), 1.0f));

            return;
        }

        rendermodel(mdlname, anim, o, yaw, d->pitch>12 ? 12 : d->pitch<-25 ? -25 : pitch, 0, flags, d, a[0].tag ? a : NULL, basetime, 0, fade, vec4(vec::hexcolor(color), trans));
    }

    void renderplayerui(gameent *d, const playermodelinfo &mdl, int cape, int color, int team)
    {
        int anim = ANIM_IDLE|ANIM_LOOP;

        modelattach a[4];
        int ai = 0;
        if(guns[d->gunselect].vwep)
        {
            int vanim = ANIM_VWEP_IDLE|ANIM_LOOP, vtime = 0;
            a[ai++] = modelattach("tag_weapon", guns[d->gunselect].vwep, vanim, vtime);
        }

        vec o = d->feetpos();

        float yaw = testanims && d==player1 ? 0 : d->yaw,
              pitch = testpitch && d==player1 ? testpitch : d->pitch;

        const char *mdlname = mdl.model[validteam(team) ? team : 0];
        a[ai++] = modelattach("tag_hat", aptitudes[player1->aptitude].apt_tete, ANIM_VWEP_IDLE|ANIM_LOOP, 0);
        a[ai++] = modelattach("tag_cape", customscapes[cape].team1capedir, ANIM_VWEP_IDLE|ANIM_LOOP, 0);

        rendermodel(mdlname, anim, o, yaw, pitch, 0, NULL, d, a[0].tag ? a : NULL, 0, 0, 1, vec4(vec::hexcolor(color), 5));
    }


    static inline void renderplayer(gameent *d, float fade = 1, int flags = 0)
    {
        int team = m_teammode && validteam(d->team) ? d->team : 0;
        renderplayer(d, getplayermodelinfo(d), getplayercolor(d, team), team, fade, flags);
    }

    void rendergame()
    {
        ai::render();

        if(intermission)
        {
            bestteams.shrink(0);
            bestplayers.shrink(0);
            if(m_teammode) getbestteams(bestteams);
            else getbestplayers(bestplayers);
        }

        bool third = isthirdperson();
        gameent *f = followingplayer(), *exclude = third ? NULL : f;
        loopv(players)
        {
            gameent *d = players[i];
            if(d == player1 || d->state==CS_SPECTATOR || d->state==CS_SPAWNING || d->lifesequence < 0 || d == exclude || (d->state==CS_DEAD && hidedead)) continue;
            renderplayer(d);

            vec dir = vec(d->o).sub(camera1->o);
            float dist = dir.magnitude();
            dir.div(dist);

            copystring(d->info, colorname(d));
            if(d->state!=CS_DEAD)
            {
                int team = m_teammode && validteam(d->team) ? d->team : 0;

                if(d->health<300 && d->health>0) switch(rnd(d->health+nbfps*2)) {case 0: gibeffect(300, d->o, d);}

                vec posA = d->abovehead();
                vec posB = camera1->o;
                vec posAtofrontofposB = (posA.add((posB.mul(vec(127, 127, 127))))).div(vec(128, 128, 128));

                if(player1->aptitude==APT_MEDECIN && team==1)
                {
                    if (d->health > 1000)
                    {
                        unsigned int lifeColor = (d->health/10 > 180) ? 80 : d->health/10-100;
                        particle_meter(d->o.dist(camera1->o)<75 ? (d->abovehead().add(camera1->o)).div(vec(2,2,2)) : posAtofrontofposB,
                                       d->health/1000.0f, PART_METER,
                                       d->o.dist(camera1->o)<250 ? 1.f : d->o.dist(camera1->o)/250.f, 0.5f,
                                       (static_cast<unsigned char>((80-lifeColor)*3.18) << 8) | (static_cast<unsigned char>(lifeColor*3.18) << 0), 0x000000,
                                       d->o.dist(camera1->o)<75 ? 1.35f : 0.02f);
                    }
                    if (d->health > 0)
                    {
                        particle_meter(d->o.dist(camera1->o)<75 ? (d->abovehead().add(camera1->o)).div(vec(2,2,2)) : posAtofrontofposB,
                                       d->health/1000.0f, PART_METER,
                                       d->o.dist(camera1->o)<250 ? 1.f : d->o.dist(camera1->o)/250.f, 0.5f,
                                       (static_cast<unsigned char>((100-d->health/10)*2.55) << 16) | (static_cast<unsigned char>(d->health/10*2.55) << 8), 0x000000,
                                       d->o.dist(camera1->o)<75 ? 1.35f : 0.02f);
                    }
                }

                if((player1->aptitude==APT_JUNKIE && team==1)&&(d->aptitude==APT_MAGICIEN || d->aptitude==APT_PHYSICIEN || d->aptitude==APT_PRETRE || d->aptitude==APT_SHOSHONE || d->aptitude==APT_ESPION))
                    particle_meter(d->o.dist(camera1->o)<75 ? (d->abovehead().add(camera1->o)).div(vec(2,2,2)) : posAtofrontofposB,
                                    d->mana/150.0f, PART_METER,
                                    d->o.dist(camera1->o)<250 ? 1.f : d->o.dist(camera1->o)/250.f, 0.5f,
                                    0xFF00FF, 0x000000,
                                    d->o.dist(camera1->o)<75 ? 1.35f : 0.02f);

                if(d->aptisort2 && d->aptitude==APT_ESPION);
                else if(((player1->aptitude==APT_ESPION && player1->aptisort3 && d!=player1) || (totalmillis-getspyability<2000)) && (!isteam(player1->team, d->team)) && d->o.dist(camera1->o) > 32)
                {
                    vec posA = d->o;
                    posA.add(vec(0, 0, -8));
                    vec posC = posA;
                    vec posB = camera1->o;

                    vec posAtofrontofposB = (posA.add((posB.mul(vec(127, 127, 127))))).div(vec(128, 128, 128));
                    int nearsize = 1.f;
                    if(d->o.dist(camera1->o) < 132) nearsize = (d->o.dist(camera1->o)-32)/100.f;
                    particle_splash(PART_VISEUR, 1, 1, d->o.dist(camera1->o)<75 ? (posC.add(camera1->o)).div(vec(2,2,2)) : posAtofrontofposB, 0xFF0000, d->o.dist(camera1->o)<75 ? 2.f*(zoom ? (guns[player1->gunselect].maxzoomfov)/100.f*nearsize : nearsize) : 0.038f*(zoom ? (guns[player1->gunselect].maxzoomfov)/100.f*nearsize : nearsize), 1, 1, 0, false, d->o.dist(camera1->o)<150 ? 1.f : d->o.dist(camera1->o)/150.f);
                }

                vec pos = d->abovehead().add(vec(0, 0,-12));

                switch(d->aptitude)
                {
                    case APT_MAGICIEN:
                        if(d->aptisort1) particle_splash(PART_SMOKE, 2, 120, d->o, 0xFF33FF, 10+rnd(5), 400,400);
                        if(d->aptisort3) particle_fireball(pos , 15.2f, PART_EXPLOSION, 5,  0x880088, 13.0f);
                        break;
                    case APT_PHYSICIEN:
                        if(d->aptisort2 && randomevent(0.05f*nbfps)) particle_splash(PART_SMOKE, 1, 300, d->o, 0x7777FF, 10+rnd(5), 400, 400);
                        if(d->aptisort3 && randomevent(0.03f*nbfps))
                        {
                            particle_splash(PART_SMOKE,  1,  150, d->feetpos(), 0x8888BB, 7+rnd(4),  100, -200);
                            particle_splash(PART_FLAME1+rnd(2),  5,  100, d->feetpos(), 0xFF6600, 1+rnd(2),  100, -20);
                        }
                        break;
                    case APT_PRETRE:
                        if(d->aptisort2) particle_fireball(pos , 16.0f, PART_ONDECHOC, 5, 0xFFFF00, 16.0f);
                        break;
                    case APT_SHOSHONE:
                        if(d->aptisort1 && randomevent(0.03f*nbfps)) particle_splash(PART_SPARK, 1, 150, d->o, 0x555555, 1+rnd(2), 200, 150);
                        if(d->aptisort2 && randomevent(0.03f*nbfps)) particle_splash(PART_SPARK, 1, 150, d->o, 0x992222, 1+rnd(2), 200, 150);
                        if(d->aptisort3 && randomevent(0.03f*nbfps)) particle_splash(PART_SPARK, 1, 150, d->o, 0xFF0000, 1+rnd(2), 200, 150);
                }

                if(d->ragemillis && randomevent(0.03f*nbfps)) particle_splash(PART_SMOKE, 2, 150, d->o, 0xFF3300, 12+rnd(5), 400, 200);
                if(d->jointmillis && randomevent(0.085f*nbfps)) regularflame(PART_SMOKE, d->abovehead().add(vec(-12, 5, -19)), 2, 3, 0x888888, 1, 1.6f, 50.0f, 1000.0f, -10);
                if(d->armourtype==A_ASSIST && d->armour>0)
                {
                    if(d->armour<1500 && randomevent(0.13f*nbfps))
                    {
                        regularflame(PART_SMOKE, d->o, 15, 3, d->armour<750 ? 0x222222 : 0x888888, 1, d->armour<750 ? 7.f : 5.f, 50.0f, 1500.0f, -10);
                        if(d->armour<1000) particle_splash(PART_FLAME2, d->armour<500 ? 2 : 1, 500, d->o, 0x992200, d->armour<500 ? 5.f : 3.f, 50, -20);
                    }
                }
            }
        }
        loopv(ragdolls)
        {
            gameent *d = ragdolls[i];
            float fade = 1.0f;
            if(ragdollmillis && ragdollfade)
                fade -= clamp(float(lastmillis - (d->lastupdate + max(ragdollmillis - ragdollfade, 0)))/min(ragdollmillis, ragdollfade), 0.0f, 1.0f);
            rendertombeplayer(d, fade);
        }
        if(exclude)
            renderplayer(exclude, 1, MDL_ONLYSHADOW);
        else if(!f && (player1->state==CS_ALIVE || (player1->state==CS_EDITING && third) || (player1->state==CS_DEAD && !hidedead)))
            renderplayer(player1, 1, third ? 0 : MDL_ONLYSHADOW);
        entities::renderentities();
        renderbouncers();
        renderprojectiles();
        if(cmode) cmode->rendergame();
    }

    VARP(hudgun, 0, 1, 1);
    VARP(hudgunsway, 0, 1, 1);

    FVAR(swaystep, 1, 35.0f, 100);
    FVAR(swayside, 0, 3, 3);
    FVAR(swayup, -1, 2, 2);

    float swayfade = 0, swayspeed = 0, swaydist = 0;
    vec swaydir(0, 0, 0);

    void swayhudgun(int curtime)
    {
        gameent *d = hudplayer();
        if(d->state != CS_SPECTATOR)
        {
            if(d->physstate >= PHYS_SLOPE)
            {
                swayspeed = min(sqrtf(d->vel.x*d->vel.x + d->vel.y*d->vel.y), 200-aptitudes[d->aptitude].apt_vitesse*0.12f);
                swaydist += swayspeed*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*swaystep);
                swayfade = 1;
            }
            else if(swayfade > 0)
            {
                swaydist += swayspeed*swayfade*curtime/1000.0f;
                swaydist = fmod(swaydist, 2*swaystep);
                swayfade -= 0.5f*(curtime*d->maxspeed)/(swaystep*1000.0f);
            }

            float k = pow(0.7f, curtime/10.0f);
            swaydir.mul(k);
            vec vel(d->vel);
            vel.add(d->falling);
            swaydir.add(vec(vel).mul((1-k)/(15*max(vel.magnitude(), d->maxspeed))));
        }
    }

    struct hudent : dynent
    {
        hudent() { type = ENT_CAMERA; }
    } guninterp;

    void drawhudmodel(gameent *d, int anim, int basetime)
    {
        const char *file = guns[d->gunselect].file;
        if(!file) return;

        vec sway;
        vecfromyawpitch(d->yaw, 0, 0, 1, sway);
        float steps = swaydist/swaystep*M_PI;
        sway.mul((swayside)*cosf(steps));
        vec weapzoom;

        if(zoom==1) sway.z = -weapposup-2; //CubeConflict, permet de faire bouger l'arme en fonction du zoom ou non
        else
        {
            sway.z = -weapposup;
            vecfromyawpitch(d->yaw, 0, -10, weapposside, weapzoom);
        }

        sway.z += swayup*(fabs(sinf(steps)) - 1);
        sway.add(swaydir).add(d->o);
        if(!hudgunsway) sway = d->o;

        vecfromyawpitch(d->yaw, 0, 0, weapposside, weapzoom);

        const playermodelinfo &mdl = getplayermodelinfo(d);
        int team = m_teammode && validteam(d->team) ? d->team : 0,
            color = getplayercolor(d, team);
        defformatstring(gunname, "%s/%s", mdl.hudguns[team], file);
        modelattach a[3];
        int ai = 0;
        d->muzzle = d->balles = vec(-1, -1, -1);
        a[ai++] = modelattach("tag_muzzle", &d->muzzle);
        a[ai++] = modelattach("tag_balles", &d->balles);
        if((d->gunselect==GUN_MINIGUN || d->gunselect==GUN_LANCEFLAMMES || d->gunselect==GUN_PULSE || d->gunselect==GUN_UZI || d->gunselect==GUN_S_GAU8) && anim!=ANIM_GUN_MELEE)
        {
            anim |= ANIM_LOOP;
            basetime = 0;
        }

        float trans = 1.0f;
        if(d->aptisort2 && d->aptitude==APT_PHYSICIEN) trans = 0.08f;
        else if(d->aptisort1 && d->aptitude==APT_MAGICIEN) trans = 0.7f;

        if(d->jointmillis)
        {
            vec sway3;
            vecfromyawpitch(d->yaw, 0, 0, 1, sway3);
            sway3.mul((swayside/1.5f)*cosf(steps));
            sway3.z += (swayup/1.5f)*(fabs(sinf(steps)) - 1);
            sway3.add(swaydir).add(d->o);

            modelattach a[2];
            d->weed = vec(-1, -1, -1);
            a[0] = modelattach("tag_joint", &d->weed);
            rendermodel("hudboost/joint", anim, sway3, d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), trans));
            if(d->weed.x >= 0) d->weed = calcavatarpos(d->weed, 12);
            if(randomevent(10)) {regularflame(PART_SMOKE, d->weed, 2, 3, 0x888888, 1, 1.3f, 50.0f, 1000.0f, -10); particle_splash(PART_FLAME2,  4, 50, d->weed, 0xFF6600, 0.6f, 20, 150);}
        }

        rendermodel(gunname, anim, weapzoom.add(sway), d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), trans));

        if(d->muzzle.x >= 0) d->muzzle = calcavatarpos(d->muzzle, 12);
        if(d->balles.x >= 0) d->balles = calcavatarpos(d->balles, 12);

        if(d->armour<=0) return;
        else
        {
            vec sway2;
            vecfromyawpitch(d->yaw, 0, 0, d->armourtype==A_ASSIST ? 1 : shieldside, sway2);
            float floatdivfactor = d->armourtype==A_ASSIST ? 6.f : 3.f;
            sway2.mul((swayside/floatdivfactor)*cosf(steps));
            sway2.z += (swayup/floatdivfactor)*(fabs(sinf(steps)) - 1);
            if(d->armourtype==A_ASSIST || !zoom) sway2.add(swaydir).add(d->o);

            rendermodel(gfx::getshielddir(d->armourtype, d->armour, true), anim, sway2, d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), trans));

            if(d->armourtype!=A_ASSIST) return;
            modelattach a[2];
            d->assist = vec(-1, -1, -1);
            a[0] = modelattach("tag_assist", &d->assist);
            rendermodel("hudshield/armureassistee/tag", anim, sway2, d->yaw, d->pitch, 0, MDL_NOBATCH, NULL, a, basetime, 0, 1, vec4(vec::hexcolor(color), trans));
            if(d->assist.x >= 0) d->assist = calcavatarpos(d->assist, 12);

            if(d->armour<1500) {switch(rnd(d->armour<1000 ? 8 : 14)){case 0: regularflame(PART_SMOKE, d->assist, 15, 3, d->armour<750 ? 0x333333 : 0x888888, 1, 3.3f, 50.0f, 1000.0f, -10);}}
            if(d->armour<1000) {switch(rnd(d->armour<600 ? 8 : 14)){case 0: particle_splash(PART_FLAME2,  1, 500, d->assist.add(vec(rnd(6)-rnd(11), rnd(6)-rnd(11), rnd(6)-rnd(11))), 0x992200, 2.5f, 70, -20);}}
        }
    }

    void drawhudgun()
    {
        gameent *d = hudplayer();
        if(d->state==CS_SPECTATOR || d->state==CS_EDITING || !hudgun || editmode)
        {
            d->muzzle = player1->muzzle = vec(-1, -1, -1);
            d->balles = player1->balles = vec(-1, -1, -1);
            return;
        }

        int anim = ANIM_GUN_IDLE|ANIM_LOOP, basetime = 0;

        if(d->lastaction && d->lastattack >= 0 && attacks[d->lastattack].gun==d->gunselect && lastmillis-d->lastaction<attacks[d->lastattack].attackdelay)
        {
            anim = attacks[d->lastattack].hudanim;
            basetime = d->lastaction;
        }
        drawhudmodel(d, anim, basetime);
    }

    void renderavatar()
    {
        drawhudgun();
    }

    void renderplayerpreview(int model, int cape, int color, int team, int weap)
    {
        static gameent *previewent = NULL;
        if(!previewent)
        {
            previewent = new gameent;
            loopi(NUMGUNS) previewent->ammo[i] = 1;
        }
        float height = previewent->eyeheight + previewent->aboveeye,
              zrad = height/1.6f;
        vec2 xyrad = vec2(previewent->xradius, previewent->yradius).max(height/3);
        previewent->o = calcmodelpreviewpos(vec(xyrad, zrad), previewent->yaw).addz(previewent->eyeheight - zrad);
        previewent->gunselect = validgun(weap) ? weap : GUN_RAIL;
        const playermodelinfo *mdlinfo = getplayermodelinfo(model);
        if(!mdlinfo) return;
        renderplayerui(previewent, *mdlinfo, cape, getplayercolor(team, color), team);
    }

    vec hudgunorigin(int gun, const vec &from, const vec &to, gameent *d)
    {
        if(d->muzzle.x >= 0) return d->muzzle;
        vec offset(from);
        if(d!=hudplayer() || isthirdperson())
        {
            vec front, right;
            vecfromyawpitch(d->yaw, d->pitch, 1, 0, front);
            offset.add(front.mul(d->radius));
            offset.z += (d->aboveeye + d->eyeheight)*0.75f - d->eyeheight;
            vecfromyawpitch(d->yaw, 0, 0, -1, right);
            offset.add(right.mul(0.5f*d->radius));
            offset.add(front);
            return offset;
        }
        offset.add(vec(to).sub(from).normalize().mul(2));
        if(hudgun)
        {
            offset.sub(vec(camup).mul(1.0f));
            offset.add(vec(camright).mul(0.8f));
        }
        else offset.sub(vec(camup).mul(0.8f));
        return offset;
    }

    void preloadweapons()
    {
        const playermodelinfo &mdl = getplayermodelinfo(player1);
        loopi(NUMGUNS)
        {
            const char *file = guns[i].file;
            if(!file) continue;
            string fname;
            if(m_teammode)
            {
                loopj(MAXTEAMS)
                {
                    formatstring(fname, "%s/%s", mdl.hudguns[1+j], file);
                    preloadmodel(fname);
                }
            }
            else
            {
                formatstring(fname, "%s/%s", mdl.hudguns[0], file);
                preloadmodel(fname);
            }
            formatstring(fname, "worldgun/%s", file);
            preloadmodel(fname);
        }
    }

    void preloadsounds()
    {
        for(int i = S_JUMP_BASIC; i <= S_DIE2; i++) preloadsound(i);
    }

    void preload()
    {
        if(hudgun) preloadweapons();
        preloadbouncers();
        preloadplayermodel();
        preloadsounds();
        entities::preloadentities();
    }

}

