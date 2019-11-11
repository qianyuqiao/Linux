#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     prefetch(pos->member.next), &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))


#define list_entry(ptr_of_member, type, member) \
	container_of(ptr_of_member, type, member)


#define container_of(ptr, type, member) ({              \         
const typeof( ((type *)0)->member ) *__mptr = (ptr);    \         
(type *)( (char *)__mptr - offsetof(type,member) );})
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
注:
0.pos是要查找的数据结构,head是类型为member的的链表节点
1.container_of的作用是根据结构体中某个指针的地址获取该结构体的地址
2.prefetch的含义是告诉cpu那些元素有可能马上就要用到，告诉cpu预取一下，这样可以提高速度，用于预取以提高遍历速度
3.对于pos = list_entry(pos->member.next, typeof(*pos), member)),首先pos->member.next的结合顺序是从左到右,将head移到下一个再取结构体
