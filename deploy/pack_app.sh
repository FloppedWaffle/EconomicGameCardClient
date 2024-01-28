#!/bin/bash

chmod +x linuxdeployqt-continuous-x86_64.AppImage
./linuxdeployqt-continuous-x86_64.AppImage AppDir/economic.desktop -bundle-non-qt-libs -always-overwrite

chmod +x appimagetool-x86_64.AppImage
./appimagetool-x86_64.AppImage AppDir

rm -rf AppDir/translations
rm -rf AppDir/doc
rm -rf AppDir/lib
rm -rf AppDir/plugins
rm -f AppDir/AppRun
rm -f AppDir/qt.conf
