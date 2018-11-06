git clean . -xdf &&
cmake -DCMAKE_BUILD_TYPE=Release . &&
make -j3 &&
macdeployqt keyboxd.app &&
cd qt &&
cmake -DCMAKE_BUILD_TYPE=Relase . &&
make -j3 &&
macdeployqt keyboxd_ui.app &&
cp ../keyboxd.app/Contents/MacOS/keyboxd keyboxd_ui.app/Contents/MacOS &&
cp ../keyboxd.app/Contents/Frameworks/*.dylib keyboxd_ui.app/Contents/Frameworks/ &&
mkdir dmg &&
mv keyboxd_ui.app dmg &&
ln -s /Applications dmg/ &&
hdiutil create -srcfolder dmg keyboxd_ui.dmg &&
echo 'all done' ||
echo 'something wrong happend.'
