// Libraries
#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <DHT.h>

// initialize
#define FanPin 9
#define FanSpeed 3

String status;

int counter = 0;
int rpm = 0;

  // Display
  #define OLED_RESET -1
  #define SCREEN_WIDTH 128
  #define SCREEN_HEIGHT 64
  Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  // WLAN settings
  const char* ssid = "Testnetz";
  const char* password = "28545169310270570382";

  //WebServer mit Port
  AsyncWebServer server(80);

  // DHT
  #define DHTTYPE DHT11
  #define DHTPIN 2 

  DHT dht(DHTPIN, DHTTYPE);

  // temp
  float temp;
  #define solltemp 26
  #define stufe1 solltemp+1
  #define stufe2 solltemp+2
  #define stufe3 solltemp+3
  #define stufe4 solltemp+4
  #define stufe5 solltemp+5

// ----- functions -----

// Display
void initDisplay() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(1000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.display();
}

// WLAN
void wlan() {

  int timer = 40;

  Serial.print("Verbindung wird hergestellt ...");
  display.println("Verbindung wird");
  display.print("hergestellt ...");
  display.display();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && timer >= 0) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
    timer = timer - 1;
  }

  display.clearDisplay();
  display.setCursor(0, 0);

  if (WiFi.status() != WL_CONNECTED) {
    
    display.println("WLAN konnte nicht");
    display.println("verbunden werden!");
    display.display();

    Serial.println();
    Serial.println("WLAN kontte nicht verbunden werden!");

  } else {
  
    display.println("WLAN verbunden.");
    display.println("");
    display.println("SSID: " + String(ssid));
    display.println("");
    display.println("IP: " + WiFi.localIP().toString());
    display.display();

    Serial.println();
    Serial.println("Ich bin mit dem WLAN verbunden!");
  
    Serial.println("IP Adresse: ");
    Serial.println(WiFi.localIP());

  }

  delay(3000);

}

// webserver
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

String website() {
  String html;
  html += "<!DOCTYPE html><html>";
  // head
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  // refresh
  // html += "<meta http-equiv=\"refresh\" content=\"1\">";
  
  // CSS style
  html += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}";
  html += "</style>";

  html += "</head>";

  // body
  html += "<body>";

  html += "<h1>WiFi Server</h1>";

  html += "</body></html>";
  return html;
}

void webserver() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", website());
  });

  server.onNotFound(notFound);

  server.begin();
}

void meassure() {
  counter = 0;
  attachInterrupt(digitalPinToInterrupt(FanSpeed), countup, RISING);
  delay(2000);
  detachInterrupt(digitalPinToInterrupt(FanSpeed));
  rpm = (counter / 2) * 30;
}

void countup() {
  counter++;
}

void setup() {
  Serial.begin(115200);

  // gpio
  pinMode(FanPin, OUTPUT);
  pinMode(FanSpeed, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  initDisplay();
  wlan();
  webserver();

  dht.begin();
  
}

void loop() {

  meassure();

  delay(2000);
  float temp = dht.readTemperature();
  if (temp <= solltemp) { digitalWrite(FanPin,LOW); status = "AUS      "; }else
  if (temp >= stufe5) { digitalWrite(FanPin,HIGH); status = "Stufe-MAX"; }else
  if (temp >= stufe4) { analogWrite(FanPin,190); status = "Stufe-4  "; }else
  if (temp >= stufe3) { analogWrite(FanPin,150); status = "Stufe-3  "; }else
  if (temp >= stufe2) { analogWrite(FanPin,90); status = "Stufe-2  "; }else
  if (temp >= stufe1) { analogWrite(FanPin,40); status = "Stufe-1  "; }else
  if (temp >= solltemp) { analogWrite(FanPin,20); status = "Stufe-LOW"; }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("L\365ftersteuerung");
  display.println("");
  display.println("Aktuell: " + String(temp) + "\337C");
  display.println("");
  display.println("Status: " + status);
  display.println("");
  display.println("Fan RPM: " + String(rpm));
  display.println("Fan Counter: " + String(counter));
  display.display();

}


