#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>

typedef struct {
    Gui* gui;
    ViewPort* view_port;
} CounterApp;

void counter_draw_callback(Canvas* canvas, void* ctx) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Counter application");
}

void counter_app_free(CounterApp* app) {
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    furi_record_close("gui");
    free(app);
}

CounterApp* counter_app_alloc() {
    CounterApp* app = furi_alloc(sizeof(CounterApp));
    app->view_port = view_port_alloc();
    app->gui = furi_record_open("gui");
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    view_port_draw_callback_set(app->view_port, counter_draw_callback, app);
    return app;
}

int32_t counter_app(void* p) {
    CounterApp* app = counter_app_alloc();

    delay(2000);

    counter_app_free(app);
    return 0;
}
