#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include "sntp.h"
#include <string.h>

const char* wifi_ssid = "Your SSID"; //replace these bits with your actual network name
const char* wifi_password = "Your Password";

const int RED = 14;
const int GREEN = 13;
const int BLUE = 12;

const int freq = 5000;
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 3;
const int resolution = 8;


//Fetch time
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;
const char* time_zone = "CET-1CEST,M3.5.0,M10.5.0/3";
String todaysBin = "";
int day = 0;
int month = 0;
int year = 0;

//Both prints the current time and updates the variables
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.print("Current time: ");
  Serial.print(timeinfo.tm_hour);
  Serial.print(":");
  Serial.print(timeinfo.tm_min);
  Serial.print(":");
  Serial.println(timeinfo.tm_sec);
  Serial.print("Current day: ");
  Serial.println(timeinfo.tm_mday);
  day = timeinfo.tm_mday;
  month = timeinfo.tm_mon;
  year = timeinfo.tm_year + 1900;  //years since 1900
}


String fetchedHTML = "";
String fetchedTime = "";
String extractedString = "";

void setup() {
  Serial.begin(115200);

  ledcSetup(redChannel, freq, resolution);
  ledcSetup(greenChannel, freq, resolution);
  ledcSetup(blueChannel, freq, resolution);

  ledcAttachPin(RED, redChannel);
  ledcAttachPin(GREEN, greenChannel);
  ledcAttachPin(BLUE, blueChannel);


  //Sets up Wifi Connection
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  fetchedHTML = fetchDataFromWebsite();
  extractedString = fetchedHTML.substring(1484, 6331);  //shorten string so string manipulation can be done without crashing


  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  // Wait for time to synchronize
  while (!time(nullptr)) {
    delay(1000);
    Serial.println("Waiting for time synchronization...");
  }

  printLocalTime();
}

//Looks for a string containing the day, month and year + a bin. If none are returned, return 'No bins'
String getBinForDay(String htmlString, int currentDay, String currentMonth, int currentYear) {
  std::string simpleHtml = htmlString.c_str();
  if (htmlString.indexOf(String(currentDay) + ", " + currentMonth + " " + String(currentYear) + " - Brown Bin") != -1) {
    return "Brown";
  } else if (htmlString.indexOf(String(currentDay) + ", " + currentMonth + " " + String(currentYear) + " - Black Bin") != -1) {
    return "Black";
  } else if (htmlString.indexOf(String(currentDay) + ", " + currentMonth + " " + String(currentYear) + " - Blue Bin") != -1) {
    return "Blue";
  } else {
    return "No bin";
  }
}

//Changes month integer to String format
String changeMonth(int currentMonth) {
  switch (currentMonth) {
    case 1:
      return "January";
    case 2:
      return "February";
    case 3:
      return "March";
    case 4:
      return "April";
    case 5:
      return "May";
    case 6:
      return "June";
    case 7:
      return "July";
    case 8:
      return "August";
    case 9:
      return "September";
    case 10:
      return "October";
    case 11:
      return "November";
    case 12:
      return "December";
  }
}

//Downloads the html code from the website
String fetchDataFromWebsite() {
  HTTPClient http;

  // Fetch URL
  http.begin("REPLACE THIS WITH YOUR BIN CALENDAR (SHEFFIELD)");
  int httpCode = http.GET();

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      return payload;
    }
  } else {
    Serial.printf("[HTTP] GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    return "Failed to fetch data from website";
  }

  http.end();
}


void loop() {
  todaysBin = getBinForDay(extractedString, day, changeMonth(month + 1), year);
  Serial.println(getBinForDay(extractedString, day, changeMonth(month + 1), year));
//Light up rainbow LED depeding on what bin is meant to be out
  if (todaysBin == "Brown") {
    ledcWrite(redChannel, 255);
    ledcWrite(greenChannel, 22);
    ledcWrite(blueChannel, 0);
  } else if (todaysBin == "Black") {
    ledcWrite(redChannel, 200);
    ledcWrite(greenChannel, 0);
    ledcWrite(blueChannel, 255);
  } else if (todaysBin == "Blue") {
    ledcWrite(redChannel, 0);
    ledcWrite(greenChannel, 0);
    ledcWrite(blueChannel, 255);
  } else if (todaysBin == "Green") {
    ledcWrite(redChannel, 0);
    ledcWrite(greenChannel, 255);
    ledcWrite(blueChannel, 0);
  } else {
    for (int i = 0; i < 2; i += 1) {
      ledcWrite(redChannel, 255);
      ledcWrite(greenChannel, 0);
      ledcWrite(blueChannel, 0);
      delay(50);
      ledcWrite(redChannel, 0);
      ledcWrite(greenChannel, 0);
      ledcWrite(blueChannel, 0);
      delay(50);
    }
  }
  printLocalTime(); //Refresh time
  delay(1000);
}
