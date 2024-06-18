
/******************************************************************************
 * GuiConfig struct contains all information about the GUI aspect of the 
 * simulation --that is the clickable buttons and the box surrounding them
******************************************************************************/
#ifndef GUICONFIG_H
#define GUICONFIG_H

#include "raylib.h"

// Contains all gui-level data
typedef struct GuiConfig
{
    Rectangle gui_background;
    Rectangle cursor_btn; // cursor is selected
    Rectangle paint_btn;  // paintbrush is selected
    Rectangle textbox;    // textbox is selected
    Rectangle efunc_btn;  // efunc_btn is pressed. Unpressed on mouse release

    int slot_capacity;
    float gui_offset;
    float button_offset;
    float gui_height;

    unsigned char selected_cursor;
    unsigned char selected_paint;
    unsigned char selected_text;
    unsigned char selected_evalue;
    unsigned char on_text;

    Texture2D cursor_btn_texture;
    Texture2D paint_btn_texture;

} GuiConfig;

// Runs once during initialization
GuiConfig *init_guiconfig();

#endif
