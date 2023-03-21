/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/gabea/Documents/IoT/SmartPlant_Watering_System/src/SmartPlant_Watering_System.ino"
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
#include "Adafruit_SSD1306.h"
#include"Adafruit_BME280.h"
#include <math.h>
#include "Air_Quality_Sensor.h"
void setup();
void loop();
void readDustSensor();
void activatePump();
void deactivatePump();
#line 16 "c:/Users/gabea/Documents/IoT/SmartPlant_Watering_System/src/SmartPlant_Watering_System.ino"
const int OLED_RESET=4;
const int SoilPin=A2;
int Soil;
int dust;
float Temp;
// const int Airsensor;
const int dustPin=D8;
unsigned long duration;
unsigned long duststarttime;
unsigned long dustsampletime_ms=30000;
unsigned long lowpulseoccupancy=0;
float ratio=0;
float concentration=0;
const int publishTime=30000; //amount of time it takes to publish to the MQTT dashboard
float tempF;
float tempC;
float humidRH;
int status;
int moistureval;
int turnOnPump;
int AirQsensor;
int currentTime;
int delaytime;
int lastTime;
const int moisturePump=D13;
unsigned int last, MQTTlastTime;   
Adafruit_BME280 bme;
String DateTime, TimeOnly;
Adafruit_SSD1306 display(OLED_RESET);
TCPClient TheClient;
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);
Adafruit_MQTT_Publish SoilFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Moisture");
Adafruit_MQTT_Publish DustFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Dust");
Adafruit_MQTT_Publish TempFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temp");
Adafruit_MQTT_Publish BMEFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/BME");

void MQTT_connect();
bool MQTT_ping();
// setup() runs once, when the device is first turned on.
void setup() {
   WiFi.on();
   WiFi.connect();
   Serial.begin(9600);
   pinMode(dustPin,INPUT);
   duststarttime=millis();//get the current time;
   pinMode(SoilPin, INPUT);
   pinMode(moisturePump,OUTPUT);
   Time.zone(-7);
   Particle.syncTime();
   Wire.begin();


  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  status=bme.begin(0x76);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
  display.display();
  if(status==false){
    Serial.printf("BME280 at address 0x%02X failed to start", 0x76);
}
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  currentTime=millis();
  moistureval=analogRead(SoilPin);
  tempC=bme.readTemperature();
  tempF=(tempC*1.8)+32;
  humidRH=bme.readHumidity();
  
  
  if((currentTime-lastTime>=10000)){
     if(moistureval>=2500){
      activatePump();
      delay(500);
      deactivatePump();
   }
  if(moistureval<2500){
      deactivatePump();
  }
      lastTime=millis();
   
  }
      display.clearDisplay();   ///function to display to the OLED 
      display.setCursor(0,0);
      display.printf("temperature is %0.2f\n",tempF);
      display.printf("humidity is %0.1f\n ",humidRH);
      display.printf("moisture is %i\n",moistureval);
      display.display();
    if((millis()-lastTime>30000)){
    if(mqtt.Update()){
      DateTime=Time.timeStr();
      TimeOnly=DateTime.substring(11, 19);
      Serial.printf("Date and time is %s\n",DateTime.c_str());
      Serial.printf("Time is %s\n",TimeOnly.c_str());
   if(mqtt.Update()) {
      SoilFeed.publish(Soil);
      Serial.printf("Publishing %0.2f \n",Soil); 
      } 
      if(mqtt.Update()) {
      DustFeed.publish(dust);
      Serial.printf("Publishing %0.2f \n",dust);
      }
      if(mqtt.Update()) {
      TempFeed.publish(Temp);
      Serial.printf("Publishing %0.2f \n",Temp);
      }
      lastTime=millis();
    }
    }
      // SoilFeed.publish(moistureval);
      // TempFeed.publish(tempF);
      // BMEFeed.publish(humidRH);
      // DustFeed.publish(concentration);
    
    MQTT_connect();
    MQTT_ping();
   Soil=analogRead(SoilPin);
   Serial.println(Soil);
}
void readDustSensor() {
  duration = pulseIn(dustPin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;

  if ((millis()-duststarttime) > dustsampletime_ms) {
    ratio = lowpulseoccupancy/(dustsampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print(lowpulseoccupancy);
    Serial.print(",");
    Serial.print(ratio);
    Serial.print(",");
    Serial.println(concentration);
    lowpulseoccupancy = 0;
    duststarttime = millis();
  }
}
void activatePump() {
    digitalWrite(moisturePump, HIGH);
    Serial.printf("pump is on"); 
  }
    
void deactivatePump(){
    digitalWrite(moisturePump, LOW);
    Serial.printf("pump is off");
  }
void MQTT_connect(){
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

bool MQTT_ping(){
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

