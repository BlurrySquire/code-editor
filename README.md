# Tea-Code
A hopefully simple code editor made in C++ with wxWidgets. The name is inspired by my enjoyment of tea.

## Deps
- toml++
- wxWidgets (wxWidgetsGTK3 on linux)

## Building
```
make all -j$(nproc)
```
If you want to make an installable package then you need to run the respective script in ``scripts/``.