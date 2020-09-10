void InitFile(){
  if(!SPIFFS.begin()){
    Serial.println(F("An Error has occurred while mounting SPIFFS"));
    return;
  }
}

void handleFileUpload(){ // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print(F("handleFileUpload Name: ")); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile) {                                    // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print(F("handleFileUpload Size: ")); Serial.println(upload.totalSize);
      server.sendHeader(F("Location"),F("/upload"));      // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, F("text/plain"), F("500: couldn't create file"));
    }
  }
}

void writeFile(String fname, String data){
  if(fname == ""){
    Serial.println(F("cant write file: no name given"));
    return;
  }
  File file = SPIFFS.open(fname, "w");
  if(!file){
    Serial.print(F("Error: could not open file ")); Serial.println(fname);
    return;
  }
  int n = file.print(data);
  if(n == 0){
    Serial.print(F("Error: could not write to file ")); Serial.println(fname);
  }
  Serial.println("File " + fname + " written");
  file.close();
}

String readFile(String fname){
  File file = SPIFFS.open(fname, "r");
  if(!file){
    Serial.print(F("Error: could not open file ")); Serial.println(fname);
    return "";
  }
  String result;
  while(file.available()){
    result += char(file.read());
  }
  file.close();
  return result;
}

StaticJsonDocument<600> json;
bool writeConfig(String key, String data){
  return writeAnyConfig(key, data, F("/config.json"), -1);
}

bool writePongConf(String key, String data){
  return writeAnyConfig(key, data, F("/pconfig.json"), -1);
}

bool addWifi(String ssid, String pw){
  if(ssid.length() > 50){
    Serial.println("ssid too long");
    return false;
  }
  if(pw.length() > 50){
    Serial.println("pw too long");
    return false;
  }
  return writeAnyConfig(ssid, pw, F("/wificonfig.json"), 20);
}

bool removeWifi(String ssid){
  return removeJsonEntry(F("/wificonfig.json"), ssid);
}

bool writeAnyConfig(String key, String data, String fname, int maxsize){
  File file = SPIFFS.open(fname, "r");
  if(!file){
    Serial.print(F("Error: could not open file ")); Serial.println(fname);
    return false;
  }
  DeserializationError err = deserializeJson(json, file);
  file.close();
  if(err){
    Serial.println(F("Error: could not deserialize config"));
    return false;
  }
  if(maxsize > 0 && json.as<JsonObject>().size() >= maxsize){
    Serial.println("could not write " + key + "=" + data + " object already full");
    return false;
  }
  
  json[key] = data;
  file = SPIFFS.open(fname, "w");
  if (serializeJson(json, file) == 0) {
    Serial.print(F("Error: Failed to write to file ")); Serial.println(fname);
    file.close();
    return false;
  }
  else{
    Serial.println(key + ": " + data);
  }
  Serial.println("written: " + key + "=" + data);
  file.close();
  return true;
}

bool removeJsonEntry(String fname, String key){
  File file = SPIFFS.open(fname, "r");
  if(!file){
    Serial.print(F("Error: could not open file ")); Serial.println(fname);
    return false;
  }
  DeserializationError err = deserializeJson(json, file);
  file.close();
  if(err){
    Serial.println(F("Error: could not deserialize config"));
    return false;
  }
  json.remove(key);
  file = SPIFFS.open(fname, "w");
  if (serializeJson(json, file) == 0) {
    Serial.print(F("Error: Failed to write to file ")); Serial.println(fname);
    file.close();
    return false;
  }
  Serial.println("removed: " + key);
  file.close();
  return true;
}

void loadConfig(){
  File file = SPIFFS.open(F("/config.json"), "r");
  if(!file){
    Serial.println(F("Error: could not open config file"));
    return;
  }
  DeserializationError err = deserializeJson(json, file);
  if(err){
    Serial.println(F("Error: could not deserialize config"));
    return;
  }
  conf.text = json["text"].as<String>();
  conf.bright = json["bright"].as<float>();
  conf.velocity = json["velocity"].as<float>();
  conf.background = json["background"].as<String>();
  conf.bgr = json["bgr"].as<uint8_t>();
  conf.bgg = json["bgg"].as<uint8_t>();
  conf.bgb = json["bgb"].as<uint8_t>();
  conf.bgbright = json["bgbright"].as<float>();
  conf.paintr = json["paintr"].as<uint8_t>();

  if(!wStr2CharArr(json["ssid"].as<String>(), &conf.ssid[0], 50))
    Serial.println(F("ERROR: wifi name too long"));
  
  if(!wStr2CharArr(json["pw"].as<String>(), &conf.pw[0], 50))
    Serial.println(F("ERROR: wifi password too long"));
    
  // TODO maze
  // two mazes generated automatically
  // two players work throgh it first one to get to the middle wins
 

  file.close();
}

void removeFile(String fname){
  SPIFFS.remove(fname);
}
void loadPongConf(){
  File file = SPIFFS.open("/pconfig.json", "r");
  if(!file){
    Serial.println(F("Error: could not open config file"));
    return;
  }
  DeserializationError err = deserializeJson(json, file);
  if(err){
    Serial.println(F("Error: could not deserialize config"));
    return;
  }
  pconf.limit = json["limit"].as<int>();
  pconf.name_p1 = json["name_p1"].as<String>();
  pconf.name_p2 = json["name_p2"].as<String>();
}

String readConfig(String key){
  File file = SPIFFS.open("/config.json", "r");
  if(!file){
    Serial.println(F("Error: could not open config file"));
    return "";
  }
  DeserializationError err = deserializeJson(json, file);
  if(err){
    Serial.println(F("Error: could not deserialize config"));
    return "";
  }
  String res = json[key];
  file.close();
  return res;
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "index.html";           // If a folder is requested, send the index file
  if(path == "/index.html"){
    loadBackground();
    conf.paintmode = false;
    conf.pongmode = false;
    conf.fouriermode = false;
    conf.wormsmode = false;
  }
  if(path == "/paint.html"){
    conf.paintmode = true;
    conf.pongmode = false;
    conf.fouriermode = false;
    conf.wormsmode = false;
    loadpaint("bu/temp");
  }
  if(path == "/worms.html"){
    if(!conf.wormsmode) startWorms();
    conf.paintmode = false;
    conf.pongmode = false;
    conf.fouriermode = false;
    conf.wormsmode = true;
  }
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

String getContentType(String filename) { // convert the file extension to the MIME type
  if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
