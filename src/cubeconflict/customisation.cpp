#include <climits>
#include <cstdlib>
#include "cubedef.h"

using namespace std;

const int itemprice(int itemtype, int itemID) // R�cup�re le prix d'un objet
{
    int price = 0;
    switch(itemtype)
    {
        case 0: price = customssmileys[itemID].smileyprice; break;
        case 1: price = customscapes[itemID].capeprice; break;
        case 2: price = customstombes[itemID].tombeprice; break;
        case 3: price = customsdance[itemID].danceprice; break;
    }
    return price;
}
ICOMMAND(getitemprice, "ii", (int *itemtype, int *itemID), intret(itemprice(*itemtype, *itemID)));

const int hasitem() // R�cup�re si un objet est poss�d� ou non
{
    int itemstat = 0;
    switch(UI_custtab)
    {
        case 0: itemstat = stat[SMI_HAP+UI_smiley]; break;
        case 1: itemstat = stat[CAPE_CUBE+UI_cape]; break;
        case 2: itemstat = stat[TOM_MERDE+UI_tombe]; break;
        case 3: itemstat = stat[VOI_CORTEX+UI_voix]; break;
    }
    return itemstat;
}
ICOMMAND(hasitem, "", (), intret(hasitem()));

void buyitem(int itemtype) //Ach�te un objet
{
    switch(itemtype)
    {
        case 0:
            if(customssmileys[UI_smiley].smileyprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3Ce smiley est trop cher pour toi !"); playsound(S_ERROR); return;}
            else if(stat[SMI_HAP+UI_smiley]>0) {conoutf(CON_GAMEINFO, "\f3Vous poss�dez d�j� ce smiley !"); playsound(S_ERROR); return;}
            else
            {
                conoutf(CON_GAMEINFO, "\f9Smiley \"%s\" achet� !", customssmileys[UI_smiley].smileyname);
                stat[STAT_CC] = stat[STAT_CC]-customssmileys[UI_smiley].smileyprice;
                stat[SMI_HAP+UI_smiley] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        case 1:
            if(customscapes[UI_cape].capeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3Cette cape est trop ch�re pour toi !"); playsound(S_ERROR); return;}
            else if(stat[CAPE_CUBE+UI_cape]>0) {conoutf(CON_GAMEINFO, "\f3Vous poss�dez d�j� cette cape !"); playsound(S_ERROR); return;}
            else
            {
                conoutf(CON_GAMEINFO, "\f9Cape \"%s\" achet�e !", customscapes[UI_cape].capename);
                stat[STAT_CC] = stat[STAT_CC]-customscapes[UI_cape].capeprice;
                stat[CAPE_CUBE+UI_cape] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        case 2:
            if(customstombes[UI_tombe].tombeprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3Cette tombe est trop ch�re pour toi !"); playsound(S_ERROR); return;}
            else if(stat[TOM_MERDE+UI_tombe]>0) {conoutf(CON_GAMEINFO, "\f3Vous poss�dez d�j� cette tombe !"); playsound(S_ERROR); return;}
            else
            {
                conoutf(CON_GAMEINFO, "\f9Tombe \"%s\" achet�e !", customstombes[UI_tombe].tombename);
                stat[STAT_CC] = stat[STAT_CC]-customstombes[UI_tombe].tombeprice;
                stat[TOM_MERDE+UI_tombe] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
        case 3:
            if(customsdance[UI_voix].danceprice > stat[STAT_CC]) {conoutf(CON_GAMEINFO, "\f3Cette voix est trop ch�re pour toi !"); playsound(S_ERROR); return;}
            else if(stat[VOI_CORTEX+UI_voix]>0) {conoutf(CON_GAMEINFO, "\f3Vous poss�dez d�j� cette voix !"); playsound(S_ERROR); return;}
            else
            {
                conoutf(CON_GAMEINFO, "\f9Voix \"%s\" achet�e !", customsdance[UI_voix].dancename);
                stat[STAT_CC] = stat[STAT_CC]-customsdance[UI_voix].danceprice;
                stat[VOI_CORTEX+UI_voix] = rnd(99)+1;
                playsound(S_CAISSEENREGISTREUSE);
                return;
            }
    }
}
ICOMMAND(buyitem, "i", (int *itemtype), buyitem(*itemtype));
