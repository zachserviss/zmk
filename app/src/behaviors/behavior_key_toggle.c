/*
 * Copyright (c) 2022 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_key_toggle

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/hid.h>
#include <zmk/event_manager.h>
#include <zmk/events/keycode_state_changed.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

<<<<<<< HEAD
static int behavior_key_toggle_init(const struct device *dev) { return 0; }
=======
enum toggle_mode {
    ON,
    OFF,
    FLIP,
};

struct behavior_key_toggle_config {
    enum toggle_mode toggle_mode;
};
>>>>>>> 207afe2ecda1ff53c7ec2af74d2aef61be87b684

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    LOG_DBG("position %d keycode 0x%02X", event.position, binding->param1);
    bool pressed = zmk_hid_is_pressed(binding->param1);
    return raise_zmk_keycode_state_changed_from_encoded(binding->param1, !pressed, event.timestamp);
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    return 0;
}

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_parameter_value_metadata param_values[] = {
    {
        .display_name = "Key",
        .type = BEHAVIOR_PARAMETER_VALUE_TYPE_HID_USAGE,
    },
};

static const struct behavior_parameter_metadata_set param_metadata_set[] = {{
    .param1_values = param_values,
    .param1_values_len = ARRAY_SIZE(param_values),
}};

static const struct behavior_parameter_metadata metadata = {
    .sets_len = ARRAY_SIZE(param_metadata_set),
    .sets = param_metadata_set,
};

#endif

static const struct behavior_driver_api behavior_key_toggle_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .parameter_metadata = &metadata,
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
};

#define KT_INST(n)                                                                                 \
<<<<<<< HEAD
    BEHAVIOR_DT_INST_DEFINE(n, behavior_key_toggle_init, NULL, NULL, NULL, POST_KERNEL,            \
=======
    static const struct behavior_key_toggle_config behavior_key_toggle_config_##n = {              \
        .toggle_mode = DT_ENUM_IDX(DT_DRV_INST(n), toggle_mode),                                   \
    };                                                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, &behavior_key_toggle_config_##n, POST_KERNEL,     \
>>>>>>> 207afe2ecda1ff53c7ec2af74d2aef61be87b684
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, &behavior_key_toggle_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KT_INST)
