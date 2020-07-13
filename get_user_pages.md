```
int get_user_pages(struct task_struct* tsk, struct mm_struct *mm, 
unsigned long start, int len, int write, int force, struct page** pages, struct vm_area_struct** vmas)
```
其中
```
tsk ：指定进程，如current表示当前进程

mm ： 进程的内存占用结构，如current->mm，

start ：要获取其页面的起始逻辑地址（也叫线性地址？），它是用户空间使用的一个地址

len ：要获取的页数

write ：是否要对该页进行写入 /* 我不知道如果是写会做什么特别的处理 */

force ：/* 不知道有什么特殊的动作 */

pages ：存放获取的struct page的指针数组

vms ： 返回各个页对应的struct vm_area_struct，可以传入NULL表示不获取，
struct vm_area_struct应该是用于组成用户区进程内存的堆的基本元素？
```
