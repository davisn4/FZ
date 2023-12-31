#include "../nfc_magic_i.h"

void nfc_magic_scene_check_gen4_fail_widget_callback(
    GuiButtonType result,
    InputType type,
    void* context) {
    NfcMagic* nfc_magic = context;
    if(type == InputTypeShort) {
        view_dispatcher_send_custom_event(nfc_magic->view_dispatcher, result);
    }
}

void nfc_magic_scene_check_gen4_fail_on_enter(void* context) {
    NfcMagic* nfc_magic = context;
    Widget* widget = nfc_magic->widget;

    notification_message(nfc_magic->notifications, &sequence_error);

    widget_add_icon_element(widget, 73, 17, &I_DolphinCommon_56x48);
    widget_add_string_element(
        widget, 1, 4, AlignLeft, AlignTop, FontPrimary, "Password auth failed");
    widget_add_string_multiline_element(
        widget,
        1,
        17,
        AlignLeft,
        AlignTop,
        FontSecondary,
        "Wrong password or\nthis is not a\ngen4 magic card.");
    widget_add_button_element(
        widget,
        GuiButtonTypeLeft,
        "Retry",
        nfc_magic_scene_check_gen4_fail_widget_callback,
        nfc_magic);

    // Setup and start worker
    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewWidget);
}

bool nfc_magic_scene_check_gen4_fail_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == GuiButtonTypeLeft) {
            consumed = scene_manager_previous_scene(nfc_magic->scene_manager);
        }
    }
    return consumed;
}

void nfc_magic_scene_check_gen4_fail_on_exit(void* context) {
    NfcMagic* nfc_magic = context;

    widget_reset(nfc_magic->widget);
}
