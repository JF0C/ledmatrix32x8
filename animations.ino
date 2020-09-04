int smiling(float x, float y, uint8_t* col, float f){
  unsigned long tstop = 1000;
  unsigned long s = t%tstop;
  int width = 12;
  float r = ((float)s/(float)tstop)*width*.5;
  for(int k = 6-floor(r); k <= 6+floor(r); k++){
    plot_antialiased(x + k, y + smilegraph(k, 5.0), col, f, false);
  }
  int xa = 6-floor(r)-1;
  float intens = r-(float)floor(r);
  plot_antialiased(x + xa, y + smilegraph(xa, 5.0), col, intens*f, false);
  int xb = 12-xa;
  plot_antialiased(x + xb, y + smilegraph(xb, 5.0), col, intens*f, false);
  return width;
}

float smilegraph(float x, float a){
  return a - ((x-6.0)*(x-6.0)/36.0*a);
}

float graph(float x, float b){
  return -x*(x-b)/b/b*4;
}

int kissing(float x, float y, uint8_t* col, float f){
  float s2 = (float)(t%2000)/2000.0;
  int width = 12;
  if(s2 <= 0.5){
    float s = (float)(t%1000)/1000.0*2.0;
    for(int k = 0; k < width; k++){
      float yk = graph(k, width-2*s);
      if(yk < 0) continue;
      plot_antialiased(x + k + s, y + 2.0 + (3.0 + s/2)*yk, col, f, false);
      if(k < s) continue;
      plot_antialiased(x + k, y + 2.0, col, f, false);
    }
    for(int k = 0; k < width/2; k++){
      float yk = graph(k, width/2-s/2);
      if(yk < 0) continue;
      plot_antialiased(x + k + s/2, y + 2.0 - (2.0 + s/2)*yk, col, f, false);
      plot_antialiased(x + width - k - s/2, y + 2.0 - (2.0 + s/2)*yk, col, f, false);
    }
  }
  else{
    float intens = sin((s2 - 0.5)*2.0*3.1416);
    smiley(x + 2, 0, col, conf.bright*intens, "heart")+1;
    return width;
    /*
    for(int yh = 0; yh < 8; yh++){
      for(int xh = 0; xh < 8; xh++){
        plot_antialiased(x + (float)xh, y + (float)yh, col, heart[xh][yh]*f*intens, false);
      }
    }
    */
  }
  return width;
}

int laughing(float x, float y, uint8_t* col, float f){
  unsigned long tstop = 1000;
  float z = (float)(t%tstop)/(float)tstop;
  float s = sin(2.0*3.1416*z)*sin(2.0*3.1416*z) + 0.15*sin(3.5*3.1416*z)*sin(3.5*3.1416*z);
  int width = 12;
  for(int k = 0; k < width; k++){
    plot_antialiased(x + k, y, col, f, false);
    plot_antialiased(x + k, y + smilegraph(k, s*3.0 + 2.0), col, f, false);
  }
  return width;
}

void plot_antialiased(float x, float y, uint8_t* col, float intens, bool linear){
  int nx = floor(x);
  int ny = floor(y);
  float fx = x - (float)nx;
  float fy = y - (float)ny;
  float f00, f01, f10, f11;
  if(linear){
    f00 = (1.0-fx)*(1.0-fy);
    f10 = (1.0-fy)*fx;
    f01 = (1.0-fx)*fy;
    f11 = fx*fx;
  }
  else{
    f00 = (1.0-fx)*(1.0-fy)*(1.0-fx)*(1.0-fy);
    f10 = (1.0-fy)*fx*(1.0-fy)*fx;
    f01 = (1.0-fx)*fy*(1.0-fx)*fy;
    f11 = fx*fx*fy*fy;
  }
  drawxy(nx, ny, col, intens*f00, false);
  drawxy(nx+1, ny, col, intens*f10, false);
  drawxy(nx, ny+1, col, intens*f01, false);
  drawxy(nx+1, ny+1, col, intens*f11, false);
}

struct star{
  unsigned long deathtime;
  uint8_t x;
  uint8_t y;
  uint8_t col[3];
  bool alive;
};

star stars[20];
unsigned long t1;
void twinkling(uint8_t* col, float f){
  if(t1 < t){
    for(uint8_t k = 0; k < 20; k++){
      if(!stars[k].alive){
        stars[k].alive = true;
        stars[k].deathtime = t + 500;
        stars[k].x = random(0, 32);
        stars[k].y = random(0, 8);
        colcp(col, &stars[k].col[0]);
        break;
      }
    }
    t1 = t + 100;
  }
  for(uint8_t k = 0; k < 20; k++){
    if(stars[k].deathtime < t) stars[k].alive = false;
    if(!stars[k].alive) continue;
    //float age = sin((float)(t - stars[k].deathtime)/500.0*3.1416);
    float age = trig(stars[k].deathtime - 500, stars[k].deathtime, stars[k].deathtime-t);
    drawxy(stars[k].x, stars[k].y, col, f*age, false);
    drawxy(stars[k].x-1, stars[k].y, col, 0.3*f*age, false);
    drawxy(stars[k].x+1, stars[k].y, col, 0.3*f*age, false);
    drawxy(stars[k].x, stars[k].y-1, col, 0.3*f*age, false);
    drawxy(stars[k].x, stars[k].y+1, col, 0.3*f*age, false);
  }
}

float trig(float s, float e, float x){
  float half = (e-s)/2.0;
  if(x < half)
    return x/half;
  else
    return 1.0 - (x-half)/half;
}
