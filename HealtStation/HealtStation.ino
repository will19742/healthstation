//Proyecto creado por Bachillerato Desarrollo de Software

#include <SoftwareSerial.h>
#include "DHT.h"

#define DHTPIN 7

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

SoftwareSerial Bluetooth(10, 9);
String Data;
int pulsePin = 0;
int blinkPin = 13;
volatile int BPM;

volatile int Signal;

volatile int IBI = 600;

volatile boolean Pulse = false;

volatile boolean QS = false;

volatile int rate[10];                    
volatile unsigned long sampleCounter = 0;          
volatile unsigned long lastBeatTime = 0;           
volatile int P = 512;                     
volatile int T = 512;                     
volatile int thresh = 512;                
volatile int amp = 100;                   
volatile boolean firstBeat = true;        
volatile boolean secondBeat = false;      

void interruptSetup() {
  
  TCCR2A = 0x02;    
  TCCR2B = 0x06;   
  OCR2A = 0X7C;      
  TIMSK2 = 0x02;     
  sei();            
}



ISR(TIMER2_COMPA_vect) {                        
  cli();                                      
  Signal = analogRead(pulsePin);              
  sampleCounter += 2;                         
  int N = sampleCounter - lastBeatTime;       

  if (Signal < thresh && N > (IBI / 5) * 3) { 
    if (Signal < T) {                       
      T = Signal;                         
    }
  }

  if (Signal > thresh && Signal > P) {       
    P = Signal;                            
  }                                        


  if (N > 250) {                                  
    if ( (Signal > thresh) && (Pulse == false) && (N > (IBI / 5) * 3) ) {
      Pulse = true;                               
      digitalWrite(blinkPin, HIGH);               
      IBI = sampleCounter - lastBeatTime;        
      lastBeatTime = sampleCounter;               

      if (secondBeat) {                      
        secondBeat = false;                  
        for (int i = 0; i <= 9; i++) {       
          rate[i] = IBI;
        }
      }

      if (firstBeat) {                       
        firstBeat = false;                   
        secondBeat = true;                   
        sei();                               
        return;                              
      }


      
      word runningTotal = 0;                  

      for (int i = 0; i <= 8; i++) {          
        rate[i] = rate[i + 1];                
        runningTotal += rate[i];              
      }

      rate[9] = IBI;                          
      runningTotal += rate[9];                
      runningTotal /= 10;                     
      BPM = 60000 / runningTotal;             
      QS = true;                              
    }
  }

  if (Signal < thresh && Pulse == true) {  
    digitalWrite(blinkPin, LOW);           
    Pulse = false;                         
    amp = P - T;                           
    thresh = amp / 2 + T;                  
    P = thresh;                            
    T = thresh;
  }

  if (N > 2500) {                          
    thresh = 512;                          
    P = 512;                               
    T = 512;                               
    lastBeatTime = sampleCounter;          
    firstBeat = true;                      
    secondBeat = false;                    
  }

  sei();                                   
}

void setup() {
  Bluetooth.begin(9600);
  Serial.begin(9600);
  dht.begin();
  sensors.begin();
  interruptSetup();
}
 
void loop(){
  //Start of Program 
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  float hic = dht.computeHeatIndex(t, h, false);

  sensors.requestTemperatures();
  dht.readHumidity();
  dht.readTemperature();
  
  if (isnan(h) || isnan(t)) {
    return;
  }
 
    if (QS == true) {
 
  Serial.print(F(" T.Corp"));
  Serial.print(sensors.getTempCByIndex(0)); //Lectura del sensor temperatura corporal
  
  Serial.print(F("  Hum: "));
  Serial.print(h);                  //Lectura del sensor DHT11 humedad
  
  Serial.print(F("%  T.Amb: "));
  Serial.print(t);                 //Lectura del sensor DHT11 temperatura ambiental
  
  Serial.print(F("°C  I.Calor: "));
  Serial.print(hic);               //Lectura del sensor DHT11 sensación térmica 
  
  Serial.print(F("  BPM "));
  Serial.println(BPM);            //Lectura del sensor ritmo cardíaco

  Bluetooth.print(sensors.getTempCByIndex(0)); 
  Bluetooth.print(String(" °C | ")); 
  Bluetooth.print(h); 
  Bluetooth.print(String(" % | ")); 
  Bluetooth.print(t); 
  Bluetooth.print(String(" °C | ")); 
  Bluetooth.print(hic); 
  Bluetooth.print(String(" | "));
  Bluetooth.println(BPM);
  Bluetooth.print(String(" | "));


   QS = false;

  }

    
    delay(1500);//Wait 5 seconds before accessing sensor again.
 
  //Fastest should be once every two seconds.
 
}// end loop
