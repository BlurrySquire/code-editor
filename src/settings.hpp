#ifndef HPP_SETTINGS
#define HPP_SETTINGS

#include <toml++/toml.hpp>

namespace settings {
    void load();

    void load_default();

    toml::table& get();
}

#endif