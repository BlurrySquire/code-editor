#include "settings.hpp"

#include <string_view>

static toml::table config;

extern std::string_view default_settings;

namespace settings {
    void load() {
        config = toml::parse_file("settings.toml");
        if (config.empty()) {
            load_default();
        }
    }

    void load_default() {
        config = toml::parse(default_settings);
    }

    toml::table& get() {
        return config;
    }
}

std::string_view default_settings = R"(
[editor]
word_wrap = true

[editor.font]
family = "Consolas"
size = 15

[editor.indent]
use_tabs = false
width = 4
show_guides = true

[files]
show_hidden = false
)";