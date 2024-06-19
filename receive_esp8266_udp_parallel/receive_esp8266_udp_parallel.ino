// Remotely control Neopixel over WiFi - UDP for parallel strands using FastLED. TODO: Send 4 bytes instead of a long string
// For the WS2811, this file runs the strings in parallel, to allow for much faster "frame rates", around 300 on the ESP8266 vs 150 without parallel.
// ESP8266: WS2811_PORTA - pins 12, 13, 14 and 15 (or pins 6,7,5 and 8 on the NodeMCU boards/pin layout).   From FastLED wiki

#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define LED_COUNT_PER_STRIP 50
#define NUM_STRIPS 3

int port = 8888;
const char *ssid = "SSID";
const char *password = "PASSWORD";

WiFiUDP UDP;
char packet[12];
char reply[] = "A";
char reply_bad[] = "BAD";

int cycle = 0; 
int set_every = 0; // run show() every n assignments

CRGB leds[LED_COUNT_PER_STRIP * NUM_STRIPS];

void setup() {
  Serial.begin(115200);
  Serial.println("Serial Begin");

  FastLED.addLeds<WS2811_PORTA,NUM_STRIPS>(leds, LED_COUNT_PER_STRIP);
  FastLED.setBrightness(255);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    leds[1] = CRGB(255, 0, 0); 
    FastLED.show();
    delay(500);
    leds[1] = CRGB(0, 255, 0); 
    FastLED.show();
  }
  leds[1] = CRGB(0, 255, 0); 
  FastLED.show();
  Serial.println(" \nConnected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Port: ");
  Serial.println(port);
  UDP.begin(port);
  Serial.println("Listening for packets...");
}

void loop() {

  // If packet received...
  int packetSize = UDP.parsePacket();
  if (packetSize) {
    //Serial.print("Received packet! Size: ");
    //Serial.println(packetSize);
    int len = UDP.read(packet, 12);

    if (len > 0)
    {
      packet[len] = '\0';
    }
      //Serial.print("Packet received: ");
      //Serial.println(packet);
    String packetStr(packet);
    String n = packetStr.substring(0, 3);
    String r = packetStr.substring(3, 6);
    String g = packetStr.substring(6, 9);
    String b = packetStr.substring(9, 12);

    leds[n.toInt()] = CRGB(r.toInt(), g.toInt(), b.toInt());
    if (cycle >= set_every) {
        FastLED.show();
        cycle = 0;
    } else {
        cycle += 1;
    }
      //Serial.println(n);
      //Serial.println(r);
      //Serial.println(g);
      //Serial.println(b);

      // Send return packet (needed to prevent overtransmitting and thus missed packets)
    UDP.beginPacket(UDP.remoteIP(), UDP.remotePort());
    UDP.write(reply);
    UDP.endPacket();
    }
  }

