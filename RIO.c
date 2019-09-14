本文是CSAPP 10.5节的学习笔记
主要讲解使用健壮I/O包(RIO)健壮的读写
1. 无缓冲的输入输出函数
为什么这里要加一个无缓冲呢？这是因为如果顺利read()会返回实际读到的字节数，
但是有时候如果你讲返回值与参数n作比较，会发现返回的字节数有时候比要求读取的字节数少，
则有一下几种可能：
读到了文件尾
从管道(pipe)或终端机读取
或者是read()被信号中断了读取动作。
当有错误发生时则返回-1，错误代码存入errno中，而文件读写位置则无法预期。  
rio_readn()与rio_writen()允许读写过程被应用的信号中断造成的实际读取的文件字节数比size_t n小

ssize_t rio_readn(int fd, void* buf, size_t n)
{
	size_t left = n;
	ssize_t nread;
	char *bufp = buf;
	while (left > 0)
	{
		if ((nread = read(fd, bufp, left)) < 0)
		{
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else if (nread == 0)
			break;
		left -= nread;
		butp += nread;
	}
	return n-left;
}


ssize_t rio_writen(int fd, void* buf, size_t n)
{
	size_t left = n;
	ssize_t nwrite;
	char *bufp = buf;
	while (left > 0)
	{
		if ((nwrite = write(fd, bufp, left)) <= 0)
		{ 
			if (errno == EINTR)
				nwrite = 0;
			else
				return -1;
		}
		// 由于必须将起始地址之后n个字节的数据写到缓冲区，所以必须等写完了才能返回
		left -= nwrite;
		butp += nwrite;
	}
	return n-left;
}
