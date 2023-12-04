#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

enum Direction {
  INTER,
  NORTH,
  SOUTH,
  EAST,
  WEST
};

class Vertex {
  public:
    Vertex *north;  /* Points to the door to the North if any. */
    Vertex *south;  /* Points to the door to the South if any. */
    Vertex *east;   /* Points to the door to the East if any. */
    Vertex *west;   /* Points to the door to the West if any. */
    string name;    /* The door name/number. */
    Direction wall; /* Which direction the the door is on. */

    /* Default Constructor (that probably isnt necessary anymore) */
    Vertex() : north(nullptr), south(nullptr), east(nullptr), west(nullptr), name(""), wall(NORTH) {}

    /* Constructor */
    Vertex(const string &_name, Direction _wall) : north(nullptr), south(nullptr), east(nullptr), west(nullptr), name(_name), wall(_wall) {}

    /* Creates an edge between this and _dest. Puts _dest to _direction from this and puts this to the opposite of _direction to _dest */
    void setEdge(Vertex &_dest, Direction _direction) {
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
    string getName() { return name; }

    /* Returns the wall this is on. */
    Direction getWall() { return wall; }

    /* Returns a pointer to the Vertex that is dir of this. Returns NULL if there is no vertex there. */
    Vertex *getVertex(Direction dir) {
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
    int getConnectivity() {
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
    void printVertex() {
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
};


class BuildingMap {
  public:
    map<string, Vertex> vertexList; /* A map containing all the vertices in this. */

    /* Adds a new vertex to this. */
    void addVertex(const string &name, Direction dir) {
      vertexList.insert(pair<string, Vertex>(name, Vertex(name, dir)));
    }

    /* creates an edge between _x and _y using _dir. */
    void setEdge(string _x, string _y, Direction _dir) {
      vertexList.at(_x).setEdge(vertexList.at(_y), _dir);
    }

    Vertex getVertex(string name) { return vertexList[name]; }

    Vertex *getNextVertex(string name, Direction dir) {
      return vertexList[name].getVertex(dir);
    }

    /* Returns true if key is present in this. */
    bool contains(string key) {
      auto it = vertexList.find(key);
      return it != vertexList.end();
    }

    void printMap() {
      cout << "printing map" << endl;
      for (auto &it : vertexList) {
        it.second.printVertex();
        cout << endl;
      }
    }
};

bool is_intersection(string name);
string cardinal_to_relative(Direction travelling, Direction side);
string traverse_hallway(Vertex *&v, Direction travelling, BuildingMap m);
string get_other_hallways(Vertex *v, Direction travelling);
string travel_all_options(Vertex *v, Direction travelling, BuildingMap m);

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
  string prompt1 = "## Building layout\n\n### Initial Position\n";
  prompt1 += "- **Current Position:** (Facing ";
  string current_pos = "(Facing "; 
  switch (orientation) {
    case NORTH:
      prompt1 += "North ";
      current_pos += "North ";
      break;
    case SOUTH:
      prompt1 += "South ";
      current_pos += "South ";
      break;
    case EAST:
      prompt1 += "East ";
      current_pos += "East ";
      break;
    case WEST:
      prompt1 += "West ";
      current_pos += "West ";
      break;
  }

  bool is_intersection = false;
  if (start == "maindoor") {
    prompt1 += "at the entrance)\n\n";
    current_pos += "at the entrance)";
  } else if (start.substr(0, start.size() - 1) == "intersection") {
    is_intersection = true;
    if (start.at(start.size() - 1) == '0') {
      prompt1 += "at a L intersection)\n\n";
      current_pos += "at a L intersection)";
    } else {
      prompt1 += "at a T intersection)\n\n";
      current_pos += "at a T intersection)";
    }
  } else {
    prompt1 += "at door " + start + ")\n\n";
    current_pos += "at door " + start + ")";
  }
  
  prompt1 += "### Desired Position\n- **Destination:** ";
  fullPrompt.push_back(prompt1);

  string prompt2 = "(MAKE SURE TO PARSE EACH HALLWAY TO ENSURE THE DESTINATION IS IN THE HALLWAY YOU THINK IT IS, THE DOOR ORDER MAY BE CONFUSING AND YOU CAN'T RELY ON CHRONOLOGICAL ANALYSIS)\n\n";
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
    prompt2 += "### Right Hallway " + current_pos + "\n";
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
            prompt2 += "**L intersection with a hallway " + get_other_hallways(next, travelling) + "\n";
            break;
          case 3:
            prompt2 += "**T intersection with hallways " + get_other_hallways(next, travelling) + "\n";
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
      prompt2 += "### Left Hallway " + current_pos + "\n";
      prompt2 += traverse_hallway(next, travelling, map);

      if (next) {
        int connectivity = next->getConnectivity();
        if (connectivity > 1) {
          switch (connectivity) {
            case 2:
              prompt2 += "**L intersection with a hallway " + get_other_hallways(next, travelling) + "\n";
              break;
            case 3:
              prompt2 += "**T intersection with hallways " + get_other_hallways(next, travelling) + "\n";
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
    prompt2 += "## hallway forward " + current_pos + "\n";
    prompt2 += traverse_hallway(next, travelling, map);
    
    if (next) {
      int connectivity = next->getConnectivity();
      if (connectivity > 1) {
        switch (connectivity) {
          case 2:
            prompt2 += "**L intersection with a hallway " + get_other_hallways(next, travelling) + "\n";
            break;
          case 3:
              prompt2 += "**T intersection with hallways " + get_other_hallways(next, travelling) + "\n";
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
    prompt2 += "## hallway behind you " + current_pos + "\n";
    prompt2 += traverse_hallway(next, travelling, map);
    
    if (next) {
      int connectivity = next->getConnectivity();
      if (connectivity > 1) {
        switch (connectivity) {
          case 2:
            prompt2 += "**L intersection with a hallway " + get_other_hallways(next, travelling) + "\n";
            break;
          case 3:
              prompt2 += "**T intersection with hallways " + get_other_hallways(next, travelling) + "\n";
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
        return "(Right)\n";
      case WEST:
        return "(Left)\n";
      default:
        /* prevent warnings */
        return "";
    }
  } else if (travelling == SOUTH) {
    switch (side) {
      case EAST:
        return "(Left)\n";
      case WEST:
        return "(Right)\n";
      default:
        /* prevent warnings */
        return "";
    }
  } else if (travelling == EAST) {
    switch (side) {
      case NORTH:
        return "(Left)\n";
      case SOUTH:
        return "(Right)\n";
      default:
        /* prevent warnings */
        return "";
    }
  } else {
    switch (side) {
      case NORTH:
        return "(Right)\n";
      case SOUTH:
        return "(Left)\n";
      default:
        /* prevent warnings */
        return "";
    }
  }
}

string traverse_hallway(Vertex *&v, Direction travelling, BuildingMap m) {
  string ret = "- **Room Sequence:**\n";
  int cnt = 1;
  do {
    if (cnt > 1) {
      v = m.getNextVertex(v->getName(), travelling);
    }
    if (is_intersection(v->getName()))
      break;
    ret += to_string(cnt) + ". " + v->getName() + " ";
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
        options.push_back("in front of you");
        break;
      case SOUTH:
        // options.push_back("behind you");
        cnt--;
        break;
      case EAST:
        options.push_back("to your left");
        break;
      case WEST:
        options.push_back("to your Right");
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
        options.push_back("in front of you");
        break;
      case EAST:
        options.push_back("to your left");
        break;
      case WEST:
        options.push_back("to your right");
        break;
    }
  }
  if (v->getVertex(EAST)) {
    cnt++;
    switch (travelling) {
      case NORTH:
        options.push_back("to your right");
        break;
      case SOUTH:
        options.push_back("to your left");
        break;
      case EAST:
        options.push_back("in front of you");
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
        options.push_back("to your left");
        break;
      case SOUTH:
        options.push_back("to your right");
        break;
      case EAST:
        // options.push_back("behind you");
        cnt--;
        break;
      case WEST:
        options.push_back("in front of you");
        break;
    }
  }
  string ret = "";
  for (int i = 0; i < cnt; i++) {
    ret += options[i] + " and ";
  }
  return ret.substr(0, ret.size() - 5);
}

string travel_all_options(Vertex *v, Direction travelling, BuildingMap m) {
  /* fuck it this will be recursive (nothing bad could possibly happen) */
  string ret = "- **next hallway ";
  Vertex *next;
  if (v->getConnectivity() == 2) {
    /* handling the L */
    switch (travelling) {
      case SOUTH:
        ret += "(Turning left at the L intersection)\n";
        next = v->getVertex(EAST);
        ret += traverse_hallway(next, EAST, m);
        break;
      case WEST:
        ret += "(Turning right at the L intersection)\n";
        next = v->getVertex(NORTH);
        ret += traverse_hallway(next, NORTH, m);
        ret += "**T intersection with hallways " + get_other_hallways(next, NORTH) + "\n\n";
        ret += travel_all_options(next, NORTH, m);
        break;
    }
  } else {
    /* handling the T */
    switch (travelling) {
      case NORTH:
        ret += "(Going straight at the T intersection)\n";
        next = v->getVertex(NORTH);
        ret += traverse_hallway(next, NORTH, m);
        ret += "\n- **next hallway (Going right at the T intersection)\n";
        next = v->getVertex(EAST);
        ret += traverse_hallway(next, EAST, m);
        break;
      case WEST:
        ret += "(Going right at the T intersection)\n";
        next = v->getVertex(NORTH);
        ret += traverse_hallway(next, NORTH, m);
        ret += "\n- **next hallway (Going left at the T intersection)\n";
        next = v->getVertex(SOUTH);
        ret += traverse_hallway(next, SOUTH, m) + "\n";
        ret += travel_all_options(next, SOUTH, m);
        break;
      case SOUTH:
        ret += "(Going left at the T intersection)\n";
        next = v->getVertex(EAST);
        ret += traverse_hallway(next, EAST, m);
        ret += "\n- **next hallway (Going straight at the T intersection)\n";
        next = v->getVertex(SOUTH);
        ret += traverse_hallway(next, SOUTH, m) + "\n";
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

/* READ THIS PLEASE!!!!
   to run this, type ./run_promptgen.sh
   after it will prompt you for the starting room and the starting orientation
   (orientation is NOT case sensitive)
   starting at doors that connect directly to an intersection provides some unwanted output.
   Other than that, I am not sure of other bugs. */
int main(int argc, char *argv[]) {
  BuildingMap ahg;
  add_all_doors(ahg);
  add_all_connectivity(ahg);
  // ahg.printMap();

  Direction orientation = INTER;
  vector<string> prompt;
  /* Validate args */
  if (argc == 3 && ahg.contains(argv[1]) && valid_direction(argv[2], orientation)) {
    /* args are valid */
    prompt = generate_prompt(argv[1], orientation, ahg);
  } else {
    /* assume default */
    prompt = generate_prompt("maindoor", EAST, ahg);
  }
  cout << prompt[0] << prompt[1] << endl;
  return 0;
}


