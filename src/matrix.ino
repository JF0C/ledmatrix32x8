#define FASTLED_ESP8266_RAW_PIN_ORDER
#define PIN1 14 //14 D5
#define NUM_LEDS 256

#include <ArduinoJson.h>
#include <FastLED.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>

CRGB s1[NUM_LEDS];
CRGB paintdata[NUM_LEDS];

ESP8266WebServer server(80);
ESP8266WiFiMulti wifi;
File fsUploadFile;
int MIC = A0;

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
uint8_t wormscol[3] = {255, 196, 77};
uint8_t blank[3] = {0,0,0};
float sineval = 0.0;
bool newtext;
String LocalIp;

enum opmodes{
  text,
  pong,
  fourier,
  worms,
  paint,
  maze
};

struct fourierconfig{
  int maxfreq = 1000;
  int minfreq = 0;
  bool mirror = true;
  CRGB audioCols[6];
  float scale;
}fconf;

struct confstruct{
  float bright = 0.4;
  float velocity = 0.05;
  float pulsing = 0;
  String text;
  opmodes opmode;
  bool initializing = true;
  
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
  unsigned long servertimer;
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
  Serial.println("matrix");
  FastLED.addLeds<WS2813, PIN1>(s1, NUM_LEDS);
  pinMode(MIC, INPUT);
  InitFile();
  loadConfig();
  InitWeb();
  loadPongConf();
  loadBackground();
  loadFourierConf();
}

void loop() {
  dt = millis() - t;
  t = millis();
  sineval = (sin(1.5708*(float)t/300.0)+1.0)/2.0;
  clear_matrix();
  displayText();
  if(!conf.initializing){
    render_fourier();
    render_pong();
    render_worms();
    render_maze();
    copypaint();
  }
  server.handleClient();
  FastLED.show();

  delay(5);
}

// TODO add text buffer [200][8][8] = [200px in x][8px in height][8bits grayscale]
void displayText(){
  if(conf.opmode != text && !conf.initializing) return;
  String text = conf.text;
  if(conf.initializing){
    text = LocalIp;
  }
  int printlength = printString(text, 0, 32);
  conf.pulsing = sin((float)t/1000*3.1416*0.5);
  conf.pulsing *= conf.pulsing;
  if(printlength > 32){
    if(conf.initializing){
      pos -= 0.06;
    }
    else{
      pos += conf.velocity*(float)dt/1000.0;
    }
    if(conf.velocity > 0){
      if(pos >= 0.0){
        printString(text, -printlength - 3, 32);
      }
      if(pos - printlength >= 0){
        pos = -3.0;
      }
    }
    else{
      if(printlength + pos < 32){
        printString(text, printlength + 3, 32);
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
  uint8_t twinklebatch = 0;
  uint8_t dropbatch = 0;
  int k = 0;
  bool pulse = false;
  uint8_t rotate = 0;
  for(int k = 0; k < l; k++){
    if(ismarkup(l-k, &k, "<smile>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "smile", &rotate)+1;
    else if(ismarkup(l-k, &k, "<frown>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "frown", &rotate)+1;
    else if(ismarkup(l-k, &k, "<laugh>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "laugh", &rotate)+1;
    else if(ismarkup(l-k, &k, "<finger>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "finger", &rotate)+1;
    else if(ismarkup(l-k, &k, "<thumb>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "thumb", &rotate)+1;
    else if(ismarkup(l-k, &k, "<heart>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "heart", &rotate)+1;
    else if(ismarkup(l-k, &k, "<moon>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "moonr", &rotate)+1;
    else if(ismarkup(l-k, &k, "<moonl>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "moonl", &rotate)+1;
    else if(ismarkup(l-k, &k, "<moonr>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "moonr", &rotate)+1;
    else if(ismarkup(l-k, &k, "<pill>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "pill", &rotate)+1;
    else if(ismarkup(l-k, &k, "<left>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "left", &rotate)+1;
    else if(ismarkup(l-k, &k, "<right>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "right", &rotate)+1;
    else if(ismarkup(l-k, &k, "<up>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "up", &rotate)+1;
    else if(ismarkup(l-k, &k, "<down>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "down", &rotate)+1;
    else if(ismarkup(l-k, &k, "<RICH>", text.substring(k)))
      offset += smiley(offset + pos, 0, color, getBright(pulse), "RICH", &rotate)+1;
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
      smiley(offset + pos, 0, white, getBright(pulse), "frown", &rotate);
      offset += smiley(offset + pos, 0, green, getBright(pulse), "puke", &rotate)+1;
    }
    else if(ismarkup(l-k, &k, "<smiling>", text.substring(k)))
      offset += smiling(offset + pos, 1, color, getBright(pulse))+1;
    else if(ismarkup(l-k, &k, "<laughing>", text.substring(k)))
      offset += laughing(offset + pos, 1, color, getBright(pulse))+1;
    else if(ismarkup(l-k, &k, "<kissing>", text.substring(k)))
      offset += kissing(offset + pos, 1, color, getBright(pulse))+1;
    else if(ismarkup(l-k, &k, "<twinkling>", text.substring(k)))
      twinkling(color, getBright(pulse), twinklebatch++);
    else if(ismarkup(l-k, &k, "<dripping>", text.substring(k)))
      dripping(color, getBright(pulse), dropbatch++);
    else if(ismarkup(l-k, &k, "<rotx>", text.substring(k)))
      rotate = 1;
    else if(ismarkup(l-k, &k, "<roty>", text.substring(k)))
      rotate = 2;
    else if(ismarkup(l-k, &k, "<rotz>", text.substring(k)))
      rotate = 4;
    else if(ismarkup(l-k, &k, "<pulsing>", text.substring(k)))
      pulse = true;
    else if(ismarkup(l-k, &k, "</pulsing>", text.substring(k)))
      pulse = false;
    else if(ismarkup(l-k, &k, "<ae>", text.substring(k)))
      offset += letter(offset + pos, 0, color, getBright(pulse), "ä")+1;
    else if(ismarkup(l-k, &k, "<oe>", text.substring(k)))
      offset += letter(offset + pos, 0, color, getBright(pulse), "ö")+1;
    else if(ismarkup(l-k, &k, "<ue>", text.substring(k)))
      offset += letter(offset + pos, 0, color, getBright(pulse), "ü")+1;
    else if(ismarkup(l-k, &k, "<AE>", text.substring(k)))
      offset += letter(offset + pos, 0, color, getBright(pulse), "Ä")+1;
    else if(ismarkup(l-k, &k, "<OE>", text.substring(k)))
      offset += letter(offset + pos, 0, color, getBright(pulse), "Ö")+1;
    else if(ismarkup(l-k, &k, "<UE>", text.substring(k)))
      offset += letter(offset + pos, 0, color, getBright(pulse), "Ü")+1;
    else if(ismarkup(l-k, &k, "<tum>", text.substring(k))){
      offset += letter(offset + pos, 0, color, getBright(pulse), "tum1");
      offset += letter(offset + pos, 0, color, getBright(pulse), "tum2");
      offset += letter(offset + pos, 0, color, getBright(pulse), "tum3");
      offset += letter(offset + pos, 0, color, getBright(pulse), "tum4")+1;
    }
    else if(ismarkup(l-k, &k, "<tums>", text.substring(k))){
      offset += letter(offset + pos, 0, color, getBright(pulse), "tums1");
      offset += letter(offset + pos, 0, color, getBright(pulse), "tums2")+1;
    }
    else
      offset += letter(offset + pos, 0, color, getBright(pulse), String(text[k]))+1;
  }
  return offset;
}

float getBright(bool pulsing){
  if(conf.initializing) return 0.9;
  if(pulsing) return conf.bright*conf.pulsing;
  else return conf.bright;
}

bool ismarkup(int l, int* k, String pattern, String text){
  if(pattern.length() > l) return false;
  if(text.startsWith(pattern)){
    *k += (pattern.length() - 1);
    return true;
  }
  return false;
}

void drawxy(int x, int y, uint8_t* color, float f, bool overridedraw){
  if(x >= 32 || y >= 8) return;
  if(x < 0 || y < 0) return;
  //if(color[0] == 0 && color[1] == 0 && color[2] == 0) return;
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

void colorAt(uint8_t x, uint8_t y, uint8_t* col, CRGB* source){
  if(y%2==0){
    col[0] = source[y*32+x].g;
    col[1] = source[y*32+x].r;
    col[2] = source[y*32+x].b;
  }
  else{
    col[0] = source[(y+1)*32 - x-1].g;
    col[1] = source[(y+1)*32 - x-1].r;
    col[2] = source[(y+1)*32 - x-1].b;
  }
}

void loadBackground(){
  if(!SPIFFS.exists("/paints/" + conf.background + ".paint")) return;
  loadpaint(conf.background);
}

int printStringSimple(String str, uint8_t* col, float f, int offset = 0){
  int p = offset;
  for(int k = 0; k < str.length(); k++){
    p += letter(p, 0, col, f, String(str[k]))+1;
  }
  return p;
}

bool wStr2CharArr(String str, char* chr, int len){
  int l = str.length();
  if(l >= len) return false;
  for(int k = 0; k < len; k++){
    if(k >= len) break;
    if(k < l) chr[k] = str[k];
    else chr[k] = 0;
  }
  return true;
}

int getMax(int* array, int size)
{
  int maximum = array[0];
  for (int i = 0; i < size; i++)
  {
    if (array[i] > maximum) maximum = array[i];
  }
  return maximum;
}

uint8_t* colorWheel(float percent) {
  static uint8_t rgb[3];
  if (percent < 0.0 || percent > 1.0) {
    rgb[0] = 0;
    rgb[1] = 0;
    rgb[2] = 0;
  }
  if (percent < 1.0 / 3) {
    rgb[0] = (uint8_t)(255.0 * 3.0 * 2.0 * (1.0 / 3.0 - percent));
    rgb[1] = (uint8_t)(255.0 * 3.0 * 2.0 * (percent - 0.0));
    rgb[2] = 0;
  }
  else if (percent < 2.0 / 3) {
    rgb[0] = 0;
    rgb[1] = (uint8_t)(255.0 * 3.0 * 2.0 * (2.0 / 3.0 - percent));
    rgb[2] = (uint8_t)(255.0 * 3.0 * 2.0 * (percent - 1.0 / 3.0));
  }
  else {
    rgb[0] = (uint8_t)(255.0 * 3.0 * 2.0 * (percent - 2.0 / 3.0));
    rgb[1] = 0;
    rgb[2] = (uint8_t)(255.0 * 3.0 * 2.0 * (1.0 - percent));
  }
  return rgb;
}

void colorWheeltoCRGB(float percent, CRGB* target){
  uint8_t* rgb = colorWheel(percent);
  target->r = rgb[0];
  target->g = rgb[1];
  target->b = rgb[2];
}

String b2s(bool val){
  if(val) return "true";
  return "false";
}
