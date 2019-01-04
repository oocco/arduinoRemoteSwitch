# arduinoRemoteSwitch
use esp8266(wemos d1 mini) realize remote switch, pwm etc..

#### 功能 Features
1.模拟轻触开关： 开启后 高电平输出500ms后拉低，可连接一个三极管，控制电脑开机关机；

2.自锁开关： 普通开光,可控制开启关闭状态，接继电器可控制其它设备电源；

3.PWM输出： 输出pwm信号，占空比0-1000， 用于pwm风扇或者灯光调节；


*可自行添加设置gpio功能，立即生效。重启后恢复默认（需自行点击保存）；

#### 说明 Detail
***

1.首页，开关操作，设置pwm口占空比。
>home panel, open switch, set pwm duty rtio.

![deviceOperate](https://github.com/oocco/arduinoRemoteSwitch/blob/master/readme/homePanel.gif)
***

2.上传或者删除文件，用于修改首页文件
>upload or delete file

![deviceOperate](https://github.com/oocco/arduinoRemoteSwitch/blob/master/readme/filePanel.gif)
***

3.删除或者添加设备
>delete deivce or add device

![deviceOperate](https://github.com/oocco/arduinoRemoteSwitch/blob/master/readme/deviceOperate.gif)
***

4.添加基本的认证，未加密
>add base authorization

![deviceOperate](https://github.com/oocco/arduinoRemoteSwitch/blob/master/readme/passwordSet.gif)
***


