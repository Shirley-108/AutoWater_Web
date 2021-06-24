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