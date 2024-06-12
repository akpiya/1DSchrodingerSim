#include <stdlib.h>

#include "guiconfig.h"
GuiConfig *init_guiconfig()
{
    GuiConfig *config = malloc(sizeof(GuiConfig));

    config->slot_capacity = 6;
    config->button_offset = 7.0;
    config->gui_offset = 40.0;
    config->gui_height = 50.0;
    
    float box_width = (
            GetScreenWidth()
            - 2*config->gui_offset
            - (config->slot_capacity+1)*config->button_offset
        ) / ((float) config->slot_capacity);

    config->selected_cursor = 1;
    config->selected_paint = 0;
    config->selected_evalue = 0;
    config->selected_text = 0;

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

    config->textbox = (Rectangle) {
        config->gui_offset + 4*config->button_offset + 3*box_width,
        config->gui_offset/2 + config->button_offset,
        2*box_width + config->button_offset,
        config->gui_height - 2*config->button_offset
    };

    config->efunc_btn = (Rectangle) {
        config->gui_offset + 6*config->button_offset + 5*box_width,
        config->gui_offset/2 + config->button_offset,
        box_width,
        config->gui_height - 2*config->button_offset
    };

    return config; 
}


