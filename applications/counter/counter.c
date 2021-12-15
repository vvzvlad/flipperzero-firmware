#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>


int32_t counter_app(void* p) {
    ViewPort* view_port = view_port_alloc();
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    delay(2000);

    gui_remove_view_port(gui, view_port);
    furi_record_close("gui");
    view_port_free(view_port);
    return 0;
}
