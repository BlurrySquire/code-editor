#! /bin/bash

if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

mkdir -p temp/TeaCode.AppDir
cp -r package/linux/* temp/TeaCode.AppDir

export NO_STRIP=true
./linuxdeploy-x86_64.AppImage \
    --appdir temp/TeaCode.AppDir \
    --executable ./tea-code \
    --desktop-file package/linux/tea-code.desktop \
    --icon-file branding/icon.png \
    --output appimage

rm -rf temp