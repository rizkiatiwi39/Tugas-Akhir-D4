#include <GravityTDS.h>
#include <EEPROM.h>
#include <time.h>
#include  "WiFi.h" 
#include <OneWire.h> 
#include <DallasTemperature.h>

//Uncomment untuk ganti device
#define device1
//#define device2
//#define device3

#define adc1 33
#define adc2 35
#define ONE_WIRE_BUS 26

OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);
WiFiClient client;
GravityTDS gravityTds;

const char* ssid = "realme C2";   // your network SSID (name) 
const char* password = "makasihyaaa";   // your network password

#ifdef device1
float kValue = 1.08;
#endif

#ifdef device2
float kValue = 1.05;
#endif

#ifdef device3
float kValue = 1.38;
#endif



String thingSpeakAddress = "api.thingspeak.com";
String writeAPIKey = "E10B160ZCFRPP0OQ";
String tsfield1Name;
String request_string;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 25200;
const int   daylightOffset_sec = 0;


float temperature = 30,tdsValue = 0;
 
void setup()
{
  Serial.begin(115200);
    
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wifi Connecting");
      delay(100);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  gravityTds.setPin(adc1);
  gravityTds.setAref(3.3);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(4096);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.setKvalue(kValue);
  gravityTds.begin();  //initialization
    
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

float test[5];

void loop()
{    
    float adcin;

    for (int i = 0;i < 5; i++) {
      adcin += analogRead(adc2);
    }
    adcin/=5.0;
        
    float PH = readPH(adcin);
    Serial.print(PH);
    Serial.println("PH");
    
    readTds();
    kirim_thingspeak(temperature,PH,tdsValue);
    
    
    printLocalTime();
    delay(1000);
}


float readPH(int adc){
  float voltage = adc*3.3/4096;
  //return 7+((2.5-voltage)/0.18);  
  //return -5.70 * voltage + calibration_value;
  float output = -5.47169 * voltage + 20.56981;
  return output;
}


void readTds(){
   sensors.requestTemperatures();
   temperature = sensors.getTempCByIndex(0);  //add your temperature sensor and read it
   float kval = gravityTds.getKvalue();
   gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
   gravityTds.update();  //sample and calculate
   tdsValue = gravityTds.getTdsValue();  // then get the value
   Serial.print(temperature);
   Serial.print(" celcius - ");
   Serial.print(tdsValue);
   Serial.print("ppm - ");
   Serial.print(kval);
   Serial.println(" kval");
   
}

void kirim_thingspeak(float par1, float par2 ,float par3) {
  if (client.connect("api.thingspeak.com", 80)) {
    request_string = "/update?";
    request_string += "key=";
    request_string += writeAPIKey;
    request_string += "&";
    
    #ifdef device1
    request_string += "field1";
    request_string += "=";
    request_string += par1;//SUHU
    request_string += "&";
    request_string += "field2";
    request_string += "=";
    request_string += par2;//PH
    request_string += "&";
    request_string += "field3";
    request_string += "=";
    request_string += par3;//PPM
    #endif
    
    #ifdef device2
    request_string += "field4";
    request_string += "=";
    request_string += par1;
    request_string += "&";
    request_string += "field5";
    request_string += "=";
    request_string += "0.0";
    request_string += "&";
    request_string += "field6";
    request_string += "=";
    request_string += par3;
    #endif
    
    #ifdef device3
    request_string += "field7";
    request_string += "=";
    request_string += par1;
    request_string += "&";
    request_string += "field8";
    request_string += "=";
    request_string += par3;
    #endif
    
    Serial.println(String("GET ") + request_string + " HTTP/1.1\r\n" +
                 "Host: " + thingSpeakAddress + "\r\n" +
                 "Connection: close\r\n\r\n");
                 
    client.print(String("GET ") + request_string + " HTTP/1.1\r\n" +
                 "Host: " + thingSpeakAddress + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    while (client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    Serial.println();
    Serial.println("closing connection");

  }
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  int tmsec = timeinfo.tm_sec;
  int tmmin = timeinfo.tm_min;
  int tmhour = timeinfo.tm_hour;
  char buffer[30];
  sprintf (buffer, "%d:%d:%d",tmhour,tmmin,tmsec);
  Serial.println(buffer);
  Serial.println();
}
