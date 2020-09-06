void render_fourier(bool firstexec){
    // pixel mahlen
  if(firstexec){
    Serial.println("[...] called render_fourier()");
  }
  
  if(!conf.fouriermode) return;

  float laugh[8][8] = {{.0, .0, .5, .9, .9, .5, .0, .0},
                     {.0, .7, .0, .0, .0, .0, .7, .0},
                     {.5, .9, .9, .0, .9, .0, .0, .5},
                     {.9, .0, .0, .0, .9, .9, .0, .9},
                     {.9, .0, .0, .0, .9, .9, .0, .9},
                     {.5, .9, .9, .0, .9, .0, .0, .5},
                     {.0, .7, .0, .0, .0, .0, .7, .0},
                     {.0, .0, .5, .9, .9, .5, .0, .0}};

  // pixel mahlen
  if(firstexec){
    Serial.println("[...] render_fourier() draw pixel");
  }
  
  drawxy(2, 2, red, conf.bright, false);
  
}
