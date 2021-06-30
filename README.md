# AutoWater_Web

## 介紹
使用 AWS IoT 製作一個可即時接收感測器資訊的網頁，可以查看當下溫濕度與種植植物的土壤濕度數值，判斷是否需要自動澆水。
## 使用資源
1. AWS EC2
    * 亞馬遜彈性雲端運算，由亞馬遜公司提供的 Web 服務。
    * 讓使用者可以租用雲端電腦運行所需應用的系統。
2. Mosquitto (MQTT)
3. node.js
    * Node.js 是能夠在伺服器端運行 JavaScript 的開放原始碼、跨平台執行環境。
4. mqtt-web-panel
    * Real-time web interface for MQTT
5. Arduino IDE
    * Arduino 集成開發環境是使用 C 和 C++ 的函數編寫的跨平台應用程序。
    * 用於編寫程序並將其上載到 Arduino 兼容的開發板。


## 硬體設備

| 設備名稱             | 數量 | 圖片                                 | 來源 |
| -------------------- | ---- | ------------------------------------ | ---- |
| LinkIt 7697          | 1    | ![](https://i.imgur.com/aUM6pvG.png) |  葉浩堯同學提供    |
| 空氣溫度、濕度感測器 | 1    | ![](https://i.imgur.com/EqUIH8X.png) |  葉浩堯同學提供    |
| 土壤濕度感測器       | 1    | ![](https://i.imgur.com/VBKcwkY.png) |  葉浩堯同學提供    |
| 杜邦線               | 數條 | ![](https://i.imgur.com/sNIO1kV.png) |  葉浩堯同學提供    |
| 繼電器               | 1    | ![](https://i.imgur.com/fJTY1Cc.png) |  葉浩堯同學提供    |
| 抽水馬達             | 1    | ![](https://i.imgur.com/bxw0vFs.png) |  葉浩堯同學提供    |
| 9V電池端子           | 1    | ![](https://i.imgur.com/pHNHRoA.jpg) |  葉浩堯同學提供    |
| 9V電池               |  1    |![](https://i.imgur.com/nWVu6mZ.png) |  葉浩堯同學提供    |

## LinkIt 7697 與各種感測器連接圖
![](https://i.imgur.com/Xwbb11d.jpg)
![](https://i.imgur.com/blYWe5p.jpg)

## 實作
### 在 AWS EC2 建立虛擬機
1. 打開 AWS console，選澤 EC2。
![](https://i.imgur.com/59Bmgmj.png)

2. 選擇左邊選單 Instances 選項。

![](https://i.imgur.com/0dF5lS2.png)

3. 選擇畫面右邊 Launch Instances。
![](https://i.imgur.com/9S5rtfy.png)

4. 選擇 OS，這邊選擇 Ubuntu 20.04。
![](https://i.imgur.com/fCazrrC.png)

5. 選擇使用免費選項。
![](https://i.imgur.com/Y3OrrZu.jpg)

6. 一直點選 next 到 key 的部分，如果沒有 key 選擇 create a new key pair。
![](https://i.imgur.com/kyOgyQS.jpg)

如果已經有 key，使用已有的 key 選 choose an existing key pair
![](https://i.imgur.com/U5DU05P.jpg)

7. 點選新建好的 instance，下方 details 顯示這台 VM 的 public IP，接著點選上方 connect 連線上 instance。
![](https://i.imgur.com/NMKgXjx.jpg)

8. 這邊使用 SSH 連線。

![](https://i.imgur.com/GkqK85u.jpg)
### 安裝各項資源
1. install mosquitto
```
sudo apt install mosquitto
sudo service mosquitto status    //check mosquitto is running
```

2. install nodejs
```
sudo apt install nodejs
node -v
npm -v
sudo apt install npm    // if npm is not installed with nodejs
```

3. install mongodb
```
sudo apt install mongodb
```

4. 下載 mqtt-web-panel
```
git clone git://github.com/mingruport/mqtt-web-panel.git
cd mqtt-web-panel/
```

5. mqtt-web-panel `.env` 設定
`vim .env`
設定 port, mqtt-web-panel default 是 3000
```
PORT=8080
MONGODB_URI='mongodb://localhost/test'
MQTT_URL='localhost'
MQTT_PORT='1883'
MQTT_USERNAME=''
MQTT_PASSWORD=''
```

6. 回到 `mqtt-web-panel/` 資料夾
port 要設定 1024 以下(如80)，需使用 `sudo npm start`
```
sudo npm install
npm start
``` 

### Arduino LinkIt 7697 設定
1. 至 [Arduino 官網](https://www.arduino.cc/en/software)下載並安裝 Arduino IDE。
2. 連結 Arduino LinkIt 7697 並放入程式。
```
#include <DHT.h>
#include <LWiFi.h>
#include <PubSubClient.h>

// DHT setup
#define DHTPIN A0     // what pin we're connected to
#define DHTTYPE DHT11

// connect to wifi
char ssid[] = "(your-wifi-ssid)";
char password[] = "(your-wifi-password)";

// connect to mqtt
char mqtt_server[] = "(your-mqtt-server-address)";    // change before use

// id sending to mqtt
char client_Id[] = "group04-linkit-7697";

// publish topic
char pub_topic1[] = "group04/Humidity";         // Humidity
char pub_topic2[] = "group04/Temperature";      // Temperature
char pub_topic3[] = "group04/Soil";             // Soil

int status = WL_IDLE_STATUS;

WiFiClient mtclient;
PubSubClient client(mtclient);

DHT dht(DHTPIN, DHTTYPE);
char Humid[] = "";
char Temp[] = "";
char SoilH[] = "";
char WCount[] = "";

const int relayPin = 3;

const int maxSoilHumid = 10;    // change according to setup
int currentHumidity = 0;


void setup() {
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    
    pinMode(LED_BUILTIN, OUTPUT);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
    dht.begin();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    DHT();
    SoilHumid();
    Autowater(currentHumidity);
    client.loop();
    delay(100);
}

void DHT() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h)) 
    {
        Serial.println("Failed to read from DHT");
    } 
    else if(h > 10 && t > 1)
    {
        Serial.print("Humidity: "); 
        Serial.print(h);
        Serial.print(" %\t");
        Serial.print("Temperature: "); 
        Serial.print(t);
        Serial.println(" *C");
        int n = sprintf(Humid, "%.1f", h);
        client.publish(pub_topic1, Humid);
        n = sprintf(Temp, "%.1f", t);
        client.publish(pub_topic2, Temp);
    }
    delay(100);
}

void SoilHumid() {
    int n = analogRead(A1);
    Serial.print("Soil ");
    Serial.print("A1: ");
    Serial.print(n);
    int h = n / maxSoilHumid;
    int x = sprintf(SoilH, "%d", h);
    Serial.println(h);
    client.publish(pub_topic3, SoilH);
    currentHumidity = h;
}



void Autowater(float humid) {
    if (buttonState == false)
    if(humid < 100) {
        digitalWrite(relayPin, true);
    }
    else if(humid >= 100) {
        digitalWrite(relayPin, false);
    }
    delay(10);
}

void printWifiStatus() {                     //print Wifi status
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void setup_wifi() {                       //setup Wifi
   // attempt to connect to Wifi network:
   Serial.print("Attempting to connect to SSID: ");
   Serial.println(ssid);
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
     delay(500);
     Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("Connected to wifi");
    printWifiStatus();
}

void callback(char* topic, byte* payload, unsigned int length) {   //MQTT sub
  Serial.print("Input Message arrived [");
  Serial.print(sub_topic);
  Serial.print(sub_topic2);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
    client.publish(pub_topic1, Humid);
    client.publish(pub_topic2, Temp);
  Serial.println();
}

void reconnect() {  //reconnect MQTT
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = client_Id;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe(sub_topic);
      client.subscribe(sub_topic2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
```
### Demolink
* http://lsa2final04.hopto.org:8080/
* 可即時查看目前溫度、空氣溼度、土壤濕度數據。
![](https://i.imgur.com/lLfynSr.png)
* 影片 Demo
https://youtu.be/o0rWqznWBAI

### 分工
葉浩堯
* AWS EC2、套件等資源安裝與連結、sensor 提供、文件撰寫。

陳佳境
* Arduino sensor 與程式研究、文件撰寫、影片剪輯。
## 參考資料
1. [溫濕度模組](https://www.taiwaniot.com.tw/product/dht22-%E6%BA%AB%E5%BA%A6%E6%A8%A1%E7%B5%84-%E6%BF%95%E5%BA%A6%E6%A8%A1%E7%B5%84-%E6%BA%AB%E6%BF%95%E5%BA%A6%E6%A8%A1%E7%B5%84-dht22/)
2. [Arduino筆記(27)：土壤濕度檢測YL-38 + YL-69](https://atceiling.blogspot.com/2017/06/arduinoyl-38-yl-69.html)
3. [mqtt-web-panel](https://github.com/mingruport/mqtt-web-panel)
4. [mosquitto](https://mosquitto.org/)
5. [linkit 環境設定](https://docs.labs.mediatek.com/resource/linkit7697-arduino/zh_tw/environment-setup)
