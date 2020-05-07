#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <math.h>
// Update these with values suitable for your network.

const char* ssid = "Device";
const char* password = "12345678";
const char* mqtt_server = "10.0.0.21";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
int Brightness = 0;
int ColorTemperature = 0;
int white = 0;
int yellow = 0;
String Power;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg_aa;
  for (int i = 0; i < length; i++) {
    msg_aa += (char)payload[i];
  }
  Serial.println(msg_aa);
  if (topic == "lamp/Brightness"){
      Brightness = msg_aa.toInt();
    }
  else if (topic == "lamp/ColorTemperature"){
       ColorTemperature = msg_aa.toInt();
    }
  else if (topic == "lamp/Power"){
    Power = msg_aa;
  }
  if(ColorTemperature >320){
    white = lround((180-(ColorTemperature-320))*255/180);
    yellow = lround((ColorTemperature-320)*255/180);
  }
  else if(ColorTemperature <320){
    white = lround((180-(ColorTemperature-140))*255/180);
    yellow = lround((ColorTemperature-140)*255/180);
  }else{
    white = 123;
    yellow = 123;
}
//恢复亮度比例
yellow = lround(yellow*Brightness/100);
white = lround(white*Brightness/100);

  analogWrite(12,yellow);
  analogWrite(13,white);
/*
  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
*/
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("lamp/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(12, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(13, OUTPUT);  
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}