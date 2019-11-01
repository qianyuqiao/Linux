struct vm_area_struct {
	/* The first cache line has the info for VMA tree walking. 
	第一个缓存行具有VMA树移动的信息*/
 
	unsigned long vm_start;		/* Our start address within vm_mm. */
	unsigned long vm_end;		/* The first byte after our end address within vm_mm. */
 
	/* linked list of VM areas per task, sorted by address
	每个任务的VM区域的链表，按地址排序*/
	struct vm_area_struct *vm_next, *vm_prev;
 
	struct rb_node vm_rb;
 
	/*
	 此VMA左侧最大的可用内存间隙（以字节为单位）。 
	 在此VMA和vma-> vm_prev之间，
	 或者在VMA rbtree中我们下面的一个VMA与其->vm_prev之间。 
	 这有助于get_unmapped_area找到合适大小的空闲区域。
	 */
	unsigned long rb_subtree_gap;
 
	/* Second cache line starts here. 
	第二个缓存行从这里开始*/
 
	struct mm_struct *vm_mm;	/* 我们所属的address space*/
	pgprot_t vm_page_prot;		/* 此VMA的访问权限 */
	unsigned long vm_flags;		/* Flags, see mm.h. */
 
	/*
	 对于具有地址空间（address apace）和后备存储(backing store)的区域，
	 链接到address_space->i_mmap间隔树，或者链接到address_space-> i_mmap_nonlinear列表中的vma。
	 */
	union {
		struct {
			struct rb_node rb;
			unsigned long rb_subtree_last;
		} linear;
		struct list_head nonlinear;
	} shared;
 
	/*
	 在其中一个文件页面的COW之后，文件的MAP_PRIVATE vma可以在i_mmap树和anon_vma列表中。
	 MAP_SHARED vma只能位于i_mmap树中。 
	 匿名MAP_PRIVATE，堆栈或brk vma（带有NULL文件）只能位于anon_vma列表中。
	 */
	struct list_head anon_vma_chain; /* Serialized by mmap_sem & * page_table_lock
										由mmap_sem和* page_table_lock序列化*/
	struct anon_vma *anon_vma;	/* Serialized by page_table_lock 由page_table_lock序列化*/
 
	/* 用于处理此结构体的函数指针 */
	const struct vm_operations_struct *vm_ops;
 
	/* 后备存储（backing store）的信息: */
	unsigned long vm_pgoff;		
    /* 假如该vm_area_struct描述的是一个文件映射的虚存空间，
    vm_file便指向被映射的文件的file结构，vm_pgoff是该虚存空间起始地址在vm_file文件里面的文件便宜，单位为物理页面*/
	struct file * vm_file;		/* 我们映射到文件（可以为NULL）*/
	void * vm_private_data;		/* 是vm_pte（共享内存） */
 
#ifndef CONFIG_MMU
	struct vm_region *vm_region;	/* NOMMU映射区域 */
#endif
#ifdef CONFIG_NUMA
	struct mempolicy *vm_policy;	/* 针对VMA的NUMA政策 */
#endif
