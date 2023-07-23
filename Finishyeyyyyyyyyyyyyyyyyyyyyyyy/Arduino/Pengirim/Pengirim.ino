#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include "MAX30105.h" //sparkfun MAX3010X library
#include "heartRate.h"
#include <SoftwareSerial.h>
SoftwareSerial mySerial(12, 11); //rx tx
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

MAX30105 particleSensor;
///////////////////////////////////////////////////////////////////////////////////
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
String dataSend = "";
int regresi, x;
int nilaiAngka;
double oxi;
#define USEFIFO

char customKey;
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
 
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal_I2C lcd(0x3F, 16, 2);//coba juga 0x27
 
char stringAngka[17];
int indexKeypad = 0;
 
void setup() {
  Serial.begin(9600);
  mySerial.begin(115200);
  mlx.begin();
  Serial.println();
 
  Wire.begin();
  Wire.beginTransmission(0x3F);
  if (Wire.endTransmission())
  {
    lcd = LiquidCrystal_I2C(0x27, 16, 2);
  }
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Masukkan");
  lcd.setCursor(0,1);
  lcd.print("No Identitas");

   // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) //Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30102 was not found. Please check wiring/power/solder jumper at MH-ET LIVE MAX30102 board. ");
    while (1);
  }
 
  //Setup to sense a nice looking saw tooth on the plotter
  //byte ledBrightness = 0x7F; //Options: 0=Off to 255=50mA
  byte ledBrightness = 70; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  //Options: 1 = IR only, 2 = Red + IR on MH-ET LIVE MAX30102 board
  int sampleRate = 200; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  // Set up the wanted parameters
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings
}
double avered = 0; double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
int i = 0;
int Num = 100;//calculate SpO2 by this sampling interval
 
double ESpO2 = 93.0;//initial value of estimated SpO2
double FSpO2 = 0.7; //filter factor for estimated SpO2
double frate = 0.95; //low pass filter for IR/red LED value to eliminate AC component
#define TIMETOBOOT 2000 // wait for this time(msec) to output SpO2
#define SCALE 88.0 //adjust to display heart beat and SpO2 in the same scale
#define SAMPLING 5 //if you want to see heart beat more precisely , set SAMPLING to 1
#define FINGER_ON 35000 // if red signal is lower than this , it indicates your finger is not on the sensor
#define MINIMUM_SPO2 0.0


void loop() {

 void keypd();
 
  //  baca permintaan
  String minta = "";
  while(mySerial.available()>0)
  {
    minta += char(mySerial.read());
  }

//  membuang space data yang di terima
  minta.trim();

  Serial.println("minta:" + minta);

//  uji variable
  if (minta == "y")
  {
    
    Sensor();
  }

  //Kosongkan variable minta
  minta = "";
  delay(100);

   customKey = customKeypad.getKey();
 
  if (customKey) {
    //Serial.println(customKey);
    switch (customKey)
    {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if (!indexKeypad)
        {
          lcd.clear();
        }
        stringAngka[indexKeypad++] = customKey;
        lcd.print(customKey);
        break;
      case '*'://reset
        lcd.clear();
        indexKeypad = 0;
        lcd.setCursor(0,0);
        lcd.print("Masukkan");
        lcd.setCursor(0,1);
        lcd.print("No Identitas");
        break;
      case '#':
        stringAngka[indexKeypad] = 0;
        lcd.setCursor(0, 1);
 
        int nilaiAngka = atoi(stringAngka);
        lcd.print(nilaiAngka);
 
        indexKeypad = 0;
        break;
    }
  }
  
  if(customKey == 'A'){
      lcd.clear();
      delay(1000);

      boolean stopLoop = false;
      while(!stopLoop){
        sensing1();
        stopLoop = berhenti();
      }
 
      
      lcd.clear();
      delay(1000);
      
     
    }else if(customKey == 'B') {
      lcd.clear();
      delay(1000);

      boolean stopLoop2 = false;
      while(!stopLoop2){
        sensing2();
        stopLoop2 = berhenti();
      }
      lcd.clear();
      delay(100);
      }
  
    
}

boolean berhenti(){
 customKey = customKeypad.getKey();
    if(customKey == '*'){
      return true; 
     }else {
      return false; 
     }
}


void sensing1(){
     
  float regresi;
  float x = mlx.readObjectTempC();
  regresi = (0.0266 * x) + (35.23037062);
  //Serial.print("Ambient = ");
  //Serial.print(mlx.readAmbientTempC());
  //Serial.print("*C\tObject = ");
  lcd.setCursor(0,1);
  lcd.print("Ruangan ");
  lcd.print(mlx.readAmbientTempC());
  lcd.print(" C");
  //Serial.print(regresi,2);
  //Serial.println("*C");
 // Serial.print("Ambient = ");
  lcd.setCursor(0,0);
  lcd.print("Suhu  ");
  lcd.print(regresi,2);
  lcd.print(" C");


Serial.println();

}
void sensing2(){
  uint32_t ir, red , green;
  double fred, fir;
  double SpO2 = 0; //raw SpO2 before low pass filtered
 
#ifdef USEFIFO
  particleSensor.check(); //Check the sensor, read up to 3 samples
 
  while (particleSensor.available()) {//do we have new data
#ifdef MAX30105
   red = particleSensor.getFIFORed(); //Sparkfun's MAX30105
    ir = particleSensor.getFIFOIR();  //Sparkfun's MAX30105
#else
    red = particleSensor.getFIFOIR(); //why getFOFOIR output Red data by MAX30102 on MH-ET LIVE breakout board
    ir = particleSensor.getFIFORed(); //why getFIFORed output IR data by MAX30102 on MH-ET LIVE breakout board
#endif
    i++;
    fred = (double)red;
    fir = (double)ir;
    avered = avered * frate + (double)red * (1.0 - frate);//average red level by low pass filter
    aveir = aveir * frate + (double)ir * (1.0 - frate); //average IR level by low pass filter
    sumredrms += (fred - avered) * (fred - avered); //square sum of alternate component of red level
    sumirrms += (fir - aveir) * (fir - aveir);//square sum of alternate component of IR level    
    if ((i % SAMPLING) == 0) {//slow down graph plotting speed for arduino Serial plotter by thin out
      if ( millis() > TIMETOBOOT) {
        if (ir < FINGER_ON) ESpO2 = MINIMUM_SPO2; //indicator for finger detached
        if (ESpO2 <= -1)
        {
          ESpO2 = 0;
        }

        if (ESpO2 > 100)
        {
          ESpO2 = 100;
        }

        oxi = ESpO2;
        
        //Serial.print(" Oxygen % = ");
        //Serial.println(ESpO2); //low pass filtered SpO2

      }
    }
    if ((i % Num) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
      // Serial.println(R);
      SpO2 = -23.3 * (R - 0.4) + 100; //http://ww1.microchip.com/downloads/jp/AppNotes/00001525B_JP.pdf
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;//low pass filter
      //  Serial.print(SpO2);Serial.print(",");Serial.println(ESpO2);
      //Serial.print(" Oxygen2 % = ");
      //Serial.println(ESpO2); //low pass filtered SpO2
      sumredrms = 0.0; sumirrms = 0.0; i = 0;
      break;
    }
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
    //Serial.println(SpO2);

       //////////////////////////////////////////////////////////////////////////////
    long irHR = particleSensor.getIR();
    if(irHR > 7000){                                           //If a finger is detected               
      //display.clearDisplay();                                   //Clear the display
      //display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);       //Draw the first bmp picture (little heart)
      lcd.setCursor(0,0);             
      lcd.print("HR");             
      lcd.print(beatAvg); 
      
      lcd.print(" /bpm"); 

      //display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE);       //Draw the first bmp picture (little heart)
  
      lcd.setCursor(0,1);              
      lcd.print("SpO");                    
      lcd.print(oxi,1); 
      
      lcd.print("  %"); 
   
      //delay(100);
      
      
    if (checkForBeat(irHR) == true)                        //If a heart beat is detected
    {              
      //display.clearDisplay();                                //Clear the display
      //display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE);    //Draw the second picture (bigger heart)
      lcd.setCursor(0,0);               
      lcd.print("HR");                 
      lcd.print(beatAvg);  
      lcd.print(" /bpm"); 
          

      lcd.setCursor(0,1);      
      lcd.print("SpO");               
      lcd.print(oxi,1); 
      lcd.print("  ");
      //display.display();
      //tone(3,1000);                                        //And tone the buzzer for a 100ms you can reduce it it will be better
      delay(100);
      //noTone(3);   
      //delay(100);
      //Deactivate the buzzer to have the effect of a "bip"
      //We sensed a beat!
      long delta = millis() - lastBeat;                   //Measure duration between two beats
      lastBeat = millis();
  
      beatsPerMinute = 60 / (delta / 1000.0);           //Calculating the BPM
  
      if (beatsPerMinute < 255 && beatsPerMinute > 20)               //To calculate the average we strore some values (4) then do some math to calculate the average
      {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable
  
        //Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }
  
  }
  if (irHR < 7000){       //If no finger is detected it inform the user and put the average BPM to 0 or it will be stored for the next measure
     beatAvg=0;              
     beatAvg=0;
     //display.clearDisplay();             
     lcd.setCursor(0,0);              
     lcd.print("Tempelkan"); 
     lcd.setCursor(0,1);
     lcd.print("Jari");  
     //display.display();
     //noTone(3);
     }
    

  }
#endif
Serial.println(); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Sensor(){
  float regresi;
  float x = mlx.readObjectTempC();
 

  Serial.print("Nilai suhu=");
  Serial.print(x);
  Serial.println("ºC");

  regresi = (0.0266 * x) + (35.23037062);
  Serial.print("Sesudah regresi = ");
  Serial.print(regresi,2);
  Serial.println("ºC");
  //Serial.print(mlx.readObjectTempF());
  //Serial.println("*F");

  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 uint32_t ir, red , green;
  double fred, fir;
  double SpO2 = 0; //raw SpO2 before low pass filtered
 
#ifdef USEFIFO
  particleSensor.check(); //Check the sensor, read up to 3 samples
 
  while (particleSensor.available()) {//do we have new data
#ifdef MAX30105
   red = particleSensor.getFIFORed(); //Sparkfun's MAX30105
    ir = particleSensor.getFIFOIR();  //Sparkfun's MAX30105
#else
    red = particleSensor.getFIFOIR(); //why getFOFOIR output Red data by MAX30102 on MH-ET LIVE breakout board
    ir = particleSensor.getFIFORed(); //why getFIFORed output IR data by MAX30102 on MH-ET LIVE breakout board
#endif
    i++;
    fred = (double)red;
    fir = (double)ir;
    avered = avered * frate + (double)red * (1.0 - frate);//average red level by low pass filter
    aveir = aveir * frate + (double)ir * (1.0 - frate); //average IR level by low pass filter
    sumredrms += (fred - avered) * (fred - avered); //square sum of alternate component of red level
    sumirrms += (fir - aveir) * (fir - aveir);//square sum of alternate component of IR level    
    if ((i % SAMPLING) == 0) {//slow down graph plotting speed for arduino Serial plotter by thin out
      if ( millis() > TIMETOBOOT) {
        if (ir < FINGER_ON) ESpO2 = MINIMUM_SPO2; //indicator for finger detached
        if (ESpO2 <= -1)
        {
          ESpO2 = 0;
        }

        if (ESpO2 > 100)
        {
          ESpO2 = 100;
        }

        oxi = ESpO2;
        
        //Serial.print(" Oxygen % = ");
        //Serial.println(ESpO2); //low pass filtered SpO2

      }
    }
    if ((i % Num) == 0) {
      double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
      // Serial.println(R);
      SpO2 = -23.3 * (R - 0.4) + 100; //http://ww1.microchip.com/downloads/jp/AppNotes/00001525B_JP.pdf
      ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2;//low pass filter
      //  Serial.print(SpO2);Serial.print(",");Serial.println(ESpO2);
      //Serial.print(" Oxygen2 % = ");
      //Serial.println(ESpO2); //low pass filtered SpO2
      sumredrms = 0.0; sumirrms = 0.0; i = 0;
      break;
    }
    particleSensor.nextSample(); //We're finished with this sample so move to next sample
    //Serial.println(SpO2);

       //////////////////////////////////////////////////////////////////////////////
    long irHR = particleSensor.getIR();
    if(irHR > 7000){                                           //If a finger is detected               
                 
      //lcd.print("HR");             
      Serial.print(beatAvg); 
      
      //lcd.print(" /bpm"); 

          
      //Serial.print("SpO");                    
      Serial.print(oxi); 
      
      //lcd.print("  %"); 
   
      //delay(100);
      
      
    if (checkForBeat(irHR) == true)                        //If a heart beat is detected
    {              
                      
      Serial.print(beatAvg);  
      
                 
      Serial.print(oxi); 
                                          //And tone the buzzer for a 100ms you can reduce it it will be better
      delay(100);
      //noTone(3);   
      //delay(100);
      //Deactivate the buzzer to have the effect of a "bip"
      //We sensed a beat!
      long delta = millis() - lastBeat;                   //Measure duration between two beats
      lastBeat = millis();
  
      beatsPerMinute = 60 / (delta / 1000.0);           //Calculating the BPM
  
      if (beatsPerMinute < 255 && beatsPerMinute > 20)               //To calculate the average we strore some values (4) then do some math to calculate the average
      {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable
  
        //Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }
  
  }

#endif
  }
     


  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    dataSend = String(regresi) + "#" + String(nilaiAngka)+ "#" + String(beatAvg)+ "#" + String(oxi);
    Serial.println(dataSend);
    mySerial.println(dataSend);
 
}
