#include "main.h"

unsigned long lastUpdate = 0;
bool atHome[LIST_SIZE];
unsigned long lastTimeSeen[LIST_SIZE];

HTTPClient http;
WiFiClient client;

const String sendGetRequest(String endpoint, bool secure) {

  bool successful = false;
  String resp = "";

  // Sanitize endpoint
  endpoint.replace(" ", "+");

  if (secure) {
    WiFiClientSecure client;
    client.setInsecure();
    successful = http.begin(client, endpoint);
  } else {
    WiFiClient client;
    successful = http.begin(client, endpoint);
  }

  Serial.printf("Sending: %s\n", endpoint.c_str());

  if (successful) {
    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      resp = http.getString();
      http.end();
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
  } else {
    Serial.print("Begin unsuccessful");
  }
  return resp;
}

// Send Telegram message
void notify(String msg) {
  char buffer[500];
  sprintf(buffer, "https://api.telegram.org/bot%s/sendMessage?chat_id=%s&text=%s", BOT_TOKEN, CHAT_ID, msg.c_str());
  sendGetRequest(String(buffer), true);
}

// Extract just the relevant part of the Hub response
String getDevicesStr(String resp) {
  int start = resp.indexOf("attach_dev = ") + 14;
  int stop = resp.indexOf(";", start);
  return resp.substring(start, stop);
}

// Check if someone arrived at home or left
void findFriends(String resp) {
  int newTokenIdx = resp.indexOf("<lf>");
  bool lastDevice = false;

  while (lastDevice == false) {
    newTokenIdx = resp.indexOf("<lf>");
    if (newTokenIdx == -1) {
      newTokenIdx = resp.indexOf(";");
      lastDevice = true;
    }
    String deviceSubstr = resp.substring(0, newTokenIdx);
    resp.remove(0, newTokenIdx + 4);
    #ifdef _DEBUG
    Serial.printf("deviceSubstr: %s\n", deviceSubstr.c_str());
    #endif

    bool lastField = false;
    uint8_t fieldIdx = 0;
    while (lastField == false) {
      int commaIdx = deviceSubstr.indexOf(",");
      if (commaIdx == -1) {
        commaIdx = deviceSubstr.length();
        lastField = true;
      }
      String field = deviceSubstr.substring(0, commaIdx);

      #ifdef _DEBUG
      Serial.printf("field%d=%s ", fieldIdx, field.c_str());
      #endif
      if (fieldIdx == MAC_FIELD) {
        for (uint8_t i = 0; i < LIST_SIZE; i++) {
          #ifdef _DEBUG
          Serial.printf("Comparing %s against %s ", friendMac[i].c_str(), field.c_str());
          #endif
          if (field.equals(friendMac[i])) {
            #ifdef _DEBUG
            Serial.println("Match");
            #endif
            lastTimeSeen[i] = millis();
            if (!atHome[i]) {
              char buffer[60];
              sprintf(buffer, "%s just arrived! 😊", friendName[i].c_str());
              Serial.println(buffer);
              notify(buffer);
              atHome[i] = true;
            }
          } else if (atHome[i] && (millis() - lastTimeSeen[i] > LEFT_THRESHOLD)) {
            char buffer[60];
            sprintf(buffer, "%s left 😔", friendName[i].c_str());
            Serial.println(buffer);
            notify(buffer);
            atHome[i] = false;
          }
        }
      }
      deviceSubstr.remove(0, commaIdx + 1);
      fieldIdx ++;
    }
    #ifdef _DEBUG
    Serial.println();
    #endif
  }
}

void setup() {
  delay(1000);
  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);  //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);  //This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.println("");

  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
  http.setReuse(true);
  notify("Bot started");
}

void loop() {
  // Update every updateDelay milliseconds
  if ((millis() - lastUpdate) > UPDATE_DELAY) {
    //Check WiFi connection status
    if(WiFi.status() == WL_CONNECTED){
      String devicesStr = getDevicesStr(sendGetRequest(ROUTER_URL, false));
      #ifdef _DEBUG
      Serial.print(devicesStr);
      #endif
      findFriends(devicesStr);
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastUpdate = millis();
  }
}