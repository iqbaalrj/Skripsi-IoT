#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup()
{
Serial.begin(9600);
mlx.begin();
}

void loop()
{
  float regresi;
  float x = mlx.readObjectTempC();
//Serial.print("Ambient = ");
//Serial.print(mlx.readAmbientTempC());
//Serial.print("*C\tObject = ");
//Serial.print(x);
//Serial.println("ºC");
Serial.print("Nilai suhu=");
Serial.print(x);
Serial.println("ºC");

regresi = (0.0266 * x) + (35.23037062);
Serial.print("Sesudah regresi = ");
Serial.print(regresi,2);
Serial.println("ºC");
//Serial.print(mlx.readObjectTempF());
//Serial.println("*F");

Serial.println();
delay(1000);
}
