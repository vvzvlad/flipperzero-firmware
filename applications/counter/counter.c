#include <furi.h>
#include <furi-hal.h>
#include <gui/gui.h>

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    osMessageQueueId_t event_queue;
} CounterApp;

void counter_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    osMessageQueuePut((osMessageQueueId_t)ctx, input_event, 0, 0);
}

void counter_draw_callback(Canvas* canvas, void* ctx) {
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Counter application");
}

void counter_app_free(CounterApp* app) {
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    osMessageQueueDelete(app->event_queue);
    furi_record_close("gui");
    free(app);
}

CounterApp* counter_app_alloc() {
    CounterApp* app = furi_alloc(sizeof(CounterApp));
    app->view_port = view_port_alloc();
    app->gui = furi_record_open("gui");
    app->event_queue = osMessageQueueNew(8, sizeof(InputEvent), NULL);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    view_port_draw_callback_set(app->view_port, counter_draw_callback, app);
    view_port_input_callback_set(app->view_port, counter_input_callback, app->event_queue);
    return app;
}

int32_t counter_app(void* p) {
    CounterApp* app = counter_app_alloc();

    while(1) {
        InputEvent input;
        osStatus_t result = osMessageQueueGet(app->event_queue, &input, NULL, osWaitForever);
        furi_check(result == osOK);
        if(input.type == InputTypeShort && input.key == InputKeyBack) {
            break;
        }
    }

    counter_app_free(app);
    return 0;
}
