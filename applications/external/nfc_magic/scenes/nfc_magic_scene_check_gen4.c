#include "../nfc_magic_i.h"

enum {
    NfcMagicSceneCheckStateCardSearch,
    NfcMagicSceneCheckStateCardFound,
};

bool nfc_magic_check_gen4_worker_callback(NfcMagicWorkerEvent event, void* context) {
    furi_assert(context);

    NfcMagic* nfc_magic = context;
    view_dispatcher_send_custom_event(nfc_magic->view_dispatcher, event);

    return true;
}

static void nfc_magic_scene_check_gen4_setup_view(NfcMagic* nfc_magic) {
    Popup* popup = nfc_magic->popup;
    popup_reset(popup);
    uint32_t state = scene_manager_get_scene_state(nfc_magic->scene_manager, NfcMagicSceneCheck);

    if(state == NfcMagicSceneCheckStateCardSearch) {
        popup_set_icon(nfc_magic->popup, 0, 8, &I_NFC_manual_60x50);
        popup_set_text(
            nfc_magic->popup, "Apply card to\nthe back", 128, 32, AlignRight, AlignCenter);
    } else {
        popup_set_icon(popup, 12, 23, &I_Loading_24);
        popup_set_header(popup, "Checking\nDon't move...", 52, 32, AlignLeft, AlignCenter);
    }

    view_dispatcher_switch_to_view(nfc_magic->view_dispatcher, NfcMagicViewPopup);
}

void nfc_magic_scene_check_gen4_on_enter(void* context) {
    NfcMagic* nfc_magic = context;

    scene_manager_set_scene_state(
        nfc_magic->scene_manager, NfcMagicSceneCheck, NfcMagicSceneCheckStateCardSearch);
    nfc_magic_scene_check_gen4_setup_view(nfc_magic);

    // Setup and start worker
    nfc_magic_worker_start(
        nfc_magic->worker,
        NfcMagicWorkerStateCheck,
        nfc_magic->dev,
        &nfc_magic->source_dev->dev_data,
        nfc_magic->new_password,
        nfc_magic_check_gen4_worker_callback,
        nfc_magic);
    nfc_magic_blink_start(nfc_magic);
}

bool nfc_magic_scene_check_gen4_on_event(void* context, SceneManagerEvent event) {
    NfcMagic* nfc_magic = context;
    bool consumed = false;

    if(event.type == SceneManagerEventTypeCustom) {
        if(event.event == NfcMagicWorkerEventSuccess) {
            // Check that the card is gen4
            if(nfc_magic->dev->type == MagicTypeGen4) {
                // If it was detected - go to gen4 actions
                scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneGen4Actions);
            } else {
                // It's not a gen4 card - go to wrong card scene
                scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneWrongCard);
            }
            consumed = true;

        } else if(event.event == NfcMagicWorkerEventNotMagic) {
            // It may be a gen4 card with wrong password
            // If it was checked with the default password - ask the user for a custom one
            if(nfc_magic->dev->password == MAGIC_GEN4_DEFAULT_PWD) {
                scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneKeyInput);
            } else {
                // If it was checked with a custom password - go to fail
                scene_manager_next_scene(nfc_magic->scene_manager, NfcMagicSceneCheckGen4Fail);
            }
            consumed = true;
        } else if(event.event == NfcMagicWorkerEventCardDetected) {
            scene_manager_set_scene_state(
                nfc_magic->scene_manager, NfcMagicSceneCheck, NfcMagicSceneCheckStateCardFound);
            nfc_magic_scene_check_gen4_setup_view(nfc_magic);
            consumed = true;
        } else if(event.event == NfcMagicWorkerEventNoCardDetected) {
            scene_manager_set_scene_state(
                nfc_magic->scene_manager, NfcMagicSceneCheck, NfcMagicSceneCheckStateCardSearch);
            nfc_magic_scene_check_gen4_setup_view(nfc_magic);
            consumed = true;
        }
    } else if(event.type == SceneManagerEventTypeBack) {
        consumed = scene_manager_search_and_switch_to_previous_scene(
            nfc_magic->scene_manager, NfcMagicSceneStart);
    }
    return consumed;
}

void nfc_magic_scene_check_gen4_on_exit(void* context) {
    NfcMagic* nfc_magic = context;

    nfc_magic_worker_stop(nfc_magic->worker);
    scene_manager_set_scene_state(
        nfc_magic->scene_manager, NfcMagicSceneCheck, NfcMagicSceneCheckStateCardSearch);
    // Clear view
    popup_reset(nfc_magic->popup);

    nfc_magic_blink_stop(nfc_magic);
}
