## 原理：
  * 死锁有两种：线程间，线程内。

      * 线程间死锁：1持有A获取B，2持有B获取A。
        	如果AB有序，规定必须从小向大获取。那假设A<B，那么2就必须先解锁B才能获取A之后再获取B。此时触发解锁操作，那就是死锁，上面的宏定义会在此时报错。

      * 线程内死锁：1持有A，1调用2,2获取A。
        				因为1和2在同一个线程，所以1持有A就是2持有A，所以此时的加锁请求会忽略。

      * 特殊情况：如果1中开线程执行2，并且1已经持有A，且2获取A，1在之后等待2的执行结束。此时还会死锁。但这样的代码我不知道开线程干什么？可能为了节省中间那段指令的时间，但又要求某处必须结束。这种情况的死锁，需要系统或语言层面共同支持。		

          * 方案一：所有线程都分离，就不会有上面的情况。		
          * 方案二：可以包装std::thread，在调用join向子线程授权所持有的全部锁（或者释放所有锁，但执行完之后又全部加回来）。
            			如果创建一个线程没有join或者detach那这个创建的线程本身就不安全。这种情况不考虑。c++标准库的thread类的析构函数会自己detach。

        关于方案二的证明：

        ​		方案二解决的是：父线程等待子线程的结束，同时父线程和子线程出现死锁，则等待前父线程放弃所有锁，这样子线程一定能结束，之后父线程重新请求自己的锁，就恢复到之前的状态了。

        ​	扩展有树形的线程关系（有根线程和叶线程，中间的节点线程），已知：线程间死锁不存在，不存在线程内死锁。故根节点一定可以执行到等待处（例1），故根节点的子节点（左右不确定）一定可以执行到等待处（例2）。故一个父子结构一定都可以执行到等待处，其中叶子节点一定可以结束。

        | 1                 |      |
        | ----------------- | ---- |
        | A                 |      |
        | B                 |      |
        | create thread 2； | 2    |
        |                   | A    |
        |                   | B    |
        | 2.join（）        |      |
        | ……                |      |

        ​																	例1

        这种情况因为1持有AB，2请求AB，所以2阻塞；当join时1释放AB，2就能结束，join顺利结束。

        | 线程 | 1    | 2    |
        | ---- | ---- | ---- |
        |      | A    |      |
        |      | B    |      |
        |      |      | C    |
        |      |      | D    |
        |      |      | A    |
        |      | C    |      |
        |      | D    |      |

        ​																		例2

        这种情况1一定先于2执行完成，因为2请求A时导致CD释放，同时因为1持有A，所以2会阻塞，且1可以请求到CD。
      
      ## RW_Lock
      
      ​	读写锁，不多说。
      
      

### IdentityNumber

全局唯一编号产生，无锁，线程安全。

### lock_base

锁的基类

### S_Lock

互斥锁

std::mutex不支持跨线程操作（一个线程加锁，一个线程解锁）

### Lock_Memger

线程上的所管理器

### Lock_Guard

栈上的锁管理，仿std::lock_guard;

### Thread

继承std::thread。主要是修改join的实现。



# 用法

Thread替换std::thread

Lock_Guard 替换 std::lock_guard

S_Lock 替换 std::mutex

当然以上组件完全和c++兼容。

仅限全套使用时解决死锁问题，否则只能解决部分死锁。

SWD_LOCK_TEST ：是否遇到死锁（可能或者已经触发）时中断。
_WAIT_TIME：睡眠多久。
