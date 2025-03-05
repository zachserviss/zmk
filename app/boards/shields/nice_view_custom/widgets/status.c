/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/display.h>
#include "status.h"
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/wpm.h>
#include <zmk/events/position_state_changed.h>
#include "bongocatart.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state {
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
};

struct layer_status_state {
    uint8_t index;
    const char *label;
};

struct wpm_status_state {
    uint8_t wpm;             // Current WPM
    uint8_t wpm_history[10]; // Historical WPM values
    uint8_t animation_state; // Current animation state
    bool key_pressed;        // Keypress state
    bool is_key_event;       // Flag for key events
};

enum anim_state { ANIM_STATE_CASUAL, ANIM_STATE_FRENZIED } current_anim_state = ANIM_STATE_CASUAL;

LV_IMG_DECLARE(bongocatrest0);
LV_IMG_DECLARE(bongocatcasual1);
LV_IMG_DECLARE(bongocatcasual2);
LV_IMG_DECLARE(bongocatfast1);
LV_IMG_DECLARE(bongocatfast2);

static bool key_pressed = false;
static bool key_released = false;
static bool use_first_frame = true; // Track which frame to use in the animation sequence

static void draw_top(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_TEXT_ALIGN_RIGHT);
    lv_draw_label_dsc_t label_dsc_wpm;
    init_label_dsc(&label_dsc_wpm, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_RIGHT);
    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);
    lv_draw_rect_dsc_t rect_white_dsc;
    init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);
    lv_draw_line_dsc_t line_dsc;
    init_line_dsc(&line_dsc, LVGL_FOREGROUND, 1);

    // Fill background
    lv_canvas_draw_rect(canvas, 0, 0, CANVAS_SIZE, CANVAS_SIZE, &rect_black_dsc);

    // Draw battery
    draw_battery(canvas, state);

    // Draw output status
    char output_text[10] = {};

    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        strcat(output_text, LV_SYMBOL_USB);
        break;
    case ZMK_TRANSPORT_BLE:
        if (state->active_profile_bonded) {
            if (state->active_profile_connected) {
                strcat(output_text, LV_SYMBOL_WIFI);
            } else {
                strcat(output_text, LV_SYMBOL_CLOSE);
            }
        } else {
            strcat(output_text, LV_SYMBOL_SETTINGS);
        }
        break;
    }

    lv_canvas_draw_text(canvas, 0, 0, CANVAS_SIZE, &label_dsc, output_text);

    // Draw WPM
    lv_canvas_draw_rect(canvas, 0, 21, 68, 42, &rect_white_dsc);
    lv_canvas_draw_rect(canvas, 1, 22, 66, 40, &rect_black_dsc);

    char wpm_text[6] = {};
    snprintf(wpm_text, sizeof(wpm_text), "%d", state->wpm[9]);
    lv_canvas_draw_text(canvas, 42, 52, 24, &label_dsc_wpm, wpm_text);

    int max = 0;
    int min = 256;

    for (int i = 0; i < 10; i++) {
        if (state->wpm[i] > max) {
            max = state->wpm[i];
        }
        if (state->wpm[i] < min) {
            min = state->wpm[i];
        }
    }

    int range = max - min;
    if (range == 0) {
        range = 1;
    }

    lv_point_t points[10];
    for (int i = 0; i < 10; i++) {
        points[i].x = 2 + i * 7;
        points[i].y = 60 - (state->wpm[i] - min) * 36 / range;
    }
    lv_canvas_draw_line(canvas, points, 10, &line_dsc);

    // Rotate canvas
    rotate_canvas(canvas, cbuf);
}

sstatic void draw_middle(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 1);

    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);
    lv_draw_rect_dsc_t rect_white_dsc;
    init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);
    lv_draw_arc_dsc_t arc_dsc;
    init_arc_dsc(&arc_dsc, LVGL_FOREGROUND, 2);
    lv_draw_arc_dsc_t arc_dsc_filled;
    init_arc_dsc(&arc_dsc_filled, LVGL_FOREGROUND, 9);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);
    lv_draw_label_dsc_t label_dsc_black;
    init_label_dsc(&label_dsc_black, LVGL_BACKGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);

    // Fill background
    lv_canvas_draw_rect(canvas, 0, 0, CANVAS_SIZE, CANVAS_SIZE, &rect_black_dsc);

    // Draw single BLE profile circle at the top
    int x = 34, y = 13;
    bool selected = true; // Always selected since we are displaying only one circle

    lv_canvas_draw_arc(canvas, x, y, 13, 0, 360, &arc_dsc);
    if (selected) {
        lv_canvas_draw_arc(canvas, x, y, 9, 0, 359, &arc_dsc_filled);
    }
    char label[2];
    snprintf(label, sizeof(label), "%d", state->active_profile_index); // Display profile number 0-4
    lv_canvas_draw_text(canvas, x - 8, y - 10, 16, (selected ? &label_dsc_black : &label_dsc),
                        label);

    // Calculate average WPM over last 5 seconds
    int recent_wpm = 0;
    for (int i = 5; i < 10; i++) {
        recent_wpm += state->wpm[i];
    }
    recent_wpm /= 5;

    // Update animation state based on WPM
    if (recent_wpm > 30) {
        current_anim_state = ANIM_STATE_FRENZIED;
    } else {
        current_anim_state = ANIM_STATE_CASUAL;
    }

    // Determine which animation frame to use
    const lv_img_dsc_t *current_frame;

    if (current_anim_state == ANIM_STATE_CASUAL) {
        if (key_pressed) {
            // Alternate between casual1 and casual2 on keypresses
            current_frame = use_first_frame ? &bongocatcasual1 : &bongocatcasual2;
            use_first_frame = !use_first_frame; // Toggle for next press
        } else {
            // Show rest frame when no key is pressed or when key is released
            current_frame = &bongocatrest0;
        }
    } else { // ANIM_STATE_FRENZIED
        if (key_pressed || key_released) {
            // Alternate between fast1 and fast2 on every key event
            current_frame = use_first_frame ? &bongocatfast1 : &bongocatfast2;
            use_first_frame = !use_first_frame; // Toggle for next frame
        } else {
            // Keep showing the last frame when no key events
            current_frame = use_first_frame ? &bongocatfast1 : &bongocatfast2;
        }
    }

    // Reset key_released flag after handling
    key_released = false;

    // Draw bongo cat animation frame
    lv_canvas_draw_rect(canvas, 0, 28, 68, 40, &rect_white_dsc);
    lv_canvas_draw_rect(canvas, 1, 29, 66, 38, &rect_black_dsc);

    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);
    lv_canvas_draw_img(canvas, 0, 28, current_frame, &img_dsc);

    // Rotate canvas
    rotate_canvas(canvas, cbuf);
}

static void draw_bottom(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 2);

    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);
    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);

    // Fill background
    lv_canvas_draw_rect(canvas, 0, 0, CANVAS_SIZE, CANVAS_SIZE, &rect_black_dsc);

    // Draw layer
    if (state->layer_label == NULL) {
        char text[10] = {};

        sprintf(text, "LAYER %i", state->layer_index);

        lv_canvas_draw_text(canvas, 0, 5, 68, &label_dsc, text);
    } else {
        lv_canvas_draw_text(canvas, 0, 5, 68, &label_dsc, state->layer_label);
    }

    // Rotate canvas
    rotate_canvas(canvas, cbuf);
}

static void set_battery_status(struct zmk_widget_status *widget,
                               struct battery_status_state state) {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

    widget->state.battery = state.level;

    draw_top(widget->obj, widget->cbuf, &widget->state);
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

    return (struct battery_status_state){
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

static void set_output_status(struct zmk_widget_status *widget,
                              const struct output_status_state *state) {
    widget->state.selected_endpoint = state->selected_endpoint;
    widget->state.active_profile_index = state->active_profile_index;
    widget->state.active_profile_connected = state->active_profile_connected;
    widget->state.active_profile_bonded = state->active_profile_bonded;

    draw_top(widget->obj, widget->cbuf, &widget->state);
    draw_middle(widget->obj, widget->cbuf2, &widget->state);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_output_status(widget, &state); }
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){
        .selected_endpoint = zmk_endpoints_selected(),
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

static void set_layer_status(struct zmk_widget_status *widget, struct layer_status_state state) {
    widget->state.layer_index = state.index;
    widget->state.layer_label = state.label;

    draw_bottom(widget->obj, widget->cbuf3, &widget->state);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_status(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){.index = index, .label = zmk_keymap_layer_name(index)};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

static void set_wpm_status(struct zmk_widget_status *widget, struct wpm_status_state state) {
    // Update WPM array
    for (int i = 0; i < 9; i++) {
        widget->state.wpm[i] = widget->state.wpm[i + 1];
    }
    widget->state.wpm[9] = state.wpm;

    // Only update key states if this was triggered by a key event
    if (state.is_key_event) {
        key_pressed = state.key_pressed;
        key_released = !state.key_pressed;

        // Force redraw on every key event
        draw_middle(widget->obj, widget->cbuf2, &widget->state);
    } else {
        // For WPM updates, only redraw if we need to change animation state
        int recent_wpm = 0;
        for (int i = 5; i < 10; i++) {
            recent_wpm += widget->state.wpm[i];
        }
        recent_wpm /= 5;

        enum anim_state new_state = (recent_wpm > 30) ? ANIM_STATE_FRENZIED : ANIM_STATE_CASUAL;

        if (new_state != current_anim_state) {
            current_anim_state = new_state;
            draw_middle(widget->obj, widget->cbuf2, &widget->state);
        } else {
            // Regular WPM update
            draw_top(widget->obj, widget->cbuf, &widget->state);
        }
    }
}

static void wpm_status_update_cb(struct wpm_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_wpm_status(widget, state); }
}

struct wpm_status_state wpm_status_get_state(const zmk_event_t *eh) {
    static uint8_t wpm_history[10] = {0}; // Keep track of history between calls
    static uint8_t current_wpm = 0;       // Keep track of current WPM between calls

    const struct zmk_wpm_state_changed *wpm_ev = as_zmk_wpm_state_changed(eh);
    const struct zmk_position_state_changed *pos_ev = as_zmk_position_state_changed(eh);

    // Update WPM if this is a WPM event
    if (wpm_ev != NULL) {
        current_wpm = wpm_ev->state;
        for (int i = 0; i < 9; i++) {
            wpm_history[i] = wpm_history[i + 1];
        }
        wpm_history[9] = current_wpm;
    }

    // Update key state if this is a position event
    bool is_key_event = false;
    if (pos_ev != NULL) {
        is_key_event = true;
        if (pos_ev->state > 0) {
            key_pressed = true;
            key_released = false;
        } else {
            key_pressed = false;
            key_released = true;
        }
    }

    return (struct wpm_status_state){
        .wpm = current_wpm,
        .wpm_history = {wpm_history[0], wpm_history[1], wpm_history[2], wpm_history[3],
                        wpm_history[4], wpm_history[5], wpm_history[6], wpm_history[7],
                        wpm_history[8], wpm_history[9]},
        .animation_state = current_anim_state,
        .key_pressed = key_pressed,
        .is_key_event = is_key_event // Add this flag to the struct
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_wpm_status, struct wpm_status_state, wpm_status_update_cb,
                            wpm_status_get_state)
ZMK_SUBSCRIPTION(widget_wpm_status, zmk_wpm_state_changed);
ZMK_SUBSCRIPTION(widget_wpm_status, zmk_position_state_changed);

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 160, 68);
    lv_obj_t *top = lv_canvas_create(widget->obj);
    lv_obj_align(top, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_canvas_set_buffer(top, widget->cbuf, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);
    lv_obj_t *middle = lv_canvas_create(widget->obj);
    lv_obj_align(middle, LV_ALIGN_TOP_LEFT, 24, 0);
    lv_canvas_set_buffer(middle, widget->cbuf2, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);
    lv_obj_t *bottom = lv_canvas_create(widget->obj);
    lv_obj_align(bottom, LV_ALIGN_TOP_LEFT, -44, 0);
    lv_canvas_set_buffer(bottom, widget->cbuf3, CANVAS_SIZE, CANVAS_SIZE, LV_IMG_CF_TRUE_COLOR);

    sys_slist_append(&widgets, &widget->node);
    widget_battery_status_init();
    widget_output_status_init();
    widget_layer_status_init();
    widget_wpm_status_init();

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
