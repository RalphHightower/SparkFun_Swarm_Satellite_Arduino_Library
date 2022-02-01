/*
  Use ESP32 WiFi to get the Two-Line Elements for the Swarm satellites
  By: SparkFun Electronics / Paul Clark
  Date: January 29th, 2022
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to:
    * Download the list of Two-Line Elements (orbit parameters) for _all_ of the Swarm satellites from CelesTrak
      and save it to microSD card
    * Download the satellite pass prediction for your location from the Swarm server and save it to microSD card
    * Cross-correlate the two data sets to build a list of the individual satellites that cover your location
  
  You can then use the TLEs and the satellite list to predict the next satellite pass for your location - offline!
  
  Please see the next examples for details: Example2_ESP32_Get_My_Swarm_TLEs and Example3_ESP32_Predict_Next_Swarm_Pass
  
  This example has a lot of calculations and file accessing to do. It is _very_ slow... But you should only need to run it
  occasionally to update the list of satellites for your location. Example2 downloads only the TLEs for your location and
  is much faster. Example3 uses the TLEs downloaded by Example2 - not the full set.

  Update secrets.h with your:
  - WiFi credentials

  This example is written for the SparkFun Thing Plus C (SPX-18018) but can be adapted for any ESP32 board.

  Use the Boards Manager to install the "ESP32 Arduino" boards.
  
  Add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
  to your "Preferences \ Additional Boards Manager URLs" if required.
  
  Select "ESP32 Dev Module" as the Board (not "SparkFun ESP32 Thing Plus").

  If the SD card is not detected ("Card Mount Failed"), try adding a pull-up resistor between
  19/CIPO and 3V3. A resistor in the range 10K - 1M seems to help.

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  SparkFun Thing Plus C - ESP32 WROOM: https://www.sparkfun.com/products/18018

*/

#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"

#include <FS.h>
#include <SD.h>
#include <SPI.h>

const int sd_cs = 5; //SparkFun Thing Plus C microSD chip select pin

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include <Sgp4.h>

Sgp4 sat;

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

const char celestrakServer[] = "https://celestrak.com";

const char getSwarmTLE[] = "NORAD/elements/gp.php?GROUP=SWARM&FORMAT=TLE";

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

const char swarmPassCheckerServer[] = "https://bumblebee.hive.swarm.space";

const char passCheckerAPI[] = "api/v1/passes?";

const char latPrefix[] = "lat=";
const char lonPrefix[] = "&lon=";
const char altPrefix[] = "&alt=";
const char mergeSuffix[] = "&merge=false";

const char myLatitude[] = "55.000";   //                  <-- Update this with your latitude
const char myLongitude[] = "-1.000";  //                  <-- Update this with your longitude
const char myAltitude[] = "100";      //                  <-- Update this with your altitude in m

// The pass checker data is returned in non-pretty JSON format:
#define startOfFirstStartPass 26 // start_pass: YYYY-MM-DDTHH:MM:SSZ
#define startPassDateTimeLength 19
#define elevationOffset 75 // Offset from the start_pass "Z" to the start of the max_elevation
#define startOffset 12 // Offset from the "s" of start_pass to the start of the year

// At the time or writing:
//  Swarm satellites are named: SPACEBEE-n or SPACEBEENZ-n
//  I guess SPACEBEENZ could be in a special orbit for New Zealand?
//  SPACEBEE numbers are 1 - 111 (8 and 9 are missing)
//  SPACEBEENZ numbers are 1 - 10
const int maxSats = 120;

// Stop checking when we find this many satellite duplications for a single satellite
// (When ~24 hours of passes have been processed)
#define satPassLimit 6

// Check for a match: start_pass +/- 5 seconds (in Julian Days)
const double predictionStartError = 5.0 / (24.0 * 60.0 * 60.0);
// Check for a match: max_elevation +/- 5 degrees (accuracy is worst at Zenith, much better at the horizon)
const double maxElevationError = 5.0;

#define nzOffset 100000 // Add this to the satellite number to indicate SPACEBEENZ

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void setup()
{
  delay(1000);

  Serial.begin(115200);
  Serial.println(F("Example : Swarm Two-Line Elements for your location"));

  while (Serial.available()) Serial.read(); // Empty the serial buffer
  Serial.println(F("Press any key to begin..."));
  while (!Serial.available()); // Wait for a keypress
  Serial.println();
/*
  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Connect to WiFi.

  Serial.print(F("Connecting to local WiFi"));

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();

  Serial.println(F("WiFi connected!"));

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Use HTTP GET to receive the TLE data

  const int URL_BUFFER_SIZE  = 256;
  char theURL[URL_BUFFER_SIZE]; // This will contain the HTTP URL
  int payloadSize = 0; // This will be updated with the length of the data we get from the server
  String payload; // This will store the data we get from the server
  HTTPClient http;
  int httpCode;

  // Assemble the URL
  // Note the slash after the first %s (celestrakServer)
  snprintf(theURL, URL_BUFFER_SIZE, "%s/%s",
    celestrakServer,
    getSwarmTLE);

  Serial.println(F("Requesting the Swarm TLE data from CelesTrak"));
  Serial.print(F("HTTP URL is: "));
  Serial.println(theURL);

  http.begin(theURL);

  httpCode = http.GET(); // HTTP GET

  // httpCode will be negative on error
  if(httpCode > 0)
  {
    // HTTP header has been sent and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\r\n", httpCode);
  
    // If the GET was successful, read the data
    if(httpCode == HTTP_CODE_OK) // Check for code 200
    {
      payloadSize = http.getSize();
      Serial.printf("Server returned %d bytes\r\n", payloadSize);
      
      payload = http.getString(); // Get the payload

      // Print the payload
      //for(int i = 0; i < payloadSize; i++)
      //{
      //  Serial.write(payload[i]);
      //}
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\r\n", http.errorToString(httpCode).c_str());
  }

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Save the TLE data to SD card
*/
  if (!SD.begin(SS)) {
    Serial.println("Card Mount Failed! Freezing...");
    while (1)
      ;
  }
/*
  writeFile(SD, "/swarmTLE.txt", payload.c_str()); // Write the payload to file

  Serial.println(F("The Swarm TLE data has been saved to SD card"));

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Use HTTP GET to receive the Swarm pass prediction for your location

  payload = ""; // Clear the existing payload
  payloadSize = 0; // This will be updated with the length of the data we get from the server

  // Assemble the URL
  // Note the slash after the first %s
  snprintf(theURL, URL_BUFFER_SIZE, "%s/%s%s%s%s%s%s%s%s",
    swarmPassCheckerServer,
    passCheckerAPI,
    latPrefix,
    myLatitude,
    lonPrefix,
    myLongitude,
    altPrefix,
    myAltitude,
    mergeSuffix);

  Serial.println(F("Requesting the Swarm pass prediction"));
  Serial.print(F("HTTP URL is: "));
  Serial.println(theURL);

  http.begin(theURL);

  httpCode = http.GET();

  // httpCode will be negative on error
  if(httpCode > 0)
  {
    // HTTP header has been sent and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\r\n", httpCode);
  
    // If the GET was successful, read the data
    if(httpCode == HTTP_CODE_OK) // Check for code 200
    {
      payloadSize = http.getSize();
      Serial.printf("Server returned %d bytes\r\n", payloadSize);
      
      payload = http.getString(); // Get the payload

      // Print the payload
      //for(int i = 0; i < payloadSize; i++)
      //{
      //  Serial.write(payload[i]);
      //}
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\r\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  
  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Save the pass prediction to SD card

  writeFile(SD, "/swarmPP.txt", payload.c_str()); // Write the payload to file

  Serial.println(F("The Swarm pass prediction has been saved to SD card"));
*/
  listDir(SD, "/", 0); // List the card directory to see if the files were written correctly

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Disconnect the WiFi as it's no longer needed

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  Serial.println();
  Serial.println(F("WiFi disconnected"));

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Set site latitude[°], longitude[°] and altitude[m]
  
  float myLat, myLon, myAlt;
  sscanf(myLatitude, "%f", &myLat);
  sscanf(myLongitude, "%f", &myLon);
  sscanf(myAltitude, "%f", &myAlt);
  sat.site((double)myLat, (double)myLon, (double)myAlt);        

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Record which satellites have been cross-correlated

  int foundNumSats = 0;
  unsigned long foundSats[maxSats];
  int satPassCount[maxSats];
  for (int i = 0; i < maxSats; i++)
  {
    foundSats[i] = 0;
    satPassCount[i] = 0;
  }

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Keep going until we hit the end of the pass prediction file
  // Or satPassCount reaches satPassLimit

  bool keepGoing = true;

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Open the pass prediction file for reading

  File ppFile = SD.open("/swarmPP.txt", FILE_READ);
  if (!ppFile) {
    Serial.println("Pass Prediction File Open Failed! Freezing...");
    while (1)
      ;
  }

  char fileChar;
  for (int i = 0; i < startOfFirstStartPass; i++) // Point at the first start_pass
  {
    if (ppFile.readBytes(&fileChar, 1) != 1)
      keepGoing = false;
  }

  Serial.println();
  Serial.println("Starting the pass prediction calculations. This will take a LONG time! Go make a cup of tea...");

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Step through the pass prediction file, extract the start_pass and max_elevation
  //
  // Keep going until we hit the end of the file

  while (keepGoing)
  {

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Extract the start_pass

    char startPass[startPassDateTimeLength + 1];
    startPass[startPassDateTimeLength] = 0; // Null-terminate the dateTime
    if (ppFile.readBytes((char *)&startPass, startPassDateTimeLength) != startPassDateTimeLength)
    {
      keepGoing = false;
      break;
    }

    Serial.println();
    Serial.print(F("Extracted start_pass: "));
    Serial.print(startPass);

    // Convert it to Julian Day
    int year, month, day, hour, minute, second;
    if (sscanf(startPass, "%4d-%2d-%2dT%2d:%2d:%2d", &year, &month, &day, &hour, &minute, &second) != 6)
    {
      keepGoing = false;
      break;      
    }
    double predictionStart;
    jday(year, month, day, hour, minute, second, 0, false, predictionStart);

    Serial.print(F("  Julian Day: "));
    Serial.print(predictionStart, 5);

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Extract the max_elevation

    for (int i = 0; i < elevationOffset; i++) // Point at the max_elevation
    {
      if (ppFile.readBytes(&fileChar, 1) != 1)
      {
        keepGoing = false;
      }
    }

    if (!keepGoing) // Exit the while loop if we hit the end of the file
    {
      Serial.println();
      break;
    }

    char maxElevationStr[7];
    maxElevationStr[6] = 0; // Null-terminate the max_elevation

    if (ppFile.readBytes((char *)&maxElevationStr, 6) != 6) // Read 3 decimal places (nn.nnn)
    {
      keepGoing = false;
      Serial.println();
      break;
    }

    float maxElevation;
    if (sscanf(maxElevationStr, "%f", &maxElevation) != 1)
    {
      keepGoing = false;
      Serial.println();
      break;
    }

    Serial.print(F("  max_elevation: "));
    Serial.println(maxElevation, 3);

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Step through the TLE file.
    // For each satellite, perform our own pass prediction using our location and predictionStart.
    // Record if we have a match.

    File tleFile = SD.open("/swarmTLE.txt", FILE_READ);
    if (!tleFile) {
      Serial.println("TLE File Open Failed! Freezing...");
      while (1)
        ;
    }

    bool keepReadingTLEs = true;
  
    while (keepReadingTLEs)
    {
  
      // Allocate 30 bytes to store the satellite name
      char satelliteName[30];
  
      // Read the satellite name
      int satNameLength = tleFile.readBytesUntil('\n', (char *)satelliteName, 30);
      satelliteName[satNameLength] = 0; // Null-terminate the name
      //Serial.print(F("Performing prediction for satellite "));
      //Serial.println(satelliteName);
  
      // Allocate 75 bytes to store the TLE line one
      char lineOne[75];
  
      // Read line one
      int lineOneLength = tleFile.readBytesUntil('\n', (char *)lineOne, 75);
      lineOne[lineOneLength] = 0; // Null-terminate the line
  
      // Allocate 75 bytes to store the TLE line two
      char lineTwo[75];
  
      // Read line two
      int lineTwoLength = tleFile.readBytesUntil('\n', (char *)lineTwo, 75);
      lineTwo[lineTwoLength] = 0; // Null-terminate the line
  
      // Check we have enough data
      if ((satNameLength < 25) || (lineOneLength < 70) || (lineTwoLength < 70))
      {
        keepReadingTLEs = false;
      }
      else
      {
        sat.init(satelliteName, lineOne, lineTwo); //initialize satellite parameters     

        double overpassStart = 0.0, overpassMaxElev = 0.0;
        double maxElevationDbl = (double)maxElevation;

        // Calculate the next overpass
        // Start one minute early 
        if (Predict(predictionStart - (1.0 / (24.0 * 60.0)), &overpassStart, &overpassMaxElev))
        {
          if  ((overpassStart < (predictionStart + predictionStartError)) // Check for a match
            && (overpassStart > (predictionStart - predictionStartError))
            && (overpassMaxElev < (maxElevationDbl + maxElevationError))
            && (overpassMaxElev > (maxElevationDbl - maxElevationError)))
          {
            Serial.print(F("Pass match found for satellite: "));
            Serial.println((char *)satelliteName);

            Serial.print(F("Prediction result was: start_pass Julian Day: "));
            Serial.print(overpassStart, 5);
            Serial.print(F("  max_elevation: "));
            Serial.println(overpassMaxElev, 3);
          
            // Record the match
            unsigned long satNum = 0;
            if (satelliteName[8] == '-') // Look for SPACEBEE-nnn
            {
              satNum = satelliteName[9] - '0'; // Extract the satellite number (max 99999)
              if ((satelliteName[10] >= '0') && (satelliteName[10] <= '9'))
              {
                satNum *= 10;
                satNum += satelliteName[10] - '0';
                if ((satelliteName[11] >= '0') && (satelliteName[11] <= '9'))
                {
                  satNum *= 10;
                  satNum += satelliteName[11] - '0';
                  if ((satelliteName[12] >= '0') && (satelliteName[12] <= '9'))
                  {
                    satNum *= 10;
                    satNum += satelliteName[12] - '0';
                    if ((satelliteName[13] >= '0') && (satelliteName[13] <= '9'))
                    {
                      satNum *= 10;
                      satNum += satelliteName[13] - '0';
                    }
                  }
                }
              }
            }
            else if ((satelliteName[8] == 'N') && (satelliteName[9] == 'Z')) // Look for SPACEBEENZ-nnn
            {
              // Add nzOffset to the SPACEBEENZ numbers when recording them
              satNum = nzOffset + satelliteName[11] - '0'; // Extract the satellite number (max 99999)
              if ((satelliteName[12] >= '0') && (satelliteName[12] <= '9'))
              {
                satNum *= 10;
                satNum += satelliteName[12] - '0';
                if ((satelliteName[13] >= '0') && (satelliteName[13] <= '9'))
                {
                  satNum *= 10;
                  satNum += satelliteName[13] - '0';
                  if ((satelliteName[14] >= '0') && (satelliteName[14] <= '9'))
                  {
                    satNum *= 10;
                    satNum += satelliteName[14] - '0';
                    if ((satelliteName[15] >= '0') && (satelliteName[15] <= '9'))
                    {
                      satNum *= 10;
                      satNum += satelliteName[15] - '0';
                    }
                  }
                }
              }
            }

            if (satNum > 0)
            {
              bool foundAgain = false;
              for (int i = 0; i < foundNumSats; i++)
              {
                if (foundSats[i] == satNum) // Have we found this satellite before?
                {
                  foundAgain = true;
                  satPassCount[i] = satPassCount[i] + 1;
                }
              }
              if ((!foundAgain) && (foundNumSats < maxSats))
              {
                foundSats[foundNumSats++] = satNum;
              }
              keepReadingTLEs = false; // Stop searching after one match
            }
          }
        }
      }
    }
    
    tleFile.close(); // Close the TLE file

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Point at the next start_pass

    bool sFound = false;
    while (!sFound && keepGoing) // Find the next "s"tart_pass 
    {
      if (ppFile.readBytes(&fileChar, 1) != 1)
        keepGoing = false;
      if (fileChar == 's')
        sFound = true;
    }
    if (sFound)
    {
      for (int i = 0; i < startOffset; i++) // Point at the year
      {
        if (ppFile.readBytes(&fileChar, 1) != 1)
          keepGoing = false;
      }
    }

    //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
    // Check if satPassCount has reached satPassLimit

    for (int i = 0; i < foundNumSats; i++)
    {
      if (satPassCount[i] >= satPassLimit)
      {
        Serial.println();
        Serial.print(F("Found "));
        Serial.print(satPassCount[i]);
        Serial.println(F(" duplicate satellite passes. Exiting..."));
        Serial.println();
        keepGoing = false;
      }
    }
  }

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Close the pass prediction file

  ppFile.close();

  //=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
  // Finally write all the found satellites to mySwarm.txt

  Serial.println(F("Writing the satellite names to mySwarm.txt"));

  File file = SD.open("/mySwarm.txt", FILE_WRITE);
  if (file)
  {
    for (int i = 0; i < foundNumSats; i++)
    {
      if (foundSats[i] > nzOffset)
      {
        file.print("SPACEBEENZ-");
        file.println(foundSats[i] - nzOffset);
        Serial.print("SPACEBEENZ-");
        Serial.println(foundSats[i] - nzOffset);
      }
      else if (foundSats[i] > 0)
      {
        file.print("SPACEBEE-");
        file.println(foundSats[i]);
        Serial.print("SPACEBEE-");
        Serial.println(foundSats[i]);
      }
    }
    Serial.println();
  }
  file.close();

  listDir(SD, "/", 0); // List the card directory to see if the files were written correctly

  Serial.println(F("\r\nAll done!"));
}
  
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void loop()
{
  // Nothing to do here
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

// Predict the next satellite pass.
// Start the prediction at startTimeJD (Julian Day).
// Return the start of the pass in overpassStart (Julian Day).
// Return the maximum elevation in maxElevation.
// Return true if the prediction was successful.
bool Predict(double startTimeJD, double *overpassStart, double *maxElevation)
{
  passinfo overpass;                      //structure to store overpass info
  sat.initpredpoint( startTimeJD , 0.0 ); //finds the startpoint
  
  bool error;
  
  //int year; int mon; int day; int hr; int minute; double sec;
  
  error = sat.nextpass(&overpass, 20, false, 15.0); //search for the next overpass, if there are more than 20 maximums below 15 degrees it returns false
  
  if ( error == 1) //no error
  {
    //invjday(overpass.jdstart, 0, false, year, mon, day, hr, minute, sec); // Time Zone 0 : use UTC
    //Serial.println("  Overpass: " + String(day) + '/' + String(mon) + '/' + String(year));
    //Serial.println("    Start: " + String(hr) + ':' + String(minute) + ':' + String(sec));
    //Serial.println("    Max Elevation = " + String(overpass.maxelevation) + "°");

    *overpassStart = overpass.jdstart;
    *maxElevation = overpass.maxelevation;
  }
  else
  {
    //Serial.println("  Prediction error");
  }

  return (error == 1);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("\tFILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}