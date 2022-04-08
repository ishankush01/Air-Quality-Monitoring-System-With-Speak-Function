
//MQ135  || So2, Co2, Nh3, O3
//MQ7 sensor  || CO
//BMP180 Sensor   || temp, pracess, Altitude,
//DHT11    || Temp, Humanity
//NodeMCU  |Microcontroller

//Coonection

//BMP180 -- VCC - VCC, GND - GND, SLC - D1, SDA - D2
//MQ7 -- A0
//MQ135 -- D3
//DHT11 -- D4


//Software

//  Arduino IDE
//  Thingspeak
//  Jupyter Notebook



#include <ESP8266WiFi.h>
#include <SFE_BMP180.h>//BMP180
#include "DHT.h"        // including the library of DHT11 temperature and humidity sensor
#include <SPI.h>
#include <Wire.h>

#define DHTTYPE DHT11
#define dht_dpin 2
DHT dht(dht_dpin, DHTTYPE);


SFE_BMP180 pressure;
#define ALTITUDE 80.0

String apiKey = "A314RVGIR2NOW2G5"; // Enter your Write API key from ThingSpeak
const char *ssid = "Realme"; // replace with your wifi ssid and wpa2 key
const char *pass = "12121212";
const char* server = "api.thingspeak.com";
WiFiClient client;
void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("REBOOT");
  dht.begin();

  // Initialize the sensor (it is important to get calibration values stored on the device).

  if (pressure.begin())
  {
    Serial.println("BMP180 init success");
    delay(5000);
  }
  else
  {
    Serial.println("BMP180 init fail\n\n");
    delay(5000);
    while (1); // Pause forever.
  }

}
void loop()
{
  float h = analogRead(A0);
  if (isnan(h))
  {
    Serial.println("Failed to read from MQ-5 sensor!");
    return;
  }

  float hum = dht.readHumidity();
  float tem = dht.readTemperature();
  Serial.println("Temperature: " + (String) tem);
  Serial.println("Humidity: " + (String) hum);
  //  ThingSpeak.writeField(myChannelNumber, 1, tem, myWriteAPIKey);
  //  ThingSpeak.writeField(myChannelNumber, 2, hum, myWriteAPIKey);
  //  delay(2000);



  char status;
  double T, P, p0, a;

  // Loop here getting pressure readings every 10 seconds.

  // If you want sea-level-compensated pressure, as used in weather reports,
  // you will need to know the altitude at which your measurements are taken.
  // We're using a constant called ALTITUDE in this sketch:

  Serial.println();
  Serial.print("provided altitude: ");
  Serial.print(ALTITUDE, 0);
  Serial.print(" meters, ");
  Serial.print(ALTITUDE * 3.28084, 0);
  Serial.println(" feet");
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.
  status = pressure.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = pressure.getTemperature(T);
    if (status != 0)
    {
      // Print out the measurement:
      Serial.print("temperature: ");
      Serial.print(T, 2);
      Serial.print(" deg C, ");
      Serial.print((9.0 / 5.0)*T + 32.0, 2);
      Serial.println(" deg F");

      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = pressure.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = pressure.getPressure(P, T);
        if (status != 0)
        {
          // Print out the measurement:
          Serial.print("absolute pressure: ");
          Serial.print(P, 2);
          Serial.print(" mb, ");
          Serial.print(P * 0.0295333727, 2);
          Serial.println(" inHg");

          // The pressure sensor returns abolute pressure, which varies with altitude.
          // To remove the effects of altitude, use the sealevel function and your current altitude.
          // This number is commonly used in weather reports.
          // Parameters: P = absolute pressure in mb, ALTITUDE = current altitude in m.
          // Result: p0 = sea-level compensated pressure in mb

          p0 = pressure.sealevel(P, ALTITUDE); // we're at 1655 meters (Boulder, CO)
          Serial.print("relative (sea-level) pressure: ");
          Serial.print(p0, 2);
          Serial.print(" mb, ");
          Serial.print(p0 * 0.0295333727, 2);
          Serial.println(" inHg");

          // On the other hand, if you want to determine your altitude from the pressure reading,
          // use the altitude function along with a baseline pressure (sea-level or other).
          // Parameters: P = absolute pressure in mb, p0 = baseline pressure in mb.
          // Result: a = altitude in m.

          a = pressure.altitude(P, p0);
          Serial.print("computed altitude: ");
          Serial.print(a, 0);
          Serial.print(" meters, ");
          Serial.print(a * 3.28084, 0);
          Serial.println(" feet");
        }
        else Serial.println("error retrieving pressure measurement\n");
      }
      else Serial.println("error starting pressure measurement\n");
    }
    else Serial.println("error retrieving temperature measurement\n");
  }
  else Serial.println("error starting temperature measurement\n");



  if (client.connect(server, 80)) // "184.106.153.149" or api.thingspeak.com
  {
    String postStr = apiKey;
    postStr += "&field1=";
    postStr += String(h / 1023 * 100);
    postStr += "r\n";
    postStr += "&field2=";
    postStr += String(T, 2);
    postStr += "&field3=";
    postStr += String(P * 0.0295333727, 2);
    postStr += "&field4=";
    postStr += String(p0 * 0.0295333727, 2);
    postStr += "&field5=";
    postStr += String(a, 0);
    postStr += "&field6=";
    postStr += String(tem, 0);
    postStr += "&field7=";
    postStr += String(hum, 0);
    postStr += "\r\n\r\n\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.print("Gas Level: ");
    Serial.println(h / 1023 * 100);
    Serial.println("Data Send to Thingspeak");
  }
  delay(500);
  client.stop();
  Serial.println("Waiting...");

  // thingspeak needs minimum 15 sec delay between updates.
  delay(1500);
}
