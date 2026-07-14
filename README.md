# code-editor
A hopefully really simple code editor made in C++ with wxWidgets. Currently the file view on the left side is for decoration only.

## Deps
- toml++
- wxWidgetsGTK3 (On linux. This probably works on windows with the windows version of the library.)

## Building
You'll need to install the deps too. On arch the packages are ``wxwidgets-gtk3-git``(AUR) and then ``tomlplusplus``.
```
make all -j$(nproc)
```