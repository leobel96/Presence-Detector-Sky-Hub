#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

#define ROUTER_URL "http://192.168.0.1"  // Change if different

#define WIFI_SSID <Your WiFi SSID>  // CHANGE ME
#define WIFI_PASSWORD <Your WiFi Password>  // CHANGE ME

// #define _DEBUG  // Uncomment it for debug purposes

#define BOT_TOKEN <Your Telegram Bot Token>  // CHANGE ME
#define CHAT_ID <Your Chat ID>  // CHANGE ME

// Define you friend's list size here
#define LIST_SIZE 3  // Above list's size
String friendName[LIST_SIZE] = {"Friend1", "Friend2", "Friend3"};
String friendMac[LIST_SIZE] = {"aa:bb:cc:dd:ee:ff", "gg:hh:ii:jj:kk:ll", "mm:nn:oo:pp:qq:rr"};

#define NAME_FIELD 0
#define MAC_FIELD 1
#define CONNECTION_FIELD 2

// After 20s that the device hasn't been seen, we consider the friend as not at home
#define LEFT_THRESHOLD 20000

// Update every 5 seconds
#define UPDATE_DELAY 5000
