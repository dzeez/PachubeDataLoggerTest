/*
  Pachube Temp & Humidity datalogger
 
 This example shows how to log data from 
 a DHT11 temperature and humidity sensor 
 to pachube.com.
 
 It also uses the Adafruit DHT sensor library from 
 https://github.com/adafruit/DHT-sensor-library
 	
 The circuit:
 * Ethernet Arduino
 
 * DHT11 sensor connected as follows:
 ** Ground connected to 5
 ** Voltage connected to 8
 ** data out connected to 7
 ** 10K resistor connected from 7 to +5V
 
 created  12 Feb 2012
 by Tom Igoe
 
 This example code is in the public domain, subject to the licenses
 of the libraries used.
 
 // dsd updated: 5-2-2012
 // major change is shift to DallasTemperature/OneWire libs
 // rather than dht
 */

#include <SPI.h>
#include <Ethernet.h>
//#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>
//#include "details.h" // API keys and such defined in here

//#define DHTPIN 7              // what pin the sensor is connected to
//#define DHTTYPE DHT11         // Which type of DHT sensor you're using: 

//#define DHT_GND 5             // ground pin of the sensor
//#define DHT_VCC 8             // voltage pin of the sensor

// Corresponding Arduino port the 1-wire data wire is plugged into
#define ONE_WIRE_BUS 6

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//#define TEMPERATURE 1         //  for the DHT sensor
//#define HUMIDITY 0            // for the DHT sensor

//DHT dht(DHTPIN, DHTTYPE);     // initialize the sensor:  
const int interval = 3000;    // the interval between sensor reads, in ms
long lastReadTime = 0;        // the last time you read the sensor, in ms

// MAC address for your Ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//dsd: changed MAC to one of old NIC I actually own (see t-shooting thing below)
IPAddress ip(192,168,2,33);
//dsd: ip changed to be compatible with this network; mainly for t-shooting issue where connectivity just drops out after a few hourd



// initialize the library instance:
EthernetClient client;

long lastConnectionTime = 0;        // last time you connected to the server, in milliseconds
boolean lastConnected = false;      // state of the connection last time through the main loop
const int postingInterval = 10*1000;  //delay between updates to Pachube.com

char apiKey[] = "INSERT_YOUR_API_KEY"; // fill in your API key
long feed = 57976;      // fill in your feed number here

void setup() {
  Serial.begin(9600);
  delay(500);
  delay(750);//dsd; for debugging aid only
  Serial.println("Ethernet-connected Arduino to pachube data pusher, v0.1 pre-prototype\n"); //dsd
  
  //start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Configure manually:
    Ethernet.begin(mac, ip);
  }
  // print the Arduino's IP address:
  Serial.println("My ethernet connection was setup successfully! Yay for me!"); //dsd
  Serial.print("My IP address: "); //dsd
  Serial.println(Ethernet.localIP());
  Serial.println("\nOk, now let's get down to business!\n\n"); //dsd

  // set up the pins for the sensor, and initialize:
  //startSensor();
  
  // Start up the [1-wire/dallastemperature] library
  sensors.begin();
}

void loop() {
  // get the current time in ms:
  long currentTime = millis();

  if (currentTime > lastReadTime + postingInterval) {
    //float temperature = readSensor(TEMPERATURE);
    sensors.requestTemperatures(); // Send the command to get temperatures
    float temperature = sensors.getTempCByIndex(0);
    //float humidity = readSensor(HUMIDITY);

    // print results:
    //Serial.print("rH,");
    //Serial.print(humidity);
    //Serial.print("\ntemp,");
    Serial.print("temp,");
    Serial.println(temperature);

    // if you're not connected, and ten seconds have passed since
    // your last connection, then connect again and send data:
    if(!client.connected()) {
      //sendData(humidity, temperature);
      sendData(temperature);
    }
    // update the time of the most current reading:
    lastReadTime = millis();
  }


  // if there's incoming data from the net connection.
  // send it out the serial port.  This is for debugging
  // purposes only:
  if (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if there's no net connection, but there was one last time
  // through the loop, then stop the client:
  if (!client.connected() && lastConnected) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
  }

  // store the state of the connection for next time through
  // the loop:
  lastConnected = client.connected();
}
/*
void startSensor() {
  // set up pins to power and read sensor:
  pinMode(DHT_VCC, OUTPUT);
  pinMode(DHT_GND, OUTPUT);
  digitalWrite(DHT_VCC, HIGH);
  digitalWrite(DHT_GND, LOW);
  // start sensor:
  dht.begin(); 
}
*/

// get the sensor readings and concatenate them in a String:
/*
float readSensor( int thisValue) {
  float result;

  if (thisValue == TEMPERATURE) {
    result = dht.readTemperature();
  } 
  //else if (thisValue == HUMIDITY) {
    // read sensor:
  //  result = dht.readHumidity();   
  //}

  // make sure you have good readings. If the reading
  // is not a number (NaN) then return an error:
  if (isnan(result)) {
    // an impossible result for either reading 
    // so it'll work as an error:
    result = -273.0; 
  }
  return result;
}
*/

// this method makes a HTTP connection to the server:
//boolean sendData(float thisHumidity, float thisTemperature) {
boolean sendData(float thisTemperature) {
  if (client.connected()) client.stop();

  // if there's a successful connection:
  if ( client.connect("api.pachube.com", 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request. 
    // fill in your feed address here:
    client.print("PUT /v2/feeds/" + String(feed));
    client.println(".csv HTTP/1.1");
    client.println("Host: api.pachube.com");
    client.print("X-PachubeApiKey:");
    client.println(apiKey);
    client.print("Content-Length: ");

    /*
     work out the length of the string:
     "rH," = 3 + 
     digits of humidity value + 
     "temp," = 5 +
     digits of temperature value + 
     "\r\n" = 2
     */
    //int length = 3 + countDigits(thisHumidity,2) + 2 + 5 + countDigits(thisTemperature,2) + 2;
    int length = 5 + countDigits(thisTemperature,2) + 2;

    // calculate the length of the sensor reading in bytes:
    client.println(length);

    // last pieces of the HTTP PUT request:
    client.println("Content-Type: text/csv");
    client.println("Connection: close");
    client.println();

    // here's the actual content of the PUT request:
    //client.print("rH,");
    //client.println(thisHumidity);
    client.print("temp,");
    client.println(thisTemperature);
    client.println();
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
    Serial.println("data uploaded");
  } 
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

// Counts digits of a floating point number, to calculate content length
// for an HTTP call.
// Based on Arduino's internal printFloat() function.

int countDigits(double number, int digits)  { 
  int n = 0;

  // Handle negative numbers
  if (number < 0.0)
  {
    n++; // "-";
    number = -number;
  }

  // Round correctly so that print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i) {
    rounding /= 10.0;
  }
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  double remainder = number - (double)int_part;

  while (int_part > 0) {
    int_part /= 10;
    n++;
  }
  // Print the decimal point, but only if there are digits beyond
  if (digits > 0) {
    n++; //"."; 
  }

  // Extract digits from the remainder one at a time
  while (digits-- > 0)
  {
    remainder *= 10.0;
    int toPrint = int(remainder);
    n ++; // += String(toPrint);
    remainder -= toPrint; 
  } 
  return n;
}




