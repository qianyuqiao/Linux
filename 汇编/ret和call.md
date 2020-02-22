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
