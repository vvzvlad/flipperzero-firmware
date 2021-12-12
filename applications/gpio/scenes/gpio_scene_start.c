#include "../gpio_app_i.h"
#include "furi-hal-power.h"

enum GpioItem {
    GpioItemUsbUart,
};


static void gpio_scene_start_var_list_enter_callback(void* context, uint32_t index) {
    furi_assert(context);
    GpioApp* app = context;
    if(index == GpioItemUsbUart) {
        view_dispatcher_send_custom_event(app->view_dispatcher, GpioStartEventUsbUart);
    }
}


void gpio_scene_start_on_enter(void* context) {
    GpioApp* app = context;
    VariableItemList* var_item_list = app->var_item_list;

    variable_item_list_set_enter_callback(
        var_item_list, gpio_scene_start_var_list_enter_callback, app);

    variable_item_list_add(var_item_list, "USB-UART bridge", 0, NULL, NULL);

    variable_item_list_set_selected_item(
        var_item_list, scene_manager_get_scene_state(app->scene_manager, GpioSceneStart));

    view_dispatcher_switch_to_view(app->view_dispatcher, GpioAppViewVarItemList);
}

bool gpio_scene_start_on_event(void* context, SceneManagerEvent event) {
    GpioApp* app = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GpioStartEventUsbUart) {
            scene_manager_set_scene_state(app->scene_manager, GpioSceneStart, GpioItemUsbUart);
            scene_manager_next_scene(app->scene_manager, GpioSceneUsbUart);
        }
        consumed = true;
    }
    return consumed;
}

void gpio_scene_start_on_exit(void* context) {
    GpioApp* app = context;
    variable_item_list_clean(app->var_item_list);
}
