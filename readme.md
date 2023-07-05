# QtDebugLog

#### 介绍

实现在qt工程里，c++ 代码里的qDebug  qWarning  qCritical  qFata qInfo, QML 里的 console.log, console.debug console.warn console.error console.info 统一日志打印并保存日志文件。

#### 软件架构

使用qt 自带的qInstallMessageHandler 函数来获取所有打印处理，qSetMessagePattern函数来设置打印格式

当前的打印效果如下：

```
##########################
[2023-07-05 18:09:24.683][D][Main.qml:-1][246466][onCompleted]: qml log [     0.396]
[2023-07-05 18:09:24.683][D][Main.qml:-1][246466][onCompleted]: qml debug [     0.396]
[2023-07-05 18:09:24.683][W][Main.qml:-1][246466][onCompleted]: qml warn [     0.396]
[2023-07-05 18:09:24.683][I][Main.qml:-1][246466][onCompleted]: qml info [     0.396]
[2023-07-05 18:09:24.683][D][main.cpp:14][246478][testdebug]: qdebug 1 [     0.396]
[2023-07-05 18:09:24.684][W][main.cpp:15][246478][testdebug]: qWaring 1 [     0.396]
[2023-07-05 18:09:24.684][C][main.cpp:16][246478][testdebug]: qCritical  1 [     0.396]
[2023-07-05 18:09:24.684][I][main.cpp:17][246478][testdebug]: qInfo  1 [     0.396]
```

#### 安装教程

1. qt环境
2. cmake
3. 编译方法可以用qt creator 打开工程来运行或者 cmake命令

#### 使用说明

设置保存文件类型时需要注意

Name_Fixed  ：日志文件名固定，则不能自动生成其他日志文件，限制文件大小和数量功能就会失效

Name_FixedOrder：会根据当前目录日志文件的最大后缀id继续往后增加，每当文件超过指定大小之后，则自动新建日志文件，例如：install_0.log install_1.log

Name_Date：根据当前日期来给日志命名，该模式下单个文件大小将不受限制，对应函数无效。注意，该类支持凌晨跨日之后，自动启用新文件日志

Name_DateTime：根据日期十分秒来给日子命名

#### 参与贡献
