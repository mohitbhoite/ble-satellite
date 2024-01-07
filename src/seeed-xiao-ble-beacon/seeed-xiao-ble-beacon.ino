/*********************************************************************
 Project: BLE Satellite
 Author: Mohit Bhoite
 Date: 01 December 2023

 Uses BLE library and code example by Adafruit.

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <bluefruit.h>
#include <Wire.h>
#include <cdm4101.h>
#include "Adafruit_SHT31.h"
#include <Adafruit_FlashTransport.h>

Adafruit_FlashTransport_QSPI flashTransport;

// Beacon uses the Manufacturer Specific Data field in the advertising
// packet, which means you must provide a valid Manufacturer ID. Update
// the field below to an appropriate value. For a list of valid IDs see:
// https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers
// 0x004C is Apple
// 0x0822 is Adafruit
// 0x0059 is Nordic
#define MANUFACTURER_ID   0x004C

// Setup a dummy BLE UUID
uint8_t beaconUuid[16] =
{
  0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78,
  0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0
};

// A valid Beacon packet consists of the following information:
// UUID, Major, Minor, RSSI @ 1M
// We are setting up a dummy packet which we will update it with temp and humi values in loop()
BLEBeacon beacon(beaconUuid, 0x0102, 0x0304, -54);

char data[25];
CDM4101 LCD;
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() 
{
  Wire.begin();
  LCD.Init();
  sht31.begin(0x44);
  
  LCD.DispStr("Test");
  delay(2000);
  //Serial.begin(115200);

  // Uncomment to blocking wait for Serial connection
  // while ( !Serial ) delay(10);

  //Serial.println("Bluefruit52 Beacon Example");
  //Serial.println("--------------------------\n");

  Bluefruit.begin();
  flashTransport.begin();
  flashTransport.runCommand(0xB9);
  flashTransport.end();

  Bluefruit.autoConnLed(false);
  // Setting the TX power to lowest possible value to lower total power comsumption
  // A low value like this significantly shortens the range of the radio
  // Find a value that works best for you
  // Check bluefruit.h for supported values
  Bluefruit.setTxPower(-40);    

  // Manufacturer ID is required for Manufacturer Specific Data
  beacon.setManufacturer(MANUFACTURER_ID);
  //Following is dummy data. We will update it with temp and humi values in loop()
  beacon.setMajorMinor(71, 61);

  // Setup the advertising packet
  startAdv();
}

void startAdv(void)
{  
  // Advertising packet
  // Set the beacon payload using the BLEBeacon class populated
  // earlier in this example
  Bluefruit.Advertising.setBeacon(beacon);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * Apple Beacon specs
   * - Type: Non connectable, undirected
   * - Fixed interval: 100 ms -> fast = slow = 100 ms
   */
  //Bluefruit.Advertising.setType(BLE_GAP_ADV_TYPE_ADV_NONCONN_IND);
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(3200, 3200);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void loop() 
{
  //read temperature and humidity from the SHT31 sensor
  int temperature = (sht31.readTemperature()*1.8)+32;
  int humidity = sht31.readHumidity();

  //ensure the readings are valid before displaying them
  if (! isnan(temperature))
    {
      sprintf(data,"%doF",temperature);
      LCD.DispStr(data);
    }
  delay(2000);
  if (! isnan(humidity))
    {
      sprintf(data,"%drH",humidity);
      LCD.DispStr(data);
    }
  delay(2000);
  //load the BLE packet with latest temp and humi values
  beacon.setMajorMinor(temperature,humidity);
  Bluefruit.Advertising.setBeacon(beacon);

}
