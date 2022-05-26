// capture.h: client and server state for capture gamemode
#ifndef PARSEMESSAGES

#ifdef SERVMODE
struct captureservmode : servmode
#else
VARP(capturetether, 0, 1, 1);
VARP(autorepammo, 0, 1, 1);
VARP(basenumbers, 0, 0, 1);

struct captureclientmode : clientmode
#endif
{
    static const int CAPTURERADIUS = 128;
    static const int CAPTUREHEIGHT = 96;
    static const int OCCUPYBONUS = 1;
    static const int OCCUPYPOINTS = 1;
    static const int OCCUPYENEMYLIMIT = 15;
    static const int OCCUPYNEUTRALLIMIT = 10;
    static const int SCORESECS = 20;
    static const int AMMOSECS = 15;
    static const int REGENSECS = 1;
    static const int REGENHEALTH = 100;
    static const int REGENARMOUR = 100;
    static const int REGENAMMO = 20;
    static const int MAXAMMO = 5;
    static const int REPAMMODIST = 32;
    static const int RESPAWNSECS = 5;
    static const int MAXBASES = 100;

    struct baseinfo
    {
        vec o;
        string owner, enemy;
#ifndef SERVMODE
        vec ammopos;
        string name, info;
#endif
        int ammogroup, ammotype, tag, ammo, owners, enemies, converted, capturetime, bliptimer;

        baseinfo() { reset(); }

        bool valid() const { return ammotype>=0 && ammotype<=I_ARTIFICE-I_RAIL+1; }

        void noenemy()
        {
            enemy[0] = '\0';
            enemies = 0;
            converted = 0;
        }

        void reset()
        {
            noenemy();
            owner[0] = '\0';
            capturetime = -1;
            ammogroup = 0;
            ammotype = 0;
            tag = -1;
            ammo = 0;
            owners = 0;
        }

        bool enter(int team)
        {
            string tmpteam;
            formatstring(tmpteam, "%d", team);

            if(!strcmp(owner, tmpteam))
            {
                owners++;
                return false;
            }
            if(!enemies)
            {
                if(strcmp(enemy, tmpteam))
                {
                    converted = 0;
                    copystring(enemy, tmpteam);
                }
                enemies++;
                return true;
            }
            else if(strcmp(enemy, tmpteam)) return false;
            else enemies++;
            return false;
        }

        bool steal(int team)
        {
            string tmpteam;
            formatstring(tmpteam, "%d", team);

            return !enemies && strcmp(owner, tmpteam);
        }

        bool leave(int team)
        {
            string tmpteam;
            formatstring(tmpteam, "%d", team);

            if(!strcmp(owner, tmpteam) && owners > 0)
            {
                owners--;
                return false;
            }
            if(strcmp(enemy, tmpteam) || enemies <= 0) return false;
            enemies--;
            return !enemies;
        }

        int occupy(const char *team, int units)
        {
            if(strcmp(enemy, team)) return -1;
            converted += units;
            if(units<0)
            {
                if(converted<=0) noenemy();
                return -1;
            }
            else if(converted<(owner[0] ? int(OCCUPYENEMYLIMIT) : int(OCCUPYNEUTRALLIMIT))) return -1;
            if(owner[0]) { owner[0] = '\0'; converted = 0; copystring(enemy, team); return 0; }
            else { copystring(owner, team); ammo = 0; capturetime = 0; owners = enemies; noenemy(); return 1; }
        }

        bool addammo(int i)
        {
            if(ammo>=MAXAMMO) return false;
            ammo = min(ammo+i, int(MAXAMMO));
            return true;
        }

        bool takeammo(const char *team)
        {
            if(strcmp(owner, team) || ammo<=0) return false;
            ammo--;
            return true;
        }
    };

    vector<baseinfo> bases;

    struct score
    {
        string team;
        int total;
    };

    vector<score> scores;

    int captures;

    void resetbases()
    {
        bases.shrink(0);
        scores.shrink(0);
        captures = 0;
    }

    bool hidefrags() { return true; }

    int getteamscore(int team)
    {
        string tmpteam;
        formatstring(tmpteam, "%d", team);
        loopv(scores)
        {
            score &cs = scores[i];
            if(!strcmp(cs.team, tmpteam)) return cs.total;
        }
        return 0;
    }

    void getteamscores(vector<teamscore> &teamscores)
    {
        int iteamA = atoi(scores[0].team), iteamB = atoi(scores[1].team);
        teamscores.add(teamscore(iteamA, scores[0].total));
        teamscores.add(teamscore(iteamB, scores[1].total));
    }

    score &findscore(const char *team)
    {
        loopv(scores)
        {
            score &cs = scores[i];
            if(!strcmp(cs.team, team)) return cs;
        }
        score &cs = scores.add();
        copystring(cs.team, team);
        cs.total = 0;
        return cs;
    }

    void addbase(int ammotype, const vec &o)
    {
        if(bases.length() >= MAXBASES) return;
        baseinfo &b = bases.add();
        b.ammogroup = min(ammotype, 0);
        b.ammotype = rnd(I_ARTIFICE-I_RAIL+1);

        b.o = o;
        if(b.ammogroup)
        {
            loopi(bases.length()-1) if(b.ammogroup == bases[i].ammogroup)
            {
                b.ammotype = bases[i].ammotype;
                return;
            }
            int uses[I_ARTIFICE-I_RAIL+1];
            memset(uses, 0, sizeof(uses));
            loopi(bases.length()-1) if(bases[i].ammogroup)
            {
                loopj(i) if(bases[j].ammogroup == bases[i].ammogroup) goto nextbase;
                uses[bases[i].ammotype-1]++;
                nextbase:;
            }
            int mintype = 0;
            loopi(I_ARTIFICE-I_RAIL+1) if(uses[i] < uses[mintype]) mintype = i;
            int numavail = 0, avail[I_ARTIFICE-I_RAIL+1];
            loopi(I_ARTIFICE-I_RAIL+1) if(uses[i] == uses[mintype]) avail[numavail++] = i+1;
            b.ammotype = avail[rnd(numavail)];
        }
    }

    void initbase(int i, int ammotype, const char *owner, const char *enemy, int converted, int ammo)
    {
        if(!bases.inrange(i)) return;
        baseinfo &b = bases[i];
        b.ammotype = ammotype;
        copystring(b.owner, owner);
        copystring(b.enemy, enemy);
        b.converted = converted;
        b.ammo = ammo;
    }

    bool hasbases(const char *team)
    {
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.owner[0] && !strcmp(b.owner, team)) return true;
        }
        return false;
    }

    bool insidebase(const baseinfo &b, const vec &o)
    {
        float dx = (b.o.x-o.x), dy = (b.o.y-o.y), dz = (b.o.z-o.z);
        return dx*dx + dy*dy <= CAPTURERADIUS*CAPTURERADIUS && fabs(dz) <= CAPTUREHEIGHT;
    }

#ifndef SERVMODE
    static const int AMMOHEIGHT = 5;

    captureclientmode() : captures(0)
    {
    }

    void respawned(gameent *d)
    {
    }

    void replenishammo()
    {
        if(!m_capture || m_regencapture) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.valid() && insidebase(b, player1->feetpos()) && player1->hasmaxammo(b.ammotype-1+I_RAIL)) return;
        }
        addmsg(N_REPAMMO, "rc", player1);
    }

    void receiveammo(gameent *d, int type)
    {
        type += I_RAIL-1;
        if(type<I_RAIL || type>I_ARTIFICE) return;
        entities::repammo(d, type, d==player1);
    }

    void checkitems(gameent *d)
    {
        if(m_regencapture || !autorepammo || d!=player1 || d->state!=CS_ALIVE) return;
        vec o = d->feetpos();
        string tmpteam;
        formatstring(tmpteam, "%d", d->team);
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.valid() && insidebase(b, d->feetpos()) && !strcmp(b.owner, tmpteam) && b.o.dist(o) < 12)
            {
                if(d->lastrepammo!=i)
                {
                    if(b.ammo > 0 && !player1->hasmaxammo(b.ammotype-1+I_RAIL)) addmsg(N_REPAMMO, "rc", d);
                    d->lastrepammo = i;
                }
                return;
            }
        }
        d->lastrepammo = -1;
    }

    void rendertether(gameent *d)
    {
        int oldbase = d->lastbase;
        d->lastbase = -1;
        vec pos(d->o.x, d->o.y, d->o.z + (d->aboveeye - d->eyeheight)/2);
        string tmpteam;
        formatstring(tmpteam, "%d", d->team);
        if(d->state==CS_ALIVE)
        {
            loopv(bases)
            {
                baseinfo &b = bases[i];
                if(!b.valid() || !insidebase(b, d->feetpos()) || (strcmp(b.owner, tmpteam) && strcmp(b.enemy, tmpteam))) continue;
                if(d->lastbase < 0 && (lookupmaterial(d->feetpos())&MATF_CLIP) == MAT_GAMECLIP) break;


                vec basepos(b.o);
                basepos.add(vec(-10+rnd(20), -10+rnd(20), -10+rnd(20)));
                basepos.sub(pos);
                basepos.normalize().mul(1300.0f);

                if(randomevent(0.1f*nbfps)) particle_flying_flare(pos, basepos, 500, randomevent(2) ? PART_ZERO : PART_ONE, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.7f+(rnd(5)/10.f), 100);

                if(oldbase < 0)
                {
                    if(strcmp(b.owner, tmpteam) && b.owner[0]) playsound(S_TERMINAL_ALARM, &b.o, NULL, 0, 0, 128, -1, 256);
                    playsound(S_TERMINAL_ENTER, d==hudplayer() ? 0 : &pos, NULL, 0, 0, 128, -1, 256);
                    particle_splash(PART_ZERO, 12, 250, pos, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.8f, 200, 50);
                    particle_splash(PART_ONE, 12, 250, pos, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.8f, 200, 50);
                }
                d->lastbase = i;
            }
        }
        if(d->lastbase < 0 && oldbase >= 0)
        {
            playsound(S_TERMINAL_ENTER, d==hudplayer() ? 0 : &pos, NULL, 0, 0, 128, -1, 256);
            particle_splash(PART_ZERO, 12, 200, pos, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.8f, 200, 50);
            particle_splash(PART_ONE, 12, 200, pos, isteam(player1->team, d->team) ? 0xFFFF00 : 0xFF0000, 0.8f, 200, 50);
        }
    }

    void preload()
    {
        static const char * const basemodels[3] = { "base/neutral", "base/red", "base/yellow" };
        loopi(3) preloadmodel(basemodels[i]);
        loopi(6) preloadsound(S_TERMINAL+i);
    }

    int bliptimer = 0;

    void rendergame()
    {
        string tmpteam;
        formatstring(tmpteam, "%d", player1->team);

        if(capturetether && canaddparticles())
        {
            loopv(players)
            {
                gameent *d = players[i];
                if(d) rendertether(d);
            }
            rendertether(player1);
        }
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(!b.valid()) continue;
            const char *basename = b.owner[0] ? (strcmp(b.owner, tmpteam) ? "base/red" : "base/yellow") : "base/neutral";
            rendermodel(basename, ANIM_MAPMODEL|ANIM_LOOP, b.o, 0, 0, 0, MDL_CULL_VFC | MDL_CULL_OCCLUDED);
            //float fradius = 1.0f, fheight = 0.5f;
            //regular_particle_flame(PART_FLAME, vec(b.ammopos.x, b.ammopos.y, b.ammopos.z - 4.5f), fradius, fheight, b.owner[0] ? isteam(d->team, player1->team) ? 0x802020 : 0x2020FF) : 0x208020, 3, 2.0f);
            //regular_particle_flame(PART_SMOKE, vec(b.ammopos.x, b.ammopos.y, b.ammopos.z - 4.5f + 4.0f*min(fradius, fheight)), fradius, fheight, 0x303020, 1, 4.0f, 100.0f, 2000.0f, -20);
            //particle_fireball(b.ammopos, 4.8f, PART_EXPLOSION, 0, b.owner[0] ? (strcmp(b.owner, player1->team) ? 0x802020 : 0x2020FF) : 0x208020, 4.8f);

            const char *ammoname = entities::entmdlname(I_RAIL+b.ammotype);
            if(m_regencapture)
            {
                vec height(0, 0, 0);
                abovemodel(height, ammoname);
                vec ammopos(b.ammopos);
                ammopos.z -= height.z/2 + sinf(lastmillis/100.0f)/20;
                rendermodel(ammoname, ANIM_MAPMODEL|ANIM_LOOP, ammopos, lastmillis/10.0f, 0, 0, MDL_CULL_VFC | MDL_CULL_OCCLUDED);
            }

            int tcolor = 0x888888, mtype = -1, mcolor = 0xFFFFFF, mcolor2 = 0;
            if(b.owner[0])
            {
                string termnalally, termnalenemy;
                formatstring(termnalally, "%s", langage ? "Ellied" : "Alli�");
                formatstring(termnalenemy, "%s", langage ? "Enemy" : "Ennemi");
                bool isowner = !strcmp(b.owner, tmpteam);
                if(b.enemy[0]) { mtype = PART_METER_VS; mcolor = 0xFF0000; mcolor2 = 0xFFFF00; if(!isowner) swap(mcolor, mcolor2); }
                if(!b.name[0]) formatstring(b.info, "Terminal %d - %s", b.tag, !b.converted ? strcmp(b.owner, tmpteam) ? termnalenemy : termnalally : langage ? "Disputed !" : "Contest� !");
                else if(basenumbers) formatstring(b.info, "%s (%d) - %s", b.name, b.tag, !b.converted ? strcmp(b.owner, tmpteam) ? termnalenemy : termnalally : langage ? "Disputed !" : "Contest� !");
                else formatstring(b.info, "%s - %s", b.name, !b.converted ? strcmp(b.owner, tmpteam) ? termnalenemy : termnalally : langage ? "Disputed !" : "Contest� !");
                tcolor = isowner ? 0xFFFF00 : 0xFF0000;

                if(!strcmp(b.owner, tmpteam) && b.o.dist(camera1->o) > 128)
                {
                    if(b.converted)
                    {
                        b.bliptimer += curtime;
                        if(b.bliptimer>1000) b.bliptimer = 0;
                    }
                    else b.bliptimer = 0;

                    vec posA = b.o;
                    posA.add(vec(0, 0, 7));
                    vec posB = camera1->o;
                    vec posAtofrontofposB = (posA.add((posB.mul(vec(127, 127, 127))))).div(vec(128, 128, 128));
                    particle_splash(PART_BLIP, 1, 1, posAtofrontofposB, b.bliptimer < 500 ? 0xFFFF00 : 0xFF0000, 0.03f*(zoom ? (guns[player1->gunselect].maxzoomfov)/100.f : b.o.dist(camera1->o) > 228 ? 1.f : (b.o.dist(camera1->o)-128)/100.f), 1, 1, 0, false, b.o.dist(camera1->o)/170.f);
                }

            }
            else if(b.enemy[0])
            {
                if(!b.name[0]) formatstring(b.info, "Terminal %d - %s", b.tag, langage ? "Hack in progress..." : "Hack en cours... ");
                else if(basenumbers) formatstring(b.info, "%s (%d) - %s", b.name, b.tag,  langage ? "Hack in progress..." : "Hack en cours... ");
                else formatstring(b.info, "%s - %s", b.name,  langage ? "Hack in progress..." : "Hack en cours... ");
                if(strcmp(b.enemy, tmpteam)) { tcolor = 0xFF0000; mtype = PART_METER; mcolor = 0xFF0000; }
                else { tcolor = 0xFFFF00; mtype = PART_METER; mcolor = 0xFFFF00; }
            }
            else if(!b.name[0]) formatstring(b.info, "Terminal %d", b.tag);
            else if(basenumbers) formatstring(b.info, "%s (%d)", b.name, b.tag);
            else copystring(b.info, b.name);

            vec above(b.ammopos);
            above.z += AMMOHEIGHT;
            if(b.info[0]) particle_text(above, b.info, PART_TEXT, 1, tcolor, 3.0f);
            if(mtype>=0)
            {
                above.z += 3.5f;
                particle_meter(above, b.converted/float((b.owner[0] ? int(OCCUPYENEMYLIMIT) : int(OCCUPYNEUTRALLIMIT))), mtype, 1, 1, mcolor, mcolor2, 3.0f);
            }
        }
    }

    void drawblips(gameent *d, float blipsize, int fw, int fh, int type, bool skipenemy = false)
    {
        string tmpteam;
        formatstring(tmpteam, "%d", player1->team);

        float scale = calcradarscale();
        int blips = 0;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(!b.valid()) continue;
            if(skipenemy && b.enemy[0]) continue;
            switch(type)
            {
                case 1: if(!b.owner[0] || strcmp(b.owner, tmpteam)) continue; break;
                case 0: if(b.owner[0]) continue; break;
                case -1: if(!b.owner[0] || !strcmp(b.owner, tmpteam)) continue; break;
                case -2: if(!b.enemy[0] || !strcmp(b.enemy, tmpteam)) continue; break;
            }
            vec dir(d->o);
            dir.sub(b.o).div(scale);
            float dist = dir.magnitude2(), maxdist = 1 - 0.05f - blipsize;
            if(dist >= maxdist) dir.mul(maxdist/dist);
            dir.rotate_around_z(-camera1->yaw*RAD);
            if(basenumbers)
            {
                static string blip;
                formatstring(blip, "%d", b.tag);
                int tw, th;
                text_bounds(blip, tw, th);
                draw_text(blip, int(0.5f*(dir.x*fw/blipsize - tw)), int(0.5f*(dir.y*fh/blipsize - th)));
            }
            else
            {
                if(!blips) { gle::defvertex(2); gle::deftexcoord0(); gle::begin(GL_QUADS); }
                float x = 0.5f*(dir.x*fw/blipsize - fw), y = 0.5f*(dir.y*fh/blipsize - fh);
                gle::attribf(x,    y);    gle::attribf(0, 0);
                gle::attribf(x+fw, y);    gle::attribf(1, 0);
                gle::attribf(x+fw, y+fh); gle::attribf(1, 1);
                gle::attribf(x,    y+fh); gle::attribf(0, 1);
            }
            blips++;
        }
        if(blips && !basenumbers) gle::end();
    }

    int respawnwait(gameent *d)
    {
        return max(0, RESPAWNSECS-(lastmillis-d->lastpain)/1000);
    }

    int clipconsole(int w, int h)
    {
        return (h*(1 + 1 + 10))/(4*10);
    }

    void drawhud(gameent *d, int w, int h)
    {
        pushhudscale(h/1800.0f);
        pushhudscale(2);
        pophudmatrix();
        resethudshader();

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        int s = 1800/4, x = 1800*w/h - s - s/10, y = s/10;

        gle::colorf(1, 1, 1, minimapalpha);
        if(minimapalpha >= 1) glDisable(GL_BLEND);
        bindminimap();
        drawminimap(d, x, y, s);
        if(minimapalpha >= 1) glEnable(GL_BLEND);
        gle::colorf(1, 1, 1);
        float margin = 0.04f, roffset = s*margin, rsize = s + 2*roffset;
        setradartex();
        drawradar(x - roffset, y - roffset, rsize);
        settexture("media/interface/hud/boussole.png", 3);
        pushhudmatrix();
        hudmatrix.translate(x - roffset + 0.5f*rsize, y - roffset + 0.5f*rsize, 0);
        hudmatrix.rotate_around_z((camera1->yaw + 180)*-RAD);
        flushhudmatrix();
        drawradar(-0.5f*rsize, -0.5f*rsize, rsize);
        pophudmatrix();
        drawplayerblip(d, x, y, s, 1.5f);
        bool showenemies = lastmillis%1000 >= 500;
        int fw = 1, fh = 1;
        if(basenumbers)
        {
            pushfont();
            setfont("digit_blue");
            text_bounds(" ", fw, fh);
        }
        else settexture("media/interface/hud/blip_blue.png", 3);
        float blipsize = basenumbers ? 0.1f : 0.05f;
        pushhudmatrix();
        hudmatrix.translate(x + 0.5f*s, y + 0.5f*s, 0);
        hudmatrix.scale((s*blipsize)/fw, (s*blipsize)/fh, 1.0f);
        flushhudmatrix();
        drawblips(d, blipsize, fw, fh, 1, showenemies);
        if(basenumbers) setfont("digit_grey");
        else settexture("media/interface/hud/blip_grey.png", 3);
        drawblips(d, blipsize, fw, fh, 0, showenemies);
        if(basenumbers) setfont("digit_red");
        else settexture("media/interface/hud/blip_red.png", 3);
        drawblips(d, blipsize, fw, fh, -1, showenemies);
        if(showenemies) drawblips(d, blipsize, fw, fh, -2);
        pophudmatrix();
        if(basenumbers) popfont();
        drawteammates(d, x, y, s);
    }

    void setup()
    {
        resetbases();
        loopv(entities::ents)
        {
            extentity *e = entities::ents[i];
            if(e->type!=BASE) continue;
            baseinfo &b = bases.add();
            b.o = e->o;
            b.ammopos = b.o;
            abovemodel(b.ammopos, "base/neutral");
            b.ammopos.z += AMMOHEIGHT-2;
            b.ammotype = e->attr1;
            defformatstring(alias, langage ? "base_en_%d" : "base_fr_%d", e->attr2);
            const char *name = getalias(alias);
            copystring(b.name, name);
            b.tag = e->attr2>0 ? e->attr2 : bases.length();
        }
    }

    void senditems(packetbuf &p)
    {
        putint(p, N_BASES);
        putint(p, bases.length());
        loopv(bases)
        {
            baseinfo &b = bases[i];
            putint(p, b.ammotype);
            putint(p, int(b.o.x*DMF));
            putint(p, int(b.o.y*DMF));
            putint(p, int(b.o.z*DMF));
        }
    }

    void updatebase(int i, const char *owner, const char *enemy, int converted, int ammo)
    {
        if(!bases.inrange(i)) return;
        baseinfo &b = bases[i];
        string tmpteam;
        formatstring(tmpteam, "%d", player1->team);
        if(owner[0])
        {
            if(strcmp(b.owner, owner))
            {
                int iowner = atoi(owner);
                if(!b.name[0]) conoutf(CON_GAMEINFO, langage ? "%s team hacked \"\fe%d\f7\" terminal." : "L'�quipe %s a hack� le terminal \"\fe%d\f7\".", teamcolor(iowner), b.tag);
                else conoutf(CON_GAMEINFO, langage ? "%s team hacked \"\fe%s\f7\" terminal." : "L'�quipe %s a hack� le terminal \"\fe%s\f7\".", teamcolor(iowner), b.name);
                playsound(!strcmp(owner, tmpteam) ? S_TERMINAL_HACKED : S_TERMINAL_HACKED_E, &b.o, NULL, 0, 0, 200, -1, 2000);
            }
        }
        else if(b.owner[0])
        {
            int ibowner = atoi(b.owner);
            if(!b.name[0]) conoutf(CON_GAMEINFO, langage ? "%s team lost the \"\fe%d\f7\" terminal." : "L'�quipe %s a perdu le terminal \"\fe%d\f7\".", teamcolor(ibowner), b.tag);
            else conoutf(CON_GAMEINFO, langage ? "%s team lost the \"\fe%s\f7\" terminal." : "L'�quipe %s a perdu le terminal \"\fe%s\f7\".", teamcolor(ibowner), b.name);
            playsound(!strcmp(owner, tmpteam) ? S_TERMINAL_LOST : S_TERMINAL_LOST_E, &b.o, NULL, 0, 0, 200, -1, 2000);
        }
        if(strcmp(b.owner, owner))
        {
            particle_splash(PART_ZERO, 25, 500, b.ammopos, owner[0] ? (!strcmp(owner, tmpteam) ? 0xFFFF00 : 0xFF0000) : 0x777777, 1.f, 400, -50);
            particle_splash(PART_ONE, 25, 500, b.ammopos, owner[0] ? (!strcmp(owner, tmpteam) ? 0xFFFF00 : 0xFF0000) : 0x777777, 1.f, 400, -50);
        }

        copystring(b.owner, owner);
        copystring(b.enemy, enemy);
        b.converted = converted;
        if(ammo>b.ammo) playsound(S_ITEMSPAWN, &b.o);
        b.ammo = ammo;
    }

    void setscore(int base, const char *team, int total)
    {
        findscore(team).total = total;
        int iteam = atoi(team);
        if(total>=10000) conoutf(CON_GAMEINFO, langage ? "%s team hacked all terminals!" : "L'�quipe %s a hack� tous les terminaux !", teamcolor(iteam));
        else if(bases.inrange(base))
        {
            baseinfo &b = bases[base];
            if(!strcmp(b.owner, team))
            {
                defformatstring(msg, "%d", total);
                vec above(b.ammopos);
                above.z += AMMOHEIGHT+1.0f;
                particle_textcopy(above, msg, PART_TEXT, 1500, isteam(iteam, player1->team) ? 0xFFFF00 : 0xFF0000, 5.0f, -5);
            }
        }
    }

    // prefer spawning near friendly base
    float ratespawn(gameent *d, const extentity &e)
    {
        string tmpteam;
        formatstring(tmpteam, "%d", d->team);

        float minbasedist = 1e16f;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(!b.owner[0] || strcmp(b.owner, tmpteam)) continue;
            minbasedist = min(minbasedist, e.o.dist(b.o));
        }
        return minbasedist < 1e16f ? proximityscore(minbasedist, 128.0f, 512.0f) : 1.0f;
    }

	bool aicheck(gameent *d, ai::aistate &b)
	{
		return false;
	}

	void aifind(gameent *d, ai::aistate &b, vector<ai::interest> &interests)
	{
		vec pos = d->feetpos();
		loopvj(bases)
		{
			baseinfo &f = bases[j];
            if(!f.valid()) continue;
			static vector<int> targets; // build a list of others who are interested in this
			targets.setsize(0);
			ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, j, true);
			gameent *e = NULL;
			int regen = !m_regencapture || d->health >= 750 || d->armour >= 1000+d->skill*10 ? 0 : 1;
			if(m_regencapture)
			{
			    if(d->armour < 1000+d->skill*10) regen = true;
				int gun = f.ammotype-1+I_RAIL;
				if(f.ammo > 0 && !d->hasmaxammo(gun))
					regen = gun != d->ai->weappref ? 2 : 4;
			}
			loopi(numdynents()) if((e = (gameent *)iterdynents(i)) && !e->ai && e->state == CS_ALIVE && isteam(d->team, e->team))
			{ // try to guess what non ai are doing
				vec ep = e->feetpos();
				if(targets.find(e->clientnum) < 0 && ep.squaredist(f.o) <= (CAPTURERADIUS*CAPTURERADIUS))
					targets.add(e->clientnum);
			}
            string tmpteam;
            formatstring(tmpteam, "%d", d->team);
			if((regen && f.owner[0] && !strcmp(f.owner, tmpteam)) || (targets.empty() && (!f.owner[0] || strcmp(f.owner, tmpteam) || f.enemy[0])))
			{
				ai::interest &n = interests.add();
				n.state = ai::AI_S_DEFEND;
				n.node = ai::closestwaypoint(f.o, ai::SIGHTMIN, false);
				n.target = j;
				n.targtype = ai::AI_T_AFFINITY;
				n.score = pos.squaredist(f.o)/(regen ? float(100*regen) : 1.f);
			}
		}
	}

	bool aidefend(gameent *d, ai::aistate &b)
	{
        if(!bases.inrange(b.target)) return false;
        baseinfo &f = bases[b.target];
        if(!f.valid()) return false;
		bool regen = !m_regencapture || d->health >= 750 || d->armour >= 1000+d->skill*10 ? false : true;
		if(!regen && m_regencapture)
		{
		    if(d->armour < 1000+d->skill*10) regen = true;
			int gun = f.ammotype-1+I_RAIL;
			if(f.ammo > 0 && !d->hasmaxammo(gun))
				regen = true;
		}
		int walk = 0;
		string tmpteam;
		formatstring(tmpteam, "%d", d->team);
		if(!regen && !f.enemy[0] && f.owner[0] && !strcmp(f.owner, tmpteam))
		{
			static vector<int> targets; // build a list of others who are interested in this
			targets.setsize(0);
			ai::checkothers(targets, d, ai::AI_S_DEFEND, ai::AI_T_AFFINITY, b.target, true);
			gameent *e = NULL;
			loopi(numdynents()) if((e = (gameent *)iterdynents(i)) && !e->ai && e->state == CS_ALIVE && isteam(d->team, e->team))
			{ // try to guess what non ai are doing
				vec ep = e->feetpos();
				if(targets.find(e->clientnum) < 0 && (ep.squaredist(f.o) <= (CAPTURERADIUS*CAPTURERADIUS*4)))
					targets.add(e->clientnum);
			}
			if(!targets.empty())
			{
				if(lastmillis-b.millis >= (201-d->skill)*33)
				{
					d->ai->trywipe = true; // re-evaluate so as not to herd
					return true;
				}
				else walk = 2;
			}
			else walk = 1;
			b.millis = lastmillis;
		}
		return ai::defend(d, b, f.o, float(CAPTURERADIUS), float(CAPTURERADIUS*(2+(walk*2))), walk); // less wander than ctf
	}

	bool aipursue(gameent *d, ai::aistate &b)
	{
		b.type = ai::AI_S_DEFEND;
		return aidefend(d, b);
	}
};

extern captureclientmode capturemode;
ICOMMAND(repammo, "", (), capturemode.replenishammo());
ICOMMAND(insidebases, "", (),
{
    vector<char> buf;
    if(m_capture && player1->state == CS_ALIVE) loopv(capturemode.bases)
    {
        captureclientmode::baseinfo &b = capturemode.bases[i];
        if(b.valid() && capturemode.insidebase(b, player1->feetpos()))
        {
            if(buf.length()) buf.add(' ');
            defformatstring(basenum, "%d", b.tag);
            buf.put(basenum, strlen(basenum));
        }
    }
    buf.add('\0');
    result(buf.getbuf());
});

#else
    bool notgotbases;

    captureservmode() : captures(0), notgotbases(false) {}

    void reset(bool empty)
    {
        resetbases();
        notgotbases = !empty;
    }

    void cleanup()
    {
        reset(false);
    }

    void setup()
    {
        reset(false);
        if(notgotitems || ments.empty()) return;
        loopv(ments)
        {
            entity &e = ments[i];
            if(e.type != BASE) continue;
            int ammotype = e.attr1;
            addbase(ammotype, e.o);
        }
        notgotbases = false;
        sendbases();
        loopv(clients) if(clients[i]->state.state==CS_ALIVE) entergame(clients[i]);
    }

    void newmap()
    {
        reset(true);
    }

    void stealbase(int n, int team)
    {
        baseinfo &b = bases[n];
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(ci->state.state==CS_ALIVE && ci->team>0 && ci->team == team && insidebase(b, ci->state.o))
                b.enter(ci->team);
        }
        sendbaseinfo(n);
    }

    void replenishammo(clientinfo *ci)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE) return;
        string tmpteam;
        formatstring(tmpteam, "%d", ci->team);
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(b.valid() && insidebase(b, ci->state.o) && !ci->state.hasmaxammo(b.ammotype-1+I_RAIL) && b.takeammo(tmpteam))
            {
                sendbaseinfo(i);
                sendf(-1, 1, "riii", N_REPAMMO, ci->clientnum, b.ammotype);
                ci->state.addammo(b.ammotype);
                break;
            }
        }
    }

    void movebases(int team, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip)
    {
        if(gamemillis>=gamelimit) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(!b.valid()) continue;
            bool leave = !oldclip && insidebase(b, oldpos),
                 enter = !newclip && insidebase(b, newpos);
            if(leave && !enter && b.leave(team)) sendbaseinfo(i);
            else if(enter && !leave && b.enter(team)) sendbaseinfo(i);
            else if(leave && enter && b.steal(team)) stealbase(i, team);
        }
    }

    void leavebases(int team, const vec &o)
    {
        movebases(team, o, false, vec(-1e10f, -1e10f, -1e10f), true);
    }

    void enterbases(int team, const vec &o)
    {
        movebases(team, vec(-1e10f, -1e10f, -1e10f), true, o, false);
    }

    void addscore(int base, const char *team, int n)
    {
        if(!n) return;
        score &cs = findscore(team);
        cs.total += n;
        sendf(-1, 1, "riisi", N_BASESCORE, base, team, cs.total);
    }

    void regenowners(baseinfo &b, int ticks)
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            string tmpteam;
            formatstring(tmpteam, "%d", ci->team);

            if(ci->state.state==CS_ALIVE && ci->team>0 && !strcmp(tmpteam, b.owner) && insidebase(b, ci->state.o))
            {
                bool notify = false;
                if(ci->state.health < ci->state.maxhealth)
                {
                    ci->state.health = min(ci->state.health + ticks*REGENHEALTH, ci->state.maxhealth);
                    notify = true;
                }

                if(ci->state.mana < 150)
                {
                    ci->state.mana = min(ci->state.mana + ticks*REGENHEALTH/10, 150);
                    notify = true;
                }

                if(ci->state.armour==0) ci->state.armourtype = A_BLUE;

                switch(ci->state.armourtype)
                {
                    case A_BLUE:
                    {
                        if(ci->state.armour<750) ci->state.armour = min(ci->state.armour + ticks*REGENARMOUR, 750);
                        else ci->state.armourtype = A_GREEN;
                        notify = true;
                        break;
                    }
                    case A_GREEN:
                    {
                        if(ci->state.armour<1250) ci->state.armour = min(ci->state.armour + ticks*REGENARMOUR, 1250);
                        else ci->state.armourtype = A_YELLOW;
                        notify = true;
                        break;
                    }
                    case A_YELLOW:
                    {
                        if(ci->state.armour<2000) ci->state.armour = min(ci->state.armour + ticks*REGENARMOUR, 2000);
                        else ci->state.armourtype = A_ASSIST;
                        notify = true;
                        break;
                    }
                    case A_ASSIST:
                    {
                        ci->state.armour = min(ci->state.armour + ticks*REGENARMOUR, 3000);
                        ci->state.ammo[GUN_ASSISTXPL] = 1;
                        notify = true;
                        break;
                    }
                }

                if(b.valid())
                {
                    int ammotype = b.ammotype-1+I_RAIL;
                    if(!ci->state.hasmaxammo(ammotype))
                    {
                        ci->state.addammo(b.ammotype, ticks*REGENAMMO, 100);
                        notify = true;
                    }
                }
                if(notify)
                    sendf(-1, 1, "ri7", N_BASEREGEN, ci->clientnum, ci->state.health, ci->state.armour, ci->state.mana, b.ammotype, b.valid() ? ci->state.ammo[b.ammotype] : 0);
            }
        }
    }

    void update()
    {
        if(gamemillis>=gamelimit) return;
        endcheck();
        int t = gamemillis/1000 - (gamemillis-curtime)/1000;
        if(t<1) return;
        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(!b.valid()) continue;
            if(b.enemy[0])
            {
                if(!b.owners || !b.enemies) b.occupy(b.enemy, OCCUPYBONUS*(b.enemies ? 1 : -1) + OCCUPYPOINTS*(b.enemies ? b.enemies : -(1+b.owners))*t);
                sendbaseinfo(i);
            }
            else if(b.owner[0])
            {
                b.capturetime += t;

                int score = b.capturetime/SCORESECS - (b.capturetime-t)/SCORESECS;
                if(score) addscore(i, b.owner, score);

                if(m_regencapture)
                {
                    int regen = b.capturetime/REGENSECS - (b.capturetime-t)/REGENSECS;
                    if(regen) regenowners(b, regen);
                }
            }
        }
    }

    void sendbaseinfo(int i)
    {
        baseinfo &b = bases[i];
        sendf(-1, 1, "riissii", N_BASEINFO, i, b.owner, b.enemy, b.enemy[0] ? b.converted : 0, b.owner[0] ? b.ammo : 0);
    }

    void sendbases()
    {
        packetbuf p(MAXTRANS, ENET_PACKET_FLAG_RELIABLE);
        initclient(NULL, p, false);
        sendpacket(-1, 1, p.finalize());
    }

    void initclient(clientinfo *ci, packetbuf &p, bool connecting)
    {
        if(connecting)
        {
            loopv(scores)
            {
                score &cs = scores[i];
                putint(p, N_BASESCORE);
                putint(p, -1);
                sendstring(cs.team, p);
                putint(p, cs.total);
            }
        }
        putint(p, N_BASES);
        putint(p, bases.length());
        loopv(bases)
        {
            baseinfo &b = bases[i];
            putint(p, b.ammotype);
            sendstring(b.owner, p);
            sendstring(b.enemy, p);
            putint(p, b.converted);
            putint(p, b.ammo);
        }
    }

    void endcheck()
    {
        const char *lastteam = NULL;

        loopv(bases)
        {
            baseinfo &b = bases[i];
            if(!b.valid()) continue;
            if(b.owner[0])
            {
                if(!lastteam) lastteam = b.owner;
                else if(strcmp(lastteam, b.owner))
                {
                    lastteam = NULL;
                    break;
                }
            }
            else
            {
                lastteam = NULL;
                break;
            }
        }

        if(!lastteam) return;
        findscore(lastteam).total = 10000;
        sendf(-1, 1, "riisi", N_BASESCORE, -1, lastteam, 10000);
        startintermission();
    }

    void entergame(clientinfo *ci)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || ci->gameclip) return;
        enterbases(ci->team, ci->state.o);
    }

    void spawned(clientinfo *ci)
    {
        if(notgotbases || ci->gameclip) return;
        enterbases(ci->team, ci->state.o);
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        if(notgotbases || ci->state.state!=CS_ALIVE || ci->gameclip) return;
        leavebases(ci->team, ci->state.o);
    }

    void died(clientinfo *ci, clientinfo *actor)
    {
        if(notgotbases || ci->gameclip) return;
        leavebases(ci->team, ci->state.o);
    }

    bool canspawn(clientinfo *ci, bool connecting)
    {
        return m_regencapture || connecting || !ci->state.lastdeath || gamemillis+curtime-ci->state.lastdeath >= RESPAWNSECS*1000;
    }

    void moved(clientinfo *ci, const vec &oldpos, bool oldclip, const vec &newpos, bool newclip)
    {
        if(notgotbases) return;
        movebases(ci->team, oldpos, oldclip, newpos, newclip);
    }

    void changeteam(clientinfo *ci, int oldteam, int newteam)
    {
        if(notgotbases || ci->gameclip) return;
        leavebases(oldteam, ci->state.o);
        enterbases(newteam, ci->state.o);
    }

    void parsebases(ucharbuf &p, bool commit)
    {
        int numbases = getint(p);
        loopi(numbases)
        {
            int ammotype = getint(p);
            vec o;
            loopk(3) o[k] = max(getint(p)/DMF, 0.0f);
            if(p.overread()) break;
            if(commit && notgotbases) addbase(ammotype>=GUN_RAIL && ammotype<=GUN_GLOCK ? ammotype : min(ammotype, 0), o);
        }
        if(commit && notgotbases)
        {
            notgotbases = false;
            sendbases();
            loopv(clients) if(clients[i]->state.state==CS_ALIVE) entergame(clients[i]);
        }
    }

    bool extinfoteam(const char *team, ucharbuf &p)
    {
        int numbases = 0;
        loopvj(bases) if(!strcmp(bases[j].owner, team)) numbases++;
        putint(p, numbases);
        loopvj(bases) if(!strcmp(bases[j].owner, team)) putint(p, j);
        return true;
    }
};

#endif

#elif SERVMODE

case N_BASES:
    if(smode==&capturemode) capturemode.parsebases(p, (ci->state.state!=CS_SPECTATOR || ci->privilege || ci->local) && !strcmp(ci->clientmap, smapname));
    break;

case N_REPAMMO:
    if((ci->state.state!=CS_SPECTATOR || ci->local || ci->privilege) && cq && smode==&capturemode) capturemode.replenishammo(cq);
    break;

#else

case N_BASEINFO:
{
    int base = getint(p);
    string owner, enemy;
    getstring(text, p);
    copystring(owner, text);
    getstring(text, p);
    copystring(enemy, text);
    int converted = getint(p), ammo = getint(p);
    if(m_capture) capturemode.updatebase(base, owner, enemy, converted, ammo);
    break;
}

case N_BASEREGEN:
{
    int rcn = getint(p), health = getint(p), armour = getint(p), mana = getint(p), ammotype = getint(p), ammo = getint(p);
    gameent *regen = rcn==player1->clientnum ? player1 : getclient(rcn);
    if(regen && m_capture)
    {
        regen->health = health;
        if(regen->armour==0) {regen->armourtype=A_BLUE; playsound(S_ITEMBBOIS, regen==hudplayer() ? NULL : &regen->o, 0, 0, 0, 150, -1, 300);}
        switch(regen->armourtype)
        {
            case A_BLUE: if(regen->armour>=750) {regen->armourtype=A_GREEN; playsound(S_ITEMBFER, regen==hudplayer() ? NULL : &regen->o, 0, 0, 0, 150, -1, 300);} break;
            case A_GREEN: if(regen->armour>=1250) {regen->armourtype=A_YELLOW; playsound(S_ITEMBOR, regen==hudplayer() ? NULL : &regen->o, 0, 0, 0, 150, -1, 300);} break;
            case A_YELLOW: if(regen->armour>=2000) {regen->armourtype=A_ASSIST; playsound(S_ITEMARMOUR, regen==hudplayer() ? NULL : &regen->o, 0, 0, 0, 150, -1, 300);}
            case A_ASSIST: regen->ammo[GUN_ASSISTXPL] = 1;
        }
        regen->armour = armour;
        regen->mana = mana;
        if(ammotype>=GUN_RAIL && ammotype<=GUN_GLOCK) regen->ammo[ammotype] = ammo;
    }
    break;
}

case N_BASES:
{
    int numbases = getint(p);
    loopi(numbases)
    {
        int ammotype = getint(p);
        string owner, enemy;
        getstring(text, p);
        copystring(owner, text);
        getstring(text, p);
        copystring(enemy, text);
        int converted = getint(p), ammo = getint(p);
        capturemode.initbase(i, ammotype, owner, enemy, converted, ammo);
    }
    break;
}

case N_BASESCORE:
{
    int base = getint(p);
    getstring(text, p);
    int total = getint(p);
    if(m_capture) capturemode.setscore(base, text, total);
    break;
}

case N_REPAMMO:
{
    int rcn = getint(p), ammotype = getint(p);
    gameent *r = rcn==player1->clientnum ? player1 : getclient(rcn);
    if(r && m_capture) capturemode.receiveammo(r, ammotype);
    break;
}

#endif