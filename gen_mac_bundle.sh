git clean . -xdf &&
cmake -DCMAKE_BUILD_TYPE=Release . &&
make -j3 &&
macdeployqt keyboxd.app &&
cd qt &&
cmake -DCMAKE_BUILD_TYPE=Release . &&
make -j3 &&
macdeployqt keyboxd_ui.app &&
cp i18n/*.qm keyboxd_ui.app/Contents/Resources/ &&
cp ../keyboxd.app/Contents/MacOS/keyboxd keyboxd_ui.app/Contents/MacOS &&
cp ../keyboxd.app/Contents/Frameworks/*.dylib keyboxd_ui.app/Contents/Frameworks/ &&
codesign --deep --force --verbose --sign 'Developer ID App' keyboxd_ui.app/ &&
mkdir -p osx-root/Applications &&
mv keyboxd_ui.app osx-root/Applications &&
mkdir -p osx-root/Library/LaunchAgents &&
cp org.keyboxwallet.daemon.plist osx-root/Library/LaunchAgents &&
pkgbuild  --root osx-root/ --identifier org.keyboxwallet.daemon --scripts scripts --component-plist components.plist --version 0.3.0 out.pkg &&
productbuild --package-path out.pkg --distribution distribution.xml  keyboxd_ui.pkg &&
echo 'all done' ||
echo 'something wrong happend.'
