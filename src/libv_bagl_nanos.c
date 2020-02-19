/*******************************************************************************
*   Vite Wallet for Ledger Nano S
*   (c) 2020 Vite Labs
*   (c) 2018 Mart Roosmaa
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#include <string.h>

#include "os.h"
#include "os_io_seproxyhal.h"

#include "glyphs.h"
#include "libv_internal.h"
#include "libv_bagl.h"

#if defined(TARGET_NANOS)

extern ux_state_t ux;

// display stepped screens
libv_state_t bagl_state;
uint16_t ux_step;
uint16_t ux_step_count;

#define ACCOUNT_BUF_LEN ( \
    LIBV_ACCOUNT_STRING_BASE_LEN \
    + 1 \
)

union {
    struct {
        char account[ACCOUNT_BUF_LEN];
    } displayAddress;
    struct {
        bool showAmount;
        bool showRecipient;
        bool showRepresentative;
        char confirmLabel[20];
        char confirmValue[MAX(AMOUNT_TEXT_MAX_LEN, MAX(ACCOUNT_BUF_LEN, 2*sizeof(libv_hash_t)+1))];
    } confirmSignBlock;
} vars;

void ui_write_user_address_truncated(char *label, const libv_public_key_t publicKey) {
    const size_t addressLen = libv_user_address_format((uint8_t *)label, publicKey);
    os_memset(label, '.', 2);
    os_memmove(label + 2, label + 5 + 40, 10);
    label[2+10] = '\0';
}

void ui_write_user_address_full(char *label, const libv_public_key_t publicKey) {
    const size_t addressLen = libv_user_address_format((uint8_t *)label, publicKey);
    label[addressLen] = '\0';
}

void ui_write_address_truncated(char *label, const libv_address_t rawAddress) {
    const size_t addressLen = libv_address_format((uint8_t *)label, rawAddress);
    os_memset(label, '.', 2);
    os_memmove(label + 2, label + 5 + 40, 10);
    label[2+10] = '\0';
}

void ui_write_address_full(char *label, const libv_address_t rawAddress) {
    const size_t addressLen = libv_address_format((uint8_t *)label, rawAddress);
    label[addressLen] = '\0';
}

void ui_write_hash_truncated(char *label, libv_hash_t hash) {
    libv_write_hex_string((uint8_t *)label, hash, sizeof(libv_hash_t));
    // Truncate hash to 12345..67890 format
    os_memset(label+5, '.', 2);
    os_memmove(label+7, label+2*sizeof(libv_hash_t)-5, 5);
    label[12] = '\0';
}

void ui_write_token_id_full(char *label,
                           const libv_token_id_t tokenId) {
    const size_t len = libv_token_id_format((uint8_t *)label, tokenId);
    label[len] = '\0';
}

void ui_write_data_truncated(char *label, uint8_t *data, uint64_t dataLen) {
    if (dataLen > 8) {
        libv_write_hex_string((uint8_t *)label, data, 8*2);
        // Truncate Data to 00112233aabbccdd.. format
        os_memset(label+8*2, '.', 2);
        label[8*2+2] = '\0';
    } else {
        libv_write_hex_string((uint8_t *)label, data, dataLen*2);
        label[dataLen*2] = '\0';
    }
}

void ui_write_text_truncated(char *label, uint8_t *data, uint64_t dataLen) {
    if (dataLen > 32) {
        os_memmove(label, data, 32);
        // Truncate Data to text.. format
        os_memset(label+32, '.', 2);
        label[32+2] = '\0';
    } else {
        os_memmove(label, data, dataLen);
        label[dataLen] = '\0';
    }
}


const ux_menu_entry_t menu_main[];
const ux_menu_entry_t menu_settings[];
const ux_menu_entry_t menu_settings_autoreceive[];

const bagl_element_t *menu_prepro(const ux_menu_entry_t *menu_entry, bagl_element_t *element) {
    // Customise the about menu appearance
    if (menu_entry->userid == 0xAB) {
        switch (element->component.userid) {
        case 0x21: // 1st line
            element->component.font_id = BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER;
            break;
        case 0x22: // 2nd line
            element->component.stroke = 10; // scrolldelay
            element->component.icon_id = 26; // scrollspeed
            break;
        }
    } else if (menu_entry->userid == 0xBA) {
        // Customise the badge for the coin
        switch (element->component.userid) {
        case 0x10: // icon
            element->text = (const char *)&C_nanos_badge_vite;
            break;
        }
    }
    return element;
}

void menu_settings_autoreceive_change(uint32_t enabled) {
    libv_set_auto_receive(enabled);
    // go back to the menu entry
    UX_MENU_DISPLAY(0, menu_settings, menu_prepro);
}

void menu_settings_autoreceive_init(uint32_t ignored) {
    UNUSED(ignored);
    UX_MENU_DISPLAY(N_libv.autoReceive ? 1 : 0,
                    menu_settings_autoreceive, NULL);
}

const ux_menu_entry_t menu_settings_autoreceive[] = {
    {NULL, menu_settings_autoreceive_change, 0, NULL, "No", NULL, 0, 0},
    {NULL, menu_settings_autoreceive_change, 1, NULL, "Yes", NULL, 0, 0},
    UX_MENU_END};

const ux_menu_entry_t menu_settings[] = {
    {NULL, menu_settings_autoreceive_init, 0, NULL, "Auto-receive", NULL, 0, 0},
    {menu_main, NULL, 1, &C_nanos_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};

const ux_menu_entry_t menu_about[] = {
    {NULL, NULL, 0xAB, NULL, "Version", APPVERSION, 0, 0},
    {NULL, NULL, 0xAB, NULL, "Developer", "Vite Labs", 0, 0},
    // URL with trailing spaces to avoid render artifacts when scrolling
    {NULL, NULL, 0xAB, NULL, "Source code", " github.com/vitelabs/ledger-app-vite ", 0, 0},
    {menu_main, NULL, 2, &C_nanos_icon_back, "Back", NULL, 61, 40},
    UX_MENU_END};

const ux_menu_entry_t menu_main[] = {
    {NULL, NULL, 0xBA, &C_nanos_badge_vite, "Use wallet to",
     "view accounts", 33, 12},
    {menu_settings, NULL, 0, NULL, "Settings", NULL, 0, 0},
    {menu_about, NULL, 0, NULL, "About", NULL, 0, 0},
    {NULL, os_sched_exit, 0, &C_nanos_icon_dashboard, "Quit app", NULL, 50, 29},
    UX_MENU_END};

void libv_bagl_idle(void) {
    bagl_state = LIBV_STATE_READY;
    ux_step_count = 0;
    UX_MENU_DISPLAY(0, menu_main, menu_prepro);
}

void ui_ticker_event(bool uxAllowed) {
    // don't redisplay if UX not allowed (pin locked in the common bolos
    // ux ?)
    if (ux_step_count > 0 && uxAllowed) {
        // prepare next screen
        ux_step = (ux_step + 1) % ux_step_count;
        // redisplay screen
        UX_REDISPLAY();
    }
}

/***
 * Display address
 */

const bagl_element_t ui_display_address[] = {
    {{/* type */ BAGL_RECTANGLE, /* userid */ 0x00,
      /* x */ 0, /* y */ 0, /* width */ 128, /* height */ 32,
      /* stroke */ 0, /* radius */ 0, /* fill */ BAGL_FILL,
      /* fgcolor */ 0x000000, /* bgcolor */ 0xFFFFFF,
      /* font_id */ 0, /* icon_id */ 0},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_ICON, /* userid */ 0x00,
      /* x */ 3, /* y */ 12, /* width */ 7, /* height */ 7,
      /* stroke */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ 0, /* icon_id */ BAGL_GLYPH_ICON_CROSS},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_ICON, /* userid */ 0x00,
      /* x */ 117, /* y */ 13, /* width */ 8, /* height */ 6,
      /* stroke */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ 0, /* icon_id */ BAGL_GLYPH_ICON_CHECK},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_LABELINE, /* userid */ 0x01,
      /* x */ 0, /* y */ 12, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ "Confirm", /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
    {{/* type */ BAGL_LABELINE, /* userid */ 0x01,
      /* x */ 0, /* y */ 26, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ "address", /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_LABELINE, /* userid */ 0x02,
      /* x */ 0, /* y */ 12, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ "Address", /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
    {{/* type */ BAGL_LABELINE, /* userid */ 0x02,
      /* x */ 23, /* y */ 26, /* width */ 82, /* height */ 12,
      /* scrolldelay */ 10 | BAGL_STROKE_FLAG_ONESHOT,
      /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 26},
     /* text */ vars.displayAddress.account, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
};

const bagl_element_t *ui_display_address_prepro(const bagl_element_t *element) {
    if (element->component.userid > 0) {
        bool display = (ux_step == element->component.userid - 1);
        if (!display) {
            return NULL;
        }

        switch (element->component.userid) {
        case 1:
            UX_CALLBACK_SET_INTERVAL(2000);
            break;
        case 2:
            UX_CALLBACK_SET_INTERVAL(MAX(
                3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
            break;
        }
    }
    return element;
}

uint32_t ui_display_address_button(uint32_t button_mask,
                                   uint32_t button_mask_counter) {
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        libv_bagl_display_address_callback(false);
        break;

    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        libv_bagl_display_address_callback(true);
        break;

    // For other button combinations return early and do nothing
    default:
        return 0;
    }

    libv_bagl_idle();
    return 0;
}

void libv_bagl_display_address(void) {
    if (libv_context_D.state != LIBV_STATE_CONFIRM_ADDRESS) {
        return;
    }
    libv_apdu_get_address_request_t *req = &libv_context_D.stateData.getAddressRequest;

    os_memset(&vars.displayAddress, 0, sizeof(vars.displayAddress));
    // Encode public key into an address string
    ui_write_user_address_full(
        vars.displayAddress.account,
        req->publicKey);

    bagl_state = LIBV_STATE_CONFIRM_ADDRESS;
    ux_step_count = 2;
    ux_step = 0;
    UX_DISPLAY(ui_display_address, ui_display_address_prepro);
}

/***
 * Confirm sign receive block
 */

const bagl_element_t ui_confirm_sign_receive_block[] = {
    {{/* type */ BAGL_RECTANGLE, /* userid */ 0x00,
      /* x */ 0, /* y */ 0, /* width */ 128, /* height */ 32,
      /* stroke */ 0, /* radius */ 0, /* fill */ BAGL_FILL,
      /* fgcolor */ 0x000000, /* bgcolor */ 0xFFFFFF,
      /* font_id */ 0, /* icon_id */ 0},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_ICON, /* userid */ 0x00,
      /* x */ 3, /* y */ 12, /* width */ 7, /* height */ 7,
      /* stroke */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ 0, /* icon_id */ BAGL_GLYPH_ICON_CROSS},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_ICON, /* userid */ 0x00,
      /* x */ 117, /* y */ 13, /* width */ 8, /* height */ 6,
      /* stroke */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ 0, /* icon_id */ BAGL_GLYPH_ICON_CHECK},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_LABELINE, /* userid */ 0x01,
      /* x */ 0, /* y */ 12, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ "Confirm", /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
    {{/* type */ BAGL_LABELINE, /* userid */ 0x01,
      /* x */ 0, /* y */ 26, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ "receive block", /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_LABELINE, /* userid */ 0x02,
      /* x */ 0, /* y */ 12, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ vars.confirmSignBlock.confirmLabel, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
    {{/* type */ BAGL_LABELINE, /* userid */ 0x03,
      /* x */ 23, /* y */ 26, /* width */ 82, /* height */ 12,
      /* scrolldelay */ 10 | BAGL_STROKE_FLAG_ONESHOT,
      /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 26},
     /* text */ vars.confirmSignBlock.confirmValue, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
};

void ui_confirm_sign_receive_block_prepare_confirm_step(void) {
    if (libv_context_D.state != LIBV_STATE_CONFIRM_RECEIVE_SIGNATURE) {
        return;
    }
    libv_apdu_sign_receive_block_request_t *req = &libv_context_D.stateData.signReceiveBlockRequest;
    uint8_t step = 1;

    if (ux_step == step++) {
        strcpy(vars.confirmSignBlock.confirmLabel, "Your account");
        ui_write_user_address_truncated(
            vars.confirmSignBlock.confirmValue,
            req->publicKey);
        return;
    }

    if (ux_step == step++) {
        strcpy(vars.confirmSignBlock.confirmLabel, "From block hash");
        ui_write_hash_truncated(
            vars.confirmSignBlock.confirmValue,
            req->fromHash);
        return;
    }
}

const bagl_element_t *ui_confirm_sign_receive_block_prepro(const bagl_element_t *element) {
    if (element->component.userid > 0) {
        // Determine which labels are hidden
        if (ux_step == 0) {
            if (element->component.userid != 0x01) {
                return NULL;
            }
        } else {
            if (element->component.userid == 0x01) {
                return NULL;
            }
        }

        // Use a single element (0x02) label to trigger
        // updating the confirm label/value strings.
        if (element->component.userid == 0x02) {
            ui_confirm_sign_receive_block_prepare_confirm_step();
        }

        switch (element->component.userid) {
        case 0x01:
            UX_CALLBACK_SET_INTERVAL(2000);
            break;
        case 0x03:
            UX_CALLBACK_SET_INTERVAL(MAX(
                3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
            break;
        }
    }
    return element;
}

uint32_t ui_confirm_sign_receive_block_button(uint32_t button_mask,
                                      uint32_t button_mask_counter) {
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        libv_bagl_confirm_sign_receive_block_callback(false);
        break;

    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        libv_bagl_confirm_sign_receive_block_callback(true);
        break;

    // For other button combinations return early and do nothing
    default:
        return 0;
    }

    libv_bagl_idle();
    return 0;
}

void libv_bagl_confirm_sign_receive_block(void) {
    if (libv_context_D.state != LIBV_STATE_CONFIRM_RECEIVE_SIGNATURE) {
        return;
    }
    libv_apdu_sign_receive_block_request_t *req = &libv_context_D.stateData.signReceiveBlockRequest;

    os_memset(&vars.confirmSignBlock, 0, sizeof(vars.confirmSignBlock));

    bagl_state = LIBV_STATE_CONFIRM_RECEIVE_SIGNATURE;
    ux_step = 0;
    ux_step_count = 3;

    UX_DISPLAY(ui_confirm_sign_receive_block, ui_confirm_sign_receive_block_prepro);
}

/***
 * Confirm sign send block
 */

const bagl_element_t ui_confirm_sign_send_block[] = {
    {{/* type */ BAGL_RECTANGLE, /* userid */ 0x00,
      /* x */ 0, /* y */ 0, /* width */ 128, /* height */ 32,
      /* stroke */ 0, /* radius */ 0, /* fill */ BAGL_FILL,
      /* fgcolor */ 0x000000, /* bgcolor */ 0xFFFFFF,
      /* font_id */ 0, /* icon_id */ 0},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_ICON, /* userid */ 0x00,
      /* x */ 3, /* y */ 12, /* width */ 7, /* height */ 7,
      /* stroke */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ 0, /* icon_id */ BAGL_GLYPH_ICON_CROSS},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_ICON, /* userid */ 0x00,
      /* x */ 117, /* y */ 13, /* width */ 8, /* height */ 6,
      /* stroke */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ 0, /* icon_id */ BAGL_GLYPH_ICON_CHECK},
     /* text */ NULL, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_LABELINE, /* userid */ 0x01,
      /* x */ 0, /* y */ 12, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ "Confirm", /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
    {{/* type */ BAGL_LABELINE, /* userid */ 0x01,
      /* x */ 0, /* y */ 26, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ "send block", /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},

    {{/* type */ BAGL_LABELINE, /* userid */ 0x02,
      /* x */ 0, /* y */ 12, /* width */ 128, /* height */ 12,
      /* scrolldelay */ 0, /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_REGULAR_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 0},
     /* text */ vars.confirmSignBlock.confirmLabel, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
    {{/* type */ BAGL_LABELINE, /* userid */ 0x03,
      /* x */ 23, /* y */ 26, /* width */ 82, /* height */ 12,
      /* scrolldelay */ 10 | BAGL_STROKE_FLAG_ONESHOT,
      /* radius */ 0, /* fill */ 0,
      /* fgcolor */ 0xFFFFFF, /* bgcolor */ 0x000000,
      /* font_id */ BAGL_FONT_OPEN_SANS_EXTRABOLD_11px | BAGL_FONT_ALIGNMENT_CENTER,
      /* scrollspeed */ 26},
     /* text */ vars.confirmSignBlock.confirmValue, /* touch_area_brim */ 0,
     /* overfgcolor */ 0, /* overbgcolor */ 0,
     /* tap */ NULL, /* out */ NULL, /* over */ NULL},
};

void ui_confirm_sign_send_block_prepare_confirm_step(void) {
    if (libv_context_D.state != LIBV_STATE_CONFIRM_SEND_SIGNATURE) {
        return;
    }
    libv_apdu_sign_send_block_request_t *req = &libv_context_D.stateData.signSendBlockRequest;
    uint8_t step = 1;

    if (ux_step == step++) {
        strcpy(vars.confirmSignBlock.confirmLabel, "Your account");
        ui_write_user_address_truncated(
            vars.confirmSignBlock.confirmValue,
            req->publicKey);
        return;
    }

    if (ux_step == step++) {
        strcpy(vars.confirmSignBlock.confirmLabel, "To address");
        ui_write_address_truncated(
            vars.confirmSignBlock.confirmValue,
            req->toAddress);
        return;
    }

    if (req->amountFmt.suffixLen == 0) {
        if (ux_step == step++) {
            strcpy(vars.confirmSignBlock.confirmLabel, "Token ID");
            ui_write_token_id_full(
                vars.confirmSignBlock.confirmValue,
                req->tokenId);
            return;
        }

        if (ux_step == step++) {
            strcpy(vars.confirmSignBlock.confirmLabel, "Send amount");
            libv_amount_format(
                &req->amountFmt,
                vars.confirmSignBlock.confirmValue,
                sizeof(vars.confirmSignBlock.confirmValue),
                req->amount);
            return;
        }
    } else {
        if (ux_step == step++) {
            strcpy(vars.confirmSignBlock.confirmLabel, "Send amount");
            libv_amount_format(
                &req->amountFmt,
                vars.confirmSignBlock.confirmValue,
                sizeof(vars.confirmSignBlock.confirmValue),
                req->amount);
            return;
        }
    }

    if (req->dataLen > 0) {
        if (ux_step == step++) {
            strcpy(vars.confirmSignBlock.confirmLabel, "Data");
            ui_write_data_truncated(
                vars.confirmSignBlock.confirmValue,
                req->data,
                req->dataLen);
            return;
        }

        // if (req->dataIsNote) {
        //     if (ux_step == step++) {
        //         strcpy(vars.confirmSignBlock.confirmLabel, "Data(ASCII)");
        //         ui_write_text_truncated(
        //             vars.confirmSignBlock.confirmValue,
        //             req->data,
        //             req->dataLen);
        //         return;
        //     }
        // }
    }
}

const bagl_element_t *ui_confirm_sign_send_block_prepro(const bagl_element_t *element) {
    if (element->component.userid > 0) {
        // Determine which labels are hidden
        if (ux_step == 0) {
            if (element->component.userid != 0x01) {
                return NULL;
            }
        } else {
            if (element->component.userid == 0x01) {
                return NULL;
            }
        }

        // Use a single element (0x02) label to trigger
        // updating the confirm label/value strings.
        if (element->component.userid == 0x02) {
            ui_confirm_sign_send_block_prepare_confirm_step();
        }

        switch (element->component.userid) {
        case 0x01:
            UX_CALLBACK_SET_INTERVAL(2000);
            break;
        case 0x03:
            UX_CALLBACK_SET_INTERVAL(MAX(
                3000, 1000 + bagl_label_roundtrip_duration_ms(element, 7)));
            break;
        }
    }
    return element;
}

uint32_t ui_confirm_sign_send_block_button(uint32_t button_mask,
                                           uint32_t button_mask_counter) {
    switch (button_mask) {
    case BUTTON_EVT_RELEASED | BUTTON_LEFT:
        libv_bagl_confirm_sign_send_block_callback(false);
        break;

    case BUTTON_EVT_RELEASED | BUTTON_RIGHT:
        libv_bagl_confirm_sign_send_block_callback(true);
        break;

    // For other button combinations return early and do nothing
    default:
        return 0;
    }

    libv_bagl_idle();
    return 0;
}

void libv_bagl_confirm_sign_send_block(void) {
    if (libv_context_D.state != LIBV_STATE_CONFIRM_SEND_SIGNATURE) {
        return;
    }
    libv_apdu_sign_send_block_request_t *req = &libv_context_D.stateData.signSendBlockRequest;

    os_memset(&vars.confirmSignBlock, 0, sizeof(vars.confirmSignBlock));

    bagl_state = LIBV_STATE_CONFIRM_SEND_SIGNATURE;
    ux_step = 0;
    
    os_memset(&req->amountFmt, 0, sizeof(libv_amount_formatter_t));

    for (size_t i = 0; i < TOKEN_INFO_ARRAY_LEN; i++)
    {
        TOKEN_INFO info = TOKEN_INFO_ARRAY[i];
        if (os_memcmp(req->tokenId, info.tokenId, sizeof(libv_token_id_t)) == 0) {
            req->amountFmt.suffix = info.suffix;
            req->amountFmt.suffixLen = strnlen(info.suffix, sizeof(info.suffix));
            req->amountFmt.unitScale = info.unitScale;
            break;
        }
    }

    ux_step_count = 4
        + (req->dataLen > 0 ? 1: 0)
        // + (req->dataIsNote ? 1: 0)
        + (req->amountFmt.suffixLen == 0 ? 1 : 0);

    UX_DISPLAY(ui_confirm_sign_send_block, ui_confirm_sign_send_block_prepro);
}

bool libv_bagl_apply_state() {
    if (!UX_DISPLAYED()) {
        return false;
    }

    switch (libv_context_D.state) {
    case LIBV_STATE_READY:
        if (bagl_state != LIBV_STATE_READY) {
            libv_bagl_idle();
            return true;
        }
        break;
    case LIBV_STATE_CONFIRM_ADDRESS:
        if (bagl_state != LIBV_STATE_CONFIRM_ADDRESS) {
            libv_bagl_display_address();
            return true;
        }
        break;
    case LIBV_STATE_CONFIRM_RECEIVE_SIGNATURE:
        if (bagl_state != LIBV_STATE_CONFIRM_RECEIVE_SIGNATURE) {
            libv_bagl_confirm_sign_receive_block();
            return true;
        }
        break;
    case LIBV_STATE_CONFIRM_SEND_SIGNATURE:
        if (bagl_state != LIBV_STATE_CONFIRM_SEND_SIGNATURE) {
            libv_bagl_confirm_sign_send_block();
            return true;
        }
        break;
    }

    return false;
}

#endif // defined(TARGET_NANOS)
