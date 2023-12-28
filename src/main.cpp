/*
# Program : GPSSpeedometr
# Author: EmbeddevIOT (Aleksey Baranov)
# Date: (create to 28.12.22)
# MCU: ESP8266
# Hardware: BareDisplay 7 Digital to 2X / GPS: NEO -6M
# WiFi + MQTT + GPS
# Links: https://randomnerdtutorials.com/esp8266-and-node-red-with-mqtt/
# Link https://kotyara12.ru/iot/esp8266mqtt/
#  https://dash.wqtt.ru/broker/
*/

// https://how2electronics.com/diy-speedometer-using-gps-module-arduino-oled/
// https://microkontroller.ru/arduino-projects/gps-spidometr-na-arduino-i-oled-displee-svoimi-rukami/
// https://diyprojectslab.com/diy-arduino-speedometer/

#include <Arduino.h>
#include "GyverSegment.h"
#include "TinyGPSPlus.h"
#include "ESP8266WiFi.h"
#include "PubSubClient.h"

#define SPEED 9600

#define TIM_DIG_UPD 50
#define TIM_SEC 1000

char fw[4] = {"0.4"};

uint32_t timer = 0;
uint32_t timer2 = 0;
unsigned long start;

boolean newData = false;
uint8_t count = 0;
uint8_t speed = 0;

struct gps
{
  boolean FiX = 0;
  boolean valid = 0;
  uint8_t speed = 0;
  uint8_t satvalue = 0;
  uint8_t altitude = 0;
};
gps GPSData;

String strData = "";
boolean recievedFlag;

// WiFi Login and Password
const char *ssid = "AECorp2G";
const char *password = "AE19Co90$";
// MQTT broker credentials (set to NULL if not required)
const char *MQTT_username = "REPLACE_WITH_MQTT_USERNAME";
const char *MQTT_password = "REPLACE_WITH_MQTT_PASSWORD";
// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char *mqtt_server = "YOUR_BROKER_IP_ADDRESS";

// Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

TinyGPSPlus GPS;
WiFiClient espClient;
PubSubClient client(espClient);

uint8_t digs[] = {D0, D1}; // пины цифр
// 0bPGFEDCBA
// uint8_t segs[] = {D8, D7, D6, D5, A0, D4, D3, D2}; // пины сегментов
uint8_t segs[] = {D8, D7, D6, D5, D2, D4, D3, A0}; // пины сегментов
// 2 digital, no points, common cathode, display
DispBare<2, false, false> disp(digs, segs);

void SetupWIFI();
void Reconnect();
void callback(String topic, byte *message, unsigned int length);
bool readgps();
void GPS_tick();
void GPS_Processing(TinyGPSTime &t, TinyGPSDate &d);

void setup()
{
  Serial.begin(SPEED);
  Serial.println("GPS Speed Controller");
  // SetupWIFI();

  // client.setServer(mqtt_server, 1883);
  // client.setCallback(callback);

  // Loading
  for (uint8 i = 0; i <= 4; i++)
  {
    disp.setCursorEnd();
    disp.printRight(true);
    disp.fillChar('-');
    disp.update();
    disp.delay(500);
    disp.clear();
    disp.update();
    disp.delay(500);
  }
}

void loop()
{
  GPS_tick();
  // if (millis() - start > 1000)
  // {
  //   Serial.println(count);

  //   newData = readgps();
  //   if (newData)
  //   {
  //     start = millis();
  //     if (GPS.location.isValid() == 1)
  //     {
  //       GPSData.valid = 1;
  //       GPSData.speed = GPS.speed.kmph();
  //       GPSData.satvalue = GPS.satellites.value();
  //       GPSData.altitude = GPS.altitude.meters();
  //       // display.print(gps.speed.kmph());
  //     }
  //     else
  //       GPSData.valid = 2;
  //   }
  // }

  // for (unsigned long start = millis(); millis() - start < 1000;)
  // {
  //     count < 99 ? count++ : count = 0;
  //     // timer2 += 1000;
  //   // while (Serial.available())
  //   // {
  //   //   if (GPS.encode(Serial.read()))
  //   //   {
  //   //     newData = true;
  //   //   }
  //   // }
  // }

  // if (newData == true)
  // {
  //   newData = false;

  //   if (GPS.location.isValid() == 1)
  //   {
  //     GPSData.valid = false;
  //     GPSData.speed = GPS.speed.kmph();
  //     GPSData.satvalue = GPS.satellites.value();
  //     GPSData.altitude = GPS.altitude.meters();
  //     // display.print(gps.speed.kmph());
  //   }
  //   else
  //     GPSData.valid = true;
  // }
  // else {

  // }

  if (millis() - timer >= 10)
  {

    if (!GPSData.valid)
    {
      disp.setCursorEnd();
      disp.printRight(true);
      disp.fillChar('-');
      disp.update();
      disp.delay(500);
      disp.clear();
      disp.update();
      disp.delay(500);
    }
    else
    {
      disp.setCursorEnd();
      disp.printRight(true);
      disp.fillChar('0');
      // disp.setCursor(1);
      // disp.print(count);
      disp.print(GPSData.speed);
      disp.update();
      disp.tick();
    }

    timer += 10;
  }

  if (millis() - timer2 >= 1000)
  {
    count < 99 ? count++ : count = 0;
    timer2 += 1000;

    GPS_Processing(GPS.time, GPS.date);
  }

  // if (!client.connected())
  // {
  //   Reconnect();
  // }
  // if (!client.loop())
  //   client.connect("ESP8266Client", MQTT_username, MQTT_password);

  // now = millis();
  // // Publishes new temperature and humidity every 30 seconds
  // if (now - lastMeasure > 30000)
  // {
  //   lastMeasure = now;
  //   // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  //   // float humidity = dht.readHumidity();
  //   // Read temperature as Celsius (the default)
  //   // float temperatureC = dht.readTemperature();
  //   // Read temperature as Fahrenheit (isFahrenheit = true)
  //   // float temperatureF = dht.readTemperature(true);

  //   // Check if any reads failed and exit early (to try again).
  //   // if (isnan(humidity) || isnan(temperatureC) || isnan(temperatureF)) {
  //   //   Serial.println("Failed to read from DHT sensor!");
  //   //   return;
  //   // }

  //   // Publishes Temperature and Humidity values
  //   // client.publish("room/temperature", String(temperatureC).c_str());
  //   // client.publish("room/humidity", String(humidity).c_str());
  // }
}
void SetupWIFI()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266
void Reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
     YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
     To change the ESP device ID, you will have to give a new name to the ESP8266.
     Here's how it looks:
       if (client.connect("ESP8266Client")) {
     You can do it like this:
       if (client.connect("ESP1_Office")) {
     Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("ESP8266Client", MQTT_username, MQTT_password))
    {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/lamp");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// This function is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte *message, unsigned int length)
{
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
  if (topic == "room/lamp")
  {
    Serial.print("Changing Room lamp to ");
    if (messageTemp == "on")
    {
      // digitalWrite(lamp, HIGH);
      Serial.print("On");
    }
    else if (messageTemp == "off")
    {
      // digitalWrite(lamp, LOW);
      Serial.print("Off");
    }
  }
  Serial.println();
}

// проверка наличия данных
bool readgps()
{
  while (Serial.available())
  {
    int b = Serial.read();
    // в TinyGPS есть ошибка: не обрабатываются данные с \r и \n
    if ('\r' != b)
    {
      if (GPS.encode(b))
      {
        GPSData.valid = 3;
        return true;
      }
    }
  }
  GPSData.valid = 4;
  return false;
}

void GPS_tick()
{
  while (Serial.available() > 0)
    GPS.encode(Serial.read());
}

void GPS_Processing(TinyGPSTime &t, TinyGPSDate &d)
{

  if (GPS.location.isValid() == 1)
  {
    GPSData.valid = true;
    GPSData.speed = GPS.speed.kmph();
    GPSData.satvalue = GPS.satellites.value();
    GPSData.altitude = GPS.altitude.meters();
    // display.print(gps.speed.kmph());
  }
  else
    GPSData.valid = false;

  // if (!t.isValid())
  // {
  //   GPSData.valid = false;
  // }
  // else
  // {
  //   GPSData.valid = false;
  // GPSTime.Age = t.age();
  // GPSTime.Sattelite = TGPL.satellites.value();
  // if ((GPSTime.Sattelite >= 1) && (GPSTime.Age < 1000))
  // {
  //   GPSTime.Fix = true;
  //   GPSTime.Hour = t.hour();
  //   GPSTime.Minute = t.minute();
  //   GPSTime.Second = t.second();

  //   GPSTime.LocalHour = GPSTime.Hour + Config.TimeZone;
  //   if (GPSTime.LocalHour >= 24)
  //   {
  //     GPSTime.LocalHour = GPSTime.LocalHour - 24;
  //   }
  //   else
  //     GPSTime.LocalHour = GPSTime.LocalHour;
  // }
  // else
  // {
  //   // ShowErrorState(NO_GPS_FIX);
  //   GPSTime.Fix = false;
  // }
  // }
}