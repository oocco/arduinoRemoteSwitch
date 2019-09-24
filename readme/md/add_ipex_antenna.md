## 添加外部天线/Add Antenna
因为ESP8266模块放置在电脑机箱内部，影响wifi信号导致无法连接网络，所以可以改为外置IPEX接口的天线。

![](https://github.com/oocco/arduinoRemoteSwitch/raw/master/readme/add_ipex_antenna_01.jpg)

#### 找到IPEX底座
一般的msata无线网卡均有，或者部分路由器天线是可拆卸的，内部也有；
不是太好拿下来，推荐使用美工刀直接刮下

![](https://github.com/oocco/arduinoRemoteSwitch/raw/master/readme/images/add_ipex_antenna_02.jpg)

#### 接线方式
如下图所示断开红色部分（直接使用锋利一点的美工刀刮下来即可），将IPEX底座焊接上去

![](https://github.com/oocco/arduinoRemoteSwitch/raw/master/readme/images/add_ipex_antenna_03.jpg)

将原来天线部分使用锡箔纸相连并覆盖。

![](https://github.com/oocco/arduinoRemoteSwitch/raw/master/readme/images/add_ipex_antenna_04.jpg)

#### 连接并测试
将天线放置于机箱外部，清除wifi连接信息，使用WiFiManager库的设置功能检查信号强度即可。

![](https://github.com/oocco/arduinoRemoteSwitch/raw/master/readme/images/add_ipex_antenna_05.jpg)
