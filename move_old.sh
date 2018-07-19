#!/usr/bin/sh

DATE_DIR=`date -d "31 days ago" +%Y%m%d`

mkdir history/$DATE_DIR

mv log/$DATE_DIR/adlist history/$DATE_DIR
mv log/$DATE_DIR/test_acc.txt history/$DATE_DIR
rm -rf log/$DATE_DIR
gzip history/$DATE_DIR/*

mv history/$DATE_DIR log
