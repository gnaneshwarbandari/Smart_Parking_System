#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#define SSD1306_LCDHEIGHT 64
#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(-1);
//-------- Customise the above values --------
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
void callback(char* topic, byte* payload, unsigned int payloadLength);
const char* ssid     = "Bandaris";
const char* password = "passw0rd";
#define ORG "gagtey"
#define DEVICE_TYPE "Signboard"
#define DEVICE_ID "12345"
#define TOKEN "12345678"
const char publishTopic[] = "iot-2/evt/data/fmt/json";
char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char topic[] = "iot-2/cmd/home/fmt/json";// cmd  REPRESENT command type AND COMMAND IS TEST OF FORMAT STRING
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
WiFiClient wifiClient;
PubSubClient client(server, 1883, callback, wifiClient);
int publishInterval = 500; // 30 seconds
long lastPublishMillis;
void publishData();
int count=2;
int s1 = 0;
int s2 = 0;
int b1 = 0;
int b2 = 0;
void setup() {
  Serial.begin(9600);
  Serial.println();
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(40,18);
  display.println("SMART");
  display.setCursor(30,38);
  display.println("PARKING");
  display.display();
  wifiConnect();
  mqttConnect();
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  //pinMode(D5, INPUT);
  //pinMode(D6, INPUT);
  //Serial.begin(9600);
}

void loop() {
  String s;
  int p1,p2;
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(1,1);
  display.print("SLOT 1: ");
  if(s1 ==1 || b1 == 1){
    display.println("OCCUPIED");
  }
  else if(s1 == 0){
    display.println("AVAILABLE");
  }
  display.setCursor(1,20);
  display.print("SLOT 2: ");
  if(s2 ==1 || b2 == 1){
    display.println("OCCUPIED");
  }
  else if(s2 == 0){
    display.println("AVAILABLE");
  }
  display.setCursor(1,40);
  display.print("No. of Free Slots: ");
  display.println(count);
  display.display();
  Serial.print("Parking Slot 1: ");
  Serial.print(s1);
  Serial.print("  || Parking Slot 2: ");
  Serial.println(s2);
  //delay(1000);
  p1 = digitalRead(D3);
  p2 = digitalRead(D4);
  //Serial.println(p1);
  if(count>0 && count < 2){
    if(p1 == 0 && s1 == 0 && b1 == 0){
      count = count-1;
      s1=1;
      s = "Car Parked - Slot 1";
      display1(s);
      Serial.println(s);
    }
    else if(p2 == 0 && s2 == 0 && b1 == 0){
      count = count-1;
      s2=1;
      s = "Car Parked - Slot 2";
      display1(s);
      Serial.println(s);
    }
    else if(p1 == 1 && s1 == 1 && b1 == 0){
      count = count+1;
      s1 = 0;
      s = "Slot 1 is Free";
      display1(s);
      Serial.println(s);
    }
    else if(p2 == 1 && s2 == 1 && b1 == 0){
      count = count+1;
      s2 = 0;
      s = "Slot 2 is Free";
      display1(s);
      Serial.println(s);
    }
  }
  else if(count == 0){
    if(p1 == 1 && s1 == 1 && b1 == 0){
      count = count+1;
      s1 = 0;
      s = "Slot 1 is Free";
      display1(s);
      Serial.println(s);
    }
    else if(p2 == 1 && s2 == 1 && b1 == 0){
      count = count+1;
      s2 = 0;
      s = "Slot 2 is Free";
      display1(s);
      Serial.println(s);
    }
  }
  else if(count == 2){
    if(p1 == 0 && s1 == 0 && b1 == 0){
      count = count-1;
      s1=1;
      s = "Car Parked - Slot 1";
      display1(s);
      Serial.println(s);
    }
    else if(p2 == 0 && s2 == 0 && b1 == 0){
      count = count-1;
      s2=1;
      s = "Car Parked - Slot 2";
      display1(s);
      Serial.println(s);
    }
  }
  delay(2000);
  if (millis() - lastPublishMillis > publishInterval)
  {
    Serial.println("Sending Payload");
    publishData(s1,s2,count);
    lastPublishMillis = millis();
  }
  
  if (!client.loop()) {
    mqttConnect();
  }
}

void callback(char* topic, byte* payload, unsigned int payloadLength) {
  String data;
  Serial.print("callback invoked for topic: ");
  Serial.println(topic);

  for (int i = 0; i < payloadLength; i++) {
    //Serial.print((char)payload[i]);
    data += (char)payload[i];
  }
  Serial.println("Received data : "+data );
  String a = data.substring(6,7);
  b1 = a.toInt();
  Serial.println(b1);
  String b = data.substring(13,14);
  b2 = b.toInt();
  Serial.println(b2);
  String c = data.substring(19,20);
  count = c.toInt();
  Serial.println(count);
}

void mqttConnect() {
  if (!client.connected()) {
    Serial.print("Reconnecting MQTT client to "); Serial.println(server);
    while (!client.connect(clientId, authMethod, token)) {
      Serial.print(".");
      delay(500);
    }
    initManagedDevice();
    Serial.println();
  }
}

void initManagedDevice() {
  if (client.subscribe(topic)) {
   // Serial.println(client.subscribe(topic));
    Serial.println("subscribe to cmd OK");
  } else {
    Serial.println("subscribe to cmd FAILED");
  }
}

void wifiConnect() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void publishData(int a, int b, int c) 
{
  String payload = "{\"s1\":";
  payload +=a;
  payload +=",""\"s2\":";
  payload +=b;
  payload +=",""\"c\":";
  payload +=c;
  payload += "}";
  Serial.print("\n");
  Serial.print("Sending payload: "); 
  Serial.println(payload);
  if (client.publish(publishTopic, (char*) payload.c_str())) {
    Serial.println("Publish OK");
  } else {
    Serial.println("Publish FAILED");
  }
}

void display1(String st){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,25);
  display.println(st);
  display.display();
}
