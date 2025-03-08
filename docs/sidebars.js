module.exports = {
  docs: [
    {
      type: "category",
      label: "Getting Started",
      link: {
        type: "doc",
        id: "intro",
      },
      collapsed: false,
      items: [
        "hardware",
        "faq",
        "user-setup",
        "customization",
        {
          type: "category",
          label: "Troubleshooting",
          link: {
            type: "doc",
            id: "troubleshooting/index",
          },
          collapsed: true,
          items: [
            "troubleshooting/building-issues",
            "troubleshooting/flashing-issues",
            "troubleshooting/connection-issues",
          ],
        },
      ],
    },
    {
      Features: [
        "features/bluetooth",
        "features/modules",
        "features/split-keyboards",
        "features/debouncing",
        "features/battery",
        "features/soft-off",
        "features/encoders",
        "features/displays",
        "features/lighting",
        "features/studio",
      ],
    },
    {
      type: "category",
      label: "Keymaps",
      link: {
        type: "doc",
        id: "keymaps/index",
      },
      collapsed: true,
      items: [
        {
          type: "category",
          label: "Behaviors",
          link: {
            type: "doc",
            id: "keymaps/behaviors/index",
          },
          collapsed: true,
          items: [
            "keymaps/behaviors/key-press",
            "keymaps/behaviors/layers",
            "keymaps/behaviors/misc",
            "keymaps/behaviors/hold-tap",
            "keymaps/behaviors/mod-tap",
            "keymaps/behaviors/mod-morph",
            "keymaps/behaviors/macros",
            "keymaps/behaviors/key-toggle",
            "keymaps/behaviors/sticky-key",
            "keymaps/behaviors/sticky-layer",
            "keymaps/behaviors/tap-dance",
            "keymaps/behaviors/caps-word",
            "keymaps/behaviors/key-repeat",
            "keymaps/behaviors/sensor-rotate",
            "keymaps/behaviors/mouse-emulation",
            "keymaps/behaviors/reset",
            "keymaps/behaviors/bluetooth",
            "keymaps/behaviors/outputs",
            "keymaps/behaviors/underglow",
            "keymaps/behaviors/backlight",
            "keymaps/behaviors/power",
            "keymaps/behaviors/soft-off",
            "keymaps/behaviors/studio-unlock",
          ],
        },
        "keymaps/modifiers",
        "keymaps/combos",
        "keymaps/conditional-layers",
        "keymaps/list-of-keycodes",
      ],
    },
    {
      type: "category",
      label: "Configuration",
      link: {
        type: "doc",
        id: "config/index",
      },
      collapsed: true,
      items: [
        "config/battery",
        "config/behaviors",
        "config/bluetooth",
        "config/combos",
        "config/displays",
        "config/encoders",
<<<<<<< HEAD
=======
        "config/lighting",
        "config/pointing",
>>>>>>> 207afe2ecda1ff53c7ec2af74d2aef61be87b684
        "config/keymap",
        "config/kscan",
        "config/power",
        "config/system",
        "config/studio",
      ],
    },
    {
      Development: [
        {
          type: "category",
          label: "Hardware Integration",
          collapsed: true,
          items: [
            "development/hardware-integration/new-shield",
            "development/hardware-integration/hardware-metadata-files",
            "development/hardware-integration/boards-shields-keymaps",
            "development/hardware-integration/studio-setup",
            "development/hardware-integration/shift-registers",
            "development/hardware-integration/encoders",
<<<<<<< HEAD
=======
            "development/hardware-integration/soft-off-setup",
            "development/hardware-integration/pointing",
            "development/hardware-integration/battery",
            {
              type: "category",
              label: "Lighting",
              link: {
                type: "doc",
                id: "development/hardware-integration/lighting/index",
              },
              collapsed: true,
              items: [
                "development/hardware-integration/lighting/underglow",
                "development/hardware-integration/lighting/backlight",
              ],
            },
>>>>>>> 207afe2ecda1ff53c7ec2af74d2aef61be87b684
          ],
        },
        {
          type: "category",
          label: "Contributing",
          collapsed: true,
          items: [
            "development/contributing/clean-room",
            "development/contributing/documentation",
          ],
        },
        {
          type: "category",
          label: "Local Toolchain",
          collapsed: true,
          items: [
            {
              type: "category",
              label: "Setup",
              link: {
                type: "doc",
                id: "development/local-toolchain/setup/index",
              },
              collapsed: true,
              items: [
                "development/local-toolchain/setup/docker",
                "development/local-toolchain/setup/native",
              ],
            },
            "development/local-toolchain/build-flash",
            "development/local-toolchain/pre-commit",
            "development/local-toolchain/ide-integration",
            "development/local-toolchain/tests",
            "development/local-toolchain/posix-board",
          ],
        },
        "development/module-creation",
        "development/usb-logging",
        "development/studio-rpc-protocol",
        "development/new-behavior",
      ],
    },
  ],
};
