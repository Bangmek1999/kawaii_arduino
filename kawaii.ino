#include <WiFi.h>
#include <PubSubClient.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <cppQueue.h>
#include <time.h>
#include <esp_wifi.h>
#include "WiFiClientSecure.h"
#include "Preferences.h"
#include "DHT.h"
#include "ApClientConfig.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


//-------------------------------------------------ConfigData----------------------------------
struct ConfigData {
  char wifi_ssid[20];
  char wifi_password[20];
  char mqtt_server[20];
  char mqtt_user[20];
  char mqtt_password[20];
  int sleep_interval;
} Config;

//-------------------------------------------------SensorsDataStruct----------------------------------
struct SensorsDataStruct {
  // char mac_add[50];
  time_t Timestamp;
  double DS_temperature1;
  double DS_temperature2;
  double DHT_temperature;
  double DHT_humidity;
  int Ec1;
  int Ec2;
  int Ec3;
  double light;
  double voltage;
} SensorsData;
Queue Q(sizeof(SensorsDataStruct), 50, FIFO);

//-------------SensorsPin------------------
//const int pin_Active_led = 2;
const int pin_led_red = 26;
const int pin_led_blue = 13;
const int pin_reset = 23;

const int pin_light_data  = 34;
const int pin_light_power = 22;

const int ONE_WIRE_BUS = 27;

const int pin_temperature_power_1 = 12;
const int pin_temperature_power_2 = 14;

const int pin_dht11_data  = 19;
const int pin_dht11_power = 18;

const int pin_ec1_data  = 35;
const int pin_ec2_data  = 36;
const int pin_ec3_data  = 39;
const int pin_ec_power  = 5;
const int pin_voltage  = 32;

const int EC_reference_dry  = 4095;
const int EC_reference_wet  = 2500;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#define DHTTYPE DHT11
DHT dht(pin_dht11_data, DHTTYPE);

//----------charConfig----------
unsigned long epoc = 0;
bool broadcast_done = false;
bool lowbatt = false;
bool charging = false;
float vLow = 3.7;
time_t timenow;
time_t prev_broadcast_time = 0;
char device_data[600];
char mqtt_tx_topic[100];
char mqtt_rx_topic[100];
char mqtt_sync_topic[100];

char clientID[20];
char wifi_ssid[50];
char wifi_password[50];

ApClientConfig ApConfig("plant", "", "GG", "gigikawaii", Config.wifi_ssid, Config.wifi_password);
Preferences ConfigManager((byte*)&Config, sizeof(Config));
//-------------------------------------------------ClientConfigIsDone----------------------------------
void ClientConfigIsDone() {
  Serial.println("Done");
  Serial.println(Config.wifi_ssid);
  Serial.println(Config.wifi_password);
  strcpy(Config.mqtt_server, "mqtt.csmju.com");
  strcpy(Config.mqtt_user, "dev");
  strcpy(Config.mqtt_password, "dev123456789");
  Config.sleep_interval = 30;
  ConfigManager.Write();
  Serial.println("Reboot");
  digitalWrite(pin_led_red, 0);
  delay(2000);
  ESP.restart();
}
//-------------------------------------------------ClientConfigIsNotDone----------------------------------
void NoConfigData() {
  digitalWrite(pin_led_red, 0);
  Serial.println("No data is Not Done");
  delay(2000);
  ApConfig.setDoneCallback(&ClientConfigIsDone);
  ApConfig.Begin();
}
WiFiClientSecure net;
PubSubClient client(net);
//-------------------------------------------------setup---------------------------------------------------
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  WiFi.mode(WIFI_MODE_STA);
  Serial.begin(115200);



  Serial.println("Booting");
  //-----------------sensors_setup---------------------
  pinMode(pin_temperature_power_1, OUTPUT);
  pinMode(pin_temperature_power_2, OUTPUT);

  pinMode(pin_dht11_power, OUTPUT);

  pinMode(pin_light_data, INPUT);
  pinMode(pin_light_power, OUTPUT);

  pinMode(pin_ec1_data, INPUT);
  pinMode(pin_ec2_data, INPUT);
  pinMode(pin_ec3_data, INPUT);
  pinMode(pin_ec_power, OUTPUT);

  pinMode(pin_reset, INPUT);
  // pinMode(pin_Active_led, OUTPUT);
  pinMode(pin_led_red, OUTPUT);
  pinMode(pin_led_blue, OUTPUT);
  pinMode(pin_voltage, INPUT);

  digitalWrite(pin_led_red, 1);   digitalWrite(pin_led_blue, 1);
  delay(1000);
  digitalWrite(pin_led_red, 0);   digitalWrite(pin_led_blue, 0);
  sensors.begin();
  dht.begin();
  analogSetPinAttenuation(pin_light_data, ADC_11db);
  analogSetPinAttenuation(pin_ec1_data, ADC_11db);
  analogSetPinAttenuation(pin_ec2_data, ADC_11db);
  analogSetPinAttenuation(pin_ec3_data, ADC_11db);
  ConfigManager.setEmptyCallback(&NoConfigData);
  ConfigManager.Read();
  delay(1000);
  Serial.println("Read Config");
  if (digitalRead(pin_reset) == 0) {
    digitalWrite(pin_led_red, 1);
    Serial.println("Reset Button");
    NoConfigData();
  }
  strcpy(Config.mqtt_server, "mqtt.csmju.com");
  Serial.println("Config Data");
  Serial.print("wifi ssid");
  Serial.println(Config.wifi_ssid);
  Serial.print("wifi password");
  Serial.println(Config.wifi_password);
  Serial.print("mqtt server");
  Serial.println(Config.mqtt_server);
  Serial.print("mqtt user");
  Serial.println(Config.mqtt_user);
  Serial.print("mqtt password");
  Serial.println(Config.mqtt_password);

  Serial.println(client.setBufferSize(1024));
  InitiateNetworkconnection();
}
//-------------------------------------------------loop---------------------------------------------------

void loop() {
  digitalWrite(pin_led_blue, 1);
  SensorsDataStruct S = ReadSensors() ;
  S.Timestamp = time(nullptr);
  charging = (S.light < 50) ? false : true;
  lowbatt = (S.voltage < vLow) ? true : false;
  sprintf(device_data,
          "Timestamp: %d\nEc1 :%d\nEc2 :%d\nEc3 :%d\nDHT_temperature :%.3f\nDHT_humidity :%.3f\n"
          "DS_temperature1 :%.3f\nnDS_temperature2 :%.3f\nlight :%.3f\nVoltage :%f\n"
          , S.Timestamp, S.Ec1 , S.Ec2 , S.Ec3 , S.DHT_temperature
          , S.DHT_humidity , S.DS_temperature1 , S.DS_temperature2 , S.light, S.voltage);
  Serial.print(device_data);
  Q.push(&S);
  Q.peek(&S);
  Serial.print("Queue lenght:");
  Serial.println(Q.getCount());
  Serial.print("Peek");
  Serial.println(S.Timestamp);
  digitalWrite(pin_led_blue, 0);
  long sleeptime = (lowbatt) ? (60 * 30) : (60 * 3); //60*30 60*5
  if (abs(time(nullptr) - prev_broadcast_time) > sleeptime) {
    BoardcastSensorData();
  }
  delay(500);
  if (charging == false) {
    esp_sleep_enable_timer_wakeup(1000000 * (Config.sleep_interval * 2));
  } else {
    esp_sleep_enable_timer_wakeup(1000000 * (Config.sleep_interval));
  }
  if (lowbatt)esp_sleep_enable_timer_wakeup(1000000 * (Config.sleep_interval * 10));
  int ret = esp_light_sleep_start();
}
//-------------------------------------------------SensorsDataStruct ReadSensors----------------------------------
SensorsDataStruct ReadSensors () {
  SensorsDataStruct S;
  Serial.println("Sensors Read!!");
  //------------Light_Sensor------------
  Serial.println("Light");
  digitalWrite(pin_light_power , 1);
  delay(500);
  S.light =((double)analogRead(pin_light_data));
  digitalWrite(pin_light_power , 0);
  //------------DHT11_Sensor------------
  digitalWrite(pin_dht11_power , 1);
  delay(1500);
  Serial.println("DHT11");
  S.DHT_humidity = dht.readHumidity();
  S.DHT_temperature = dht.readTemperature();
  S.DHT_temperature = isnan(S.DHT_temperature) ? -127 : S.DHT_temperature;
  S.DHT_humidity = isnan(S.DHT_humidity) ? -127 : S.DHT_humidity;
  digitalWrite(pin_dht11_power , 0);
  //------------Temp_Dallas------------
  Serial.println("Temp_Dallas_1");
  digitalWrite(pin_temperature_power_1 , 1);
  delay(500);
  S.DS_temperature1 = 0;
  for (int i = 0; i < 3; i++) {
    sensors.requestTemperatures();
    S.DS_temperature1 = sensors.getTempCByIndex(0);
    delay(10);
  }
  digitalWrite(pin_temperature_power_1 , 0);

  Serial.println("Temp_Dallas_2");
  digitalWrite(pin_temperature_power_2 , 1);
  delay(500);
  S.DS_temperature2 = 0;
  for (int i = 0; i < 3; i++) {
    sensors.requestTemperatures();
    S.DS_temperature2 = sensors.getTempCByIndex(0);
    delay(10);
  }
  digitalWrite(pin_temperature_power_2 , 0);
  //------------EC------------
  Serial.println("EC");
  digitalWrite(pin_ec_power , 1);
  delay(1000);
  double valEC1 = 0.0;
  double valEC2 = 0.0;
  double valEC3 = 0.0;
  for (int i = 0; i < 20; i++) {
    valEC1 = valEC1 + (((double)analogRead(pin_ec1_data)) / 20.0);
    valEC2 = valEC2 + (((double)analogRead(pin_ec2_data)) / 20.0);
    valEC3 = valEC3 + (((double)analogRead(pin_ec3_data)) / 20.0);
  }
  S.Ec1 = map(valEC1, 4095, 2850, 0, 100);
  S.Ec2 = map(valEC2, 4095, 2850, 0, 100);
  S.Ec2 = map(valEC3, 4095, 2850, 0, 100);

  S.Ec1 = constrain(map(round(valEC1), EC_reference_dry, EC_reference_wet , 0, 100), 0, 100);
  S.Ec2 = constrain(map(round(valEC2), EC_reference_dry, EC_reference_wet , 0, 100), 0, 100);
  S.Ec3 = constrain(map(round(valEC3), EC_reference_dry, EC_reference_wet , 0, 100), 0, 100);

  digitalWrite(pin_ec_power , 0);
  //------------battery------------
  Serial.println("Battery Voltage");
  S.voltage = ((double)analogRead(pin_voltage) / 4095.0) * 7.362;

  Serial.println("Read sensors is Done");
  return S;
}
//-------------------------------------------------receivedCallback----------------------------------
void receivedCallback() {

}
//-------------------------------------------------mqtt_connect----------------------------------
void mqtt_connect() {
  int timeout = 0;
  int giveup = 0;
  client.setSocketTimeout (3);
  while (!client.connected()) {
    Serial.println("MQTT connecting...");
    if (client.connect(Config.mqtt_server, Config.mqtt_user, Config.mqtt_password)) {
      Serial.println("MQTT connected");
      client.subscribe(mqtt_rx_topic);
      client.publish(mqtt_sync_topic, "WakeUp", false);
      client.loop();
    } else {
      timeout++;
      if (timeout > 60) {
        ESP.restart();
      }
      Serial.println("MQTT Connection Try again");
      delay(1000);
    }
  }
}
//-------------------------------------------------wifi_on----------------------------------
void wifi_on() {
  digitalWrite(pin_led_red, 1);
  if (WiFi.status() == WL_CONNECTED)return ;
  WiFi.mode(WIFI_MODE_STA);
  Serial.println("Turn on radio");
  strcpy(clientID, WiFi.macAddress().c_str());
  Serial.print("Mac Address ");
  Serial.println(clientID);
  sprintf(mqtt_tx_topic , "kawaii/clients/%s/tx" , clientID);
  sprintf(mqtt_rx_topic , "kawaii/clients/%s/rx" , clientID);
  sprintf(mqtt_sync_topic , "kawaii/clients/%s/sync" , clientID);
  WiFi.setHostname(clientID);
  //  WiFi.begin("Thalathai_com", "151617211");

  WiFi.begin(Config.wifi_ssid, Config.wifi_password );
  Serial.println("WiFi_Connecting...");
  int timeout = 0;
  int giveup = 0;
  while (WiFi.status() != WL_CONNECTED) {
    timeout++;
    Serial.print("watting...");
    Serial.println(timeout);
    delay(1000);
    if (timeout > 60) {
      esp_sleep_enable_timer_wakeup(1000000 * 60 * 10);
      int ret = esp_light_sleep_start();
      Serial.println("Reboot!");
      delay(1000);
      ESP.restart();
    }
  }
  SetNTP();
  Serial.println("Wifi_Connected!");
}
//-----------------------------------wifi_off------------------------------------------------
void wifi_off() {
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  digitalWrite(pin_led_red, 0);
  Serial.println("Turn off radio");
}
//-----------------------------------mqtt_on------------------------------------------------
void mqtt_on() {
  while (!client.connected()) {
    wifi_on();
    mqtt_connect();
  }
}
//-----------------------------------mqtt_off------------------------------------------------
void mqtt_off() {
  client.disconnect();
  Serial.println("MQTT_Disconnected!");
  while (client.connected()) {
    Serial.print("+");
    delay(10);
  }
}
//-----------------------------------Set_NTP------------------------------------------------
void SetNTP() {
  configTime(-5 * 3600, 0, "mqtt.csmju.com", "time.nist.gov");
  timenow = time(nullptr);
  int timeout = 0;
  while (timenow < 1510592825) {
    delay(500);
    timeout++;
    timenow = time(nullptr);
    if (timeout > (500 * 2 * 60)) {
      esp_sleep_enable_timer_wakeup(1000000 * 60 * 20);
      int ret = esp_light_sleep_start();
      timeout = 0;
      ESP.restart();
    }
    Serial.print(".");
    timenow = time(nullptr);
  }
}
//-----------------------------------InitiateNetworkconnection------------------------------------------------
void InitiateNetworkconnection() {
  wifi_on();
  digitalWrite(pin_led_red, 0);
  struct tm timeinfo;
  gmtime_r(&timenow , &timeinfo);
  Serial.print("Current time :");
  Serial.println(asctime(&timeinfo));
  client.setServer(Config.mqtt_server, 8883);
  //    client.setCallback(receivedCallback);
  mqtt_connect();
  client.publish(mqtt_sync_topic, "Boot", false);
  delay(1000);
  mqtt_off();
  wifi_off();
}
//-------------------------------------------------BoardcastSensorData----------------------------------
void BoardcastSensorData() {
  Serial.print("BoardcastSensorData");
  wifi_on();
  mqtt_connect();
  SensorsDataStruct S;
  broadcast_done = false;
  while (!Q.isEmpty()) {
    client.loop();
    if (client.connected()) {
      Q.pop(&S);
      sprintf(device_data, "%d,%d,%d,%d,%f,%f,%f,%f,%f", S.Timestamp, S.Ec1 , S.Ec2 , S.Ec3 , S.DHT_temperature
              , S.DHT_humidity , S.DS_temperature1 , S.DS_temperature2 , S.light);
      while (!client.publish(mqtt_tx_topic, device_data, false)) {
        client.loop();
      };
      client.loop();
      Serial.println(device_data);
      delay(10);
    }
  }
  Q.clean();
  client.loop();
  client.publish(mqtt_sync_topic, "ESP_sleep", false);
  client.loop();
  long  prev_time = millis();
  Serial.println("Wait for EOF");
  while (broadcast_done == false) {
    client.loop();
    delay(10);
    if (abs(millis() - prev_time) > 5000) {
      Serial.println("Time Out");
      break;
    }
  };
  Serial.println("Broadcast is Done!");
  mqtt_off();
  wifi_off();
  Serial.println("########################");
  prev_broadcast_time = time(nullptr);
}
