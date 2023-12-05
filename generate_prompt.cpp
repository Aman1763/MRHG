#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include "generate_prompt.h"


/* All of the Vertex methods. */
/* Default Constructor (that probably isnt necessary anymore) */
Vertex::Vertex() : north(nullptr), south(nullptr), east(nullptr), west(nullptr), name(""), wall(NORTH) {}

/* Constructor */
Vertex::Vertex(const string &_name, Direction _wall) : north(nullptr), south(nullptr), east(nullptr), west(nullptr), name(_name), wall(_wall) {}

/* Creates an edge between this and _dest. Puts _dest to _direction from this and puts this to the opposite of _direction to _dest */
void Vertex::setEdge(Vertex &_dest, Direction _direction) {
  switch(_direction) {
    case NORTH:
      north = &_dest;
      _dest.south = this;
      break;
    case SOUTH:
      south = &_dest;
      _dest.north = this;
      break;
    case EAST:
      east = &_dest;
      _dest.west = this;
      break;
    case WEST:
      west = &_dest;
      _dest.east = this;
      break;
  }
}

/* Returns the name of this. */
string Vertex::getName() { return name; }

/* Returns the wall this is on. */
Direction Vertex::getWall() { return wall; }

/* Returns a pointer to the Vertex that is dir of this. Returns NULL if there is no vertex there. */
Vertex *Vertex::getVertex(Direction dir) {
  switch(dir) {
    case NORTH:
      return north;
    case SOUTH:
      return south;
    case EAST:
      return east;
    case WEST:
      return west;
    default:
      /* only here to prevent warnings */
      return nullptr;
  }
}

/* Returns the number of other vertices this is connected to */
int Vertex::getConnectivity() {
  int cnt = 0;
  if (north)
    cnt++;
  if (south)
    cnt++;
  if(east)
    cnt++;
  if(west)
    cnt++;
  return cnt;
}

/* Prints this in a readable format. */
void Vertex::printVertex() {
  cout << "name: " << name << endl;
  switch (wall) {
    case NORTH:
      cout << "on a wall to the North in its hallway." << endl;
      break;
    case SOUTH:
      cout << "on a wall to the South in its hallway." << endl;
      break;
    case EAST:
      cout << "on a wall to the East in its hallway." << endl;
      break;
    case WEST:
      cout << "on a wall to the West in its hallway." << endl;
      break;
    default:
      cout << "is an intersection." << endl;
      break;
  }
  if (north)
    cout << "\tTo the North: " << north->name << endl;
  if (south)
    cout << "\tTo the South: " << south->name << endl;
  if (east)
    cout << "\tTo the East: " << east->name << endl;
  if (west)
    cout << "\tTo the West: " << west->name << endl;
}

/* All of the BuildingMap methods. */

/* Adds a new vertex to this. */
void BuildingMap::addVertex(const string &name, Direction dir) {
  vertexList.insert(pair<string, Vertex>(name, Vertex(name, dir)));
}

/* creates an edge between _x and _y using _dir. */
void BuildingMap::setEdge(string _x, string _y, Direction _dir) {
  vertexList.at(_x).setEdge(vertexList.at(_y), _dir);
}

Vertex BuildingMap::getVertex(string name) { return vertexList[name]; }

Vertex *BuildingMap::getNextVertex(string name, Direction dir) {
  return vertexList[name].getVertex(dir);
}

/* Returns true if key is present in this. */
bool BuildingMap::contains(string key) {
  auto it = vertexList.find(key);
  return it != vertexList.end();
}

void BuildingMap::printMap() {
  cout << "printing map" << endl;
  for (auto &it : vertexList) {
    it.second.printVertex();
    cout << endl;
  }
}

void add_all_doors(BuildingMap &map);
void add_all_connectivity(BuildingMap &map);
bool is_intersection(string name);
string cardinal_to_relative(Direction travelling, Direction side);
string traverse_hallway(Vertex *&v, Direction travelling, BuildingMap m);
string get_other_hallways(Vertex *v, Direction travelling);
string travel_all_options(Vertex *v, Direction travelling, BuildingMap m);
bool valid_direction(string dir, Direction &o);

/* Adds all doors to the map.
   MUST HAVE "all_door.txt" IN THE SAME FOLDER!!! */
void add_all_doors(BuildingMap &map) {
  cout << "adding all the doors..." << endl;
  ifstream inputFile("all_doors.txt");

  if (inputFile.is_open()) {
    string line;
    while (getline(inputFile, line)) {
      istringstream iss(line);
      string name;
      string sWall;
      Direction wall;

      if (iss >> name >> sWall) {
        if (sWall == "NORTH") {
          wall = NORTH;
        } else if (sWall == "SOUTH") {
          wall = SOUTH;
        } else if (sWall == "EAST") {
          wall = EAST;
        } else if (sWall == "WEST") {
          wall = WEST;
        } else {
          wall = INTER;
        }
      }
      map.addVertex(name, wall);
    }
  }
  cout << "done adding the doors" << endl;
}

/* Creates the connectivity between all of the doors in map.
   MUST HAVE "door_connectivity.txt" IN THE SAME FOLDER!!! */
void add_all_connectivity(BuildingMap &map) {
  cout << "generating connectivity..." << endl;
  ifstream inputFile("door_connectivity.txt");

  if (inputFile.is_open()) {
    string line;
    while (getline(inputFile, line)) {
      istringstream iss(line);
      string name;
      string north;
      string south;
      string west;
      string east;

      if (iss >> name >> north >> south >> west >> east) {
        if (north != "null") {
          map.setEdge(name, north, NORTH);
        }
        if (south != "null") {
          map.setEdge(name, south, SOUTH);
        }
        if (west != "null") {
          map.setEdge(name, west, WEST);
        }
        if (east != "null") {
          map.setEdge(name, east, EAST);
        }
      }
    }
  }
  cout << "done doing the thing" << endl;
}

/* Generates the prompt for ChatGPT using start as the initial position
   orientation as the direction it is facing.
   Returns the prompt split in two where the destination is needed.
   (in retrospect couldve done this recursively quite easily) */
vector<string> generate_prompt(string start, Direction orientation, BuildingMap map) {
  vector<string> fullPrompt;
  string prompt1 = R"(## Building layout\n\n### Initial Position\n)";
  prompt1 += R"(- **Current Position:** (Facing )";
  string current_pos = R"((Facing )"; 
  switch (orientation) {
    case NORTH:
      prompt1 += R"(North )";
      current_pos += R"(North )";
      break;
    case SOUTH:
      prompt1 += R"(South )";
      current_pos += R"(South )";
      break;
    case EAST:
      prompt1 += R"(East )";
      current_pos += R"(East )";
      break;
    case WEST:
      prompt1 += R"(West )";
      current_pos += R"(West )";
      break;
  }

  bool is_intersection = false;
  if (start == R"(maindoor)") {
    prompt1 += R"(at the entrance)\n\n)";
    current_pos += R"(at the entrance))";
  } else if (start.substr(0, start.size() - 1) == R"(intersection)") {
    is_intersection = true;
    if (start.at(start.size() - 1) == '0') {
      prompt1 += R"(at a L intersection)\n\n)";
      current_pos += R"(at a L intersection))";
    } else {
      prompt1 += R"(at a T intersection)\n\n)";
      current_pos += R"(at a T intersection))";
    }
  } else {
    prompt1 += R"(at door )" + start + R"()\n\n)";
    current_pos += R"(at door )" + start + R"())";
  }
  
  prompt1 += R"(### Desired Position\n- **Destination:** )";
  fullPrompt.push_back(prompt1);

  string prompt2 = R"((MAKE SURE TO PARSE EACH HALLWAY TO ENSURE THE DESTINATION IS IN THE HALLWAY YOU THINK IT IS, THE DOOR ORDER MAY BE CONFUSING AND YOU CAN'T RELY ON CHRONOLOGICAL ANALYSIS)\n\n)";
  Vertex startingPoint = map.getVertex(start);
  Direction travelling;
  /* Determine which cardinal direction is Right. */
  switch (orientation) {
    case NORTH:
      travelling = EAST;
      break;
    case SOUTH:
      travelling = WEST;
      break;
    case EAST:
      travelling = SOUTH;
      break;
    case WEST:
      travelling = NORTH;
      break;
  }

  Vertex *next = map.getNextVertex(start, travelling);
  /* traverse to the right (if it exists) */
  if (next) {
    prompt2 += R"(### Right turn )" + current_pos + R"(\n)";
    prompt2 += traverse_hallway(next, travelling, map);

    /* Here we are either at the end of a hallway or we are at an intersection. */
    /* This is where it gets super fucking complicated... */
    if (next) {
      /* we know this is an intersection */
      /* Next step is to figure out what kind it is (T or L) */
      int connectivity = next->getConnectivity();
      if (connectivity > 1) {
        switch (connectivity) {
          case 2:
            prompt2 += R"(**L intersection with a hallway )" + get_other_hallways(next, travelling) + R"(\n)";
            break;
          case 3:
            prompt2 += R"(**T intersection with hallways )" + get_other_hallways(next, travelling) + R"(\n)";
            break;
        }
        /* next step is to travel all directions we can. */
        prompt2 += travel_all_options(next, travelling, map);
      }
    }
  }

  /* Determine which direction is Left. */
  switch (orientation) {
    case NORTH:
      travelling = WEST;
      break;
    case SOUTH:
      travelling = EAST;
      break;
    case EAST:
      travelling = NORTH;
      break;
    case WEST:
      travelling = SOUTH;
      break;
  }
    
    next = map.getNextVertex(start, travelling);
    if (next) {
      prompt2 += R"(### Left turn )" + current_pos + R"(\n)";
      prompt2 += traverse_hallway(next, travelling, map);

      if (next) {
        int connectivity = next->getConnectivity();
        if (connectivity > 1) {
          switch (connectivity) {
            case 2:
              prompt2 += R"(**L intersection with a hallway )" + get_other_hallways(next, travelling) + R"(\n)";
              break;
            case 3:
              prompt2 += R"(**T intersection with hallways )" + get_other_hallways(next, travelling) + R"(\n)";
              break;
          }
          prompt2 += travel_all_options(next, travelling, map);
        }
      }
    }

  /* Determine which direction is Forward (not necessary) */
  travelling = orientation;
  next = map.getNextVertex(start, travelling);
  if (next) {
    prompt2 += R"(## hallway forward )" + current_pos + R"(\n)";
    prompt2 += traverse_hallway(next, travelling, map);
    
    if (next) {
      int connectivity = next->getConnectivity();
      if (connectivity > 1) {
        switch (connectivity) {
          case 2:
            prompt2 += R"(**L intersection with a hallway )" + get_other_hallways(next, travelling) + R"(\n)";
            break;
          case 3:
            prompt2 += R"(**T intersection with hallways )" + get_other_hallways(next, travelling) + R"(\n)";
            break;
        }
        prompt2 += travel_all_options(next, travelling, map);
      }
    }
  }

  /* Determine which direction is backward */
  switch (orientation) {
    case NORTH:
      travelling = SOUTH;
      break;
    case SOUTH:
      travelling = NORTH;
      break;
    case EAST:
      travelling = WEST;
      break;
    case WEST:
      travelling = EAST;
      break;
  }

  next = map.getNextVertex(start, travelling);
  if (next) {
    prompt2 += R"(## hallway behind you )" + current_pos + R"(\n)";
    prompt2 += traverse_hallway(next, travelling, map);
    
    if (next) {
      int connectivity = next->getConnectivity();
      if (connectivity > 1) {
        switch (connectivity) {
          case 2:
            prompt2 += R"(**L intersection with a hallway )" + get_other_hallways(next, travelling) + R"(\n)";
            break;
          case 3:
            prompt2 += R"(**T intersection with hallways )" + get_other_hallways(next, travelling) + R"(\n)";
            break;
        }
        prompt2 += travel_all_options(next, travelling, map);  
      }
    }
  }

  fullPrompt.push_back(prompt2);
  return fullPrompt;
}

bool is_intersection(string name) {
  return name.substr(0, name.size() - 1) == "intersection";
}

string cardinal_to_relative(Direction travelling, Direction side) {
  if (travelling == NORTH) {
    switch (side) {
      case EAST:
        return R"((Right)\n)";
      case WEST:
        return R"((Left)\n)";
      default:
        /* prevent warnings */
        return "";
    }
  } else if (travelling == SOUTH) {
    switch (side) {
      case EAST:
        return R"((Left)\n)";
      case WEST:
        return R"((Right)\n)";
      default:
        /* prevent warnings */
        return "";
    }
  } else if (travelling == EAST) {
    switch (side) {
      case NORTH:
        return R"((Left)\n)";
      case SOUTH:
        return R"((Right)\n)";
      default:
        /* prevent warnings */
        return "";
    }
  } else {
    switch (side) {
      case NORTH:
        return R"((Right)\n)";
      case SOUTH:
        return R"((Left)\n)";
      default:
        /* prevent warnings */
        return "";
    }
  }
}

string traverse_hallway(Vertex *&v, Direction travelling, BuildingMap m) {
  string ret = R"(- **Room Sequence:**\n)";
  int cnt = 1;
  do {
    if (cnt > 1) {
      v = m.getNextVertex(v->getName(), travelling);
    }
    if (is_intersection(v->getName()))
      break;
    ret += to_string(cnt) + R"(. )" + v->getName() + R"( )";
    Direction side = v->getWall();
    ret += cardinal_to_relative(travelling, side);
    cnt++;
  } while (m.getNextVertex(v->getName(), travelling));
  return ret;
}

string get_other_hallways(Vertex *v, Direction travelling) {
  vector<string> options;
  int cnt = 0;
  if (v->getVertex(NORTH)) {
    cnt++;
    switch (travelling) {
      case NORTH:
        options.push_back(R"(in front of you)");
        break;
      case SOUTH:
        // options.push_back("behind you");
        cnt--;
        break;
      case EAST:
        options.push_back(R"(to your left)");
        break;
      case WEST:
        options.push_back(R"(to your Right)");
        break;
    }
  }
  if (v->getVertex(SOUTH)) {
    cnt++;
    switch (travelling) {
      case NORTH:
        // options.push_back("behind you");
        cnt--;
        break;
      case SOUTH:
        options.push_back(R"(in front of you)");
        break;
      case EAST:
        options.push_back(R"(to your left)");
        break;
      case WEST:
        options.push_back(R"(to your Right)");
        break;
    }
  }
  if (v->getVertex(EAST)) {
    cnt++;
    switch (travelling) {
      case NORTH:
        options.push_back(R"(to your Right)");
        break;
      case SOUTH:
        options.push_back(R"(to your left)");
        break;
      case EAST:
        options.push_back(R"(in front of you)");
        break;
      case WEST:
        // options.push_back("behind you");
        cnt--;
        break;
    }
  }
  if (v->getVertex(WEST)) {
    cnt++;
    switch (travelling) {
      case NORTH:
        options.push_back(R"(to your left)");
        break;
      case SOUTH:
        options.push_back(R"(to your Right)");
        break;
      case EAST:
        // options.push_back("behind you");
        cnt--;
        break;
      case WEST:
        options.push_back(R"(in front of you)");
        break;
    }
  }
  string ret = R"()";
  for (int i = 0; i < cnt; i++) {
    ret += options[i] + R"( and )";
  }
  return ret.substr(0, ret.size() - 5);
}

string travel_all_options(Vertex *v, Direction travelling, BuildingMap m) {
  /* fuck it this will be recursive (nothing bad could possibly happen) */
  string ret = R"(- **next hallway )";
  Vertex *next;
  if (v->getConnectivity() == 2) {
    /* handling the L */
    switch (travelling) {
      case SOUTH:
        ret += R"((Turning left at the L intersection)\n)";
        next = v->getVertex(EAST);
        ret += traverse_hallway(next, EAST, m);
        break;
      case WEST:
        ret += R"((Turning right at the L intersection)\n)";
        next = v->getVertex(NORTH);
        ret += traverse_hallway(next, NORTH, m);
        ret += R"(**T intersection with hallways )" + get_other_hallways(next, NORTH) + R"(\n\n)";
        ret += travel_all_options(next, NORTH, m);
        break;
    }
  } else {
    /* handling the T */
    switch (travelling) {
      case NORTH:
        ret += R"((Going straight at the T intersection)\n)";
        next = v->getVertex(NORTH);
        ret += traverse_hallway(next, NORTH, m);
        ret += R"(\n- **next hallway (Going right at the T intersection)\n)";
        next = v->getVertex(EAST);
        ret += traverse_hallway(next, EAST, m);
        break;
      case WEST:
        ret += R"((Going right at the T intersection)\n)";
        next = v->getVertex(NORTH);
        ret += traverse_hallway(next, NORTH, m);
        ret += R"(\n- **next hallway (Going left at the T intersection)\n)";
        next = v->getVertex(SOUTH);
        ret += traverse_hallway(next, SOUTH, m) + R"(\n)";
        ret += travel_all_options(next, SOUTH, m);
        break;
      case SOUTH:
        ret += R"((Going left at the T intersection)\n)";
        next = v->getVertex(EAST);
        ret += traverse_hallway(next, EAST, m);
        ret += R"(\n- **next hallway (Going straight at the T intersection)\n)";
        next = v->getVertex(SOUTH);
        ret += traverse_hallway(next, SOUTH, m) + R"(\n)";
        ret += travel_all_options(next, SOUTH, m);
        break;
    }
  }
  return ret;
}

bool valid_direction(string dir, Direction &o) {
  for (char &c : dir) {
    c = tolower(static_cast<unsigned char>(c));
  }
  if (dir == "north") {
    o = NORTH;
    return true;
  } else if (dir == "south") {
    o = SOUTH;
    return true;
  } else if (dir == "east") {
    o = EAST;
    return true;
  } else if (dir == "west") {
    o = WEST;
    return true;
  } else {
    o = INTER;
    return false;
  }
}

BuildingMap init_map() {
  BuildingMap m;
  add_all_doors(m);
  add_all_connectivity(m);
  return m;
}
