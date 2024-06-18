#include <stdlib.h>

#include "guiconfig.h"

GuiConfig *init_guiconfig()
{
    GuiConfig *config = malloc(sizeof(GuiConfig));

    config->slot_capacity = 6;
    config->button_offset = 7.0;
    config->gui_offset = 40.0;
    config->gui_height = 50.0;
    
    // width of each button field
    float box_width = (
            GetScreenWidth()
            - 2*config->gui_offset
            - (config->slot_capacity+1)*config->button_offset
        ) / ((float) config->slot_capacity);


    config->selected_cursor = 1;
    config->selected_paint = 0;
    config->selected_evalue = 0;
    config->selected_left = 0;
    config->selected_right = 0;

    // Rectangle structs contain data about the location and size of each element
    config->gui_background = (Rectangle) {
        config->gui_offset,
        config->gui_offset/2.0,
        GetScreenWidth() - 2*config->gui_offset,
        config->gui_height
    };

    config->cursor_btn = (Rectangle) {
        config->gui_offset + config->button_offset, 
        config->gui_offset / 2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    config->paint_btn = (Rectangle) {
        config->gui_offset + 2*config->button_offset + box_width,
        config->gui_offset/2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    config->left_btn = (Rectangle) {
        config->gui_offset + 3*config->button_offset + 2*box_width,
        config->gui_offset/2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    config->num_btn = (Rectangle) {
        config->gui_offset + 4*config->button_offset + 3*box_width,
        config->gui_offset/2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    config->right_btn = (Rectangle) {
        config->gui_offset + 5*config->button_offset + 4*box_width,
        config->gui_offset/2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    config->efunc_btn = (Rectangle) {
        config->gui_offset + 6*config->button_offset + 5*box_width,
        config->gui_offset/2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };
    
    // Loading textures that are overlayed on top of the rectangles.
    config->cursor_btn_texture = LoadTextureFromImage(LoadImage("assets/cursor.png"));
    config->paint_btn_texture = LoadTextureFromImage(LoadImage("assets/paint.png"));
    config->left_btn_texture = LoadTextureFromImage(LoadImage("assets/left.png"));
    config->right_btn_texture = LoadTextureFromImage(LoadImage("assets/right.png"));

    config->cursor_btn_texture.height = config->cursor_btn.height;
    config->cursor_btn_texture.width = config->cursor_btn.width-10*config->button_offset; 

    config->paint_btn_texture.height = config->paint_btn.height;
    config->paint_btn_texture.width = config->paint_btn.width - 10*config->button_offset;

    config->left_btn_texture.height = config->left_btn.height;
    config->left_btn_texture.width = config->left_btn.width - 10*config->button_offset;

    config->right_btn_texture.height = config->right_btn.height;
    config->right_btn_texture.width = config->left_btn.width - 10*config->button_offset;

    return config; 
}