StaticJsonDocument<800> paintinput;

void paintfromweb(String data){
  DeserializationError err = deserializeJson(paintinput, data);
  if(err){
    Serial.println(F("Error: could not deserialize data"));
    return;
  }
  for(JsonVariant el : paintinput.as<JsonArray>()){
    uint8_t col[3] = {el["r"].as<uint8_t>(), el["g"].as<uint8_t>(), el["b"].as<uint8_t>()};
    paintxy(el["x"].as<int>(), el["y"].as<int>(), &col[0]);
  }
}

void clearpaint(){
  for(int k = 0; k < NUM_LEDS; k++){
    paintdata[k] = 0;
  }
}

void savepaint(String fname){
  String data = "";
  for(int k = 0; k < NUM_LEDS; k++){
    data += String(paintdata[k].r) + "," + String(paintdata[k].g) + "," + String(paintdata[k].b) + ",";
  }
  writeFile("/paints/" + fname + ".paint", data);
}

void loadpaint(String fname){
  Serial.println("loading " + fname);
  if(!SPIFFS.exists("/paints/" + fname + ".paint")){
    Serial.println("/paints/" + fname + ".paint does not exist");
    return; 
  }
  File file = SPIFFS.open("/paints/" + fname + ".paint", "r");
  Serial.println("opened file");
  char buff[10];
  String current = "";
  uint8_t c = 0;
  uint8_t idx = 0;
  ESP.wdtFeed();
  while(file.available()){
    int n = file.readBytesUntil(',', buff, sizeof(buff));
    for(int k = 0; k < n; k++){
      current += buff[k];
    }
    if(c==0) paintdata[idx].r = (uint8_t)current.toInt();
    else if(c==1) paintdata[idx].g = (uint8_t)current.toInt();
    else if(c==2){
      paintdata[idx].b = (uint8_t)current.toInt();
      idx++;
    }
    
    current = "";
    c++;
    c%=3;
    if(idx >= NUM_LEDS) break;
  }
  Serial.println("written " + fname);
}

void paintxy(int x, int y, uint8_t* color){
  if(x >= 32 || y >= 8) return;
  if(x < 0 || y < 0) return;
  if(y%2 == 0){
    draw_color(y*32 + x, color);
  }
  else{
    draw_color((y+1)*32 - x-1, color);
  }
}

void draw_color(uint8_t pos, uint8_t* color){
  paintdata[pos].g = color[0];
  paintdata[pos].r = color[1];
  paintdata[pos].b = color[2];
}

void copypaint(){
  if(!conf.paintmode || conf.pongmode || conf.fouriermode) return;
  for(int k = 0; k < NUM_LEDS; k++){
    s1[k].r = (uint8_t)((float)paintdata[k].r*conf.bright);
    s1[k].g = (uint8_t)((float)paintdata[k].g*conf.bright);
    s1[k].b = (uint8_t)((float)paintdata[k].b*conf.bright);
  }
}
