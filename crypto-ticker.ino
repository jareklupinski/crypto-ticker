// Prerequisities:
// - Install the ESP32 framework by going to File -> Preferences and adding this line to Additional Boards Manager URL: 
// -- https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
// - Install the required libraries using Tools -> Manage Libraries, searching for these names, and installing the latest version of each:
// -- M5Stack
// -- ArduinoJSON
// -- ArduinoWebsockets
// - Sign up at https://www.cryptocompare.com/cryptopian/api-keys to get an API key, then paste it here:
#define API_KEY     ""
#define WIFI_SSID   ""            // Your WiFi Network Name
#define WIFI_PASS   ""            // WiFi Network Password for above network
#define TICKER1     "BTC"         // Ticker symbols to follow, check https://min-api.cryptocompare.com/documentation/websockets?key=Channels&cat=Ticker
#define TICKER2     "ETH"
#define TICKER3     "ICP"

// No user-serviceable parts beyond this line :P
#include <WiFi.h>
#include <M5Stack.h>
#include <ArduinoJson.h>
#include <ArduinoWebsockets.h>

// The root certificate for cryptocompare.com
const char* ssl_ca_cert = \
                          "-----BEGIN CERTIFICATE-----\n"\
                          "MIIEADCCAuigAwIBAgIBADANBgkqhkiG9w0BAQUFADBjMQswCQYDVQQGEwJVUzEh\n"\
                          "MB8GA1UEChMYVGhlIEdvIERhZGR5IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBE\n"\
                          "YWRkeSBDbGFzcyAyIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTA0MDYyOTE3\n"\
                          "MDYyMFoXDTM0MDYyOTE3MDYyMFowYzELMAkGA1UEBhMCVVMxITAfBgNVBAoTGFRo\n"\
                          "ZSBHbyBEYWRkeSBHcm91cCwgSW5jLjExMC8GA1UECxMoR28gRGFkZHkgQ2xhc3Mg\n"\
                          "MiBDZXJ0aWZpY2F0aW9uIEF1dGhvcml0eTCCASAwDQYJKoZIhvcNAQEBBQADggEN\n"\
                          "ADCCAQgCggEBAN6d1+pXGEmhW+vXX0iG6r7d/+TvZxz0ZWizV3GgXne77ZtJ6XCA\n"\
                          "PVYYYwhv2vLM0D9/AlQiVBDYsoHUwHU9S3/Hd8M+eKsaA7Ugay9qK7HFiH7Eux6w\n"\
                          "wdhFJ2+qN1j3hybX2C32qRe3H3I2TqYXP2WYktsqbl2i/ojgC95/5Y0V4evLOtXi\n"\
                          "EqITLdiOr18SPaAIBQi2XKVlOARFmR6jYGB0xUGlcmIbYsUfb18aQr4CUWWoriMY\n"\
                          "avx4A6lNf4DD+qta/KFApMoZFv6yyO9ecw3ud72a9nmYvLEHZ6IVDd2gWMZEewo+\n"\
                          "YihfukEHU1jPEX44dMX4/7VpkI+EdOqXG68CAQOjgcAwgb0wHQYDVR0OBBYEFNLE\n"\
                          "sNKR1EwRcbNhyz2h/t2oatTjMIGNBgNVHSMEgYUwgYKAFNLEsNKR1EwRcbNhyz2h\n"\
                          "/t2oatTjoWekZTBjMQswCQYDVQQGEwJVUzEhMB8GA1UEChMYVGhlIEdvIERhZGR5\n"\
                          "IEdyb3VwLCBJbmMuMTEwLwYDVQQLEyhHbyBEYWRkeSBDbGFzcyAyIENlcnRpZmlj\n"\
                          "YXRpb24gQXV0aG9yaXR5ggEAMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcNAQEFBQAD\n"\
                          "ggEBADJL87LKPpH8EsahB4yOd6AzBhRckB4Y9wimPQoZ+YeAEW5p5JYXMP80kWNy\n"\
                          "OO7MHAGjHZQopDH2esRU1/blMVgDoszOYtuURXO1v0XJJLXVggKtI3lpjbi2Tc7P\n"\
                          "TMozI+gciKqdi0FuFskg5YmezTvacPd+mSYgFFQlq25zheabIZ0KbIIOqPjCDPoQ\n"\
                          "HmyW74cNxA9hi63ugyuV+I6ShHI56yDqg+2DzZduCLzrTia2cyvk0/ZM/iZx4mER\n"\
                          "dEr/VxqHD3VILs9RaRegAhJhldXRQLIQTO7ErBBDpqWeCtWVYpoNz4iCxTIM5Cuf\n"\
                          "ReYNnyicsbkqWletNw+vHX/bvZ8=\n"\
                          "-----END CERTIFICATE-----\n";

String ticker1symbol = TICKER1;
String ticker2symbol = TICKER2;
String ticker3symbol = TICKER3;
String ticker1price;
String ticker2price;
String ticker3price;

using namespace websockets;
DynamicJsonDocument doc(4096);
WebsocketsClient client;

void onMessageCallback(WebsocketsMessage message) {
  // Debug: print out message received from websockets
  Serial.print("Got Message: ");
  Serial.println(message.data());

  // Unpack message received from ticker
  String socketMessage = message.data();
  DeserializationError error = deserializeJson(doc, socketMessage);
  if (error) {
    Serial.printf("deserializeJson() failed: %s\n", error.c_str());
    return;
  }

  // See if message contains a price
  String price = doc["PRICE"];
  if (price == "null") {
    return;
  }

  // Set price for currently captured message symbol
  String fromSymbol = doc["FROMSYMBOL"];
  // TODO functional programming
  if (fromSymbol == ticker1symbol) {
    ticker1price = price;
  }
  if (fromSymbol == ticker2symbol) {
    ticker2price = price;
  }
  if (fromSymbol == ticker3symbol) {
    ticker3price = price;
  }

  // Display prices on the screen
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("%s:%s\n\n", ticker1symbol, ticker1price);
  M5.Lcd.printf("%s:%s\n\n", ticker2symbol, ticker2price);
  M5.Lcd.printf("%s:%s", ticker3symbol, ticker3price);
}

void onEventsCallback(WebsocketsEvent event, String data) {
  switch (event) {
    case WebsocketsEvent::ConnectionOpened:
      Serial.println("Connnection Opened");
      break;
    case WebsocketsEvent::ConnectionClosed:
      Serial.println("Connnection Closed");
      break;
    case WebsocketsEvent::GotPing:
      Serial.println("Got a Ping!");
      break;
    case WebsocketsEvent::GotPong:
      Serial.println("Got a Pong!?!?");
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // Start M5 Support Library
  M5.begin();
  M5.Power.begin();

  // Initialize Display
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(3);

  M5.Lcd.print("Connecting to Wifi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    M5.Lcd.print(".");
    delay(1000);
  }
  M5.Lcd.println("\nConnected!");

  M5.Lcd.println("Registering Webhook callbacks");
  client.onMessage(onMessageCallback);
  client.onEvent(onEventsCallback);

  M5.Lcd.println("Setting Certificate");
  client.setCACert(ssl_ca_cert);

  M5.Lcd.println("Starting Websocket Connection");
  String connection_url = "wss://streamer.cryptocompare.com/v2?api_key=";
  connection_url = connection_url + API_KEY;
  client.connect(connection_url);

  M5.Lcd.println("Requesting Ticker Symbols");
  String connection_request = "";
  connection_request = connection_request+"{\"action\":\"SubAdd\", \"subs\":[\"5~CCCAGG~"+TICKER1+"~USD\",\"5~CCCAGG~"+TICKER2+"~USD\",\"5~CCCAGG~"+TICKER3+"~USD\"]}";
  client.send(connection_request);
  
  M5.Lcd.setTextSize(4);
  M5.Lcd.clear();
}

void loop() {
  // Check for new messages
  client.poll();
}
