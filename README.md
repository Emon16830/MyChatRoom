# MyChatRoom
 采用C/S架构，基于TCP的网络聊天室


Update on May 2th :
  由于之前是跟着别人的写的，用的都是库函数，不是很了解底层的实现原理和知识，所以重新打算用类重新写一遍，以加深理解。
  采用pipe（可改进为双工的sockpair）使用，父进程读，子进程写实现了客户端的基本功能。
