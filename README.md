# AutoWater_Web

## 介紹
使用 AWS IoT 製作一個可即時接收感測器資訊的網頁，可以查看當下溫濕度與種植植物的土壤濕度數值，判斷是否需要自動澆水。
## 使用資源
1. AWS EC2
    * 亞馬遜彈性雲端運算，由亞馬遜公司提供的Web服務。
    * 讓使用者可以租用雲端電腦運行所需應用的系統。
2. node.js
    * Node.js 是能夠在伺服器端運行 JavaScript 的開放原始碼、跨平台執行環境。
3. Arduino IDE
    * Arduino集成開發環境是使用C和C ++的函數編寫的跨平台應用程序。
    * 用於編寫程序並將其上載到Arduino兼容的開發板。
4. Mosquitto (MQTT)
5. [mqtt-web-panel](https://github.com/mingruport/mqtt-web-panel)
    * Real-time web interface for MQTT

## 硬體設備

| 設備名稱 | 數量 | 圖片 | 來源 |
| -------- | -------- | -------- | -------- | 
| LinkIt 7697 | 1 | ![](https://i.imgur.com/Umzsgp6.png) |
| 空氣溫度、濕度感測器 | 1 | ![](https://i.imgur.com/EqUIH8X.png) |
| 土壤濕度感測器 | 1 | ![](https://i.imgur.com/mhnrlHU.png) |
| 杜邦線 | 數條 | ![](https://i.imgur.com/sNIO1kV.png) | 
| 麵包板 | 1 | ![](https://i.imgur.com/Yqd6V5X.png) |
| 繼電器 | 1 | ![](https://i.imgur.com/dQ3HgWv.png) |
| 沉水馬達 | 1 | ![](https://i.imgur.com/F2Pk1LC.png) |

## LinkIt 7697 與各種感測器連接圖
![](https://i.imgur.com/blYWe5p.jpg)

## LinkIt 7697 程式
```
#include <DHT_U.h>
#include <DHT.h>

#include <LWiFi.h>
#include <PubSubClient.h>

#define DHTPIN A0     // what pin we're connected to
#define DHTTYPE DHT11

char ssid[] = "R17";
char password[] = "yololife";
char mqtt_server[] = "18.232.133.157";  // change this  // 163.22.20.128
char client_Id[] = "linkit-7697";
char sub_topic[] = "test";

char pub_topic1[] = "Humidity";
char pub_topic2[] = "Temperature";
char pub_topic3[] = "Soil";

int status = WL_IDLE_STATUS;

WiFiClient mtclient;     
PubSubClient client(mtclient);
long lastMsg = 0;
char msg[50];
int value = 0;

DHT dht(DHTPIN, DHTTYPE);

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
  client.loop();
}

char Humid[] = "";
char Temp[] = "";

void DHT() {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h)) 
    {
        Serial.println("Failed to read from DHT");
    } 
    else 
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
  Serial.print("] ");
//   Serial.print((char)payload[0]);
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

## 實作

<!--## 分工-->

## 資考資料
1. [溫濕度模組](https://www.taiwaniot.com.tw/product/dht22-%E6%BA%AB%E5%BA%A6%E6%A8%A1%E7%B5%84-%E6%BF%95%E5%BA%A6%E6%A8%A1%E7%B5%84-%E6%BA%AB%E6%BF%95%E5%BA%A6%E6%A8%A1%E7%B5%84-dht22/)
2. [Arduino筆記(27)：土壤濕度檢測YL-38 + YL-69](https://atceiling.blogspot.com/2017/06/arduinoyl-38-yl-69.html)
3. [mqtt-web-panel](https://github.com/mingruport/mqtt-web-panel)
4. [mosquitto](https://mosquitto.org/)
