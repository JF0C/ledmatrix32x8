// main call for worms in every iteration of loop
float sine = 0.0;
void render_worms(){
  if(!conf.wormsmode) return;
  sine = (sin(1.5708*(float)t)+1.0)/2.0;
  drawmap();

  
}

enum weapons{
  bazooka,
  rifle,
  laser,
  bat
};

enum worms_states{
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
  float vx, vy;
  float x, y;
  float damage;
  float radius;
  float ay;
  bool alive;
};

projectile bullets[10];

struct worm{
  float x;
  float y;
  float dy;
  bool look_left;
  int health;
  bool selected;
  weapons weapon;
};

struct wormsconfiguration{
  String names[2];
  int tokens[2];
  worm worms[2][4];
  worms_states state;
}wormsconf;

String getWormsState(){
  String result = "{\"state\":" + String((int)wormsconf.state) + ",";
  for(uint8_t k = 0; k < 2; k++){
    for(uint8_t l = 0; l < 4; l++){
      result += "\"worm" + String(k) + "." + String(l) + "\":" + String(wormsconf.worms[k][l].health) + ",";
    }
  }
  result += "\"p1_won\":" + hasWonStr(0) + ",";
  result += "\"p2_won\":" + hasWonStr(1) + ",";
  result += "\"p1_name\":\"" + wormsconf.names[0] + "\",";
  result += "\"p2_name\":\"" + wormsconf.names[0] + "\"}";
  return result;
}

String hasWonStr(int p){
  if(hasWon(p)) return "true";
  return "false";
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
  if((col[0] + col[1] + col[2]) > (float)400.0) return true;
  return false;
}

void select_worm(int token, int worm, bool confirmed){
  int player = playerFromToken(token);
  if(player == -1) return;
  if(worm < 0 || worm > 3) return;
  if(player == 0 && wormsconf.state != 0) return;
  if(player == 1 && wormsconf.state != 2) return;
  int sel = 0;
  for(uint8_t k = 0; k < 4; k++){
    wormsconf.worms[player][k].selected = worm == k;
    if(worm == k) sel = k;
  }
  if(confirmed && wormsconf.worms[player][sel].health > 0) 
    wormsconf.state = (worms_states)(((int)wormsconf.state)+1);
}

void select_weapon(int token, int weapon){
  int player = playerFromToken(token);
  if(player == -1) return;
  int worm = wormFromPlayer(player);
  if(worm == -1) return;
  if(player == 0 && wormsconf.state != 1) return;
  if(player == 1 && wormsconf.state != 3) return;
  if(weapon < 0 || weapon > 3) return;
  wormsconf.worms[player][worm].weapon = (weapons)weapon;
  int look = 1;
  if(wormsconf.worms[player][worm].look_left) look = -1;
  setWeapon(weapon, wormsconf.worms[player][worm].x, wormsconf.worms[player][worm].y, wormsconf.worms[player][worm].dy, look);
}

void setWeapon(int weapon, float x, float y, float dy, int look){
  switch(weapon){
    case bazooka:
      bullets[0].alive = false;
      bullets[0].damage = 70;
      bullets[0].radius = 5;
      bullets[0].x = x + look;
      bullets[0].vx = -4;
      bullets[0].y = y + 1;
      bullets[0].vy = dy*4;
      bullets[0].ay = 0.5;
      break;
    case rifle:
      for(uint8_t k = 0; k < 10; k++){
        bullets[k].damage = 6;
        bullets[k].alive = false;
        bullets[k].radius = 1;
        bullets[k].x = x + look;
        bullets[k].vx = 8*look + 0.01*(float)(random(100)-50);
        bullets[k].y = y + 1;
        bullets[k].vy = dy*8 + 0.01*(float)(random(100)-50);
        bullets[k].ay = 0.1; 
      }
      break;
    case laser:
      bullets[0].alive = false;
      bullets[0].damage = 15;
      bullets[0].radius = 1;
      bullets[0].x = x + look;
      bullets[0].vx = 10*look;
      bullets[0].y = y + 1;
      bullets[0].vy = dy * 10;
      bullets[0].ay = 0;
      break;
    case bat:
      bullets[0].alive = false;
      bullets[0].damage = 60;
      bullets[0].radius = 2;
      bullets[0].x = x + 2*look;
      bullets[0].y = y + 1;
      bullets[0].vx = 0;
      bullets[0].vy = 0;
      bullets[0].ay = 0;
      break;
  }
}

unsigned long shoot_start;
void shoot(int token){
  int player = playerFromToken(token);
  if(player == -1) return;
  int worm = wormFromPlayer(player);
  if(worm == -1) return;
  switch(wormsconf.worms[player][worm].weapon){
    case bazooka: bullets[0].alive = true; break;
    case rifle: for(uint8_t k = 0; k < 10; k++){bullets[k].alive = true;} break;
    case bat: bullets[0].alive = true; break;
    case laser: bullets[0].alive = true; break;
  }
  shoot_start = t;
  wormsconf.state = (worms_states)4;
}

void move_worm(int token, bool left, float y){
  int player = playerFromToken(token);
  if(player == -1) return;
  if(player == 0 && wormsconf.state != 1) return;
  if(player == 1 && wormsconf.state != 3) return;
  int worm = wormFromPlayer(player);
  if(worm == -1) return;
  if(wormsconf.worms[player][worm].health <= 0) return;
  if(left){
    int x = ceil(wormsconf.worms[player][worm].x);
    int y = wormsconf.worms[player][worm].y;
    if(!isSolid(x-1, y-1) && !isSolid(x-1, y-2) && !(isSolid(x-1, y-3) && isSolid(x-1, y)) && x >= 0.25){
      wormsconf.worms[player][worm].x -= 0.25;
    }
    x = floor(wormsconf.worms[player][worm].x);
    if(isSolid(x-1, y)) wormsconf.worms[player][worm].y -= 1;
  }
  else{
    int x = floor(wormsconf.worms[player][worm].x);
    int y = wormsconf.worms[player][worm].y; 
    if(!isSolid(x+1, y-1) && !isSolid(x+1, y-2) && !(isSolid(x+1, y-3) && isSolid(x+1, y)) && x <= 30.75){
      wormsconf.worms[player][worm].x += 0.25;
    }
    x = ceil(wormsconf.worms[player][worm].x);
    if(isSolid(x-1, y)) wormsconf.worms[player][worm].y -= 1;
  }

  int fallen = 0;
  int x = floor(wormsconf.worms[player][worm].x);
  while(!isSolid(x, wormsconf.worms[player][worm].y + 1) &&
        !isSolid(x+1, wormsconf.worms[player][worm].y + 1) &&
        wormsconf.worms[player][worm].y < 7){
    wormsconf.worms[player][worm].y++;
    fallen++;
  }
  if(wormsconf.worms[player][worm].y > 7) wormsconf.worms[player][worm].y = 7;
  wormsconf.worms[player][worm].health -= 10*fallen;
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
  int pos = random(16) * player * 16;
  uint8_t blank[3] = {0,0,0};
  bool pos_found = false;
  for(uint8_t y = 2; y < 7; y++){
    if(!isSolid(pos, y-2) && !isSolid(pos, y-1) && !isSolid(pos, y)){
      pos_found = true;
      wormsconf.worms[player][worm].x = pos;
      wormsconf.worms[player][worm].y = y;
    }
  }
  if(!pos_found){
    for(int px = pos-1; px < pos+2; px++)
      for(int py = 0; py < 3; py++)
        paintxy(px, py, &blank[0]);
    wormsconf.worms[player][worm].x = pos;
    wormsconf.worms[player][worm].y = 2;
  }
  if(player == 1) wormsconf.worms[player][worm].look_left = true;
  else wormsconf.worms[player][worm].look_left = false;
}

int initPlayer(String plname, int player){
  if(player != 0 && player != 1) return 0;
  if(wormsconf.tokens[player] != 0) return 0;
  wormsconf.names[player] = plname;
  wormsconf.tokens[player] = random(10000) + 1;
  for(uint8_t k = 0; k < 4; k++){
    initWorm(k, player);
  }
  return wormsconf.tokens[player];
}

void initWorms(String paint){
  loadpaint(paint);
  wormsconf.state = (worms_states)0;
  for(uint8_t l = 0; l < 2; l++){
    wormsconf.tokens[l] = 0;
    for(uint8_t k = 0; k < 4; k++){
      wormsconf.worms[l][k].health = 100;
      if(k==0)
        wormsconf.worms[l][k].selected = true;
    }
  }
}

bool hasWon(int player){
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
  for(uint8_t k = 0; k < 2; k++){
    for(uint8_t l = 0; l < 4; l++){
      uint8_t* pcol;
      if(k == 0) pcol = red;
      if(k == 1) pcol = blue;
      float x = wormsconf.worms[k][l].x;
      float y = wormsconf.worms[k][l].y;
      float look = 1;
      if(wormsconf.worms[k][l].look_left) look = -1;
      if(wormsconf.worms[k][l].health <= 0){
        // cross if died
        plot_antialiased(x, y, wormscol, conf.bright, false);
        plot_antialiased(x, y-1, wormscol, conf.bright, false);
        plot_antialiased(x, y-3, wormscol, conf.bright, false);
        plot_antialiased(x-1, y-2, wormscol, conf.bright, false);
        plot_antialiased(x+1, y-2, wormscol, conf.bright, false);
        // player color
        plot_antialiased(x, y-2, pcol, conf.bright, false);
        continue;
      }
      // lower body
      plot_antialiased(x, y, wormscol, conf.bright, false);
      // player color / badge
      plot_antialiased(x, y-1, pcol, conf.bright, false);
      //tail
      plot_antialiased(x-look, y, wormscol, conf.bright, false);
      //head
      plot_antialiased(x, y-2, wormscol, conf.bright, false);
      int yoff = -3;
      if(y <= 2) yoff = 1;
      lifebar(x-1, y+yoff, wormsconf.worms[k][l].health);
      // highlight
      if(wormsconf.worms[k][l].selected && 
        (wormsconf.state == 0 || wormsconf.state == 1 || wormsconf.state == 5) &&
        k == 0){
        plot_antialiased(x-2, y+yoff, red, conf.bright*sine, false);
        plot_antialiased(x+2, y+yoff, red, conf.bright*sine, false);
      }
      if(wormsconf.worms[k][l].selected && 
        (wormsconf.state == 2 || wormsconf.state == 3 || wormsconf.state == 6) &&
        k == 1){
        plot_antialiased(x-2, y+yoff, blue, conf.bright*sine, false);
        plot_antialiased(x+2, y+yoff, blue, conf.bright*sine, false);
      }
      switch(wormsconf.worms[k][l].weapon){
        case bazooka:
          plot_antialiased(x-1, y+2, white, conf.bright, false);
          plot_antialiased(x+1, y+2, white, conf.bright, false);
          plot_antialiased(x+2*look, y+2, white, conf.bright, false);
          plot_antialiased(x+look, y+1, white, conf.bright, false);
          break;
        case rifle:
          plot_antialiased(x-1, y+1, white, conf.bright, false);
          plot_antialiased(x+1, y+1, white, conf.bright, false);
          plot_antialiased(x+2*look, y+1, white, conf.bright, false);
          plot_antialiased(x+look, y, white, conf.bright*0.5, false);
          break;
        case bat:
          plot_antialiased(x+look, y+1, yellow, conf.bright, false);
          plot_antialiased(x+2*look, y+2, yellow, conf.bright, false);
          plot_antialiased(x, y, yellow, 0.5*conf.bright, false);
          break;
        case laser:
          plot_antialiased(x+look, y+1, white, conf.bright, false);
          plot_antialiased(x+2*look, y+1, green, conf.bright*sine, false);
          break;
      }
    }
  }
}

void draw_trajectory(float x, float y, float dy, float ay){
  TODO
}

void lifebar(uint8_t x, uint8_t y, int health){
  float h = (float)3.0*health/100.0;
  float hi = floor(h);
  float hf = h - (float)hi;
  for(int k = 0; k < hi; k++){
    drawxy(x + k, y, green, conf.bright, false);
  }
  drawxy(x + hi, y, green, conf.bright*hf, false);
}

void paintxyf(float x, float y, uint8_t* col, float intens){
  int nx = floor(x);
  int ny = floor(y);
  float fx = x - (float)nx;
  float fy = y - (float)ny;
  float f00, f01, f10, f11;
  f00 = (1.0-fx)*(1.0-fy)*(1.0-fx)*(1.0-fy);
  f10 = (1.0-fy)*fx*(1.0-fy)*fx;
  f01 = (1.0-fx)*fy*(1.0-fx)*fy;
  f11 = fx*fx*fy*fy;
  uint8_t c[3] = {f00*intens*col[0], f00*intens*col[1], f00*intens*col[2]};
  paintxy(x, y, &c[0]);
  c[0] = f01*intens*col[0]; c[1] = f01*intens*col[1]; c[2] = f01*intens*col[2];
  paintxy(x, y+1, &c[0]);
  c[0] = f10*intens*col[0]; c[1] = f10*intens*col[1]; c[2] = f10*intens*col[2];
  paintxy(x+1, y, &c[0]);
  c[0] = f11*intens*col[0]; c[1] = f11*intens*col[1]; c[2] = f11*intens*col[2];
  paintxy(x+1, y+1, &c[0]);
}

void drawmap(){
  for(int k = 0; k < NUM_LEDS; k++){
    uint8_t col[3] = {paintdata[k].g, paintdata[k].r, paintdata[k].b};
    draw_pixel(k, &col[0], conf.bright, false);
  }
}