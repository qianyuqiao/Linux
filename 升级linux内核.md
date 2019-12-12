## 升级Ubuntu16.04到18.04
### 1.确保软件版本都是最新的
执行
```
apt-get update
```
和
```
apt-get upgrade
```
这一步非常重要，反正执行这两个命令，直到没有任何报错信息才能进行下一步，如果有报错信息，请更换源或者百度谷歌解决方案。
<br>
### 2.升级Ubuntu版本
do-release-upgrade
<br>
### 3.重启
然后输入 
```
lsb_release -a
```
正常从Ubuntu16.04升级到的Ubuntu18.04，应该显示
<br>
```
LSB Version:	core-9.20170808ubuntu1-noarch:printing-9.20170808ubuntu1-noarch:security-9.20170808ubuntu1-noarch
Distributor ID:	Ubuntu
Description:	Ubuntu 18.04.3 LTS
Release:	18.04
Codename:	bionic
```
然后查看内核版本
<br>输入
```
uname -sr
```
显示
```
Linux 4.15.0-72-generic
```
这个时候内核版本默认是4.15，如果还需要继续升级请参考下面的步骤
## 继续升级Ubuntu18.04的内核
1.下载期望的内核版本
<br>
访问https://kernel.ubuntu.com/~kernel-ppa/mainline/
下载对应版本的内核,具体可参考
https://upload-images.jianshu.io/upload_images/17029102-a2ef7d1bc12d486c.png?imageMogr2/auto-orient/strip|imageView2/2/w/958/format/webp
的第四条

## 难点：升级之后如果想实现切换内核，
有一个参考链接https://www.tiny777.com/190614UbuntuUpdateKernel/
<br>
具体操作可总结如下
### 1.查看grub可选项
```
cat /boot/grub/grub.cfg | grep "menuentry" | grep "Ubuntu"
```
我的显示
```
menuentry 'Ubuntu' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-simple-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
submenu 'Advanced options for Ubuntu' $menuentry_id_option 'gnulinux-advanced-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
	menuentry 'Ubuntu, with Linux 5.3.11-050311-generic' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-5.3.11-050311-generic-advanced-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
	menuentry 'Ubuntu, with Linux 5.3.11-050311-generic (recovery mode)' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-5.3.11-050311-generic-recovery-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
	menuentry 'Ubuntu, with Linux 4.15.0-72-generic' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.15.0-72-generic-advanced-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
	menuentry 'Ubuntu, with Linux 4.15.0-72-generic (recovery mode)' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.15.0-72-generic-recovery-a9ea9ad8-c306-4d42-a094-b361053fbc13' {

```
第四行开始就是可以启动的内核列表
如果想开机切换到Ubuntu, with Linux 4.15.0-72-generic，请看第二步

### 2.修改grub
```
nano /etc/default/grub
```
修改为下面这个样子，记住，一定要把GRUB_TIMEOUT_STYLE这一行这注释掉否则还是会开机的时候还是会隐藏GRUB界面
```
GRUB_DEFAULT=2
#GRUB_TIMEOUT_STYLE=hidden
GRUB_TIMEOUT=10
GRUB_DISTRIBUTOR=`lsb_release -i -s 2> /dev/null || echo Debian`
GRUB_CMDLINE_LINUX_DEFAULT=""
GRUB_CMDLINE_LINUX=""
```
### 3.更新grub选项
```
update-grub
```
### 4.检查grub.cfg
```
cat /boot/grub/grub.cfg | grep "set default"
```
这个时候应该显示
```
set default="${next_entry}"
set default="2"
```
才对
### 5.开机的时候就会出现grub界面了
