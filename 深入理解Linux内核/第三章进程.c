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
