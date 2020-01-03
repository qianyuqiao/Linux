参考链接:http://blog.chinaunix.net/uid-27863080-id-3442374.html
1.数据包的目的地址是本机，数据包发往Input链并交给本地进程处理
2.目的地址不是本机，将会被转发并通过Forward链
3.本地进程产生的数据包将送往Output链
