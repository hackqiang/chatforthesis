#!/bin/bash
#这是打包整个工程目录的脚本。
cli_path=/home/qiang/thesis/client
srv_path=/home/qiang/thesis/server
cd $cli_path
make clean
cd $srv_path
make clean
cd /home/qiang
tar -zcvf ~/thesis_backup/chatforthesis.`date +%Y.%m.%d`.tar.gz thesis/
echo 备份文件 chatforthesis.`date +%Y.%m.%d`.tar.gz 已在~/thesis_backup目录下创建
exit
