git clean . -xdf &&
cmake -DCMAKE_BUILD_TYPE=Release . &&
make -j3 &&
macdeployqt keyboxd.app &&
codesign --deep --force --verbose --sign 'Developer ID App' keyboxd.app/ &&
mkdir -p osx-root/Applications &&
mv keyboxd.app osx-root/Applications &&
mkdir -p osx-root/Library/LaunchAgents &&
cp mac/org.keyboxwallet.daemon.plist osx-root/Library/LaunchAgents &&
pkgbuild  --root osx-root/ --identifier org.keyboxwallet.daemon --scripts mac/scripts --component-plist mac/components.plist --version 0.3.0 out.pkg &&
productbuild --package-path out.pkg --distribution mac/distribution.xml  keyboxd.pkg &&
echo 'all done' ||
echo 'something wrong happend.'
