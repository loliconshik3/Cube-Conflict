#ifndef BUTTONS_H
#define BUTTONS_H

#include <string>
#include <functional>
#include <SDL_image.h>

namespace buttons
{
    struct Button
    {
        int x, y; // Position on the window
        int width, height; // Size
        int color, hoverColor; // hexadecimal RGBA color
        std::string text; // Button text
        int onTex, offTex; // textures based on on/off states
        std::function<void()> action; // Action to perform on click

        // Constructor
        Button(SDL_Renderer* renderer, int x, int y, int width, int height, int color, int hoverColor, const std::string& text, int onTex, int offTex, std::function<void()> action)
                : x(x), y(y), width(width), height(height), color(color), hoverColor(hoverColor), text(text), onTex(onTex), offTex(offTex), action(std::move(action)) {
        }

        bool hovered = false; // hovered state
        bool isOn = true;

        bool isHovered(int mouseX, int mouseY) const { return mouseX > x && mouseX < x + width && mouseY > y && mouseY < y + height; } // Check if the mouse is over the button

        void updateHoverState(int mouseX, int mouseY) { hovered = isHovered(mouseX, mouseY); }

        void click() const { action(); } // Execute the button's action
    };

    extern void render(SDL_Renderer* renderer);                     // render the buttons
    extern void update(SDL_Event &e, int mouseX, int mouseY);       // update the events related to buttons
    extern void init();                                             // create the buttons
    extern void destroy();                                          // destroy the buttons
}

#endif
