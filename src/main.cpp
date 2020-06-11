#include <Arduino.h>
#include "WiFi.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#define PWM_FREQ 32000
#define PWM_RESOLUTION 8
#define WLAN_SSID "SSID"
#define WLAN_PASS "123456"
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "admin"
#define AIO_KEY "aio_xxxxx"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe LED_Control = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/rgb-light-feed");
void MQTT_connect();
void setupPWM(int pin);
void analogWrite(int pin, uint8_t val);
String get_color_string(String var);
void change_color(const String &var);

String color;
int Led_Green = 27;
int Led_Red = 14;
int Led_Blue = 32;
int _adc = 0;
int ADC_MAP[48] = {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1};

void setup()
{
  Serial.begin(115200);
  delay(10);
  pinMode(Led_Red, OUTPUT);
  pinMode(Led_Green, OUTPUT);
  pinMode(Led_Blue, OUTPUT);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  mqtt.subscribe(&LED_Control);
}

void loop()
{
  MQTT_connect();
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    if (subscription == &LED_Control)
    {
      Serial.print(F("Got: "));
      color = get_color_string((char *)LED_Control.lastread);
      Serial.println(color);
    }
  }
  change_color(color);
}

void MQTT_connect()
{
  int8_t ret;
  if (mqtt.connected())
  {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0)
    {
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected!");
}

void setupPWM(int pin)
{
  pinMode(pin, OUTPUT);
#ifdef USE_SIGMADELTA
  sigmaDeltaSetup(_adc, PWM_FREQ);
  sigmaDeltaAttachPin(pin, _adc);
#else
  ledcSetup(_adc, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(pin, _adc);
#endif
  ADC_MAP[pin] = _adc;

  _adc = _adc + 1;
}

void analogWrite(int pin, uint8_t val)
{
  if (ADC_MAP[pin] == -1)
    setupPWM(pin);
#ifndef USE_SIGMADELTA
  ledcWrite(ADC_MAP[pin], val);
#else
  sigmaDeltaWrite(ADC_MAP[pin], val);
#endif
}

String get_color_string(String var)
{
  int i = var.length() - 1;
  for (; i >= 0; i--)
  {
    if (var[i] == ' ')
    {
      i++;
      break;
    }
  }
  if (i < 0)
    i = 0;
  for (int j = i; j < var.length() - 1; j++)
  {
    if (var[j] >= 'A' && var[j] <= 'Z')
      var[j] = var[j] - 'A' + 'a';
  }
  return var.substring(i);
}

void change_color(const String &var)
{
  if (!strcmp(var.c_str(), "red"))
  {
    analogWrite(Led_Red, 255);
    analogWrite(Led_Blue, 0);
    analogWrite(Led_Green, 0);
    return;
  }
  if (!strcmp(var.c_str(), "blue"))
  {
    analogWrite(Led_Red, 0);
    analogWrite(Led_Blue, 255);
    analogWrite(Led_Green, 0);
    return;
  }
  if (!strcmp(var.c_str(), "green"))
  {
    analogWrite(Led_Red, 0);
    analogWrite(Led_Blue, 0);
    analogWrite(Led_Green, 255);
    return;
  }
  if (!strcmp(var.c_str(), "green"))
  {
    analogWrite(Led_Red, 0);
    analogWrite(Led_Blue, 0);
    analogWrite(Led_Green, 255);
    return;
  }
  if (!strcmp(var.c_str(), "rainbow"))
  {
    int val;
    for (val = 255; val > 0; val--)
    {
      analogWrite(Led_Red, val);
      analogWrite(Led_Blue, 255 - val);
      analogWrite(Led_Green, 128 - val);
      delay(15);
    }
    for (val = 0; val < 255; val++)
    {
      analogWrite(Led_Red, val);
      analogWrite(Led_Blue, 255 - val);
      analogWrite(Led_Green, 128 - val);
      delay(15);
    }
    return;
  }
  if (!strcmp(var.c_str(), "off"))
  {
    analogWrite(Led_Red, 0);
    analogWrite(Led_Blue, 0);
    analogWrite(Led_Green, 0);
    return;
  }
  return;
}