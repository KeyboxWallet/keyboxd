#!/bin/sh
cd "$(dirname $BASH_SOURCE)"
content=`launchctl list org.keyboxwallet.daemon`
result=$?
content=`echo $content | grep '"PID"'`
if  [ $result -ne 0 ]; then
   echo "没有这个服务,请重新安装"
   exit 1
fi
if  [  "$content" != "" ] ; then
   echo "keybox 守护程序安装OK,成功运行,您可以打开钱包程序了"
else
   echo "keybox 守护程序安装OK，但没有启动，现在启动"
   launchctl start org.keyboxwallet.daemon
fi

