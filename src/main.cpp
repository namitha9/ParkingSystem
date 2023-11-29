#include "Wifi.h"
#include "SPIFFS.h"
#include <WifiClientSecure.h>
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

const int IR_Sensor1 = 34;
const int IR_Sensor2 = 35;
const int IR_Sensor3 = 26;
const int IR_Sensor4 = 18;

volatile uint64_t count1 = 0;
volatile uint64_t currentCount1 = -1;

volatile uint64_t count2 = 0;
volatile uint64_t currentCount2 = -1;

volatile uint64_t count3 = 0;
volatile uint64_t currentCount3 = -1;

volatile uint64_t count4 = 0;
volatile uint64_t currentCount4 = -1;

unsigned long currentTime1 = 0;
unsigned long lastReadTime1 = 0;

unsigned long currentTime2 = 0;
unsigned long lastReadTime2 = 0;

unsigned long currentTime3 = 0;
unsigned long lastReadTime3 = 0;

unsigned long currentTime4 = 0;
unsigned long lastReadTime4 = 0;

unsigned int intervalDelay = 500;

const char* ssid = "Namitha iPone";
const char* password = "namitha9";
const char* mqtt_server = "a1tpt3xh52itq4-ats.iot.ap-southeast-1.amazonaws.com";
const int mqtt_port = 8883;

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

String Read_rootca;
String Read_cert;
String Read_privatekey;

#define BUFFER_LEN 256
long lastMsg = 0;
char msg[BUFFER_LEN];
int Value = 0;
byte mac[6];
char mac_Id[18];

void IRAM_ATTR isr1_rising()
{
  currentTime1 = millis();
  // IR Sensor is noisy so we add a debounce mechanism here
  if (currentTime1 - lastReadTime1 > intervalDelay)
  {
    count1++;
    lastReadTime1 = currentTime1;
  }
}

void IRAM_ATTR isr1_falling()
{
  currentTime1 = millis();
  // IR Sensor is noisy so we add a debounce mechanism here
  if (currentTime1 - lastReadTime1 > intervalDelay)
  {
    count1++;
    lastReadTime1 = currentTime1;
  }
}

void IRAM_ATTR isr2_rising()
{
  currentTime2 = millis();
  // IR Sensor is noisy so we add a debounce mechanism here
  if (currentTime2 - lastReadTime2 > intervalDelay)
  {
    count2++;
    lastReadTime2 = currentTime2;
  }
}

void IRAM_ATTR isr2_falling()
{
  currentTime2 = millis();
  // IR Sensor is noisy so we add a debounce mechanism here
  if (currentTime2 - lastReadTime2 > intervalDelay)
  {
    count2++;
    lastReadTime2 = currentTime2;
  }
}

void IRAM_ATTR isr3_rising()
{
  currentTime3 = millis();
  // IR Sensor is noisy so we add a debounce mechanism here
  if (currentTime3 - lastReadTime3 > intervalDelay)
  {
    count3++;
    lastReadTime3 = currentTime3;
  }
}

void IRAM_ATTR isr3_falling()
{
  currentTime3 = millis();
  // IR Sensor is noisy so we add a debounce mechanism here
  if (currentTime3 - lastReadTime3 > intervalDelay)
  {
    count3++;
    lastReadTime3 = currentTime3;
  }
}

void IRAM_ATTR isr4_rising()
{
  currentTime4 = millis();
  // IR Sensor is noisy so we add a debounce mechanism here
  if (currentTime4 - lastReadTime4 > intervalDelay)
  {
    count4++;
    lastReadTime4 = currentTime4;
  }
}

void IRAM_ATTR isr4_falling()
{
  currentTime4 = millis();
  // IR Sensor is noisy so we add a debounce mechanism here
  if (currentTime4 - lastReadTime4 > intervalDelay)
  {
    count4++;
    lastReadTime4 = currentTime4;
  }
}

void setup_wifi() {
  delay(10);
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("ei_out", "hello world");
      // ... and resubscribe
      client.subscribe("ei_in");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// Check your I2C LCD Address
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Function to print to LCD
void printToLCD(const String &message, uint8_t column, uint8_t row, bool isClear)
{
  if (isClear)
  {
    lcd.clear();
  }
  // set cursor to  column,  row
  lcd.setCursor(column, row);
  if (message.length() == 0)
  {
    lcd.setCursor(0, 1);
    for (int n = 0; n < 16; n++)
    {
      lcd.print(" ");
    }
  }
  else
  {
    lcd.print(message);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(IR_Sensor1, INPUT);
  pinMode(IR_Sensor2, INPUT);
  pinMode(IR_Sensor3, INPUT);
  pinMode(IR_Sensor4, INPUT);

  attachInterrupt(digitalPinToInterrupt(IR_Sensor1), isr1_rising, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_Sensor1), isr1_falling, FALLING);
  attachInterrupt(digitalPinToInterrupt(IR_Sensor2), isr2_rising, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_Sensor2), isr2_falling, FALLING);
  attachInterrupt(digitalPinToInterrupt(IR_Sensor3), isr3_rising, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_Sensor3), isr3_falling, FALLING);
  attachInterrupt(digitalPinToInterrupt(IR_Sensor4), isr4_rising, RISING);
  attachInterrupt(digitalPinToInterrupt(IR_Sensor4), isr4_falling, FALLING);

  setup_wifi();
  Wire.begin();

  delay(1000);
  //=============================================================
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //=======================================
  //Root CA File Reading.
  File file2 = SPIFFS.open("/AmazonRootCA1.pem", "r");
  if (!file2) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("Root CA File Content:");
  while (file2.available()) {
    Read_rootca = file2.readString();
    Serial.println(Read_rootca);
  }
  //=============================================
  // Cert file reading
  File file4 = SPIFFS.open("/client.crt", "r");
  if (!file4) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("Cert File Content:");
  while (file4.available()) {
    Read_cert = file4.readString();
    Serial.println(Read_cert);
  }
  //=================================================
  //Privatekey file reading
  File file6 = SPIFFS.open("/client.key", "r");
  if (!file6) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("privateKey File Content:");
  while (file6.available()) {
    Read_privatekey = file6.readString();
    Serial.println(Read_privatekey);
  }
  //=====================================================

  char* pRead_rootca;
  pRead_rootca = (char *)malloc(sizeof(char) * (Read_rootca.length() + 1));
  strcpy(pRead_rootca, Read_rootca.c_str());

  char* pRead_cert;
  pRead_cert = (char *)malloc(sizeof(char) * (Read_cert.length() + 1));
  strcpy(pRead_cert, Read_cert.c_str());

  char* pRead_privatekey;
  pRead_privatekey = (char *)malloc(sizeof(char) * (Read_privatekey.length() + 1));
  strcpy(pRead_privatekey, Read_privatekey.c_str());

  Serial.println("================================================================================================");
  Serial.println("Certificates that passing to espClient Method");
  Serial.println();
  Serial.println("Root CA:");
  Serial.write(pRead_rootca);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("Cert:");
  Serial.write(pRead_cert);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("privateKey:");
  Serial.write(pRead_privatekey);
  Serial.println("================================================================================================");

  net.setCACert(pRead_rootca);
  net.setCertificate(pRead_cert);
  net.setPrivateKey(pRead_privatekey);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //====================================================================================================================
  WiFi.macAddress(mac);
  snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(mac_Id);
  //=====================================================================================================================
  delay(2000);
  delay(1000);
}
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.printf("Count1 :: %lld\r\n", count1);
  Serial.printf("Count2 :: %lld\r\n", count2);
  Serial.printf("Count3 :: %lld\r\n", count3);
  Serial.printf("Count4 :: %lld\r\n", count4);

  if (currentCount1 != count1)
  {
    currentCount1 = count1;
    char buffer[50];
    ltoa(count1, buffer, 10);
    String d1 = String(count1%2);
    snprintf (msg, BUFFER_LEN, "{\"availability\" : \"%s\"}", d1.c_str());
    Serial.print("Publish message: ");
    Serial.print(count1);
    Serial.println(msg);
    client.publish("carpark/0/availability", msg);
  }

  if (currentCount2 != count2)
  {
    currentCount2 = count2;
    char buffer[50];
    ltoa(count2, buffer, 10);
    String d2 = String(count2%2);
    snprintf (msg, BUFFER_LEN, "{\"availability\" : \"%s\"}", d2.c_str());
    Serial.print("Publish message: ");
    Serial.print(count2);
    Serial.println(msg);
    client.publish("carpark/1/availability", msg);
  }

  if (currentCount3 != count3)
  {
    currentCount3 = count3;
    char buffer[50];
    ltoa(count3, buffer, 10);
    String d3 = String(count3%2);
    snprintf (msg, BUFFER_LEN, "{\"availability\" : \"%s\"}", d3.c_str());
    Serial.print("Publish message: ");
    Serial.print(count3);
    Serial.println(msg);
    client.publish("carpark/2/availability", msg);
  }
  
  if (currentCount4 != count4)
  {
    currentCount4 = count4;
    char buffer[50];
    ltoa(count4, buffer, 10);
    String d4 = String(count4%2);
    snprintf (msg, BUFFER_LEN, "{\"availability\" : \"%s\"}", d4.c_str());
    Serial.print("Publish message: ");
    Serial.print(count4);
    Serial.println(msg);
    client.publish("carpark/3/availability", msg);
  }
}