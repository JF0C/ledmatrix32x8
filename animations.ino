int smiling(float x, float y, uint8_t* col, float f){
  unsigned long tstop = 1000;
  unsigned long s = t%tstop;
  int width = 12;
  float r = ((float)s/(float)tstop)*width*.5;
  for(int k = 6-floor(r); k <= 6+floor(r); k++){
    plot_antialiased(x + k, y + smilegraph(k, 5.0), col, f, false, false);
  }
  int xa = 6-floor(r)-1;
  float intens = r-(float)floor(r);
  plot_antialiased(x + xa, y + smilegraph(xa, 5.0), col, intens*f, false, false);
  int xb = 12-xa;
  plot_antialiased(x + xb, y + smilegraph(xb, 5.0), col, intens*f, false, false);
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
      plot_antialiased(x + k + s, y + 2.0 + (3.0 + s/2)*yk, col, f, false, false);
      if(k < s) continue;
      plot_antialiased(x + k, y + 2.0, col, f, false, false);
    }
    for(int k = 0; k < width/2; k++){
      float yk = graph(k, width/2-s/2);
      if(yk < 0) continue;
      plot_antialiased(x + k + s/2, y + 2.0 - (2.0 + s/2)*yk, col, f, false, false);
      plot_antialiased(x + width - k - s/2, y + 2.0 - (2.0 + s/2)*yk, col, f, false, false);
    }
  }
  else{
    float intens = sin((s2 - 0.5)*2.0*3.1416);
    smiley(x + 2, 0, col, conf.bright*intens, "heart", 0)+1;
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

// TODO control shape using parameters for width height
// loudness, frequency, change of frequency, 
int laughing(float x, float y, uint8_t* col, float f){
  unsigned long tstop = 1000;
  float z = (float)(t%tstop)/(float)tstop;
  float s = sin(2.0*3.1416*z)*sin(2.0*3.1416*z) + 0.15*sin(3.5*3.1416*z)*sin(3.5*3.1416*z);
  int width = 12;
  for(int k = 0; k < width; k++){
    plot_antialiased(x + k, y, col, f, false, false);
    plot_antialiased(x + k, y + smilegraph(k, s*3.0 + 2.0), col, f, false, false);
  }
  return width;
}

void plot_antialiased(float x, float y, uint8_t* col, float intens, bool linear, bool ov){
  if(x < 0 || x > 31 || y < 0 || y > 7) return;
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
  drawxy(nx, ny, col, intens*f00, ov);
  drawxy(nx+1, ny, col, intens*f10, ov);
  drawxy(nx, ny+1, col, intens*f01, ov);
  drawxy(nx+1, ny+1, col, intens*f11, ov);
}

struct star{
  unsigned long deathtime;
  uint8_t x;
  uint8_t y;
  bool alive;
};

star stars[30];
unsigned long spawntimers[6];
void twinkling(uint8_t* col, float f, uint8_t batch){
  if(batch > 5) return;
  if(spawntimers[batch] < t){
    for(uint8_t k = batch*5; k < batch*5 + 5; k++){
      if(!stars[k].alive){
        stars[k].alive = true;
        stars[k].deathtime = t + 500;
        stars[k].x = random(0, 32);
        stars[k].y = random(0, 8);
        break;
      }
    }
    spawntimers[batch] = t + 100;
  }
  for(uint8_t k = batch*5; k < batch*5 + 5; k++){
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

struct ripple{
  bool alive;
  unsigned long deathtime;
  uint8_t x, y;
};

ripple ripples[4];
unsigned long droptimer;
const unsigned long rippletime = 2100;
void dripping(uint8_t* col, float f, uint8_t batch){
  if(batch > 0) return;
  if(droptimer < t){
    for(uint8_t k = 0; k < 3; k++){
      if(!ripples[k].alive){
        ripples[k].alive = true;
        ripples[k].deathtime = t + rippletime;
        ripples[k].x = random(0, 32);
        ripples[k].y = random(0, 8);
        break;
      }
    }
    droptimer = t + 700;
  }
  for(uint8_t k = 0; k < 3; k++){
    if(ripples[k].deathtime < t) ripples[k].alive = false;
    if(!ripples[k].alive) continue;
    float age = (float)(rippletime - ripples[k].deathtime + t)/((float)rippletime);
    for(int x = -8; x < 8; x++){
      for(int y = -8; y < 8; y++){
        float r = sqrt((float)(x*x + y*y));
        if(r > 9*age+2) continue;
        int px = x + ripples[k].x;
        int py = y + ripples[k].y;
        if(px < 0 || px > 31.0) continue;
        if(py < 0 || py > 7.0) continue;
        float val = wavelet(r, age);
        drawxy(px, py, col, f*val, false);
      }
    }
  }
}

const float trigsize = 4;
float wavelet(float r, float age){
  if(r > 9*age + 2) return 0;
  float xt = 4.8*age;
  float s = xt - trigsize;
  float e = xt + trigsize;
  float red = 1.0+12.0*age;
  float roff = 1.0;
  if(r > 9*age){
    roff = (r-9*age)/2.0;
  }
  float rp = r-16.0*age;
  if(r < s) return 0;
  if(r > e) return 0;
  return 0.5*(1.0+cos(1.5708*rp))*(1.0-abs(xt-r)/trigsize)*(1-age)*roff;
}
