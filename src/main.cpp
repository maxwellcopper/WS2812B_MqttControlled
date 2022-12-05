#include "secrets.h"
#include <FastLED.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "WiFi.h"
#define DEBUG

#ifdef DEBUG
#define debug(x)    Serial.print(x)
#define debugln(x)  Serial.println(x)
#endif

#ifdef RELEASE
#define debug(x)
#define debugln(x)
#endif

//mqtt declare 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "light_bulb/color" 
#define topic_status    "light_bulb/status"
#define topic_intensity "light_bulb/intensity"
#define topic_color     "light_bulb/color"
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

//ws2812b declare 
#define LED_PIN     4
#define NUM_LEDS    100
CRGB leds[NUM_LEDS];
char messej[50];
bool enable_led = false;
uint16_t intensity = 0;
String Str_intensity = "";
String Str_color = "";

volatile byte red = 0, green = 0, blue = 255; //default blue light first appear

byte StringtoInt(String value){
    return value.toInt();
}

void messageHandler(char* topic, byte* payload, unsigned int length)
{
  debug("incoming: ");
  debug(topic);
  debugln("Message:");
  
  for (int i = 0; i < length; i++) {
     debug((char) payload[i]);
     messej[i] = (char)payload[i];
  }
  debugln();
  debugln(messej);
  debugln("-----------------------");

 if(!strcmp(topic, topic_status)){
   if(!strcmp(&messej[0], "1")){
     digitalWrite(LED_BUILTIN, 1);
     enable_led = true;
     debugln("on");
    }
  else if(!strcmp(&messej[0], "0")){
     digitalWrite(LED_BUILTIN, 0);
     debugln("led Off");
     enable_led = false;
    }
 }

 if(!strcmp(topic, topic_intensity)){
    Str_intensity = messej;
    intensity = Str_intensity.toInt();
    if(intensity > 0 && intensity < 255){FastLED.setBrightness(intensity);}
 } 

 if(!strcmp(topic, topic_color)){ //ex. data : #1f1e33
    Str_color = messej;
    red   = StringtoInt(Str_color.substring(1, 3));
    green = StringtoInt(Str_color.substring(3, 5));
    blue  = StringtoInt(Str_color.substring(5, 7));
 }

}


void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // publish and subscribe
  client.subscribe(topic_status);
  client.subscribe(topic_color);
  client.subscribe(topic_intensity);
  
  client.publish(AWS_IOT_PUBLISH_TOPIC, "bili");
 
  Serial.println("AWS IoT Connected!");
}
 
 
void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  connectAWS();

}
 
void loop()
{ 
 client.loop();
 if(enable_led){
      for (int i = 0; i <= NUM_LEDS; i++) {
      leds[i] = CRGB ( red, green, blue);
      FastLED.show();
      delay(10);
    }
 }else{
      for (int i = 0; i <= NUM_LEDS; i++) {
      leds[i] = CRGB ( red, green, blue);
      FastLED.show();
      delay(10);
    }
 }

}