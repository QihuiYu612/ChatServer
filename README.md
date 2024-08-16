
基于muduo库的C++集群聊天服务器

24.8.6
更新连接池，连接池与原代码交替部分有些乱，待修改
配置连接池文件为mysql.ini
连接池效果还是非常好的
![image](https://github.com/user-attachments/assets/62fdcaed-d272-40f0-9386-f77a11e5cc0f)


技术栈：
  1. Json序列化和反序列化
  2. muduo网络库开发
  3. nginx源码编译安装和环境部署
  4. nginx的tcp负载均衡器配置
  5. redis缓存服务器编程实践
  6. 基于发布-订阅的服务器中间件redis消息队列编程实践
  7. MySQL数据库编程
  8. CMake构建编译环境
  9. Github托管项目
     
项目需求：
  1. 客户端新用户注册
  2. 客户端用户登录
  3. 添加好友和添加群组
  4. 好友聊天
  5. 群组聊天
  6. 离线消息
  7. nginx配置tcp负载均衡
  8. 集群聊天系统支持客户端跨服务器通信

项目目标：
  1. 掌握服务器的网络I/O模块，业务模块，数据模块分层的设计思想
  2. 掌握C++ muduo网络库的编程以及实现原理
  3. 掌握Json的编程应用
  4. 掌握nginx配置部署tcp负载均衡器的应用以及原理
  5. 掌握服务器中间件的应用场景和基于发布-订阅的redis编程实践以及应用原理
  6. 掌握CMake构建自动化编译环境
  7. 掌握Github管理项目
____________________________

编译方式:
  1.cd build
  2.rm -rf *
  3.cmake ..
  4.make

