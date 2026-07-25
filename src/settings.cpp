#include "settings.hpp"

#include <string_view>

static toml::table config;

extern std::string_view default_settings;

namespace settings {
    void load() {
        try {
            config = toml::parse_file("settings.toml");
        }
        catch (const toml::parse_error&) {
            load_default();
            return;
        }

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
family = "Menlo"
size = 12

[editor.indent]
use_tabs = false
width = 4
show_guides = true

[files]
show_hidden = false
)";
