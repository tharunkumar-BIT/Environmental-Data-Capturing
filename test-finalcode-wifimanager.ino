#include <Arduino.h>
//#include <HttpClient.h>
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
#include "Base64.h"
#include "esp_camera.h"
AsyncWebServer server(80);
camera_config_t config;
//const char* ssid     = "vivo Y21G";   //your network SSID
//const char* password = "Binu@2001";   //your network password
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
  </form><br>
</body></html>)rawliteral";
void notFound(AsyncWebServerRequest* request)
{
  request->send(404, "text/plain", "Not found");
}
void setup() 
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  delay(10);
  WiFiManager wm;
  bool res;
  res = wm.autoConnect("DataCapture","password"); // password protected ap
  if(!res) 
  {
    Serial.println("Failed to connect");
    //ESP.restart();
  } 
  else
  {
    //if you get here you have connected to the WiFi    
    Serial.println("connected...yeey :)");
    Serial.print("Stored SSID: ");
    Serial.println(wm.getWiFiSSID());
    Serial.print("Stored passphrase: ");
    Serial.println(wm.getWiFiPass());
  }
  WiFi.begin(wm.getWiFiSSID(),wm.getWiFiPass());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print(WiFi.localIP()); 
  /*WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi connected");
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());*/
  //camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  config.jpeg_quality = 10;
  config.fb_count = 1;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
  dht.begin();
  pressure_sensor.begin(DOUT, SCLK, GAIN);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request)
  {
    request->send_P(200, "text/html", index_html);
  });
  server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request)
  {
    String inputMessage1;
    String inputParam1;
    String inputMessage2;
    String inputParam2;
    if (request->hasParam(PARAM_INPUT_1))
    {
      inputMessage1 = request->getParam(PARAM_INPUT_1)->value();
      inputParam1 = PARAM_INPUT_1;
    }
    else
    {
      inputMessage1 = "No message sent";
      inputParam1 = "none";
    }
    if (request->hasParam(PARAM_INPUT_2))
    {
      inputMessage2 = request->getParam(PARAM_INPUT_2)->value();
      inputParam2 = PARAM_INPUT_2;
    }
    else
    {
      inputMessage2 = "No message sent";
      inputParam2 = "none";
    }
    spreadsheetUrl = inputMessage2;
    spreadsheetId = extractSpreadsheetId(spreadsheetUrl);
    ScriptDriveUrl = inputMessage1;
    myScript = extractMyScriptFromUrl(ScriptDriveUrl);
    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);
    GSheet.setTokenCallback(tokenStatusCallback);
    GSheet.setPrerefreshSeconds(10 * 60);
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) 
    {
      Serial.printf("Camera init failed with error 0x%x", err);
      delay(1000);
      ESP.restart();
    }
  });
  server.onNotFound(notFound);
  server.begin();
}
boolean enviar = true;
void loop()
{
  //if(enviar) {
  bool ready = GSheet.ready();
  if (ready && millis() - lastTime > timerDelay)
  {
    lastTime = millis();
    FirebaseJson response;
    Serial.println("\nAppend spreadsheet values...");
    Serial.println("----------------------------");
    FirebaseJson valueRange;
    temp = dht.readTemperature();
    hum = dht.readHumidity();
    if (isnan(temp) || isnan(hum))
    {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    if (pressure_sensor.is_ready()) 
    {
      pressurePascal = pressure_sensor.pascal();
      pressureATM = pressure_sensor.atm();
      pressuremmHg = pressure_sensor.mmHg();
      pressurePSI = pressure_sensor.psi();
    } 
    else 
    {
      Serial.println("Pressure sensor not found.");
      return;
    }
    String timestamp = getTimestamp();
    Serial.print("Timestamp: ");
    Serial.println(timestamp);
    valueRange.add("majorDimension", "COLUMNS");
    valueRange.set("values/[0]/[0]", timestamp);
    valueRange.set("values/[1]/[0]", temp);
    valueRange.set("values/[2]/[0]", hum);
    valueRange.set("values/[3]/[0]", pressurePascal);
    valueRange.set("values/[4]/[0]", pressureATM);
    valueRange.set("values/[5]/[0]", pressuremmHg);
    valueRange.set("values/[6]/[0]", pressurePSI);
    bool success = GSheet.values.append(&response /* returned response */, spreadsheetId.c_str() /* spreadsheet Id to append */, "Sheet1!A1" /* range to append */, &valueRange /* data range to append */);
    if (success)
    {
      response.toString(Serial, true);
      valueRange.clear();
    }
    else
    {
      Serial.println(GSheet.errorReason());
    }
    Serial.println();
    Serial.println(ESP.getFreeHeap());
    saveCapturedImage();
    enviar = false;
    delay(60000);
  //}
  }
}
void tokenStatusCallback(TokenInfo info) 
{
  if (info.status == token_status_error)
  {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  }
  else
  {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
  }
}
void saveCapturedImage() 
{
  Serial.println(myScript);
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure client;
  client.setInsecure();
  if (client.connect(myDomain, 443)) 
  {
    Serial.println("Connection successful");
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();  
    if(!fb) 
    {
      Serial.println("Camera capture failed");
      delay(1000);
      ESP.restart();
      return;
    }
    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "";
    for (int i=0;i<fb->len;i++) 
    {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    String Data = myFilename+mimeType+myImage;
    esp_camera_fb_return(fb);
    Serial.println("Send a captured image to Google Drive.");
    client.println("POST " + myScript + " HTTP/1.1");
    client.println("Host: " + String(myDomain));
    client.println("Content-Length: " + String(Data.length()+imageFile.length()));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.println();
    client.print(Data);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index+1000) 
    {
      client.print(imageFile.substring(Index, Index+1000));
    }
    Serial.println("Waiting for response.");
    long int StartTime=millis();
    while (!client.available()) 
    {
      Serial.print(".");
      delay(100);
      if ((StartTime+waitingTime) < millis()) 
      {
        Serial.println();
        Serial.println("No response.");
        //If you have no response, maybe need a greater value of waitingTime
        break;
      }
    }
    Serial.println();   
    while (client.available()) 
    {
      Serial.print(char(client.read()));
    }  
  } 
  else 
  {         
    Serial.println("Connected to " + String(myDomain) + " failed.");
  }
  client.stop();
}
//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++)
    {
      c=str.charAt(i);
      if (c == ' ')
      {
        encodedString+= '+';
      } 
      else if (isalnum(c))
      {
        encodedString+=c;
      } 
      else
      {
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9)
        {
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9)
        {
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}