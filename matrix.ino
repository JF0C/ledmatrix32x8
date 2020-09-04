#define FASTLED_ESP8266_RAW_PIN_ORDER
#define PIN1 14 //D5
#define NUM_LEDS 256

#include <ArduinoJson.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>

CRGB s1[NUM_LEDS];
CRGB paintdata[NUM_LEDS];

ESP8266WebServer server(80);
File fsUploadFile;

unsigned long t = 0;
uint8_t dt = 0;
float pos = 0.0;
uint8_t white[3] = {255, 255, 255};
uint8_t purple[3] = {186, 0, 255};
uint8_t orange[3] = {255, 200, 5};
uint8_t blue[3] = {0, 0, 255};
uint8_t red[3] = {255, 0, 0};
uint8_t green[3] = {0, 255, 0};
uint8_t yellow[3] = {255, 255, 0};
uint8_t cyan[3] = {0, 255, 255};
uint8_t magenta[3] = {255, 0, 255};
uint8_t rich[3] = {255, 109, 12};

struct confstruct{
  float bright = 0.4;
  float velocity = 0.05;
  String text;
  char ssid[50] = "R.I.C.H.";
  char pw[50] = "r1chl1k35b33r4nd$";
  bool pongmode = false;
  bool paintmode = false;
  String background = "";
  float bgbright = 0.2;
  uint8_t bgr;
  uint8_t bgg;
  uint8_t bgb;
  
  uint8_t paintr;
  uint8_t paintg;
  uint8_t paintb;
  uint8_t painthard;
  uint8_t paintsize;
  bool painterase;
}conf;

struct pongconfiguration{
  bool player1, player2;
  String name_p1, name_p2;
  int pos_p1, pos_p2, points_p1, points_p2, limit=5;
  float ballx, bally;
  float vx, vy;
  unsigned long tstart;
}pconf;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("dragon_node");
  FastLED.addLeds<WS2813, PIN1>(s1, NUM_LEDS);
  InitFile();
  InitWeb();
  loadConfig();
  loadPongConf();
  loadBackground();
}

void loop() {
  // put your main code here, to run repeatedly:
  dt = millis() - t;
  t = millis();
  clear_matrix();
  displayText();
  render_pong();
  copypaint();
  server.handleClient();
  FastLED.show();
  //Serial.println("cycle: " + String(dt));
  delay(5);
}

void displayText(){
  if(conf.pongmode || conf.paintmode) return;
  int printlength = printString(conf.text, 0, 32);
  
  if(printlength > 32){
    pos += conf.velocity*(float)dt/1000.0;
    if(conf.velocity > 0){
      if(pos >= 0.0){
        printString(conf.text, -printlength - 3, 32);
      }
      if(pos - printlength >= 0){
        pos = -3.0;
      }
    }
    else{
      if(printlength + pos < 32){
        printString(conf.text, printlength + 3, 32);
      }
      if(pos + printlength <= 0){
        pos = 3.0;
      }
    }
  }
  else{
    pos = (32-printlength)/2;
  }
}

int printString(String text, int offset, int len){
  drawBackground();
  int l = text.length();
  uint8_t color[3];
  colcp(&white[0], &color[0]);
  int k = 0;
  for(int k = 0; k < l; k++){
    if(ismarkup(l-k, &k, "<smile>", text.substring(k))) 
      offset += smiley(offset + pos, 0, color, conf.bright, "smile")+1;
    else if(ismarkup(l-k, &k, "<frown>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "frown")+1;
    else if(ismarkup(l-k, &k, "<laugh>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "laugh")+1;
    else if(ismarkup(l-k, &k, "<finger>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "finger")+1;
    else if(ismarkup(l-k, &k, "<thumb>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "thumb")+1;
    else if(ismarkup(l-k, &k, "<heart>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "heart")+1;
    else if(ismarkup(l-k, &k, "<moon>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "moonr")+1;
    else if(ismarkup(l-k, &k, "<moonl>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "moonl")+1;
    else if(ismarkup(l-k, &k, "<moonr>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "moonr")+1;
    else if(ismarkup(l-k, &k, "<pill>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "pill")+1;
    else if(ismarkup(l-k, &k, "<left>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "left")+1;
    else if(ismarkup(l-k, &k, "<right>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "right")+1;
    else if(ismarkup(l-k, &k, "<up>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "up")+1;
    else if(ismarkup(l-k, &k, "<down>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, conf.bright, "down")+1;
    else if(ismarkup(l-k, &k, "<blue>", text.substring(k)))
      colcp(&blue[0], &color[0]);
    else if(ismarkup(l-k, &k, "<red>", text.substring(k)))
      colcp(&red[0], &color[0]);
    else if(ismarkup(l-k, &k, "<white>", text.substring(k)))
      colcp(&white[0], &color[0]);
    else if(ismarkup(l-k, &k, "<blue>", text.substring(k)))
      colcp(&blue[0], &color[0]);
    else if(ismarkup(l-k, &k, "<purple>", text.substring(k)))
      colcp(&purple[0], &color[0]);
    else if(ismarkup(l-k, &k, "<green>", text.substring(k)))
      colcp(&green[0], &color[0]);
    else if(ismarkup(l-k, &k, "<yellow>", text.substring(k)))
      colcp(&yellow[0], &color[0]);
    else if(ismarkup(l-k, &k, "<magenta>", text.substring(k)))
      colcp(&magenta[0], &color[0]);
    else if(ismarkup(l-k, &k, "<rich>", text.substring(k)))
      colcp(&rich[0], &color[0]);
    else if(ismarkup(l-k, &k, "<cyan>", text.substring(k)))
      colcp(&cyan[0], &color[0]);
    else if(ismarkup(l-k, &k, "<puke>", text.substring(k))){
      smiley(offset + pos, 0, white, conf.bright, "frown");
      offset += smiley(offset + pos, 0, green, conf.bright, "puke")+1;
    }
    else if(ismarkup(l-k, &k, "<smiling>", text.substring(k)))
      offset += smiling(offset + pos, 1, color, conf.bright)+1;
    else if(ismarkup(l-k, &k, "<laughing>", text.substring(k)))
      offset += laughing(offset + pos, 1, color, conf.bright)+1;
    else if(ismarkup(l-k, &k, "<kissing>", text.substring(k)))
      offset += kissing(offset + pos, 1, color, conf.bright)+1;
    else if(ismarkup(l-k, &k, "<twinkling>", text.substring(k)))
      twinkling(color, conf.bright);
    else if(ismarkup(l-k, &k, "<ae>", text.substring(k)))
      offset += letter(offset + pos, 0, color, conf.bright, "ä")+1;
    else if(ismarkup(l-k, &k, "<oe>", text.substring(k)))
      offset += letter(offset + pos, 0, color, conf.bright, "ö")+1;
    else if(ismarkup(l-k, &k, "<ue>", text.substring(k)))
      offset += letter(offset + pos, 0, color, conf.bright, "ü")+1;
    else if(ismarkup(l-k, &k, "<AE>", text.substring(k)))
      offset += letter(offset + pos, 0, color, conf.bright, "Ä")+1;
    else if(ismarkup(l-k, &k, "<OE>", text.substring(k)))
      offset += letter(offset + pos, 0, color, conf.bright, "Ö")+1;
    else if(ismarkup(l-k, &k, "<UE>", text.substring(k)))
      offset += letter(offset + pos, 0, color, conf.bright, "Ü")+1;
    else if(ismarkup(l-k, &k, "<tum>", text.substring(k))){
      offset += letter(offset + pos, 0, color, conf.bright, "tum1");
      offset += letter(offset + pos, 0, color, conf.bright, "tum2");
      offset += letter(offset + pos, 0, color, conf.bright, "tum3");
      offset += letter(offset + pos, 0, color, conf.bright, "tum4")+1;
    }
    else if(ismarkup(l-k, &k, "<tums>", text.substring(k))){
      offset += letter(offset + pos, 0, color, conf.bright, "tums1");
      offset += letter(offset + pos, 0, color, conf.bright, "tums2")+1;
    }
    else
      offset += letter(offset + pos, 0, color, conf.bright, String(text[k]))+1;
  }
  return offset;
}

bool ismarkup(int l, int* k, String pattern, String text){
  if(pattern.length() > l) return false;
  if(text.startsWith(pattern)){
    *k += (pattern.length() - 1);
    return true;
  }
}

void drawxy(int x, int y, uint8_t* color, float f, bool overridedraw){
  if(x >= 32 || y >= 8) return;
  if(x < 0 || y < 0) return;
  if(y%2 == 0){
    draw_pixel(y*32 + x, color, f, overridedraw);
  }
  else{
    draw_pixel((y+1)*32 - x-1, color, f, overridedraw);
  }
}
void draw_pixel(uint8_t pos, uint8_t* color, float f, bool overridedraw) {
  if(pos >= NUM_LEDS) return;
  /*
  uint16_t sum1 = (uint16_t)s1[pos].g + (uint16_t)s1[pos].r + (uint16_t)s1[pos].b;
  uint16_t sum2 = floor(f * (float)(color[0] + color[1] + color[2]));

  if (sum2 > sum1 || overridedraw) {
    s1[pos].g = floor(f * (float)color[0]);
    s1[pos].r = floor(f * (float)color[1]);
    s1[pos].b = floor(f * (float)color[2]);
  }
  */
  if (overridedraw) {
    s1[pos].g = floor(f * (float)color[0]);
    s1[pos].r = floor(f * (float)color[1]);
    s1[pos].b = floor(f * (float)color[2]);
    return;
  }
  s1[pos].g = s1[pos].g + floor((1.0-((float)s1[pos].g/255.0))*f*(float)color[0]);
  s1[pos].r = s1[pos].r + floor((1.0-((float)s1[pos].r/255.0))*f*(float)color[1]);
  s1[pos].b = s1[pos].b + floor((1.0-((float)s1[pos].b/255.0))*f*(float)color[2]);
}

void clear_matrix(){
  for(int k = 0; k < NUM_LEDS; k++){
    s1[k] = 0;
  }
}

void colcp(uint8_t* src, uint8_t* dst){
  for(int k = 0; k < 3; k++){
    dst[k] = src[k];
  }
}

void drawBackground(){
  if(conf.background == "") return;
  if(conf.background == "<color>"){
    uint8_t col[3] = {conf.bgr, conf.bgg, conf.bgb};
    for(int k = 0; k < NUM_LEDS; k++){
      draw_pixel(k, &col[0], conf.bright*conf.bgbright, false);
    }
  }
  else{
    for(int k = 0; k < NUM_LEDS; k++){
      uint8_t col[3] = {paintdata[k].g, paintdata[k].r, paintdata[k].b};
      draw_pixel(k, &col[0], conf.bright*conf.bgbright, false);
    }
  }
}

void loadBackground(){
  if(!SPIFFS.exists("/paints/" + conf.background + ".paint")) return;
  loadpaint(conf.background);
}