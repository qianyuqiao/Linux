1.real_parent 和 parent的区别与联系
static task_t *copy_process(unsigned long clone_flags,
                            unsigned long stack_start,
                            struct pt_regs *regs,
                            unsigned long stack_size,
                            int __user *parent_tidptr,
                            int __user *child_tidptr,
                            int pid)
{
    // ...

    /* CLONE_PARENT re-uses the old parent */
    if (clone_flags & (CLONE_PARENT|CLONE_THREAD))
        p->real_parent = current->real_parent;
    else
        p->real_parent = current;// real_parent本来是指创建这个子进程的父进程
    p->parent = p->real_parent; // parent默认也是real_parent
    // parent now is the same as real_parent

    // ...

    // if the PT_PTRACED bit is set
    if (p->ptrace & PT_PTRACED)
        __ptrace_link(p, current->parent);

    // ...
}


/*
 * ptrace a task: make the debugger its new parent and
 * move it to the ptrace list.
 *
 * Must be called with the tasklist lock write-held.
 */
void __ptrace_link(task_t *child, task_t *new_parent)
{
    if (!list_empty(&child->ptrace_list))
        BUG();
    if (child->parent == new_parent)
        return;
    // added to parent’s trace list
    // note: child->parent == child->real_parent by default
    list_add(&child->ptrace_list,
             &child->parent->ptrace_children);// ??????????????
    REMOVE_LINKS(child);

    // child->parent now points to the debugger
    child->parent = new_parent;
    SET_LINKS(child);
}


2.pid_hash表及链表

struct hlist_head {
  struct hlist_node *first;
};

struct hlist_node {
  struct hlist_node *next, **pprev;
};

struct upid {
	/* Try to keep pid_chain in the same cacheline as nr for find_vpid */
	int nr;
	struct pid_namespace *ns;
	struct hlist_node pid_chain;
};
 
struct pid
{
	atomic_t count;
	unsigned int level;
	/* lists of tasks that use this pid */
	struct hlist_head tasks[PIDTYPE_MAX];
	struct rcu_head rcu;
	struct upid numbers[1];
};
 
struct pid_link
{
	struct hlist_node node;
	struct pid *pid;
};

struct task_struct *find_task_by_pid_ns(pid_t nr, struct pid_namespace *ns)
{
	rcu_lockdep_assert(rcu_read_lock_held(),
			   "find_task_by_pid_ns() needs rcu_read_lock()"
			   " protection");
	return pid_task(find_pid_ns(nr, ns), PIDTYPE_PID);
}


#define pid_hashfn(nr, ns)	\
	hash_long((unsigned long)nr + (unsigned long)ns, pidhash_shift)
static struct hlist_head *pid_hash;
static unsigned int pidhash_shift = 4;
struct pid *find_pid_ns(int nr, struct pid_namespace *ns)
{
	struct upid *pnr;
 
	hlist_for_each_entry_rcu(pnr, &pid_hash[pid_hashfn(nr, ns)], pid_chain) // 根据pid_hash表中的链表找到符合要求的pnr
		if (pnr->nr == nr && pnr->ns == ns)
			return container_of(pnr, struct pid,
					numbers[ns->level]);
 
	return NULL;
}


struct task_struct *pid_task(struct pid *pid, enum pid_type type)
{
	struct task_struct *result = NULL;
	if (pid) {
		struct hlist_node *first;
		first = rcu_dereference_check(hlist_first_rcu(&pid->tasks[type]),
					      lockdep_tasklist_lock_is_held());
		if (first)         //pid中的task[type] 与task_struct.pid[type].node  指向的是同一个节点
			result = hlist_entry(first, struct task_struct, pids[(type)].node);   //node实体在task_struct结构中，所以可以利用first指针得到task_struct结构体指针
	}
	return result;
}

#define hlist_first_rcu(head)	(*((struct hlist_node __rcu **)(&(head)->first)))
其中,&pid->tasks[PIDTYPE_PID]实际上是&(pid->tasks[PIDTYPE_PID]),即 hlist_head* head;
然后，#define hlist_entry(ptr, type, member) container_of(ptr,type,member)即通过task_struct中的hlist_node结构获得task_struct
