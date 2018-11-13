#include <Wire.h>
#include <SPI.h>
#include <WiFi101.h>
#include <Adafruit_LSM9DS1.h>
#include <Adafruit_Sensor.h>  // not used in this demo but required!

#include "arduino_secrets.h" 

#define ROTARY_ANGLE_SENSOR A0 //Use analog pin A0 for the Rotary Angle Sensor
#define ADC_REF 3.3 //Reference voltage of ADC is 3.3v
#define FULL_ANGLE 300.0 //Full value of the rotary angle is 300 degrees

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;  

int status = WL_IDLE_STATUS;

// Initialize the WiFi client library
WiFiClient client;

// server address:
char server[] = "kynetech.herokuapp.com";
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // delay between updates, in milliseconds

/*
-------------------------------------------------------------------------------------------------------------
Definitions for the gyro/acceleratometer
*/

Adafruit_LSM9DS1 lsm = Adafruit_LSM9DS1();

#define LSM9DS1_SCK A5
#define LSM9DS1_MISO 12
#define LSM9DS1_MOSI A4
#define LSM9DS1_XGCS 6
#define LSM9DS1_MCS 5

float prev_value = 0;


/*
-------------------------------------------------------------------------------------------------------------
Definitions for the flex sensor
*/

const int FLEX_PIN = A0; // Pin connected to voltage divider output

// Measure the voltage at 5V and the actual resistance of your
// 47k resistor, and enter them below:
const float VCC = 4.98; // Measured voltage of Ardunio 5V line
const float R_DIV = 47500.0; // Measured resistance of 3.3k resistor

// Upload the code, then try to adjust these values to more
// accurately calculate bend degree.
const float STRAIGHT_RESISTANCE = 11000.0; // resistance when straight
const float BEND_RESISTANCE = 25000.0; // resistance at 90 deg


/*
-------------------------------------------------------------------------------------------------------------
Definitions for the EMG sensor
*/

const int EMG_PIN = A1;


/*
-------------------------------------------------------------------------------------------------------------
Rest of the code
*/

void setupSensor()
{
  // 1.) Set the accelerometer range
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_2G);
  
  // 2.) Set the magnetometer sensitivity
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);

  // 3.) Setup the gyroscope
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}


void setup() 
{
  Serial.begin(9600);
  pinMode(FLEX_PIN, INPUT);

  while (!Serial) {
    delay(1); // will pause Zero, Leonardo, etc until serial console opens
  }
  
  Serial.println("LSM9DS1 data read demo");
  
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm.begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1);
  }
  Serial.println("Found LSM9DS1 9DOF");

  // helper to just set the default scaling we want, see above!
  setupSensor();
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWiFiStatus();
}


void loop() 
{
  String values;

  /* Accelerometer & Gyro sensor data collection*/
  lsm.read();
  sensors_event_t a, m, g, temp;
  lsm.getEvent(&a, &m, &g, &temp); 
  float roll = atan2(a.acceleration.y, a.acceleration.z);

  /* EMG sensor data collection*/
  int sensorValue = analogRead(EMG_PIN);
  float voltage = sensorValue * (5.0 / 1023.0);

  /* Potentiometer data collection */
  int angle = getDegrees();

  // /* Flex sensor data collection*/
  // int flexADC = analogRead(FLEX_PIN);
  // float flexV = flexADC * VCC / 1023.0;
  // float flexR = R_DIV * (VCC / flexV - 1.0);
  // float angle = map(flexR, STRAIGHT_RESISTANCE, BEND_RESISTANCE, 0, 90.0);

  if (angle < 0) {
    angle = 0;
  }
  
  if (abs(prev_value - angle) < 5) {
    values = String(roll) + "," + String(angle) + "," + String(voltage);
    httpRequest(values);
  }

  prev_value = angle;
  
  delay(300);
}

int getDegrees()
{
   //Read the raw sensor value
    int sensor_value = analogRead(ROTARY_ANGLE_SENSOR);

    //Convert the sensor reading to degrees and return that value
    float voltage = (float)sensor_value * ADC_REF / 1023; 
    float degrees = (voltage * FULL_ANGLE) / ADC_REF; 
    return degrees;
}

void httpRequest(String data) {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println("connecting...");
    // send the HTTP PUT request:
    client.println("POST /post HTTP/1.1");
    client.println("Host: kynetech.herokuapp.com");
    client.println("Content-Type: text/plain");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }
  else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}


void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
