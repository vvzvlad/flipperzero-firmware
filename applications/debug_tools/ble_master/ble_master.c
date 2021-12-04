#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <bt/bt_service/bt.h>
#include <furi-hal-bt.h>
#include <furi-hal-bt-hid.h>
#include <furi-hal-usb-hid.h>

#define TAG "BleMasterApp"

typedef enum {
    EventTypeInput,
} EventType;

typedef struct {
    union {
        InputEvent input;
    };
    EventType type;
} BleMasterEvent;

static void ble_master_render_callback(Canvas* canvas, void* ctx) {
    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "BLE master demo (no.)");

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 0, 63, "Hold [back] to exit");
}

static void ble_master_input_callback(InputEvent* input_event, void* ctx) {
    osMessageQueueId_t event_queue = ctx;

    BleMasterEvent event;
    event.type = EventTypeInput;
    event.input = *input_event;
    osMessageQueuePut(event_queue, &event, 0, osWaitForever);
}

int32_t ble_master_app(void* p) {
    Bt* bt = furi_record_open("bt");
    if(!bt_set_profile(bt, BtProfileHidKeyboard)) {
        FURI_LOG_E(TAG, "Failed to switch profile");
        furi_record_close("bt");
        return -1;
    }

    osMessageQueueId_t event_queue = osMessageQueueNew(8, sizeof(BleMasterEvent), NULL);
    furi_check(event_queue);
    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, ble_master_render_callback, NULL);
    view_port_input_callback_set(view_port, ble_master_input_callback, event_queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open("gui");
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    BleMasterEvent event;
    while(1) {
        osStatus_t event_status = osMessageQueueGet(event_queue, &event, NULL, osWaitForever);

        if(event_status == osOK) {
            if(event.type == EventTypeInput) {
                if(event.input.type == InputTypeLong && event.input.key == InputKeyBack) {
                    break;
                }

                if(event.input.key == InputKeyBack) {
                    if(event.input.type == InputTypePress) {
                        furi_hal_bt_hid_kb_press(KEY_ESC);
                    } else if(event.input.type == InputTypeRelease) {
                        furi_hal_bt_hid_kb_release(KEY_ESC);
                    }
                }

                if(event.input.key == InputKeyOk) {
                    if(event.input.type == InputTypePress) {
                        furi_hal_bt_hid_kb_press(KEY_ENTER);
                    } else if(event.input.type == InputTypeRelease) {
                        furi_hal_bt_hid_kb_release(KEY_ENTER);
                    }
                }

                if(event.input.key == InputKeyRight) {
                    if(event.input.type == InputTypePress) {
                        furi_hal_bt_hid_kb_press(KEY_RIGHT_ARROW);
                    } else if(event.input.type == InputTypeRelease) {
                        furi_hal_bt_hid_kb_release(KEY_RIGHT_ARROW);
                    }
                }

                if(event.input.key == InputKeyLeft) {
                    if(event.input.type == InputTypePress) {
                        furi_hal_bt_hid_kb_press(KEY_LEFT_ARROW);
                    } else if(event.input.type == InputTypeRelease) {
                        furi_hal_bt_hid_kb_release(KEY_LEFT_ARROW);
                    }
                }

                if(event.input.key == InputKeyDown) {
                    if(event.input.type == InputTypePress) {
                        furi_hal_bt_hid_kb_press(KEY_DOWN_ARROW);
                    } else if(event.input.type == InputTypeRelease) {
                        furi_hal_bt_hid_kb_release(KEY_DOWN_ARROW);
                    }
                }

                if(event.input.key == InputKeyUp) {
                    if(event.input.type == InputTypePress) {
                        furi_hal_bt_hid_kb_press(KEY_UP_ARROW);
                    } else if(event.input.type == InputTypeRelease) {
                        furi_hal_bt_hid_kb_release(KEY_UP_ARROW);
                    }
                }
            }
        }
        view_port_update(view_port);
    }

    // remove & free all stuff created by app
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    osMessageQueueDelete(event_queue);
    furi_record_close("gui");
    bt_set_profile(bt, BtProfileSerial);
    furi_record_close("bt");
    return 0;
}
