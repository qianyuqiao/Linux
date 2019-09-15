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
		nread = read(fp, bufp, left);
		if (nread > 0)
		{
			left -= nread;
			butp += nread;		
		}
		else if (nread < 0)
		{
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}
		else (nread == 0)
			break;

	}
	return n - left;
}


ssize_t rio_writen(int fd, void* buf, size_t n)
{
	size_t left = n;
	ssize_t nwrite;
	char *bufp = buf;
	while (left > 0)
	{
		nwrite = write(fd, bufp, left)
		if (nwrite > 0)
		{
			// 由于必须将起始地址之后n个字节的数据写到缓冲区，所以必须等写完了才能返回
			left -= nwrite;
			butp += nwrite;
		}
		else
		{ 
			if (errno == EINTR)
				nwrite = 0;
			else
				return -1;
		}

	}
	return n-left;
}

2.带缓冲区的输入函数
为什么要带缓冲区呢？这是因为可能存在下面这种情况：需要统计一个文本文件的行数，如果单纯的重复调用read系统调用每次
读取一个字节读到换行符统计的话可能会因为上下文的切换造成大量的开销。但是如果采用一个缓冲区，在读取之前先读进来一段
文本到缓冲区中的话可以显著的降低read系统调用的次数从而提高性能。
其中最重要的是一个数据结构和三个函数

数据结构
typedef struct {
    int rio_fd;                //与内部缓冲区关联的描述符
    int rio_cnt;               //缓冲区中剩下的字节数
    char *rio_bufp;          //指向缓冲区中下一个未读的字节
    char rio_buf[RIO_BUFSIZE]; 
} rio_t;

static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
	if (rp->rio_cnt == 0)
	{
		while (rp->rio_cnt <= 0)
		{
			rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, RIO_BUFSIZE)
			if (rp->rio_cnt > 0)
			{
				rp->rio_bufp = rp->rio_buf;			
			}
			else if (rp->rio_cnt < 0)
			{
				if (errno != EINTR) return -1;
			}
			else return 0;
		}
	}// 这里没有考虑的很复杂，没有继续上次的位置而是重新read
	int cnt = n;
	if (rp->rio_cnt < n) cnt = rp->rio_cnt;
	memcpy(usrbuf, rp->rio_bufp, cnt);
	rp->rio_cnt -= cnt;
	rp->rio_bufp += cnt;
	return cnt;
}

ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
	size_t left = n;
	ssize_t readn;
	char* buf = usrbuf;
	while (left > 0)
	{
		readn = rio_read(rp, usrbuf, n);
		if (readn > 0)
		{
			left -= readn;
			buf += readn;
		}
		else if (readn < 0) return -1;
		else (readn == 0) break;
	}
	return n - left;
}

ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) // 在maxlen或者\n处终止
{
	int n;
	int nread;
	char c;
	char* buf = usrbuf;
	for (int n = 1; n < maxlen; n++)
	{
		nread = rio_read(rp->fd, &c, 1);
		if (nread == 1)
		{
			*buf = c;
			buf++;
			if (c == '\n')
			{
				n++;
				break;
			}
		}	
		else if (nread == 0)
		{
			if (n == 1) return 0;
			else break;
		}
		else
			return -1;

	}
	*buf = 0;
	return n - 1;
}
