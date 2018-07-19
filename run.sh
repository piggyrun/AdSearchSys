#!/usr/bin/sh

export AD_CODE_ROOT=/search/wushuang/BASE/adquery
export LD_LIBRARY_PATH=/lib:/usr/local/lib:$AD_CODE_ROOT/Library

ulimit -c unlimited

number="13810855631"

day=1
DATE_IE=`date -d"$day days ago" "+%F"`
DATE_DIR=`date -d"$day days ago" +"20%y%m%d"`
#DATE_PRE=`date -d"2 days ago" +"20%y%m%d"`
DATE_PRE=`date -d"$day days ago" +"%Y%m%d"`
DATE_pre=`date -d"$day days ago" +"%y%m%d"`
echo $DATE_IE
((day--))
DATE_AD=`date -d"$day days ago" +"%Y%m%d"`
echo $DATE_AD

# 省出内存
killall exe_AdSearch

mkdir log/$DATE_DIR

# 取cheatpid
#cp /dfs_sq/whitepage/cheatpid4marui/cheatpid_$DATE_DIR.txt ./cheatpid
rsync 10.12.8.185::whitepage/cheatpid4marui/cheatpid_$DATE_DIR.txt ./cheatpid

echo "check data avalibility ..."
date
# 验证数据是否存在
title="ad_search_index_WARNING"
mail=notice_mail
local=`pwd`
rsync 10.10.98.179::root/search/wushuang/process_log/log/$DATE_DIR/log_finished 2>err
err_sz=`wc -l err | awk '{print $1}'`
cnt=0
while [ $err_sz -ne 0 ]; do
	sleep 300 
	((cnt++))
	if [ $cnt -eq 10 ]
	then
		echo "pv_log later" > $mail
		cd /search/wushuang/MailEnv
		php send_system_monitor.php  $title $local/$mail
		cd $local
	fi  
	rsync 10.10.98.179::root/search/wushuang/process_log/log/$DATE_DIR/log_finished 2>err
	err_sz=`wc -l err | awk '{print $1}'`
done
rm -f err

echo "retrieving pv & click log ..."
date
rsync 10.10.98.179::root/search/wushuang/process_log/log/$DATE_DIR/pv_log ./log/$DATE_DIR
rsync 10.10.98.179::root/search/wushuang/process_log/log/$DATE_DIR/click_log ./log/$DATE_DIR

title="ad_search_index_ERROR"
echo "retrieving valid_keyad ..."
date
#valid_keyad
rsync --progress 10.10.64.61::root/DATA/wushuang/data_flow_left_regression/valid_keyad/history/valid_keyad.$DATE_AD 2>err
err_sz=`wc -l err | awk '{print $1}'`
if [ $err_sz -ne 0 ]
then
	echo "valid keyad failed" > $mail
	cd /search/wushuang/MailEnv
	php send_system_monitor.php  $title $local/$mail
	exit -1
fi
rm -f err
rsync --progress 10.10.64.61::root/DATA/wushuang/data_flow_left_regression/valid_keyad/history/valid_keyad.$DATE_AD ./log/$DATE_DIR/valid_keyad0
rsync --progress 10.10.64.61::root/DATA/wushuang/data_flow_left_regression/valid_keyad/history/valid_keyad.$DATE_PRE ./log/$DATE_DIR/valid_keyad1
#rsync --progress 10.10.64.61::root/DATA/analyzer/context/$DATE_AD/valid_keyad ./log/$DATE_DIR/valid_keyad0
#rsync --progress 10.10.64.61::root/DATA/analyzer/context/$DATE_PRE/valid_keyad ./log/$DATE_DIR/valid_keyad1
cat log/$DATE_DIR/valid_keyad1 log/$DATE_DIR/valid_keyad0  > log/$DATE_DIR/valid_keyad
#cat log/$DATE_PRE/valid_keyad0 log/$DATE_DIR/valid_keyad0  > log/$DATE_DIR/valid_keyad
#cp ./log/$DATE_DIR/valid_keyad0 ./log/$DATE_DIR/valid_keyad

echo "retrieving test acc ..."
#test_acc_file
#rsync --progress 10.10.96.53::root/search/BiddingQueryInstant/data/test_acc.txt ./log/$DATE_DIR/
#rsync --progress 10.11.89.132::root/search/task/adindex_flow/daily_data/test_acc.txt ./log/$DATE_DIR/
rsync --progress 10.10.66.144::root/search/odin/sql_operator/data/free_account ./log/$DATE_DIR/test_acc.txt

echo "generating 9-dim ad list ..."
date
#perl AdList.pl $DATE_DIR
#./exe_AdList $DATE_DIR
./exe_stat_adlist $DATE_DIR
if [ $? -ne 0 ]
then
	desc="[AdSearch system] adlist failed, please take a look"
	url="http://op-portal/portal/mobile/smsproxy.php?number=$number&desc=$desc"
	wget "$url" -O /dev/null
	echo $desc > $mail
	cd /search/wushuang/MailEnv
	php send_system_monitor.php  $title $local/$mail
	exit -1
fi

######## 生成adm需要数据
awk -F"\t" '{pv[$5"\t"$8"\t"$12]+=$1; clk[$5"\t"$8"\t"$12]+=$2;}END{for(a in pv) print a"\t"pv[a]"\t"clk[a];}' ./log/$DATE_DIR/adlist > ./log/$DATE_DIR/cpc_pv_click_$DATE_IE
rsync -az ./log/$DATE_DIR/cpc_pv_click_$DATE_IE 192.168.131.152::cpcvp_for_adr/
###############################

echo "building invert list ..."
date
./exe_genIndex log/$DATE_DIR/adlist log/$DATE_DIR/test_acc.txt $DATE_DIR
if [ $? -ne 0 ]
then
	desc="[AdSearch system] genIndex failed, please take a look"
	url="http://op-portal/portal/mobile/smsproxy.php?number=$number&desc=$desc"
	wget "$url" -O /dev/null
	title="ad_search_index_ERROR"
	echo $desc > $mail
	cd /search/wushuang/MailEnv
	php send_system_monitor.php  $title $local/$mail
	exit -1
fi

echo "delete unused files ..."
rm -f ./log/$DATE_DIR/valid_keyad0
rm -f ./log/$DATE_DIR/valid_keyad1
rm -f ./log/$DATE_DIR/valid_keyad
rm -f ./log/$DATE_DIR/pv_log
rm -f ./log/$DATE_DIR/click_log

# 生成完成确认文件
touch ./log/$DATE_DIR/data_finished

echo "finish time is"
date

# 重启系统
sh restart.sh

# 发送成功邮件
title="ad_search_index_COMPLETE"
echo "index data of ad search system complete successfully! and system restarting!" > $mail
cd /search/wushuang/MailEnv
php send_system_monitor.php  $title $local/$mail
