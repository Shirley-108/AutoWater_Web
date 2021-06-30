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
