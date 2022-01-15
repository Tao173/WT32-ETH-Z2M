# WT32-ETH-Z2M

### 起因

最近各种zigbee2mqtt的开源网关出现，我也跟着折腾了一下，自己跟着相关项目做了一个网关，无线的嫌弃它不稳定（心理因素），串口的嫌弃它占硬件接口（XX），有线的嫌弃它不方便（这个确实，要网线就得路由器附近）。话说Z2M真好，我居然在寻找以前wifi设备的可替代zigbee同类商品。

自己做的是有线的版本，WT32-ETH网口驱动在esphome很简单就实现了，但是就是闲，折腾好了有线又想保留WT32-ETH的无线能力，查询资料发现esphome不支持双网络同时使用，遂折腾[tasmota](https://tasmota.github.io/docs/)，发现这货支持，但是得自己编译。

### 相关代码

tasmota编译方法比较多，选了docker编译，部署本地的，[编译文档](https://github.com/benzino77/tasmocompiler)。推荐云端编译。

自定义代码部分：

```
#define USE_TCP_BRIDGE
#define USE_SERIAL_BRIDGE
// WT32-ETH01
#define USE_ETHERNET
#undef  ETH_TYPE
#define ETH_TYPE        0   // ETH_PHY_LAN8720
#undef  ETH_CLKMODE
#define ETH_CLKMODE     0   // ETH_CLOCK_GPIO0_IN
#undef  ETH_ADDRESS
#define ETH_ADDRESS     1

```

添加之后可以使用网口，编译时不填写WiFi配置，会自动以AP模式配置WiFi。

刷入固件之后需要进入控制台输入命令配置tcp端口和自启

```
backlog rule1 on system#boot do TCPStart 8880 endon ; rule1 1 ;之后需要网页重启而不是断电
```

WT32-ETH配置网口（可选）：

```
backlog template {"NAME":"WT32-ETH01","GPIO":[0,3200,1,3232,1,1,0,0,1,0,1,1,5536,1,5600,0,0,0,0,5568,0,0,0,0,0,0,0,0,1,1,0,1,1,0,0,1],"FLAG":0,"BASE":1}; EthAddress 1; EthType 0; EthClockMode 0; module 0;

```

命令行配置MQTT服务（网页配置时mqtt密码长度不能短于5位（SB））：

```
Backlog MqttHost 192.168.124.23; MqttUser <user>; MqttPassword <password>;  
```

在配置里面设置好tcp引脚，一切OK。

有一份自己配置好的备份文件，可以恢复到你的设备。

设备不一样的引脚可能需要自己修改（看原理图）


### 20220115更新


添加[ZigStarGW-FW](https://github.com/xyzroe/ZigStarGW-FW)汉化版本，去除多余内容，help相关页面，支持有线网络同时支持无线网络。

UTF8编码汉化，gulp --tasks修改web页，gulp default修改网页模块，需要nodejs和npm。

参考

[前端修改](https://github.com/esprfid/esp-rfid#frontend)
