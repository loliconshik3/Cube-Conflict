#ifndef TEXTURES_H
#define TEXTURES_H

#include <SDL.h>
#include <string>

enum { TEX_BACKGROUND = 0, TEX_LOGO, TEX_FRENCH, TEX_ENGLISH, TEX_RUSSIAN, TEX_SPANISH, TEX_AUDIOON, TEX_AUDIOOFF, TEX_SERVER, TEX_ABOUT, TEX_MINIMIZE, TEX_REDCROSS, NUMTEXTURES };

struct TextureConfig
{
    SDL_Texture* texture;
    std::string path;
};

extern TextureConfig textures[NUMTEXTURES];

namespace texture
{
    extern bool isValid(int id);    // checks if the texture is valid or not
    extern void init();             // load all textures
}

#endif