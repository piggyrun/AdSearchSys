#!/usr/bin/sh

export AD_CODE_ROOT=/search/wushuang/BASE/adquery
export LD_LIBRARY_PATH=/lib:/usr/local/lib:$AD_CODE_ROOT/Library

ulimit -c unlimited

DATE_DIR=`date -d"1 days ago" +"20%y%m%d"`

echo "restart the ad search system ..."
killall exe_AdSearch

echo "restart time is"
date

./exe_AdSearch $DATE_DIR 1>ad.log 2>ad.err &
