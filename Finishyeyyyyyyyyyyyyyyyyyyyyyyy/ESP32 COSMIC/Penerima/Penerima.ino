// Master Serial
// Note:
// sudah dapat menerima dari slave

#include <AntaresLoRaWAN.h>

#define ACCESSKEY "b95d155b4eb8307c:bee1a623ffdf096a"
#define DEVICEID "dd43791d"

long interval = 10000;    // 5 s interval to send message
long previousMillis = 0;  // will store last time message sent

String dataSend;
int regresi, x;
int nilaiAngka, beatAvg, oxi;
AntaresLoRaWAN antares;

//variabel parsing
String arrData[4];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial2.begin(115200); // komunikasi with arduino uno (TX1 RX1)

  antares.setTxInterval(1);
  antares.setPins(2, 14, 12);   // Set pins for NSS, DIO0, and DIO1
  antares.init(ACCESSKEY, DEVICEID);
  antares.setDataRateTxPow(DR_SF10, 17);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long currentMillis = millis();

  // Check interval overflow
  if(currentMillis - previousMillis > interval) 
  {
    previousMillis = currentMillis;

    String Data = "";

    // berikut code untuk membaca data dari perangkat lain
    while (Serial2.available()>0)
    {
      Data += char(Serial2.read());
    }
      
    //Membuang serial
    Data.trim();

    //cek data
    if (Data != "")
    {
      int index = 0; // data mobil
      for(int i=0; i<= Data.length(); i++)
      {
        char pemisah = '#';
        if (Data[i] != pemisah)
        {
          arrData[index] += Data[i];
        }
        else
        {
          index++;  //variable selanjutnya  
        }
      }

      if (index == 1)
      {
        // tampilkan
        Serial.println("Suhu Tubuh : " + arrData[0]);
        Serial.println("No ID : " + arrData[1]);   
        Serial.println("Hear Rate : " + arrData[2]); 
        Serial.println("Spo2 : " + arrData[3]);      
      }  

      //ISI VARIABEL YANG DI SEND
      regresi = arrData[0].toInt();
      nilaiAngka = arrData[1].toInt();
      beatAvg = arrData[2].toInt();
      oxi = arrData[3].toInt();

      dataSend = "{""\" regresi :" + String(regresi) + " ""\",\" No Id :" + String(nilaiAngka) + "\",\" Hr :" + String(beatAvg) + "\"\" Spo2 :" + String(oxi) + "\"}";
      
      arrData[0] = "";
      arrData[1] = "";
      arrData[2] = "";
      arrData[3] = "";
    }

    sendPacket(dataSend);

    //Minta data ke arduino uno
    Serial2.println("y");
  }
}

//Fungsi send data, disarankan untuk tidak merubah fungsi ini
void sendPacket(String &input) {
  String dataA = String (input);
  antares.send(dataA);

  char dataBuf[100];
  int dataLen = dataA.length();

  dataA.toCharArray(dataBuf, dataLen + 1);
  Serial.println("Data :" + (String)dataLen );
  if (dataLen > 1)
  {
    Serial.println("\n[ANTARES] Data: " + dataA + "\n");

    if (LMIC.opmode & OP_TXRXPEND) 
    {
      Serial.println(F("OP_TXRXPEND, not sending"));
    } 
    else 
    {
      LMIC_setTxData2(1, (uint8_t*)dataBuf, dataLen, 0);
      Serial.println(F("Packet queued"));
      esp_sleep_enable_timer_wakeup(10 * 1000000);
      esp_deep_sleep_start();
    }
  }
//  else
//  {
//    Serial.println("\n[ANTARES] Data: Kosong\n");
//  }
//  delay(1000);
 }
