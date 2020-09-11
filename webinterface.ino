void InitWeb(){
  Serial.println(F("\nstarting wifi"));
  WiFi.softAP("matrix", "tsvrxvu2");
//  WiFi.softAPConfig(local_ip, gateway, subnet);
  Serial.println(F("creating acces point"));

  
  WiFi.begin(conf.ssid, conf.pw);
  //delay(100);
  if(WiFi.status() != WL_CONNECTED)
    tryOtherWifis();
  
  if (!MDNS.begin("matrix")) {             // Start the mDNS responder for node.local
    Serial.println(F("Error setting up MDNS responder!"));
  }

  server.on(F("/pong"), HTTP_POST, handlepong);
  server.on(F("/set"), HTTP_POST, handleset);
  server.on(F("/conf"), HTTP_GET, showconfig);
  server.on(F("/framerate"), HTTP_GET, handleframerate);
  server.on(F("/paint"), HTTP_POST, handlepaint);
  server.on(F("/fourier"), HTTP_POST, handlefourier);
  server.on(F("/debugscreen"), HTTP_GET, sendscreen);
  server.on(F("/listfiles"), HTTP_GET, listfiles);
  server.on(F("/changefile"), HTTP_POST, handleFileChange);
  server.on(F("/worms"), HTTP_POST, handleworms);
  server.on(F("/addwifi"), HTTP_POST, handleAddWifi);
  server.on(F("/upload"), HTTP_GET,[](){
    server.send(200, "text/html", F("<form method=\"post\" enctype=\"multipart/form-data\"><input type=\"file\" name=\"name\"><input class=\"button\" type=\"submit\" value=\"Upload\"></form>"));
    });
  server.on("/upload", HTTP_POST,                       // if the client posts to the upload page
    [](){ server.send(200); },                          // Send status 200 (OK) to tell the client we are ready to receive
    handleFileUpload                                    // Receive and save the file
  );
  server.onNotFound([]() {                              // If the client requests any URI
    if (!handleFileRead(server.uri()))                  // send it if it exists
      server.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
  });
  server.begin();
  Serial.println(F("wifi started"));
}

void handleset(){
  String message = "invalid command";
  for (int i = 0; i < server.args(); i++) {
    String argname = server.argName(i);
    String value = server.arg(i);
    if(argname == "text"){
      conf.opmode = text;
      writeConfig("text", value);
      conf.text = value;
      message = "text set to: " + value;
    }
    else if(argname == F("velocity")){
      float velocity = value.toFloat();
      if(velocity > 15.0){
        velocity = 15.0;
      }
      if(velocity < -15.0){
        velocity = -15.0;
      }
      writeConfig("velocity", value);
      message = "set velocity to: " + value;
      conf.velocity = velocity;
    }
    else if(argname == "bright"){
      float bright = value.toFloat();
      if(bright > 1.0){
        bright = 1.0;
      }
      if(bright < 0.0){
        bright = 0.0;
      }
      writeConfig("bright", String(bright));
      conf.bright = bright;
      message = "set brightness to " + String(bright);
    }
    else if(argname == "lasttexts"){
      writeConfig("lasttexts", value);
      message = "last texts: " + value;
    }
    else if(argname == "background"){
      if(SPIFFS.exists("/paints/" + value + ".paint")){
        conf.background = value;
        writeConfig("background", conf.background);
        loadBackground();
        message = "loaded " + value;
      }
      else if(value == "<off>"){
        conf.background = "";
        writeConfig("background", "");
        message = F("background off");
      }
      else if(value.startsWith("color:")){
        conf.background = "<color>";
        String col = value.substring(value.indexOf(":")+1);
        conf.bgr = (uint8_t)col.substring(0, col.indexOf(",")).toInt();
        col = col.substring(col.indexOf(",")+1);
        conf.bgg = (uint8_t)col.substring(0, col.indexOf(",")).toInt();
        col = col.substring(col.indexOf(",")+1);
        conf.bgb = (uint8_t)col.toInt();
        writeConfig("background", String(conf.background));
        writeConfig("bgr", String(conf.bgr));
        writeConfig("bgg", String(conf.bgg));
        writeConfig("bgb", String(conf.bgb));
        message = "background color: (" + String(conf.bgr) + "," + String(conf.bgg) + "," + String(conf.bgb) + ")";
      }
      else if(value.startsWith("bright:")){
        String bright = value.substring(value.indexOf(":")+1);
        conf.bgbright = bright.toFloat();
        writeConfig("bgbright", String(conf.bgbright));
        message = "background brightness: " + String(conf.bgbright);
      }
    }
    else if(argname == "ssid"){
      int l = value.length();
      if(l >= 50){
        server.send(0, "text/plain", "ssid too long");
        break;
      }
      for(uint8_t k = 0; k < 50; k++){
        if(k < l)
          conf.ssid[k] = value[k];
        else
          conf.ssid[k] = 0;
      }
      writeConfig("ssid", conf.ssid);
      message = "set ssid to " + value;
    }
    else if(argname == "pw"){
      int l = value.length();
      if(l >= 50){
        server.send(0, "text/plain", "password too long");
        break;
      }
      for(uint8_t k = 0; k < 50; k++){
        if(k < l)
          conf.pw[k] = value[k];
        else
          conf.pw[k] = 0;
      }
      writeConfig("pw", conf.pw);
      message = "set wifi pw";
    }
  }
  server.send(200, "text/plain", message);
}

void handleAddWifi(){
  String msg = "";
  for (int i = 0; i < server.args(); i++) {
    if(i > 0) break;
    String argname = server.argName(i);
    String value = server.arg(i);
    if(argname == "remove"){
      if(removeWifi(value)) msg = "{\"remove\": true}";
      else msg = "{\"remove\": false}";
    }
    else{
      if (argname.length() < 1 || value.length() < 8){
        msg = "{\"add\": false}";
        break;
      }
      if (addWifi(argname, value)) msg = "{\"add\": true}";
      else msg = "{\"add\": false}";
    }
  }
  server.send(200, "text/json", msg);
}

void showconfig(){
  String msg = "text: " + conf.text;
  msg += "\nbrightness: " + String(conf.bright);
  msg += "\nvelocity: " + String(conf.velocity);
  server.send(200, "text/plain", msg);
}

void handlepong(){
  String msg = "";
  if(conf.opmode != pong) conf.opmode = pong;
  for (int i = 0; i < server.args(); i++) {
    String argname = server.argName(i);
    String value = server.arg(i);
    msg += argname + ": " + value;
    //Serial.println(argname + ": " + value);
    if(argname == "p1"){
      activate_pong(true);
      if(value == "left") move_bat(true, false);
      if(value == "right") move_bat(true, true);
    }
    else if(argname == "p2"){
      activate_pong(false);
      if(value == "left") move_bat(false, true);
      if(value == "right") move_bat(false, false);
    }
    else if(argname == "limit"){
      int points = value.toInt();
      if(points < 1){
        points = 1;
      }
      if(points > 100){
        points = 100;
      }
      pconf.limit = points;
      writePongConf("limit", String(points));
    }
    else if(argname == "name_p1"){
      pconf.name_p1 = value.substring(0,3);
      writePongConf("name_p1", pconf.name_p1);
    }
    else if(argname == "name_p2"){
      pconf.name_p2 = value.substring(0,3);
      writePongConf("name_p2", pconf.name_p2);
    }
  }
  server.send(200, "text/plain", msg);
}

void handlepaint(){
  conf.opmode = paint;
  String msg = "";
  for (int i = 0; i < server.args(); i++) {
    String argname = server.argName(i);
    String value = server.arg(i);
    if(argname == "pixels"){
      paintfromweb(value);
      msg = "pixels received";
    }
    else if(argname == "clear"){
      clearpaint();
      msg = "cleared";
    }
    else if(argname == "save"){
      savepaint(value);
      msg = "saved to " + value;
    }
    else if(argname == "load"){
      loadpaint(value);
      msg = "loaded";
    }
    else if(argname == "leaving"){
      savepaint("bu/temp");
      msg = "saved to /paints/bu/temp.paint";
    }
    else if(argname == "red"){
      conf.paintr = (uint8_t)value.toInt();
      writeConfig("paintr", String(conf.paintr));
      msg = "red=" + String(conf.paintr);
    }
    else if(argname == "green"){
      conf.paintg = (uint8_t)value.toInt();
      writeConfig("paintg", String(conf.paintg));
      msg = "green=" + String(conf.paintg);
    }
    else if(argname == "blue"){
      conf.paintb = (uint8_t)value.toInt();
      writeConfig("paintb", String(conf.paintb));
      msg = "blue=" + String(conf.paintb);
    }
    else if(argname == "hard"){
      conf.painthard = (uint8_t)value.toInt();
      writeConfig("painthard", String(conf.painthard));
      msg = "hardness=" + String(conf.painthard);
    }
    else if(argname == "size"){
      conf.paintsize = (uint8_t)value.toInt();
      writeConfig("paintsize", String(conf.paintsize));
      msg = "size=" + String(conf.paintsize);
    }
    else if(argname == "erase"){
      if(value == "true")
        conf.painterase = true;
      else
        conf.painterase = false;
      writeConfig("painterase", String(conf.painterase));
      msg = "eraser=" + String(conf.painterase);
    }
  }
  server.send(200, "text/plain", msg);
}

void listfiles(){
  Dir root = SPIFFS.openDir("/");
  String msg = "[\n";
  while(root.next()){
    msg += "  \"" + root.fileName() + "\",\n";
  }
  msg += "  \"\"\n]";
  server.send(200, "text/json", msg);
}

void handleFileChange(){
  String msg = "";
  for (int i = 0; i < server.args(); i++) {
    String argname = server.argName(i);
    String value = server.arg(i);
    if(argname == "remove"){
      removeFile(value);
      msg = "removed " + value;
    }
  }
  server.send(200, "text/plain", msg);
}

void sendscreen(){
  String data = "";
  for(int k = 0; k < NUM_LEDS; k++){
    data += String(s1[k].r) + "," + String(s1[k].g) + "," + String(s1[k].b) + ",";
  }
  server.send(200, "text/plain", data);
}

void handlefourier(){
  String msg = "";
  //Serial.println("called handlefourier()");
  
  conf.opmode = fourier;
  
  for (int i = 0; i < server.args(); i++) {
    String argname = server.argName(i);
    String value = server.arg(i);
    if(argname == "pixels"){
      //paintfromweb(value);
      msg = F("pixels received");
    }
    else if(argname.startsWith("col")){
      argname.replace("col", "");
      int pos = argname.toInt();
      if(pos < 0 || pos >= sizeof(conf.audioCols)/sizeof(CRGB)){
        msg += F("error: invalid color index");
        continue;
      }
      int r, g, b;
      r = value.substring(0, value.indexOf(",")).toInt();
      value = value.substring(value.indexOf(",") + 1);
      g = value.substring(0, value.indexOf(",")).toInt();
      value = value.substring(value.indexOf(",") + 1);
      b = value.toInt();
      conf.audioCols[pos].r = r;
      conf.audioCols[pos].g = g;
      conf.audioCols[pos].b = b;
      writeAudioCols();
      msg += "{\"color" + String(pos) + "\": \"rgb(" + String(r) + "," + String(g) + "," + String(b) + ")\"}";
    }
    else if(argname == "test"){
      int nCols;
      CRGB colors[6];
      uint8_t col[3];
      getAudioColors(&nCols, colors);
      for(uint8_t k = 0; k < 32; k++){
        audioColor(k, nCols, colors, col);
        msg += String(k) + "(" + col[0] + "," + col[1] + "," + col[2] + "), ";
      }
    }
  }
<<<<<<< HEAD
  server.send(200, "text/plain", msg); 
=======
  server.send(200, "text/plain", "Sucksess"); 
  //call_FFT();
>>>>>>> Cedric
}

void handleframerate(){
  server.send(200, "text/plain", String((float)1000.0/dt));
}

void handleworms(){  
  conf.opmode = worms;
  String msg = "{";
  int token = 0;
  for (int i = 0; i < server.args(); i++) {
    String argname = server.argName(i);
    String value = server.arg(i);
    if(argname == "token"){
      token = value.toInt();
      msg += "\"token\":" + validToken(token);
    }
    else if(argname == "move"){
      int dir = 0;
      String l = "";
      if(value.startsWith("l")){
        dir = -1;
        l = "l";
        value.replace("l", "");
      }
      if(value.startsWith("r")){
        dir = 1;
        l = "r";
        value.replace("r", "");
      }
      float dy = value.toFloat();
      Serial.println("elevation: " + String(dy));
      if(isnan(dy)) dy = 0.0;
      if(dy > 2.0) dy = 2.0;
      if(dy < -2.0) dy = -2.0;
      move_worm(token, dir, dy);
      msg += "\"move\":\"" + l + String(dy) + "\"";
    }
    else if(argname == "worm"){
      bool confirmed = false;
      if(value.startsWith("c")){
        confirmed = true;
        value.replace("c", "");
      }
      select_worm(token, value.toInt(), confirmed);
      msg += "\"worm\":\"" + value + "\"";
    }
    else if(argname == "weapon"){
      select_weapon(token, value.toInt());
      msg += "\"weapon\":\"" + value + "\"";
    }
    else if(argname == "shoot"){
      shoot(token);
      msg += "\"shot\":\"true\"";
    }
    else if(argname == "setmap"){
      int res = initMap(token, value);
      msg += "\"setmap\":\"";
      if(res == 0) msg += value + "\"";
      else if(res == 1) msg += "invalid token\"";
      else if(res == 2) msg += "invalid map\"";
      else msg += "unknown error\"";
    }
    else if(argname == "mapconfirm"){
      bool res = confirmMap(token);
      msg += "\"mapconfirm\":" + b2s(res);
    }
    else if(argname == "player"){
      if(value.startsWith("0")) 
        msg += "\"player\":" + String(initPlayer(value.substring(1), 0));
      else if(value.startsWith("1")) 
        msg += "\"player\":" + String(initPlayer(value.substring(1), 1));
      else msg += "\"player\":null";
    }
    else if(argname == "verify"){
      msg += "\"verify\":" + verify_token(value.toInt());
    }
    else if(argname == "state"){
      msg += getWormsState();
    }

    if(i+1 < server.args()) msg += ",";
  }
  msg += "}";
  server.send(200, "text/json", msg);
}
