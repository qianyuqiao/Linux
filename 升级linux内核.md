https://www.tiny777.com/190614UbuntuUpdateKernel/

难点：
## 1.升级之后切换内核
cat /boot/grub/grub.cfg | grep "menuentry" | grep "Ubuntu"
<br>
<p>
menuentry 'Ubuntu' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-simple-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
submenu 'Advanced options for Ubuntu' $menuentry_id_option 'gnulinux-advanced-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
	menuentry 'Ubuntu, with Linux 5.3.11-050311-generic' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-5.3.11-050311-generic-advanced-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
	menuentry 'Ubuntu, with Linux 5.3.11-050311-generic (recovery mode)' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-5.3.11-050311-generic-recovery-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
	menuentry 'Ubuntu, with Linux 4.15.0-72-generic' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.15.0-72-generic-advanced-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
	menuentry 'Ubuntu, with Linux 4.15.0-72-generic (recovery mode)' --class ubuntu --class gnu-linux --class gnu --class os $menuentry_id_option 'gnulinux-4.15.0-72-generic-recovery-a9ea9ad8-c306-4d42-a094-b361053fbc13' {
</p>
第四行开始就是可以启动的内核列表
如果想开机切换到Ubuntu, with Linux 4.15.0-72-generic，请看第二步

## 2.修改grub
nano /etc/default/grub
修改为下面这个样子
`
GRUB_DEFAULT=2
#GRUB_TIMEOUT_STYLE=hidden
GRUB_TIMEOUT=10
GRUB_DISTRIBUTOR=`lsb_release -i -s 2> /dev/null || echo Debian`
GRUB_CMDLINE_LINUX_DEFAULT=""
GRUB_CMDLINE_LINUX=""
`
## 3.别忘了
update-grub

## 4.检查grub.cfg
cat /boot/grub/grub.cfg | grep "set default"
这个时候应该是
`
   set default="${next_entry}"
   set default="2"
`
才对
## 5.开机的时候就会出现grub界面了
