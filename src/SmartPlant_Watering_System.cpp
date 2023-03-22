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
float readDustSensor();
void activatePump();
void deactivatePump();
#line 16 "c:/Users/gabea/Documents/IoT/SmartPlant_Watering_System/src/SmartPlant_Watering_System.ino"
const int OLED_RESET=4;   //Pin set to OLED
const int SoilPin=A2;    //Pin set to Moisture Sensor
const int airQPin=A4;    //Pin set for Airqualitysensor
int Soil;
float dust;
float Temp;
float ratio=0;
  float concentration=0;
const int dustPin=D8;    //Pin set to Dust sensor
unsigned long duration;
unsigned long duststarttime;
unsigned long dustsampletime_ms=30000;
unsigned long lowpulseoccupancy=0;

const int publishTime=30000; //amount of time it takes to publish to the MQTT dashboard
float tempF;
float tempC;
float humidRH;
int status;
int moistureval;    //moisture value read for the pump to turn on 
int turnOnPump;    //function that turns pump on
int currentTime;    ///current time for the Pump Timer 
int PumplastTime;
int lastTime;      ///last time for the pump Timer
int currentQuality;  //currentquality of the air sensor
const int moisturePump=D13;   //pin set to the Pump
unsigned int last, MQTTlastTime; 
int waterButtonVal;  
AirQualitySensor AirQsensor(airQPin);
Adafruit_BME280 bme;
String DateTime, TimeOnly;
Adafruit_SSD1306 display(OLED_RESET);
TCPClient TheClient;
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY);

Adafruit_MQTT_Publish SoilFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Moisture");
Adafruit_MQTT_Publish DustFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Dust");
Adafruit_MQTT_Publish TempFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temp");
Adafruit_MQTT_Publish AirFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Air");
Adafruit_MQTT_Publish HumidFeed=Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Hum");

Adafruit_MQTT_Subscribe waterButton=Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/button");

void MQTT_connect();
bool MQTT_ping();
// setup() runs once, when the device is first turned on.
void setup() {
   WiFi.on();
   WiFi.connect();
   Serial.begin(9600);
   pinMode(dustPin,INPUT);
   duststarttime=millis();//get the current time for dust sensor
   pinMode(SoilPin, INPUT);
   pinMode(moisturePump,OUTPUT);
   Time.zone(-7);
   Particle.syncTime();
   Wire.begin();
   mqtt.subscribe(&waterButton);

    currentQuality=-1;
   AirQsensor.init();//get current value of AirQsensor
   


  
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
  moistureval=analogRead(SoilPin);  //AnalogRead Pin for soil sensor
  tempC=bme.readTemperature();
  tempF=(tempC*1.8)+32;
  humidRH=bme.readHumidity();
  dust=readDustSensor();
  if((currentTime-PumplastTime>=10000)){
     if(moistureval>=2500){
      activatePump();
      delay(500);
      deactivatePump();
    }
  if(moistureval<2500){
      deactivatePump();
    }
      PumplastTime=millis();
   
  }
      display.clearDisplay();   ///function to display to the OLED 
      display.setCursor(0,0);
      display.printf("temperature is %0.2f\n",tempF);
      display.printf("humidity is %0.1f\n ",humidRH);
      display.printf("moisture is %i\n",moistureval);
      display.display();

Adafruit_MQTT_Subscribe *subscription;
  while ((subscription=mqtt.readSubscription(4000))) {
    if (subscription==&waterButton) {
        waterButtonVal=atoi((char *)waterButton.lastread);
  Serial.printf("waterbuttonval%i\n",waterButtonVal);
    }
  }
    if (waterButtonVal==HIGH) {
      activatePump();
      delay(500);
      deactivatePump();
      Serial.printf("Dashboard Water Button Pressed ON \n");
    }
    else{
      deactivatePump();
      Serial.printf("Dashboard Water Button Pressed OFF \n");
    }
  


    if((millis()-lastTime>30000)){
    if(mqtt.Update()){
         SoilFeed.publish(moistureval);
        Serial.printf("Publishing %i \n",moistureval); 
         DustFeed.publish(dust);
        Serial.printf("Publishing %0.2f \n",dust);
        TempFeed.publish(tempF);
        Serial.printf("Publishing %0.2f \n",tempF);
        AirFeed.publish(currentQuality);
        Serial.printf("Publishing %i\n",currentQuality);
        HumidFeed.publish(humidRH);
        Serial.printf("Publishing %0.2f\n",humidRH);
        lastTime=millis();
      } 
    }

      currentQuality=AirQsensor.slope();
    if(currentQuality>=0){// if a valid data returned.
    
        if (currentQuality==0)
            Serial.printf("High pollution! Force signal active\n");
        else if (currentQuality==1)
              Serial.printf("High pollution\n!");
        else if (currentQuality==2)
              Serial.printf("Low pollution!\n");
        else if (currentQuality ==3)
              Serial.printf("Fresh air\n");
    }
    MQTT_connect();
    MQTT_ping();

}
float readDustSensor() {
  duration = pulseIn(dustPin, LOW);
  lowpulseoccupancy = lowpulseoccupancy+duration;
  if ((millis()-duststarttime) > dustsampletime_ms){
    ratio = lowpulseoccupancy/(dustsampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    Serial.print(lowpulseoccupancy);
    Serial.print(",");
    Serial.print(ratio);
    Serial.print(",");
    Serial.println(concentration);
    lowpulseoccupancy=0;
    duststarttime=millis();
  }
return concentration;
}
void activatePump() {
    digitalWrite(moisturePump, HIGH);
    Serial.printf("pump is on\n"); 
  }
    
void deactivatePump(){
    digitalWrite(moisturePump, LOW);
    // Serial.printf("pump is off\n");
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

