void InitWeb(){
  Serial.println("\nstarting wifi");
  WiFi.begin(conf.ssid, conf.pw);
  //WiFi.begin("Oberhochsteg", "ImGaertle1234");
  /*
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
    if(i > 5 && WiFi.status() != WL_CONNECTED){*/
    IPAddress local_ip(192,168,1,1);
    IPAddress gateway(192,168,1,1);
    IPAddress subnet(255,255,255,0);
      //WiFi.mode(WIFI_STA);
      WiFi.softAP("matrix", "tsvrxvu2");
      WiFi.softAPConfig(local_ip, gateway, subnet);
      Serial.println("creating acces point");
      /*
      break;
    }
  }*/
  if (!MDNS.begin("matrix")) {             // Start the mDNS responder for node.local
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/pong", HTTP_POST, pong);
  server.on("/set", HTTP_POST, handleset);
  server.on("/set", HTTP_GET, readtext);
  server.on("/conf", HTTP_GET, showconfig);
  server.on("/paint", HTTP_POST, handlepaint);
  server.on("/debugscreen", HTTP_GET, sendscreen);
  server.on("/listfiles", HTTP_GET, listfiles);
  server.on("/changefile", HTTP_POST, handleFileChange);
  server.on("/upload", HTTP_GET,[](){
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
  Serial.println("wifi started");
}
void readtext(){
  server.send(200, "text/plain", readFile("text.txt"));
}
void handleset(){
  String message = "invalid command";
  for (int i = 0; i < server.args(); i++) {
    String argname = server.argName(i);
    String value = server.arg(i);
    if(argname == "text"){
      conf.pongmode = false;
      conf.paintmode = false;
      writeConfig("text", value);
      conf.text = value;
      message = "text set to: " + value;
    }
    else if(argname == "velocity"){
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
        message = "background off";
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

void showconfig(){
  String msg = "text: " + conf.text;
  msg += "\nbrightness: " + String(conf.bright);
  msg += "\nvelocity: " + String(conf.velocity);
  server.send(200, "text/plain", msg);
}

void pong(){
  String msg = "";
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
  String msg = "";
  for (int i = 0; i < server.args(); i++) {
    String argname = server.argName(i);
    String value = server.arg(i);
    conf.paintmode = true;
    conf.pongmode = false;
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
