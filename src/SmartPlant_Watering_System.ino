/*
 * Project SmartPlant_Watering_System
 * Description:Smart watering system
 * Author:Gabriel A
 * Date:3-17-2023
 */

#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include"Adafruit_BME280.h"
#include <math.h>
const int OLED_RESET=4;
int Pin1=A2;
int val;
int dustPin=D8;
unsigned long duration;
unsigned long starttime;
unsigned long sampletime_ms = 30000;
unsigned long lowpulseoccupancy=0;
float ratio=0;
float concentration=0;
float tempF;
 unsigned int last, lastTime;
SYSTEM_MODE(SEMI_AUTOMATIC);   
Adafruit_BME280 bme;
String DateTime, TimeOnly;
Adafruit_SSD1306 display(OLED_RESET);
TCPClient TheClient;
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_MQTT_Publish SoilFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Pin1");
Adafruit_MQTT_Publish DustFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Pin8");
// setup() runs once, when the device is first turned on.
void setup() {
  Serial.begin(9600);
   pinMode(dustPin,INPUT);
    starttime = millis();//get the current time;
  pinMode(Pin1, INPUT);
  Time.zone(-7);
  Particle.syncTime();
   display.begin (SSD1306_SWITCHCAPVCC, 0x3c);
  display.clearDisplay();
  display.drawBitmap(16,20, myBitmap,128, 64, 1);
  display.display();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display.setTextSize(1);
  display.setTextColor(BLACK,WHITE);
  display.printf("Pin1%c",33);
  display.display();

  // Put initialization like pinMode and begin functions here.

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  tempC=bme.readTemperature();
  tempF=(tempC*1.8)+32;
   duration = pulseIn(pin, LOW);
    lowpulseoccupancy = lowpulseoccupancy+duration;

    if ((millis()-starttime) > sampletime_ms)//if the sampel time == 30s
    {
        ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
        concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
        Serial.print(lowpulseoccupancy);
        Serial.print(",");
        Serial.print(ratio);
        Serial.print(",");
        Serial.println(concentration);
        lowpulseoccupancy = 0;
        starttime = millis();
    }
}
  MQTT_connect();
  MQTT_ping();
   val=analogRead(Pin1);
   Serial.println(val);
   DateTime=Time.timeStr();
   TimeOnly=DateTime.substring(11, 19);
   Serial.printf("Date and time is %s\n",DateTime.c_str());
   Serial.printf("Time is %s\n",TimeOnly.c_str());
   if(mqtt.Update()) {
      pubFeed.publish(Pin1);
      Serial.printf("Publishing %0.2f \n",Pin1); 
      } 
    lastTime = millis();
  
void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}

