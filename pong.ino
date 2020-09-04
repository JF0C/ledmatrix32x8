void activate_pong(bool p1){
  if(conf.pongmode) return;
  if(p1) pconf.player1 = true;
  else pconf.player2 = true;
  if(pconf.player1 && pconf.player2){
    conf.pongmode = true;
    conf.paintmode = false;
    pconf.points_p1 = 0;
    pconf.points_p2 = 0;
    initball(0);
    randomSeed(t);
  }
  Serial.println("Pongmode: " + String(conf.pongmode));
}

void initball(int winner){
  //Serial.println("ball initialized");
  pconf.ballx = 15.5;
  pconf.bally = 3.5;
  pconf.vy = 0;
  pconf.tstart = t + 3000;
  if(pconf.points_p1 > pconf.points_p2){
    pconf.vx = 5;
  }
  else if(pconf.points_p2 > pconf.points_p1){
    pconf.vx = -5;
  }
  else if(random(100)%2 == 0){
    pconf.vx = 5;
  }
  else{
    pconf.vx = -5;
  }
  if(winner > 0) pconf.tstart = t + 6000;
}

void move_bat(bool p1, bool left){
  if(p1){
    if(left) pconf.pos_p1++;
    else pconf.pos_p1--;
    if(pconf.pos_p1 < 0) pconf.pos_p1 = 0;
    if(pconf.pos_p1 > 5) pconf.pos_p1 = 5;
  }
  else{
    if(left) pconf.pos_p2++;
    else pconf.pos_p2--;
    if(pconf.pos_p2 < 0) pconf.pos_p2 = 0;
    if(pconf.pos_p2 > 5) pconf.pos_p2 = 5;
  }
}

void render_pong(){
  if(!conf.pongmode) return;
  if(pconf.tstart > t && pconf.vy == 0){
    drawgame();
    return;
  }
  if(pconf.bally <= 0){
    pconf.bally = 0;
    pconf.vy = -pconf.vy;
  }
  if(pconf.bally >= 7){
    pconf.bally = 7;
    pconf.vy = -pconf.vy;
  }
  if(pconf.ballx <= 1 && pconf.ballx > 0.5){
    float d = pconf.bally - pconf.pos_p1;
    if(d >= 0.0 && d < 1.0){
      pconf.vy -= 1.0;
      pconf.vx = -pconf.vx;
      pconf.vx += 1.0;
    }
    if(d >= 1.0 && d < 2.0){
      pconf.vx = -pconf.vx;
    }
    if(d >= 2.0 && d < 3.0){
      pconf.vy += 1.0;
      pconf.vx = -pconf.vx;
      pconf.vx += 1.0;
    }
  }
  if(pconf.ballx <= 0.5){
    pconf.points_p2++;
    if(pconf.points_p2 < pconf.limit)
      initball(0);
    else
      initball(2);
  }
  if(pconf.ballx >= 30 && pconf.ballx < 30.5){
    float d = pconf.bally - pconf.pos_p2;
    if(d >= 0.0 && d < 1.0){
      pconf.vy -= 1.0;
      pconf.vx = -pconf.vx;
      pconf.vx -= 1.0;
    }
    if(d >= 1.0 && d < 2.0){
      pconf.vx = -pconf.vx;
    }
    if(d >= 2.0 && d < 3.0){
      pconf.vy += 1.0;
      pconf.vx = -pconf.vx;
      pconf.vx -= 1.0;
    }
  }
  if(pconf.ballx >= 30.5){
    pconf.points_p1++;
    if(pconf.points_p1 < pconf.limit)
      initball(0);
    else
      initball(1);
  }
  pconf.ballx += (pconf.vx*dt/1000.0);
  pconf.bally += (pconf.vy*dt/1000.0);
  drawgame();
}

void drawgame(){
  if(pconf.tstart > t + 3000){
    uint8_t color[3];
    String msg = "";
    if(pconf.points_p1 >= pconf.limit){
      colcp(&red[0], &color[0]);
      if(pconf.name_p1 != "") msg = pconf.name_p1 + "Wins";
      else msg = "P1 wins!";
    }
    else if(pconf.points_p2 >= pconf.limit){
      colcp(&blue[0], &color[0]);
      if(pconf.name_p2 != "") msg = pconf.name_p2 + "Wins";
      else msg = "P2 wins!";
    }
    int p = 1;
    float f = (float)(pconf.tstart - t - 3000.0)/3000.0;
    for(int k = 0; k < msg.length(); k++){
      p += letter(p, 0, color, conf.bright*f, String(msg[k]))+1;
    }
    return;
  }
  if(pconf.tstart > t && pconf.tstart <= t + 3000){
    if(pconf.points_p2 >= pconf.limit || pconf.points_p1 >= pconf.limit){
      pconf.points_p2 = 0;
      pconf.points_p1 = 0;
    }
    float f = (float)(pconf.tstart - t)/3000.0;
    int p = 6;
    String p1_points = String(pconf.points_p1);
    for(int k = 0; k < p1_points.length(); k++){
      p += letter(p, 0, white, f*conf.bright, String(p1_points[k]))+1;
    }
    letter(15, 0, white, f*conf.bright, ":");
    p = 20;
    String p2_points = String(pconf.points_p2);
    if(p2_points.length() > 1) p = 17;
    for(int k = 0; k < p2_points.length(); k++){
      p += letter(p, 0, white, f*conf.bright, String(p2_points[k]))+1;
    }
  }
  int dxi = floor(pconf.ballx);
  int dyi = floor(pconf.bally);
  float dxf = pconf.ballx - (float)dxi;
  float dyf = pconf.bally - (float)dyi;
  if(pconf.tstart < t + 1000){
    drawxy(dxi, dyi, white, conf.bright*(float)(1.0-dyf)*(1.0-dyf)*(1.0-dyf)*(1.0-dyf), false);
    drawxy(dxi+1, dyi, white, conf.bright*(float)(1.0-dyf)*(1.0-dyf)*dxf*dxf, false);
    drawxy(dxi, dyi+1, white, conf.bright*(float)(1.0-dxf)*(1.0-dxf)*dyf*dyf, false);
    drawxy(dxi+1, dyi+1, white, conf.bright*(float)dxf*dyf*dxf*dyf, false);
  }

  for(int k = 0; k < 3; k++){
    drawxy(1, pconf.pos_p1 + k, red, conf.bright, false);
    drawxy(30, pconf.pos_p2 + k, blue, conf.bright, false);
  }
}
