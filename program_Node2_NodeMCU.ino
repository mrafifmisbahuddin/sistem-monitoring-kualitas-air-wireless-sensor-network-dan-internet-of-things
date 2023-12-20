#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

#define BLYNK_TEMPLATE_ID "TMPLu4EYgb5F"
#define BLYNK_DEVICE_NAME "Monitoring Kualitas Air Tambak Udang"
#define BLYNK_AUTH_TOKEN "zro62wr7OP732Mn1WzLA9a75fRzab7H4"

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = BLYNK_AUTH_TOKEN;


// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Edge";
char pass[] = "terserahh";
//char ssid[] = "ALJAZARI";
//char pass[] = "01122018";

BLYNK_WRITE(V3) 
{
  int status_tombol = param.asInt();
  Serial.print("status tombol: ");
  Serial.println(status_tombol); 

  if(status_tombol == 0)
  {
    digitalWrite(D0, LOW);
  }
  if(status_tombol == 1)
  {
    digitalWrite(D0, HIGH);
  }
}

BLYNK_CONNECTED() 
{
  // Synchronize time on connection
  Blynk.syncVirtual(V3);
}


void setup()
{
  Serial.begin(115200);

  pinMode(D0, OUTPUT); // relay kincir air
  digitalWrite(D0, HIGH); // Relay OFF
  
  pinMode(D4, OUTPUT);
  digitalWrite(D4, LOW); // LED ON
  
  // inisialisasi Blynk
  Serial.print("Initializing... Blynk");  
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  Serial.println("Connected");
  
  digitalWrite(D4, HIGH); // LED OFF
}

void loop()
{
  Blynk.run(); // jalankan Blynk server
  delay(100);
}
