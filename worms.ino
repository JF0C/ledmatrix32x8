enum weapons{
  bazooka = 10,
  rifle = 11,
  laser = 12,
  bat = 13
};
enum worms_states{
  game_start,
  p1_select,
  p1_move,
  p1_wait_shoot,
  p1_move_after,
  p2_select,
  p2_move,
  p2_wait_shoot,
  p2_move_after,
  game_over
};
struct projectile{
  float vx, vy, vbase;
  float x, y;
  float damage, destruct;
  float radius;
  float ay;
  bool alive;
  unsigned long timer;
};

projectile bullets[14];

struct worm{
  float x;
  float y;
  float dy;
  bool look_left;
  int health;
  bool selected;
  unsigned long timer;
  weapons weapon;
};
struct wormsconfiguration{
  String names[2] = {"", ""};
  int tokens[2];
  worm worms[2][4];
  worms_states state;
  unsigned long timer;
  unsigned long t_rest;
  String mapname;
  bool shot;
  bool worm_selected;
  bool initialized;
  bool game_over;
  bool map_selected;
}wormsconf;

const unsigned long tmove = 15000;
const unsigned long tafter = 8000;
// main call for worms in every iteration of loop
void render_worms(){
  if(conf.opmode != worms) return;
  drawmap();
  draw_worms();
  int shooting_done = draw_bullets();

  if((hasWon(0) || hasWon(1)) && wormsconf.state != game_start && !wormsconf.game_over){
    Serial.println("game over");
    wormsconf.game_over = true;
    wormsconf.state = game_over;
    wormsconf.timer = t;
  }
  
  switch(wormsconf.state){
    case game_start:
      print_start();
      if(checkInit() && t - wormsconf.timer > 500){
        wormsconf.game_over = false;
        if(random(2)) wormsconf.state = p1_select;
        else wormsconf.state = p2_select;
        wormsconf.map_selected = false;
      }
      break;
    case p1_select:
      if(wormsconf.worm_selected){
        wormsconf.worm_selected = false;
        wormsconf.state = p1_move;
        wormsconf.timer = t;
      }
      break;
    case p1_move:
      wormsconf.t_rest = tmove - (t - wormsconf.timer);
      if(wormsconf.shot || t - wormsconf.timer > tmove){
        if(!wormsconf.shot) shoot(wormsconf.tokens[0]);
        wormsconf.shot = false;
        wormsconf.state = p1_wait_shoot;
        wormsconf.timer = t;
      }
      break;
    case p1_wait_shoot:
      if(shooting_done == 1){
        wormsconf.state = p1_move_after;
        wormsconf.timer = t;
      }
      break;
    case p1_move_after:
      wormsconf.t_rest = tafter - (t - wormsconf.timer);
      if(t - wormsconf.timer > tafter){
        wormsconf.state = p2_select;
      }
      break;
    case p2_select:
      if(wormsconf.worm_selected){
        wormsconf.worm_selected = false;
        wormsconf.state = p2_move;
        wormsconf.timer = t;
      }
      break;
    case p2_move:
      wormsconf.t_rest = tmove - (t - wormsconf.timer);
      if(wormsconf.shot || t - wormsconf.timer > tmove){
        if(!wormsconf.shot) shoot(wormsconf.tokens[1]);
        wormsconf.shot = false;
        wormsconf.state = p2_wait_shoot;
        wormsconf.timer = t;
      }
      break;
    case p2_wait_shoot:
      if(shooting_done == 1){
        wormsconf.state = p2_move_after;
        wormsconf.timer = t;
      }
      break;
    case p2_move_after:
      wormsconf.t_rest = tafter - (t - wormsconf.timer);
      if(t - wormsconf.timer > tafter){
        wormsconf.state = p1_select;
      }
      break;
    case game_over:
      print_winner();
      if(t - wormsconf.timer > 2000){
        startWorms();
      }
      break;
  }
}

String getWormsState(){
  String result = "\"state\":" + String((int)wormsconf.state) + ",";
  for(uint8_t k = 0; k < 2; k++){
    for(uint8_t l = 0; l < 4; l++){
      result += "\"worm" + String(k) + "." + String(l) + "\":{" +
        "\"health\":" + String(wormsconf.worms[k][l].health) + "," +
        "\"x\":" + String(wormsconf.worms[k][l].x) + "," +
        "\"y\":" + String(wormsconf.worms[k][l].y) + "," +
        "\"weapon\":" + String(wormsconf.worms[k][l].weapon) + "," +
        "\"dy\":" + String(wormsconf.worms[k][l].dy) + "," + 
        "\"selected\":" + b2s(wormsconf.worms[k][l].selected) + "},";
    }
  }
  result += "\"t_rest\":" + String(wormsconf.t_rest) + ",";
  result += "\"p1_won\":" + b2s(hasWon(0)) + ",";
  result += "\"p2_won\":" + b2s(hasWon(1)) + ",";
  result += "\"p1_name\":\"" + wormsconf.names[0] + "\",";
  result += "\"p2_name\":\"" + wormsconf.names[1] + "\"";
  return result;
}

String validToken(int token){
  if(wormsconf.tokens[0] == token || wormsconf.tokens[1] == token) return "true";
  return "false";
}

bool isSolid(uint8_t x, uint8_t y){
  uint8_t col[3];
  if(x < 0 || x > 31) return false;
  if(y > 7 || y < 0) return true;
  colorAt(x, y, &col[0], &paintdata[0]);
  if((col[0] + col[1] + col[2]) > (float)330.0) return true;
  return false;
}

bool isWorm(uint8_t x, uint8_t y){
  for(uint8_t k = 0; k < 2; k++){
    for(uint8_t l = 0; l < 4; l++){
      if(wormsconf.worms[k][l].health <= 0) continue;
      int wx = round(wormsconf.worms[k][l].x);
      int wy = round(wormsconf.worms[k][l].y);
      if(wx == x && (wy == y || wy-1 == y || wy-2 == y)) return true;
    }
  }
  return false;
}

void select_worm(int token, int worm, bool confirmed){
  int player = playerFromToken(token);
  if(player == -1) return;
  if(worm < 0 || worm > 3) return;
  if(player == 0 && wormsconf.state != p1_select) return;
  if(player == 1 && wormsconf.state != p2_select) return;
  int sel = 0;
  for(uint8_t k = 0; k < 4; k++){
    wormsconf.worms[player][k].selected = worm == k;
    if(worm == k) sel = k;
  }
  if(confirmed && wormsconf.worms[player][sel].health > 0) 
    wormsconf.worm_selected = true;
}
void select_weapon(int token, int weapon){
  int player = playerFromToken(token);
  if(player == -1) return;
  int worm = wormFromPlayer(player);
  if(worm == -1) return;
  if(player == 0 && wormsconf.state != p1_move) return;
  if(player == 1 && wormsconf.state != p2_move) return;
  if(weapon < 10 || weapon > 13) return;
  wormsconf.worms[player][worm].weapon = (weapons)weapon;
}

void shoot(int token){
  int player = playerFromToken(token);
  if(player == -1) return;
  int worm = wormFromPlayer(player);
  if(worm == -1) return;
  if(wormsconf.state != p1_move && wormsconf.state != p2_move) return;
  switch(wormsconf.worms[player][worm].weapon){
    case bazooka: bullets[0].alive = true; break;
    case rifle:
      for(uint8_t k = 0; k < 10; k++){
        bullets[k].alive = true;
        bullets[k].timer = t + 50 * k;
      } 
      break;
    case bat: bullets[0].alive = true; break;
    case laser: bullets[0].alive = true; break;
  }
  int look = 1;
  if(wormsconf.worms[player][worm].look_left) look = -1;
  // TODO refactor this
  bulletcp(wormsconf.worms[player][worm].weapon, 0, look,
           wormsconf.worms[player][worm].dy,
           wormsconf.worms[player][worm].x + look,
           wormsconf.worms[player][worm].y - 1);
  bullets[0].alive = true;
  if(wormsconf.worms[player][worm].weapon == bat){
    bullets[0].x = wormsconf.worms[player][worm].x + 2*look;
  }
  if(wormsconf.worms[player][worm].weapon == rifle){
    for(uint8_t k = 1; k < 10; k++){
      bulletcp(wormsconf.worms[player][worm].weapon, k, look,
               wormsconf.worms[player][worm].dy,
               wormsconf.worms[player][worm].x + look, 
               wormsconf.worms[player][worm].y - 1);
      bullets[k].vy += ((float)random(100)-50.0)*0.03;
      bullets[k].alive = true;
    }
  }
  wormsconf.shot = true;
  // until here
}

void bulletcp(int src, int dst, int dir, float dy, float x, float y){
  bullets[dst].alive = bullets[src].alive;
  bullets[dst].damage = bullets[src].damage;
  bullets[dst].radius = bullets[src].radius;
  bullets[dst].vx = bullets[src].vbase * dir;
  bullets[dst].vy = dy * bullets[src].vbase;
  bullets[dst].ay = bullets[src].ay;
  bullets[dst].x = x;
  bullets[dst].y = y;
}

void move_worm(int token, int dir, float y){
  const float step_width = 1.0;
  int player = playerFromToken(token);
  if(player == -1) return;
  if(player == 0 && (wormsconf.state != p1_move && wormsconf.state != p1_move_after)) return;
  if(player == 1 && (wormsconf.state != p2_move && wormsconf.state != p2_move_after)) return;
  int worm = wormFromPlayer(player);
  if(worm == -1) return;
  if(wormsconf.worms[player][worm].health <= 0) return;
  wormsconf.worms[player][worm].dy = y;
  if(dir < 0){
    wormsconf.worms[player][worm].look_left = true;
    int x = ceil(wormsconf.worms[player][worm].x);
    int y = wormsconf.worms[player][worm].y;
    if(!isSolid(x-1, y-1) && !isSolid(x-1, y-2) && !(isSolid(x-1, y-3) && isSolid(x-1, y)) && x >= step_width){
      wormsconf.worms[player][worm].x -= step_width;
    }
    x = floor(wormsconf.worms[player][worm].x);
    if(isSolid(x-1, y)) wormsconf.worms[player][worm].y -= 1;
    worm_fall(player, worm);
  }
  if(dir > 0){
    wormsconf.worms[player][worm].look_left = false;
    int x = floor(wormsconf.worms[player][worm].x);
    int y = wormsconf.worms[player][worm].y; 
    if(!isSolid(x+1, y-1) && !isSolid(x+1, y-2) && !(isSolid(x+1, y-3) && isSolid(x+1, y)) && x <= 31-step_width){
      wormsconf.worms[player][worm].x += step_width;
    }
    x = ceil(wormsconf.worms[player][worm].x);
    if(isSolid(x-1, y)) wormsconf.worms[player][worm].y -= 1;
    worm_fall(player, worm);
  }
  if(wormsconf.worms[player][worm].health < 0) wormsconf.shot = true;
}
int playerFromToken(int token){
  if(wormsconf.tokens[0] == token) return 0;
  if(wormsconf.tokens[1] == token) return 1;
  return -1;
}
int wormFromPlayer(int player){
  if(player < 0 || player > 1) return -1;
  for(uint8_t k = 0; k < 4; k++){
    if(wormsconf.worms[player][k].selected) return k;
  }
  return -1;
}
void initWorm(int worm, int player){
  int pos = random(16) + player * 16;
  while(xHasWorm(pos)){
    pos = random(16) + player * 16;
  }
  bool pos_found = false;
  for(uint8_t y = 2; y < 7; y++){
    if(!isSolid(pos, y-2) && !isSolid(pos, y-1) && !isSolid(pos, y) && isSolid(pos, y+1)){
      pos_found = true;
      wormsconf.worms[player][worm].x = pos;
      wormsconf.worms[player][worm].y = y;
      break;
    }
  }
  if(!pos_found){
    for(int px = pos-1; px < pos+2; px++)
      for(int py = 0; py < 3; py++)
        paintxy(px, py, &blank[0]);
    wormsconf.worms[player][worm].x = pos;
    wormsconf.worms[player][worm].y = 2;
  }
  wormsconf.worms[player][worm].health = 100;
  wormsconf.worms[player][worm].weapon = bazooka;
  wormsconf.worms[player][worm].timer = 0;
  if(worm == 0){
    wormsconf.worms[player][worm].selected = true;
  }
  else{
    wormsconf.worms[player][worm].selected = false;
  }
  if(player == 1) wormsconf.worms[player][worm].look_left = true;
  else wormsconf.worms[player][worm].look_left = false;
}

bool xHasWorm(int x){
  for(uint8_t p = 0; p < 2; p++){
    for(uint8_t w = 0; w < 4; w++){
      if(wormsconf.worms[p][w].x == x) return true;
    }
  }
  return false;
}

int initPlayer(String plname, int player){
  if(player != 0 && player != 1) return 0;
  if(wormsconf.tokens[player] != 0) return 0;
  if(plname.length() > 3) plname = plname.substring(0, 3);
  wormsconf.names[player] = plname;
  wormsconf.tokens[player] = random(10000) + 1;
  return wormsconf.tokens[player];
}
bool checkInit(){
  //Serial.print("map exists: "); Serial.println(SPIFFS.exists("/paints/" + wormsconf.mapname + ".paint"));
  return SPIFFS.exists("/paints/" + wormsconf.mapname + ".paint") && wormsconf.tokens[0] != 0 && wormsconf.tokens[1] != 0 && wormsconf.map_selected;
}
void print_winner(){
  String message = "";
  uint8_t col[3];
  
  if(hasWon(0) && !hasWon(1)) {
    if(wormsconf.names[0] == "") message = "Pl1 wins!";
    else message = wormsconf.names[0] + " wins!";
    colcp(red, col);
  }
  if(!hasWon(0) && hasWon(1)) {
    if(wormsconf.names[1] == "") message = "Pl2 wins!";
    else message = wormsconf.names[1] + " wins!";
    colcp(blue, col);
  }
  if(hasWon(0) && hasWon(1)){
    message = "draw";
    colcp(white, col);
  }
  printStringSimple(message, col, conf.bright*sineval);
}
String verify_token(int token){
  if(wormsconf.tokens[0] == token || wormsconf.tokens[1] == token) return "true";
  else return "false";
}
void print_start(){
  String msg = "";
  char rnd_char[] = {'#', '3', ' ', '*', '?', '/', '-'};
  if(wormsconf.tokens[0] == 0 && wormsconf.tokens[1] == 0){
    msg = "WORMS!";
    String msg2 = "";
    for(uint8_t k = 0; k < msg.length(); k++){
      int r1 = random(100);
      if(r1 >= 6){
        msg2 += msg[k];
      }
      else{
        msg2 += rnd_char[r1];
      }
    }
    printStringSimple(msg2, white, conf.bright, 2);
  }
  else{
    if(wormsconf.names[0] == "") printStringSimple("Pl1", red, conf.bright);
    else printStringSimple(wormsconf.names[0], red, conf.bright);
    printStringSimple(":", white, conf.bright, 15);
    if(wormsconf.names[1] == "") printStringSimple("Pl2", blue, conf.bright, 17);
    else printStringSimple(wormsconf.names[1], blue, conf.bright, 17);
  }
}

void startWorms(){
  Serial.println("starting worms");
  wormsconf.state = game_start;
  wormsconf.tokens[0] = 0;
  wormsconf.tokens[1] = 0;
  wormsconf.mapname = "";
  wormsconf.timer = t;
  wormsconf.game_over = false;
  wormsconf.shot = false;
  wormsconf.worm_selected = false;
  wormsconf.initialized = false;
  wormsconf.map_selected = false;
}

bool confirmMap(int token){
  if(wormsconf.state != game_start) return false;
  if(wormsconf.tokens[0] == 0 || wormsconf.tokens[1] == 0 || 
     (wormsconf.tokens[0] != token && wormsconf.tokens[1] != token)) return false;
  if(!SPIFFS.exists("/paints/" + wormsconf.mapname + ".paint")) return false;
  wormsconf.map_selected = true;
}

int initMap(int token, String paint){
  if(playerFromToken(token) == -1) return 1;
  if(!SPIFFS.exists("/paints/" + paint + ".paint")) return 2;
  wormsconf.mapname = paint;
  loadpaint(paint);
  for(uint8_t p = 0; p < 2; p++){
    for(uint8_t w = 0; w < 4; w++){
      initWorm(w, p);
    }
  }

  bullets[bazooka].alive = false;
  bullets[bazooka].damage = 70;
  bullets[bazooka].destruct = 0.6;
  bullets[bazooka].radius = 3;
  bullets[bazooka].vbase = 4;
  bullets[bazooka].ay = 0.7;
  
  bullets[rifle].damage = 6;
  bullets[rifle].alive = false;
  bullets[rifle].radius = 1.0;
  bullets[rifle].destruct = 0.4;
  bullets[rifle].vbase = 7;
  bullets[rifle].ay = 0.2;
      
  bullets[laser].alive = false;
  bullets[laser].damage = 20;
  bullets[laser].radius = 0.5;
  bullets[laser].destruct = 0.5;
  bullets[laser].vbase = 10;
  bullets[laser].ay = 0;
  
  bullets[bat].alive = false;
  bullets[bat].damage = 60;
  bullets[bat].radius = 1;
  bullets[bat].destruct = 0.9;
  bullets[bat].vbase = 0;
  bullets[bat].ay = 0;
  return 0;
}

bool hasWon(int player){
  if(wormsconf.state == game_start) return false;
  bool won = true;
  int p = -1;
  if(player == 1) p = 0;
  else if(player == 0) p = 1;
  else return false;
  for(int k = 0; k < 4; k++){
    if(wormsconf.worms[p][k].health > 0) won = false;
  }
  return won;
}

void draw_worms(){
  if(wormsconf.state == game_over || wormsconf.state == game_start) return;
  for(uint8_t k = 0; k < 2; k++){
    for(uint8_t l = 0; l < 4; l++){
      uint8_t* pcol;
      if(k == 0) pcol = red;
      if(k == 1) pcol = blue;
      float x = wormsconf.worms[k][l].x;
      float y = wormsconf.worms[k][l].y;
      float look = 1;
      bool turn_p1 = wormsconf.state == p1_select || wormsconf.state == p1_move || wormsconf.state == p1_move_after;
      bool turn_p2 = wormsconf.state == p2_select || wormsconf.state == p2_move || wormsconf.state == p2_move_after;
      if(wormsconf.worms[k][l].look_left) look = -1;
      if(wormsconf.worms[k][l].health <= 0){
        if(wormsconf.worms[k][l].timer == 0){
          wormsconf.worms[k][l].timer = t;
        }
        if(t - wormsconf.worms[k][l].timer > tmove) continue;
        float f = 1.0 - (float)(t - wormsconf.worms[k][l].timer)/(float)tmove;
        // cross if died
        int xcross = floor(x);
        int ycross = floor(y);
        plot_antialiased(xcross, ycross, wormscol, conf.bright*f, false, false);
        plot_antialiased(xcross, ycross-1, wormscol, conf.bright*f, false, false);
        plot_antialiased(xcross, ycross-3, wormscol, conf.bright*f, false, false);
        plot_antialiased(xcross-1, ycross-2, wormscol, conf.bright*f, false, false);
        plot_antialiased(xcross+1, ycross-2, wormscol, conf.bright*f, false, false);
        // player color
        plot_antialiased(xcross, ycross-2, pcol, 0.5*conf.bright*sineval, false, false);
        
        continue;
      }
      // highlights
      if(wormsconf.worms[k][l].selected && turn_p1 && k == 0){
        // selected marker
        plot_antialiased(x, y-1, pcol, conf.bright*sineval, false, false);
        if(wormsconf.state == p1_move){
          projectile* b = &(bullets[wormsconf.worms[k][l].weapon]);
          draw_trajectory(x + look, y - 1, wormsconf.worms[k][l].dy, b->ay, b->vbase, look);
        }
      }
      
      if(wormsconf.worms[k][l].selected && turn_p2 && k == 1){
        // selected marker
        plot_antialiased(x, y-1, pcol, conf.bright*sineval, false, false);
        if(wormsconf.state == p2_move){
          projectile* b = &(bullets[wormsconf.worms[k][l].weapon]);
          draw_trajectory(x + look, y - 1, wormsconf.worms[k][l].dy, b->ay, b->vbase, look);
        }
      }
      
      // normal marker
      if(!wormsconf.worms[k][l].selected || (turn_p2 && k==0) || (turn_p1 && k==1)){
        plot_antialiased(x, y-1, pcol, conf.bright, false, false);
      }
      // lower body
      plot_antialiased(x, y, wormscol, conf.bright, false, false);
      //tail
      plot_antialiased(x-look, y, wormscol, conf.bright, false, false);
      //head
      plot_antialiased(x, y-2, wormscol, conf.bright, false, false);
      int yoff = -3;
      if(y <= 2) yoff = 1;
      lifebar(x, y+yoff, wormsconf.worms[k][l].health);
      
      if(!wormsconf.worms[k][l].selected) continue;
      if(k == 0 && turn_p2) continue;
      if(k == 1 && turn_p1) continue;
      switch(wormsconf.worms[k][l].weapon){
        case bazooka:
          plot_antialiased(x-1, y-2, orange, conf.bright, false, false);
          plot_antialiased(x+1, y-2, orange, conf.bright, false, false);
          plot_antialiased(x+2*look, y-2, orange, conf.bright, false, false);
          plot_antialiased(x+look, y-1, orange, conf.bright, false, false);
          break;
        case rifle:
          plot_antialiased(x-1, y-1, orange, conf.bright, false, false);
          plot_antialiased(x+1, y-1, orange, conf.bright, false, false);
          plot_antialiased(x+2*look, y-1, orange, conf.bright, false, false);
          plot_antialiased(x+look, y, orange, conf.bright*0.5, false, false);
          break;
        case bat:
          if((wormsconf.state == p1_wait_shoot && k == 0) || (wormsconf.state == p2_wait_shoot && k == 1)) break;
          plot_antialiased(x+look, y-1, yellow, conf.bright, false, false);
          plot_antialiased(x+2*look, y-2, yellow, conf.bright, false, false);
          plot_antialiased(x, y, yellow, 0.5*conf.bright, false, false);
          break;
        case laser:
          plot_antialiased(x+look, y-1, orange, conf.bright, false, false);
          plot_antialiased(x+2*look, y-1, green, conf.bright*sineval, false, false);
          break;
      }
    }
  }
}

void worm_fall(int p, int w){
  int fallen = 0;
  float x = floor(wormsconf.worms[p][w].x);
  float y = wormsconf.worms[p][w].y;
  while(!isSolid(x, y + 1) && !isSolid(x+1, y+1) && wormsconf.worms[p][w].y < 7){
    wormsconf.worms[p][w].y += 1.0;
    fallen += 1.0;
  }
  if(wormsconf.worms[p][w].y > 7.0) wormsconf.worms[p][w].y = 7.0;
  if(fallen < 1) return;
  wormsconf.worms[p][w].health -= (fallen-1)*8;
  Serial.println("fall damage to worm " + String(p) + "." + String(w) + ": " + String((fallen-1)*8));
}

void draw_trajectory(float x, float y, float dy, float ay, float vbase, int dir){
  for(uint8_t k = 0; k < 6; k++){
    float f = sin(-(float)k/6.0*3.1416 + ((float)t/500.0)) + 1;
    f /= 2.0;
    plot_antialiased(x + dir*k, y + k*dy + k*k*ay/vbase/vbase, white, conf.bright*f, false, false);
  }
}

void lifebar(uint8_t x, uint8_t y, int health){
  float h = (float)health/100.0;
  if(h < 0.25) h *= sineval;
  else h *= h;
  drawxy(x, y, green, conf.bright*h, false);
}

int draw_bullets(){
  if(wormsconf.state != p1_wait_shoot && wormsconf.state != p2_wait_shoot) return 0;
  int player;
  if(wormsconf.state == p1_wait_shoot) player = 0;
  if(wormsconf.state == p2_wait_shoot) player = 1;
  uint8_t worm = wormFromPlayer(player);
  uint8_t weapon = wormsconf.worms[player][worm].weapon;
  float wx = wormsconf.worms[player][worm].x;
  float wy = wormsconf.worms[player][worm].y;
  float delta = (float)dt/1000.0;
  bool no_bullets = true;
  for(uint8_t k = 0; k < 10; k++){
    if(!bullets[k].alive) continue;
    no_bullets = false;
    int dir = 0;
    float elevation = 0;
    if(bullets[k].vx != 0) elevation = bullets[k].vy / abs(bullets[k].vx);
    if(bullets[k].vx > 0) dir = 1;
    if(bullets[k].vx < 0) dir = -1;
    switch(weapon){
      case bazooka:
        if(bullets[k].vx == 0){
          float f = (float)(t - bullets[k].timer)/500.0;
          if(f >= 1.0) {
            bullets[k].alive = false;
            break;
          }
          float r = bullets[k].radius * f;
          for(int bx = floor(-r-1); bx <= ceil(r); bx++){
            for(int by = floor(-r-1); by <= ceil(r); by++){
              if(bx*bx + by*by < r*r)
                plot_antialiased(bullets[k].x + bx, bullets[k].y + by, red, conf.bright * (1-f), false, false);
            }
          }
          break;
        }
        if(bullets[k].vx*delta > bullets[k].radius) Serial.println("warning: bazooka skipping pixels");
        bullets[k].x += bullets[k].vx*delta;
        bullets[k].y += bullets[k].vy*delta;
        bullets[k].vy += bullets[k].ay*delta;
        plot_antialiased(bullets[k].x, bullets[k].y, white, conf.bright, false, false);
        plot_antialiased(bullets[k].x - dir, bullets[k].y - elevation, red, conf.bright, false, false);
        if((((isSolid(bullets[k].x, bullets[k].y) || isWorm(bullets[k].x, bullets[k].y)) && bullets[k].y > 0) 
           || bullets[k].x > 31 || bullets[k].x < 0)  && bullets[k].vx != 0){
          damage_calc(bullets[k].x, bullets[k].y, weapon);
          bullets[k].vx = 0;
          bullets[k].vy = 0;
          bullets[k].ay = 0;
          bullets[k].timer = t;
        }
        break;
      case rifle:
        if(bullets[k].vx == 0){
          float f = t - bullets[k].timer;
          if(f >= 500) bullets[k].alive = false;
          f = 1.0 - f/500.0;
          plot_antialiased(bullets[k].x, bullets[k].y, red, conf.bright*f, false, false);
          break;
        }
        if(bullets[k].timer > t) break;
        if(bullets[k].vx*delta > bullets[k].radius) Serial.println("warning: rifle skipping pixels");
        bullets[k].x += bullets[k].vx*delta;
        bullets[k].y += bullets[k].vy*delta;
        bullets[k].vy += bullets[k].ay*delta;
        plot_antialiased(bullets[k].x, bullets[k].y, white, conf.bright, false, false);
        plot_antialiased(bullets[k].x - dir, bullets[k].y - elevation, white, conf.bright*0.2, false, false);
        if(isSolid(bullets[k].x, bullets[k].y) || isWorm(bullets[k].x, bullets[k].y) || bullets[k].x > 31 || bullets[k].x < 0){
          plot_antialiased(bullets[k].x, bullets[k].y, red, conf.bright, false, false);
          damage_calc(bullets[k].x, bullets[k].y, weapon);
          bullets[k].vx = 0;
          bullets[k].vy = 0;
          bullets[k].ay = 0;
          bullets[k].timer = t;
        }
        break;
      case laser:
        if(bullets[k].vx == 0){
          float f = t - bullets[k].timer;
          if(f >= 500) bullets[k].alive = false;
          f = 1.0 - f/500.0;
          int dir = -1;
          float elevation = (float)(bullets[k].y-wy+1)/abs(bullets[k].x-wx);
          if(bullets[k].x < 1) dir = 1;
          for(uint8_t o = 0; o < bullets[k].ay; o++){
            plot_antialiased(bullets[k].x - o*dir, bullets[k].y - o*elevation, red, f*conf.bright, false, false);
          }
          break;
        }
        else{
          if(bullets[k].ay == 0){
            float damage_x = bullets[k].x;
            float damage_y = bullets[k].y;
            while(damage_x > 0 && damage_x < 31){
              damage_calc(damage_x, damage_y, weapon);
              damage_x += dir;
              damage_y += elevation;
            }
          }
          bullets[k].ay += dir*bullets[k].vx*delta;
          bullets[k].x += bullets[k].vx*delta;
          bullets[k].y += bullets[k].vy*delta;
          for(uint8_t o = 0; o < bullets[k].ay; o++){
            plot_antialiased(bullets[k].x - o*dir, bullets[k].y - o*elevation, red, conf.bright, false, false);
          }
          if(bullets[k].x > 31 || bullets[k].x < 0){
            bullets[k].vx = 0;
            bullets[k].vy = 0;
            bullets[k].timer = t;
          }
        }
        break;
      case bat:
        if(bullets[k].ay < 1){
          damage_calc(bullets[k].x, bullets[k].y, weapon);
          bullets[k].timer = t;
          bullets[k].ay = 1;
        }
        else{
          float fstate = ((float)(t - bullets[k].timer))/1000;
          if(fstate >= 1.0){
            bullets[k].alive = false;
            break;
          }
          int dir = 1;
          if(wormsconf.worms[player][worm].look_left) dir = -1;
          if(fstate > 0.5){
            plot_antialiased(bullets[k].x, bullets[k].y + 1, yellow, conf.bright, false, false);
            plot_antialiased(bullets[k].x - dir, bullets[k].y, yellow, conf.bright, false, false);
            plot_antialiased(bullets[k].x - 2*dir, bullets[k].y - 1, yellow, conf.bright*0.5, false, false);
            break;
          }
          float l = fstate*4.0*2.5;
          if(fstate > 0.25){
            l = 2.5-4.0*fstate;
          }
          for(int p = 0; p < l; p++){
            plot_antialiased(bullets[k].x + dir * (p - 1), bullets[k].y - 1, yellow, conf.bright, false, false);
          }
          plot_antialiased(bullets[k].x + dir * (ceil(l) - 1), bullets[k].y - 1, yellow, conf.bright*(l - (float)floor(l)), false, false);
        }
        break;
    }
  }
  if(no_bullets) return 1;
  else return 0;
}

void damage_calc(float x, float y, uint8_t weapon){
  float r2 = bullets[weapon].radius*bullets[weapon].radius;
  for(int dx = floor(-bullets[weapon].radius-1); dx <= ceil(bullets[weapon].radius); dx++){
    int d2 = dx * dx;    
    for(int dy = floor(-bullets[weapon].radius-1); dy <= ceil(bullets[weapon].radius); dy++){
      int dist = d2 + dy*dy;
      if(dist <= 1) 
        paint_reduce(x + dx, y + dy, bullets[weapon].destruct);
      else if(dist < r2)
        paint_reduce(x + dx, y + dy, (r2 - dist + 0.5) / r2 * bullets[weapon].destruct);
    }
  }
  for(uint8_t p = 0; p < 2; p++){
    for(uint8_t w = 0; w < 4; w++){
      if(wormsconf.worms[p][w].health <= 0) continue;
      worm* w1 = &(wormsconf.worms[p][w]);
      float dx = w1->x - x;
      float dy = w1->y - y;
      float dist = min(min(sqrt(dx*dx + dy*dy), sqrt(dx*dx + (dy-1)*(dy-1))), sqrt(dx*dx + (dy-2)*(dy-2)));
      if(dist > bullets[weapon].radius) continue;
      int h1 = w1->health;
      if(dist < 1) w1->health -= bullets[weapon].damage;
      else w1->health -= (bullets[weapon].radius - dist + 0.5) / bullets[weapon].radius * bullets[weapon].damage;
      Serial.println("damage to worm " + String(p) + "." + String(w) + ": " + String(h1 - w1->health));
      worm_fall(p, w);
    }
  }
}

void paint_reduce(float x, float y, float reduce_by){
  if(x < 0 || x > 31 || y < 0 || y > 7 || reduce_by < 0 || reduce_by > 1) return;
  int nx = floor(x);
  int ny = floor(y);
  float fx = x - (float)nx;
  float fy = y - (float)ny;
  float weights[4];
  weights[0] = (1.0-fx)*(1.0-fy);//*(1.0-fx)*(1.0-fy);
  weights[1] = (1.0-fy)*fx;//*(1.0-fy)*fx;
  weights[2] = (1.0-fx)*fy;//*(1.0-fx)*fy;
  weights[3] = fx*fx;//*fy*fy;
  for(uint8_t k = 0; k < 4; k++){
    int px = nx + (k & 1);
    int py = ny + (k & 2 > 0);
    uint8_t c[3];
    colorAt(px, py, c, paintdata);
    for(uint8_t l = 0; l < 3; l++)
      c[l] = (1.0 - reduce_by*weights[k]) * (float)c[l];
      
    if((c[0] + c[1] + c[2]) > (float)400.0) paintxy(px, py, c);
    else paintxy(px, py, blank);
  }
}

void drawmap(){
  if(wormsconf.state == game_over) return;
  float f2 = 0.7;
  if(wormsconf.state == game_start) f2 = 0.3;
  for(int k = 0; k < NUM_LEDS; k++){
    uint8_t col[3] = {paintdata[k].g, paintdata[k].r, paintdata[k].b};
    draw_pixel(k, &col[0], conf.bright*f2, false);
  }
}
