#include <Arduino.h>
#include <WiFiManager.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include "time.h"
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>
#include "HX710B.h"
#include <WiFiClientSecure.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include "mbedtls/base64.h"  // Include mbedtls for Base64 encoding
AsyncWebServer server(80);
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* myDomain = "script.google.com";
#define PROJECT_ID "sensor-data-dht11"
#define CLIENT_EMAIL "sensor-data-dht11@sensor-data-dht11.iam.gserviceaccount.com"
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQChgYgv8UBM/nAg\nKn/THTSJf0Ov1TBef5a1vcsAwaF3LtLYbz1ypOJnT3YC9rjJBlJ4Vl3ApoKJE5zC\nAISt+j8f0jMmewOPDe8I3FY7ldN6Cu6Vgua3ReG0eCuIqYUfRLlAB/eZSndPqNJb\ngpMJpwHSVE2kTUUKWsxaA224Bs/GMTWxJQkPysm5e5i6CxkAnmE1pfOlkkn9nlLd\njmM0H7gxpw48MF5Z77XlfJuHdn6XY5RgB67pkNIrv2+3zJOxgp/c7VvCTwNAXDxx\nt9opXbCu88kZIVO4RJk+cNdai02sH/40RLTNKeBURCMLoMpHt7A7H6vtjZiWSOeh\nMK3DFtaZAgMBAAECggEACwOv+4EK/xU9JVxwoj+0JQpqXdP+u/sclqFudks+TUgC\ntiMEAE0RabVIGZSk7oTBXBabxzcz4NBXGP37DnhqFKv7tAygpgFvw11vfhnONd0H\n/2dRf79C0NF/Oo9BFmxT6/N/JdOnxmV/jyEdEQP0QllggCuK4VVm6V6hfXLteZkm\nz2CE/8JWwlQgtvTPjAReLtk/2JESrrC97ZXQohoHUrZLuHqGd2SehVfsiSCGZ09c\nfTnZHHkYzm0tj4W3jysFx/aOzmaGBNnuthbJ2xytxs+1QBYStCQ95YteYqcDaelj\nAKmKlDaFqANNcGaWzA3BCa2JHBdhmaOwk/s6nDLRXwKBgQDQk3D6GJ0PAxiyf4eY\nR6jLpT7XIdKhC3JqL97l4TQvkXnq8p7WYBl/2RWgPvIUfazmXV/abnUvJ3mhg8WV\nELXzL/wixVZ07iVLQC3X29Sc8bmSdzJYj0XyEoEIeEf90mFRgmlJLUPRlbMAIhWT\nY9xqByr/P+SWUOXANg3XVwZXfwKBgQDGOktc7q6ot8R36HZ421SYcbTEmtYDK8Sa\nVVJYLzomnFxPNASpSYhvCpKTjTBjH0EZ8a6nwfn4cbOT6Y0as1LvWM+6jSWqkODo\nix7YwQhhwErn+WfgXuybySWuaBFrdmFnW6cOKZZHMGD/at+3J2l1mojrebXbSvSK\nGN5bicud5wKBgQC4dIwHQVk0e4KN6SabT1RBtMOcMfc6IQtwSSGQntct8jOlyica\nYyZp4Cu16bTg1jvgBlfMrr7HktuIMSa9K2gdxyPuBCLvieRFZl6kdW7T1RpOI0fM\n9fffmumt+eGA4VrbYL+HzZYadY814O6/yCKXSfuGfJTFMoncsHERLY8YNQKBgQCz\nVSW9C4ojxS0PNdWIfRTJXYUmoNWgxMX2wd3fPcqkB61TUkItTQMLQktdBcySwEHL\nUOSrjEezRloqJI6tazJKqtQzd2JuLufqOYJMokYM+URLHU2mpPn17kISyL+Jsu1n\n7njxzRnRiakXKf7+2b8E0CuyrMejfupAzULNGZqGXQKBgFyv+diWG1+CYGWp8eOR\nQZoX9XJuIF9RY90fjlfJkqzJVSHo3mCPdifmJq2fC/anWmkv5qGBuSNG8W/rc3xq\nVzu2ngubWYRyc6qmbqkuFSDPTtBtEz3KtBJgMu6KM4ntN9hmsgbnsQnxFEdwx6si\nw6yGYzyWsMc/dUieiOjSaJMl\n-----END PRIVATE KEY-----\n";
String myFilename = "filename=ESP32_SERVICE.jpg";
String mimeType = "&mimetype=image/jpeg";
String myImage = "&data=";
int waitingTime = 30000;
String spreadsheetUrl;
String spreadsheetId;
String ScriptDriveUrl;
String myScript;
String extractMyScriptFromUrl(String url) {
  int startIndex = url.indexOf("/macros/");
  if (startIndex == -1) {
    Serial.println("Error: Invalid DriveUrl format.");
    return "";
  }
  return url.substring(startIndex);
}

String extractSpreadsheetId(String url)
{
  int idStart = url.indexOf("/d/") + 3;
  int idEnd = url.indexOf("/edit");
  return url.substring(idStart, idEnd);
}

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

void tokenStatusCallback(TokenInfo info);

#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float temp;
float hum;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 0;

const int DOUT = 13;   
const int SCLK = 12;   
const int GAIN = 128;  
HX710B pressure_sensor;

float pressurePascal;
float pressureATM;
float pressuremmHg;
float pressurePSI;

String getTimestamp()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    return "Failed to obtain time";
  }
  char timeStringBuff[50];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(timeStringBuff);
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Environmental Image Capturing and Storing</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Script URL: <input type="text" name="input1">
    GSheet URL: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest* request) {
  request->send(404, "text/plain", "Not found");
}

void setup()
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  pinMode(4, OUTPUT);
  dht.begin();
  pressure_sensor.begin(DOUT, SCLK, GAIN);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  WiFiManager wm;
  WiFi.mode(WIFI_STA);
  bool res;
  res = wm.autoConnect("ESP32_Device", "password");
  if (!res) {
    Serial.println("Failed to connect");
    ESP.restart();
  } else {
    Serial.println("connected...yeey :)");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
    String inputMessage1;
    String inputMessage2;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      ScriptDriveUrl = inputMessage1;
      myScript = extractMyScriptFromUrl(ScriptDriveUrl);
    } else {
      inputMessage1 = "No message sent";
    }
    if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      spreadsheetUrl = inputMessage2;
      spreadsheetId = extractSpreadsheetId(spreadsheetUrl);
    } else {
      inputMessage2 = "No message sent";
    }
    request->send(200, "text/html", "ScriptDriveUrl: " + ScriptDriveUrl + "<br>GsheetUrl: " + spreadsheetUrl);
  });

  server.onNotFound(notFound);
  server.begin();
}

String sendImagePost(String myScript, String myData) {
  String response = "";
  String body = myFilename + mimeType + myImage + myData;
  WiFiClientSecure client;
  client.setInsecure();
  if (!client.connect(myDomain, 443)) {
    Serial.println("Connection to Google script failed!");
    return "";
  }

  client.print(String("POST ") + myScript + " HTTP/1.1\r\n" +
               "Host: " + myDomain + "\r\n" +
               "Content-Type: application/x-www-form-urlencoded\r\n" +
               "Content-Length: " + body.length() + "\r\n" +
               "Connection: close\r\n\r\n" +
               body);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return "";
    }
  }

  while (client.available()) {
    response += client.readString();
  }
  client.stop();
  return response;
}

String base64EncodeImage(camera_fb_t* fb) {
  size_t outputLen = 0;
  String imageFile;
  size_t encodedLen = 4 * ((fb->len + 2) / 3);
  unsigned char* output = (unsigned char*)malloc(encodedLen);
  if (output) {
    mbedtls_base64_encode(output, encodedLen, &outputLen, fb->buf, fb->len);
    imageFile = String((char*)output);
    free(output);
  }
  return imageFile;
}

String takePhoto() {
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "";
  }
  String imageFile = base64EncodeImage(fb);
  esp_camera_fb_return(fb);
  return imageFile;
}

void loop()
{
  if ((millis() - lastTime) > timerDelay) {
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    pressurePascal = pressure_sensor.read();
    pressureATM = pressurePascal / 101325.0;
    pressuremmHg = pressurePascal * 0.00750062;
    pressurePSI = pressurePascal * 0.000145038;

    Serial.printf("Temperature: %.2f °C\n", temp);
    Serial.printf("Humidity: %.2f %%\n", hum);
    Serial.printf("Pressure: %.2f Pa\n", pressurePascal);
    Serial.printf("Pressure: %.6f atm\n", pressureATM);
    Serial.printf("Pressure: %.6f mmHg\n", pressuremmHg);
    Serial.printf("Pressure: %.6f psi\n", pressurePSI);

    String picture = takePhoto();
    if (picture != "") {
      String response = sendImagePost(myScript, picture);
      Serial.println(response);
    }

    lastTime = millis();
  }
}
