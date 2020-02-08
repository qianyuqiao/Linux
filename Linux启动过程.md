### 关于BOOTSEG和SYSSEG
```
BOOTSEG     = 0x07C0        /* original address of boot-sector */
SYSSEG      = 0x1000        /* historical load address >> 4 */
```
这两个都是实模式下（20位地址，1MB）的段基址，<br>
磁盘第一个分区（也叫做MBR）的代码将被加载到0x7C00这个物理地址
其他的内核setup代码加载到0x10000物理地址
