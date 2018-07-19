#!/usr/bin/sh

number="13810855631"
desc="[ad search system] system down! restarting ..."

title="ad_search_DOWN"
mail=notice_mail
local=`pwd`
line=`ps aux | grep exe_AdSearch | grep -v grep | wc -l`
if [ $line -eq 0 ]
then
	cp ad.err $mail
	sh restart.sh

	cd /exports/disk2/wushuang/MailEnv
	php send_system_monitor.php  $title $local/$mail

	url="http://op-portal/portal/mobile/smsproxy.php?number=$number&desc=$desc"
	wget "$url" -O /dev/null
fi
