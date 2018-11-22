git clean . -xdf &&
cmake -DCMAKE_BUILD_TYPE=Release . &&
make -j3 &&
mkdir keyboxd.app/Contents/libs &&
local_libs=`otool -L keyboxd.app/Contents/MacOS/keyboxd | grep /usr/local | awk '{print $1}'` &&
for lib in $local_libs ; do 
   cp $lib keyboxd.app/Contents/libs
   install_name_tool -change $lib @executable_path/../libs/`basename $lib` keyboxd.app/Contents/MacOS/keyboxd
done &&
mkdir -p osx-root/usr/local &&
mv keyboxd.app osx-root/usr/local/keyboxd &&
mkdir -p osx-root/Library/LaunchAgents &&
cp mac/org.keyboxwallet.daemon.plist osx-root/Library/LaunchAgents &&
pkgbuild  --root osx-root/ --identifier org.keyboxwallet.daemon --scripts mac/scripts --component-plist mac/components.plist --version 0.3.0 out.pkg &&
productbuild --package-path out.pkg --distribution mac/distribution.xml  keyboxd.pkg &&
echo 'all done' ||
echo 'something wrong happend.'
