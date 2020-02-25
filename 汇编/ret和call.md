### 1.ret和retf
ret指令的作用相当于：
```
pop IP
```
retf指令的作用相当于：
```
pop IP
pop CS
```
### call命令
call所进行的操作有：
```
1.将当前的IP或CS+IP压入栈中；
2.转移；
```
以下面的代码为例子：
```
int add(int x, int y)
{

}

int main()
{
    ...
    int y = add(1, 2);
    return 0;
}
```
原文链接：

对于call add指令, 做了下面几件事：
```
1.push ip
2.跳转
3.
push ebp
mov ebp esp
....
4.返回时
mov ebp esp
pop ebp
pop ip
```
