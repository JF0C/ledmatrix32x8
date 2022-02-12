struct maze_node{
  bool visited;
  uint8_t x, y;
  maze_node* neighbours[4];
};

maze_node maze_nodes[64];
enum maze_states{
  wait_for_players,
  init_maze,
  build_maze,
  play_maze,
  won_maze
};

struct mazeconfig{
  maze_states state;
  bool single_player, p1_joined, p2_joined;
  int moving[2];
  maze_node* current;
  uint8_t nodes_count;
  uint8_t players[2][2];
  uint8_t ptr;
  uint8_t stack[64][2];
  unsigned long timer;
  int iexit;
}mconf;

void render_maze(){
  if(conf.opmode != maze) {
    mconf.state = wait_for_players;
    return;
  }
  switch(mconf.state){
    case wait_for_players:
      printStringSimple("MAZE", yellow, conf.bright, 5);
      if(mconf.p1_joined && mconf.p2_joined || 
         mconf.p1_joined && mconf.single_player) 
           mconf.state = init_maze;
      break;
    case init_maze: // initialize maze
      clear_maze();
      randomSeed(t);
      mconf.state = build_maze;
      mconf.timer = t;
      mconf.current =  &(maze_nodes[0]);
      mconf.current->y = 0;
      mconf.current->x = 0;
      mconf.players[0][0] = 0;
      mconf.players[0][1] = 0;
      mconf.players[1][0] = 31;
      mconf.players[1][1] = 7;
      mconf.current->visited = true;
      mconf.nodes_count = 1;
      mconf.ptr = 0;
      mconf.stack[mconf.ptr][0] = mconf.current->x;
      mconf.stack[mconf.ptr][1] = mconf.current->y;
      mconf.iexit = -1;
      //mconf.single_player = true;
      break;
    case build_maze: // build maze
      if(mconf.single_player && t - mconf.timer < 20)break;
      if(!mconf.single_player && t - mconf.timer < 50)break;
      mconf.timer = t;
      if(addNode()){
        mconf.state = play_maze;
        //addNeighbour(mconf.stack[0][0], mconf.stack[0][1], mconf.stack[1][0], mconf.stack[1][1]);
        //addNeighbour(mconf.stack[1][0], mconf.stack[1][1], mconf.stack[0][0], mconf.stack[0][1]);
      }
      break;
    case play_maze: // play maze
        if(mconf.single_player && mconf.players[0][0] >= 31) {
          mconf.state = won_maze;
          mconf.timer = t;
        }
        if(!mconf.single_player && (mconf.players[0][0] >= 15 || mconf.players[1][0] <= 16)){
          mconf.state = won_maze;
          mconf.timer = t;
        }
      break;
    case won_maze:
      float f = 1.0 - (t - mconf.timer)/5000.0;
      if(mconf.single_player && mconf.players[0][0] >= 31) {
        printStringSimple("Pl1 wins", yellow, conf.bright * f, 0);
      }
      if(!mconf.single_player && mconf.players[0][0] >= 15){
        printStringSimple("Pl1 wins", red, conf.bright * f, 0);
      }
      else if(!mconf.single_player && mconf.players[1][0] <= 16){
        printStringSimple("Pl2 wins", blue, conf.bright * f, 0);
      }
      if(t - mconf.timer > 5000){
        mconf.state = wait_for_players;
        startMaze();
      }
      break;
  }
  draw_maze();
  move_players();
}

void startMaze(){
  mconf.p1_joined = false;
  mconf.p2_joined = false;
  mconf.state = wait_for_players;
}

void setSinglePlayer(bool val){
  if(mconf.state == wait_for_players){
    mconf.single_player = val;
  }
}

void move_players(){
  if(mconf.state != play_maze) return;
  for(uint8_t k = 0; k < 2; k++){
    switch(mconf.moving[k]){
      case 1: // up
        if(!isWall(mconf.players[k][0], mconf.players[k][1] - 1))
          mconf.players[k][1] = mconf.players[k][1] - 1;
        break;
      case 2: // down
        if(!isWall(mconf.players[k][0], mconf.players[k][1] + 1))
          mconf.players[k][1] = mconf.players[k][1] + 1;
        break;
      case 3: // left
        if(!isWall(mconf.players[k][0] - 1, mconf.players[k][1]))
          mconf.players[k][0] = mconf.players[k][0] - 1;
        break;
      case 4: // right
        if(!isWall(mconf.players[k][0] + 1, mconf.players[k][1]))
          mconf.players[k][0] = mconf.players[k][0] + 1;
        break;
    }
    mconf.moving[k] = 0;
  }
}

bool isWall(int x, int y){
  uint8_t col[3];
  if(x > 31 || x < 0) return true;
  if(y > 7 || y < 0) return true;
  colorAt(x, y, col, s1);
  for(uint8_t k = 0; k < 3; k++)
    col[k] = (float)col[k]/conf.bright;

  //Serial.println("color at (" + String(x) + "," + String(y) + " = (" + String(col[0]) + "," + String(col[1]) + "," + String(col[2]) + ")");
  if(col[0] > 200 && col[1] > 200 && col[2] > 200) return true;
  return false;
}

bool addNode(){
  mconf.current->visited = true;
  if(mconf.nodes_count >= 32*(1+mconf.single_player)) return true;
                   //up, down, left, right
  uint8_t dirs[4] = {1,  1,    1,    1};
  if(visited(mconf.current->x, mconf.current->y-1)) dirs[0] = 0;
  if(visited(mconf.current->x, mconf.current->y+1)) dirs[1] = 0;
  if(visited(mconf.current->x-1, mconf.current->y)) dirs[2] = 0;
  if(visited(mconf.current->x+1, mconf.current->y)) dirs[3] = 0;
  if(dirs[0] == 0 && dirs[1] == 0 && dirs[2] == 0 && dirs[3] == 0){
    mconf.ptr = mconf.ptr - 1;
    mconf.current = &(maze_nodes[mconf.stack[mconf.ptr][0]*4 + mconf.stack[mconf.ptr][1]]);
    return false;
  }
  while(dirs[0] + dirs[1] + dirs[2] + dirs[3] > 1){
    dirs[random(4)] = 0;
  }
  mconf.nodes_count = mconf.nodes_count + 1;
  mconf.ptr = mconf.ptr + 1;
  if(dirs[0] == 1){ //up
    mconf.stack[mconf.ptr][0] = mconf.current->x;
    mconf.stack[mconf.ptr][1] = mconf.current->y - 1;
    addNeighbour(mconf.current->x, mconf.current->y, mconf.current->x, mconf.current->y-1);
    addNeighbour(mconf.current->x, mconf.current->y-1, mconf.current->x, mconf.current->y);
  }
  else if(dirs[1] == 1){ //down
    mconf.stack[mconf.ptr][0] = mconf.current->x;
    mconf.stack[mconf.ptr][1] = mconf.current->y + 1;
    addNeighbour(mconf.current->x, mconf.current->y, mconf.current->x, mconf.current->y+1);
    addNeighbour(mconf.current->x, mconf.current->y+1, mconf.current->x, mconf.current->y);
  }
  else if(dirs[2] == 1){ //left
    mconf.stack[mconf.ptr][0] = mconf.current->x - 1;
    mconf.stack[mconf.ptr][1] = mconf.current->y;
    addNeighbour(mconf.current->x, mconf.current->y, mconf.current->x-1, mconf.current->y);
    addNeighbour(mconf.current->x-1, mconf.current->y, mconf.current->x, mconf.current->y);
  }
  else if(dirs[3] == 1){ //right
    mconf.stack[mconf.ptr][0] = mconf.current->x + 1;
    mconf.stack[mconf.ptr][1] = mconf.current->y;
    addNeighbour(mconf.current->x, mconf.current->y, mconf.current->x+1, mconf.current->y);
    addNeighbour(mconf.current->x+1, mconf.current->y, mconf.current->x, mconf.current->y);
  }
  mconf.current = &(maze_nodes[mconf.stack[mconf.ptr][0]*4 + mconf.stack[mconf.ptr][1]]);
  return false;
}

bool visited(uint8_t x, uint8_t y){
  if(x < 0 || x >= 8*(1+mconf.single_player)) return true;
  if(y < 0 || y > 3) return true;
  return maze_nodes[x*4 + y].visited;
}

void clear_maze(){
  for(uint8_t x = 0; x < 16; x++){
    for(uint8_t y = 0; y < 4; y++){
      maze_nodes[x*4 + y].visited = false;
      maze_nodes[x*4 + y].neighbours[0] = NULL;
      maze_nodes[x*4 + y].neighbours[1] = NULL;
      maze_nodes[x*4 + y].neighbours[2] = NULL;
      maze_nodes[x*4 + y].neighbours[3] = NULL;
      maze_nodes[x*4 + y].x = x;
      maze_nodes[x*4 + y].y = y;
    }
  }
}

void addNeighbour(uint8_t tx, uint8_t ty, uint8_t nx, uint8_t ny){
  maze_node* n = &(maze_nodes[tx * 4 + ty]);
  for(uint8_t k = 0; k < 4; k++){
    if(n->neighbours[k] == NULL){
      n->neighbours[k] = &(maze_nodes[nx * 4 + ny]);
      break;
    }
  }
}

void draw_maze(){
  if(mconf.state == build_maze || mconf.state == play_maze){
    for(uint8_t x = 0; x < 32; x++){
      for(uint8_t y = 0; y < 8; y++){
        drawxy(x, y, white, conf.bright, false);
      }
    }
    for(uint8_t x = 0; x < 8*(1+mconf.single_player); x++){
      for(uint8_t y = 0; y < 4; y++){
        if(!maze_nodes[x*4 + y].visited) continue;
        for(uint8_t k = 0; k < 4; k++){
          if(maze_nodes[x*4 + y].neighbours[k] != NULL)
            drawxy((maze_nodes[x*4 + y].neighbours[k]->x + x), (maze_nodes[x*4 + y].neighbours[k]->y + y), blank, conf.bright, true);
        }
        drawxy(2*x, 2*y, blank, conf.bright, true);
      }
    }
  }
  if(mconf.state == build_maze)
    drawxy(mconf.stack[mconf.ptr][0]*2, mconf.stack[mconf.ptr][1]*2, red, conf.bright*0.5, true);
  if(!mconf.single_player && (mconf.state == build_maze || mconf.state == play_maze)){
    uint8_t col[3];
    for(uint8_t x = 0; x < 16; x++){
      for(uint8_t y = 0; y < 8; y++){
        colorAt(x, y, col, s1);
        drawxy(31 - x, 7 - y, col, 1.0, true);
      }
    }
    if(mconf.state == play_maze){
      drawxy(15, 3, green, 0.5*conf.bright, true);
      drawxy(16, 3, green, 0.5*conf.bright, true);
      drawxy(15, 4, green, 0.5*conf.bright, true);
      drawxy(16, 4, green, 0.5*conf.bright, true);
      drawxy(mconf.players[0][0], mconf.players[0][1], red, sineval*conf.bright, true);
      drawxy(mconf.players[1][0], mconf.players[1][1], blue, sineval*conf.bright, true);
    }
  }
  else{
    if(mconf.iexit == -1){
      mconf.iexit = 2*random(4);
    }
    if(mconf.state == play_maze){
      drawxy(31, mconf.iexit, green, 0.5*conf.bright, true);
      drawxy(mconf.players[0][0], mconf.players[0][1], red, sineval*conf.bright, true);
    }
  }
}
