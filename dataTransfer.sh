#!/usr/bin/sh

export AD_CODE_ROOT=/exports/disk1/lihuabei/adquery/adquery_1.2.7_20090316
export LD_LIBRARY_PATH=/lib:/usr/local/lib:$AD_CODE_ROOT/Library

DATE_DIR=`date -d"1 days ago" +"20%y%m%d"`

#mkdir log/$DATE_DIR

local=`pwd`
title="ad_search_WARNING"
mail=notice_mail
cnt=0
echo "retrieving index and data ..."
rsync --progress 10.10.98.179::root/search/wushuang/AdSearchSys/log/$DATE_DIR/data_finished 2>err
err_sz=`wc -l err | awk '{print $1}'`
while [ $err_sz -ne 0 ]; do
	sleep 300;
	((cnt++))
	if [ $cnt -eq 10 ]
	then
		echo "index data later" > $mail
		cd /exports/disk2/wushuang/MailEnv
		php send_system_monitor.php  $title $local/$mail
		cd $local
	fi  
	echo "data not finished, try again later ....."
	date
	rsync --progress 10.10.98.179::root/search/wushuang/AdSearchSys/log/$DATE_DIR/data_finished 2>err
	err_sz=`wc -l err | awk '{print $1}'`
done
rm -f err

# 按奇偶来存放数据，奇数在disk2，主目录下；偶数在disk1，在主目录下用软连访问
((MK=$DATE_DIR%2))
echo $MK
if [ $MK -eq 1 ]; then
	mkdir log/$DATE_DIR
else
	mkdir /exports/disk1/wushuang/AdSearchSys/log/$DATE_DIR
	ln -s /exports/disk1/wushuang/AdSearchSys/log/$DATE_DIR ./log
fi
rsync --progress 10.10.98.179::root/search/wushuang/AdSearchSys/log/$DATE_DIR/*.index ./log/$DATE_DIR
rsync --progress 10.10.98.179::root/search/wushuang/AdSearchSys/log/$DATE_DIR/*.dat ./log/$DATE_DIR

echo "finish time is"
date

echo "restart the ad search system ..."
killall exe_AdSearch
./exe_AdSearch $DATE_DIR 1>adlog 2>aderr &

# 发送成功邮件
title="ad_search_COMPLETE"
mail=notice_mail
echo "data transfer process of ad search system complete successfully! and system restarts" > $mail
cd /exports/disk2/wushuang/MailEnv
php send_system_monitor.php  $title $local/$mail
