git clean . -xdf &&
cmake -DCMAKE_BUILD_TYPE=Release . &&
make -j3 &&
macdeployqt keyboxd.app &&
cd qt &&
cmake -DCMAKE_BUILD_TYPE=Release . &&
make -j3 &&
macdeployqt keyboxd_ui.app &&
cp ../keyboxd.app/Contents/MacOS/keyboxd keyboxd_ui.app/Contents/MacOS &&
cp ../keyboxd.app/Contents/Frameworks/*.dylib keyboxd_ui.app/Contents/Frameworks/ &&
codesign --deep --force --verbose --sign 'Developer ID App' keyboxd_ui.app/ &&
appdmg dmg_spec.json keyboxd_ui.dmg &&
echo 'all done' ||
echo 'something wrong happend.'
