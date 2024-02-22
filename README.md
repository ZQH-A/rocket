# rocket

这个项目是 bilibili 上的一个rpc项目，具体链接：https://www.bilibili.com/video/BV1cg4y1j7Wr/?spm_id_from=333.337.search-card.all.click&vd_source=69a8e7a7afa009d6fa9ddea37424446d。


### 1. 日志模块开发

日志模块：

'''
1.日志级别 （有几种输出的类型：比如 DEBUG、ERROR、INFO） 
2.打印到文件，支持日期命名，以及日志滚动
3.c 格式化风格
4.线程安全 （多线程所以需要考虑线程安全）
'''
大概有以下几个类：

LogLevel:  //日志级别
'''
Debug
Info
Error
'''

LogEvent: //日志事件，用于打印日志到文件
'''
//日志中需要打印的东西
文件名、行号
MsgNo //消息编号
进程号
线程号
日期、以及时间，精确到ms
自定义消息
'''


日志格式
'''
 //日志级别 时间                     进程号/线程号     文件名/行号    自定义消息
[Level][%y-%m-%d %H:%M:%S.%ms]\t[pid:thread_id]\t[fine_name:line][%msg]
'''

Logger 日志器
'''
1.提供打印日志的方法
2.设置日志输出的路径
'''