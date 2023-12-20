#define BLYNK_PRINT Serial

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define BLYNK_TEMPLATE_ID "TMPL6EPtAc94-"
#define BLYNK_DEVICE_NAME "monitoring kualitas air"
#define BLYNK_AUTH_TOKEN "_jzTa-zLpFFw5OYdRLEd0yFDBRA_UaXz"
char auth[] = BLYNK_AUTH_TOKEN;
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Edge";
char pass[] = "terserahh";
//char ssid[] = "ALJAZARI";
//char pass[] = "01122018";




#include <OneWire.h>
#include <DallasTemperature.h>

const int oneWireBus = 4; // GPIO where the DS18B20 is connected to
OneWire oneWire(oneWireBus);    // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensor_ds18b20(&oneWire);    // Pass our oneWire reference to Dallas nilai_suhu sensor





const int pH_Sensor_Pin = 32;
const float adc_resolution = 4096.0;
float nilai_pH;



const int Tds_Sensor_Pin = 35;
const float VREF = 3.3;         // analog reference voltage_ph(Volt) of the ADC
const int SCOUNT = 30;          // sum of sample point 
int analogBuffer[SCOUNT];       // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averagevoltage_ph = 0;
float nilai_TDS = 0;
float nilai_suhu = 0;

#include <RunningMedian.h>
RunningMedian samples_voltage_ph = RunningMedian(SCOUNT);


void setup()
{
  Serial.begin(115200);

  pinMode(pH_Sensor_Pin, INPUT);
  pinMode(Tds_Sensor_Pin, INPUT);
  sensor_ds18b20.begin();

  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH); // LED ON
  
  Serial.print("Initializing... Blynk");
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  Serial.println("Connected"); 

  digitalWrite(2, LOW);   // LED OFF
}


float ph (float voltage_ph)
{
  float calibration_offset_adjusment = 0;
  //float pH_step = 0.9;
  float PH7_5 = 2.35; // tegangan saat di PH 7,3 dari ph tester itu tegagannya 2,30 volt
  float PH2_8 = 3.30; // tegangan saat di PH 2,9 dari ph tester itu tegagannya 3,20 volt
  //float pH_step = (PH4 - PH7) / 3; tegangan ph saat di ph 4 dan saat di ph 7 lalu dibagi 3 karena 7-4 = 3
  float pH_step = (PH2_8 - PH7_5) / (7.5-2.8); // referensi kalibrasi pH https://www.youtube.com/watch?v=3_w4GhrEoQ8
  float pH = 7 + ((2.50 - voltage_ph) / pH_step) + calibration_offset_adjusment;
  return pH;
}

 
void loop()
{ 
  // baca suhu
  sensor_ds18b20.requestTemperatures();
  nilai_suhu = sensor_ds18b20.getTempCByIndex(0);
  
  // baca TDS
  while(1)
  {
    int nilai_ADC_pH = analogRead(pH_Sensor_Pin);
    float voltage_ph = (3.3 / 4096.0) * nilai_ADC_pH; // untuk ESP32 w/ resolusi_ADC  4096
    //float voltage_ph = (3.3 / 1024.0) * nilai_ADC_pH; // untuk ESP8266
    //float voltage_ph = (5.0 / 1024.0) * nilai_ADC_pH; // untuk Arduino Uno/ NANO    
    samples_voltage_ph.add(voltage_ph);
    float v = samples_voltage_ph.getAverage();
    nilai_pH = ph(v);

    
        
    analogBuffer[analogBufferIndex] = analogRead(Tds_Sensor_Pin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
    {
      analogBufferIndex = 0;

      for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      {
        analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      }

      // referensi: https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
      // jika pake NodeMCU / arduino 1024. tapi jika pake ESP32 pake 4096.0
      averagevoltage_ph = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage_ph value
      float compensationCoefficient = 1.0 + 0.02 * (nilai_suhu - 25.0); //nilai_suhu compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge = averagevoltage_ph / compensationCoefficient; //nilai_suhu compensation
      nilai_TDS = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
      //nilai_TDS += 45; // adjusment
      break;
    }
    else
      delay(5);
  }

  // send data to serial monitor
  Serial.print("Suhu: ");
  Serial.print(nilai_suhu);
  Serial.print("ºC");
  Serial.print("\t");
  
  Serial.print("TDS: ");
  Serial.print(nilai_TDS, 0);
  Serial.print("ppm");
  Serial.print("\t");

  Serial.print("pH: ");
  Serial.print(nilai_pH);
  
  Serial.print("\n");


  //send data to blynk
  Blynk.run(); // jalankan Blynk server
  Blynk.virtualWrite(V0, nilai_suhu);
  Blynk.virtualWrite(V1, nilai_pH);
  Blynk.virtualWrite(V2, nilai_TDS);
}


int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}
