//VARIABLES ESP32S
#include <SPI.h>
#include <OneWire.h>
#include <Wire.h>

//VARIABLES SIM800l(VCC:BAT/TX:4/RX:2/GND:GNDBAT Y GND)
#define rxPin 4
#define txPin 2
HardwareSerial sim800(1);
const String PHONE = "+541123890675";
String smsStatus,senderNumber,receivedDate,msg;
boolean isReply = false;

//VARIABLES GPS NEO6MV2(VCC:3,3V/RX:17/TX:16/GND:GND)
#include <TinyGPS++.h>
#define RXD2 16
#define TXD2 17
HardwareSerial neogps(2);
TinyGPSPlus gps;
int horas;
String ceroh = "";
int minutos;
String cerom = "";
int segundos;
String ceros = "";
int dias;
int meses;
int anos;

//VARIABLES SD (VCC:5V/CS:5/MOSI:23/CLK:18/MISO:19/GND:GND)
#include "FS.h"
#include <SD.h>
String Data;

//VARIABLES DHT22 (VCC:3,3V/OUT:0/GND:GND)
#include <DHT.h>
#define DHTPIN 0
DHT dht(DHTPIN, DHT22);
float h;
float t;
float f;

//VARIABLES BMP280(VCC:3,3V/GND:GND/SCL:22/SDA:21/SDO:GND)
#include <Adafruit_BMP280.h>
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)
Adafruit_BMP280 bmp;
float p;
float a;

//VARIABLES MQ135(VCC:3,3V/GND:GND Y 10K/A0:470/10K Y 470 UNIDOS EN LA SALIDA/LA SALIDA AL PIN 12)
#include <MQUnifiedsensor.h>
#define placa "ESP32 Dev Module"
#define Voltage_Resolution 3.3
#define pinm 12
#define typem "MQ-135"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6  
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pinm, typem);
double CO2 = (0);

void setup() {
//INICIACION DEL PUERTO SERIE Y EL ESP32S
  delay(5000);
  Serial.begin(115200);
  Serial.println("ESP32 INICIADO CORRECTAMENTE.");

  //INICIACION DEL SIM800l
  sim800.begin(9600, SERIAL_8N1, rxPin, txPin);
  Serial.println("SIM800L INICIADO CORRECTAMENTE.");
  smsStatus = "";
  senderNumber="";
  receivedDate="";
  msg="";
  sim800.println("AT+CMGF=1"); //SMS text mode
  delay(1000);
  sim800.println("AT+CMGD=1,4"); //delete all saved SMS
  delay(1000);

//INICIACION DEL GPS NEO6MV2
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial.println("NEO6MV2 INICIADO CORRECTAMENTE.");
  
//INICIACION DEL SD
  if(!SD.begin()){
        Serial.println("FALLO EN LA SD.");
        return;
    }
    uint8_t cardType = SD.cardType();
  if(cardType == CARD_NONE){
        Serial.println("NO DETECTA SD.");
        return;
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    
//INICIACION DEL DHT22
  dht.begin();

//INICIACION DEL BMP280
  unsigned status;
  status = bmp.begin();
  if (!status) {
    Serial.println(F("ALGO SALIO MAL CON EL BMP280"));
    Serial.println(bmp.sensorID(),16);
  while (1) delay(10);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

//INICIACION DEL MQ135
    MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b   
    MQ135.setA(110.47); 
    MQ135.setB(-2.862);  
    MQ135.init();    
    float calcR0 = 0;   
  for(int i = 1; i<=10; i ++)   {     
        MQ135.update();     
        calcR0 += MQ135.calibrate(RatioMQ135CleanAir);    
        }   
    MQ135.setR0(calcR0/10);   
  if(isinf(calcR0)) { Serial.println("CIRCUITO ABIERTO DEL MQ135."); while(1);}   
  if(calcR0 == 0){Serial.println("CORTOCIRCUITO DEL MQ135."); while(1);}
    MQ135.update();
}

void loop() {
//LLAMADO DE FUNCIONES
      OBTENER();
      DATA();
      COMUNICACION();
      delay(600000); 
}

void OBTENER(){
//MEDICION DE LOS DIVERSOS SENSORES
    p = bmp.readPressure();
    a = bmp.readAltitude();
    h = dht.readHumidity();
    t = dht.readTemperature();
    f = dht.readTemperature(true);
    CO2 = MQ135.readSensor();
    boolean newData = false;
    for (unsigned long start = millis(); millis() - start < 2000;)
    {
    while (neogps.available())
      {
      if (gps.encode(neogps.read()))
        {
        newData = true;
        }
      }
    }
  
//MUESTRA EN PANTALLA DE LAS MEDICIONES CORRESPONDIENTES
    Serial.println();
    Serial.println("PUERTO SERIE");
    Serial.print(gps.date.day());
    dias = (gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.month());
    meses = (gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
    anos = (gps.date.year());
    Serial.print(F(" "));  
    Serial.print("  ");
    if (gps.time.hour() < 10) 
    { 
    Serial.print(F("0"));
    ceroh = "0";
    }
    Serial.print(gps.time.hour()-3);
    Serial.print(F(":"));
    horas = (gps.time.hour()-3);
    if (gps.time.minute() < 10) 
    {
    Serial.print(F("0"));
    cerom = "0";
    }
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    minutos = (gps.time.minute());
    if (gps.time.second() < 10)
    {
    Serial.print(F("0"));
    ceros = "0";
    }
    Serial.println(gps.time.second());
    segundos = (gps.time.second());
    Serial.println();
    Serial.println("DATOS OBTENIDOS");
    Serial.print(F("Temperatura: "));
    Serial.print(t);
    Serial.println("*C");
    Serial.print(F("Humedad: "));
    Serial.print(h);
    Serial.println("%RH");
    Serial.print(F("Presion: "));
    Serial.print(p/100);
    Serial.println("hPa");
    Serial.print(F("Altitud: "));
    Serial.print(a);
    Serial.println("m");
    Serial.print(F("Calidad del aire(CO2): "));
    Serial.print(CO2+400);
    Serial.println("ppm");
    Serial.print("Velocidad de movimiento: ");  
    Serial.print(gps.speed.kmph());
    Serial.println("km/h");
    delay(300);
    Serial.println();
    Serial.println("UBICACION DEL DISPOSITIVO");
    Serial.print("Latitud: "); 
    Serial.print(gps.location.lat(), 6);
    Serial.print(" Longitud: "); 
    Serial.println(gps.location.lng(), 6);
    delay(300);
    Serial.print(F("Link: "));
    Serial.print("http://maps.google.com/maps?q=loc:");
    Serial.print(gps.location.lat(), 6);
    Serial.print(",");
    Serial.print(gps.location.lng(), 6);
    delay(100);
    Serial.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
    Serial.println();
    newData = false;
  }

void DATA() {
//GUARDADO EN EL SD
  Data = String(dias) + "/" + String(meses) + "/" + String(anos) + "   " + String(ceroh) + String(horas) + ":" + String(cerom) + String(minutos) + ":" + String(ceros) + String(segundos) + "\n" + "\n" + "DATOS OBTENIDOS" + "\n" + "Temperatura:" + String(t) + "*C" + "\n" + "Humedad:" + String(h) + "%RH" + "\n" + "Presion:" + String(p/100) + "hPa" + "\n" + "Altitud:" + String(a) + "m" + "\n" + "Calidad de Aire:" + String(CO2+400) + "ppm" + "\n" + "Velocidad de movimiento:" + String(gps.speed.kmph()) + "km/h" + "\n" + "\n" +"UBICACION DEL DISPOSITIVO" + "\n" + "Latitud:" + String(gps.location.lat(), 6) + " Longitud:" + String(gps.location.lng(), 6) + "\n" + "Link:" + "http://maps.google.com/maps?q=loc:" + String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + "\n";
  Serial.println();
  Serial.println("DATOS GUARDADOS: ");
  Serial.println(Data);
  DATOS(SD, "/DATOS.txt", Data.c_str());
  ceroh = "";
  cerom = "";
  ceros = "";
}

void DATOS(fs::FS &fs, const char * A, const char * MENSAJE) {
//CONTENIDO DEL SD
  Serial.printf("NUEVO ARCHIVO DE TEXTO GENERADO", A);
  File ARCHIVO = fs.open(A, FILE_APPEND);
  if(!ARCHIVO) {
    Serial.println();
    Serial.println("SALIO MAL ALGO RELACIONADO AL ARCHIVO DE TEXTO");
    return;
  }
  if(ARCHIVO.print(MENSAJE)) {
    Serial.println();
    Serial.println("DATOS AGREGADOS");
  } else {
    Serial.println();
    Serial.println("SALIO MAL ALGO RELACIONADO AL AGREGADO DE DATOS");
  }
  ARCHIVO.close();
}

void COMUNICACION()
{
//CONTENIDO DE LA COMUNICACION
    sim800.print("AT+CMGF=1\r");
    delay(1000);
    sim800.print("AT+CMGS=\""+PHONE+"\"\r");
    delay(1000);
    sim800.println("PROYECTO EN COMUNICACION");
    delay(100);
    sim800.print(gps.date.day());
    sim800.print(F("/"));
    sim800.print(gps.date.month());
    sim800.print(F("/"));
    sim800.print(gps.date.year());
    sim800.print(F(" "));  
    sim800.print("  ");
    if (gps.time.hour() < 10) sim800.print(F("0"));
    sim800.print(gps.time.hour()-3);
    sim800.print(F(":"));
    if (gps.time.minute() < 10) sim800.print(F("0"));
    sim800.print(gps.time.minute());
    sim800.print(F(":"));
    if (gps.time.second() < 10) sim800.print(F("0"));
    sim800.println(gps.time.second());
    delay(100);
    sim800.println();
    sim800.println("DATOS OBTENIDOS");   
    sim800.print(F("Temperatura: "));
    sim800.print(t);
    sim800.println("*C");
    sim800.print(F("Humedad: "));
    sim800.print(h);
    sim800.println("%RH");
    sim800.print(F("Presion: "));
    sim800.print(p/100);
    sim800.println("hPa");
    sim800.print(F("Altitud: "));
    sim800.print(a);
    sim800.println("m");
    sim800.print(F("Calidad del aire(CO2): "));
    sim800.print(CO2+400);
    sim800.println("ppm");
    sim800.print("Velocidad de movimiento: ");
    sim800.print(gps.speed.kmph());
    sim800.println("km/h");
    delay(100);
    sim800.println();
    sim800.println("UBICACION DEL DISPOSITIVO");
    sim800.print("Latitud: "); 
    sim800.print(gps.location.lat(), 6);
    sim800.println(" Longitud: "); 
    sim800.println(gps.location.lng(), 6);
    delay(300);
    sim800.print(F("Link:"));
    sim800.print("http://maps.google.com/maps?q=loc:");
    sim800.print(gps.location.lat(), 6);
    sim800.print(",");
    sim800.println(gps.location.lng(), 6);
    delay(100);
    sim800.write(0x1A); //ascii code for ctrl-26 //sim800.println((char)26); //ascii code for ctrl-26
    delay(1000);
}
