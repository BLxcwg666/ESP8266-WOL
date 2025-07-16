# ESP8266-WOL
A Web-controlled Wake-on-LAN trigger running on ESP8266, featuring HTTP and WebSocket command support.

## Introduction
这是一个基于 ESP8266 的 Wake-on-Lan（WOL）解决方案  
特点：  
- Cloud based  
无需拥有公网IP，仅需一个可以部署在云上的环境，即可实现异地远程开机  
- C/S  
ESP 作为客户端设备，启动后主动连接到云上，从而允许你在任何地点通过浏览器向你的局域网发送 WOL 数据包  
- Open API  
你可以通过 API 自由扩展，比如：Telegram Bot ?
- Easy Auth  
单参数鉴权，简单，~~安不安全我不知道，但是能用~~

## Run
- 首先打开你的 `Arduino IDE` 选择你的板子并把我的史山烧录进去  
- 然后把 `Server` 目录下的 Node.JS 项目部署到你喜欢的地方（注意`.env.example`，且要求可以公网访问<不过这点就无所谓了，你拿 frp 之类的东西都行>）  
- 然后启动你的板子，查看你的 WIFI 列表，应当会有一个叫`ESP8266-Wol`的 WIFI，用密码`12345678`连接它，然后在你的浏览器打开`192.168.4.1`并按照提示配置 WIFI（这个 WIFI 要能访问到你的 Server）  
- 连接完 WIFI 之后会自动关掉板子的热点并重定向到你内网的地址，然后在里面配置你`Server`的 Websocket 地址（有 Guide，自己看）  
- 然后确保已连接之后用浏览器打开你的`Server`，输入你在`.env.example`配置的米奇妙妙密码，然后你就会用了吧

## Warn
ESP8266 不支持 TLS v1.1+，所以不要走 wss，会连不上

## API
[看这](API.md)

## End
反正呢，只要你的远端服务器和 ESP8266 活着，就一直能用  
喜欢就点 Star 谢谢喵

## License
AGPL v3
