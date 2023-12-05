#ifndef GENERATE_PROMPT_H
#define GENERATE_PROMPT_H

#include <vector>
#include <string>
#include <map>

using namespace std;

enum Direction {
  INTER,
  NORTH,
  SOUTH,
  EAST,
  WEST
};

class Vertex {
  private:
    Vertex *north;  /* Points to the door to the North if any. */
    Vertex *south;  /* Points to the door to the South if any. */
    Vertex *east;   /* Points to the door to the East if any. */
    Vertex *west;   /* Points to the door to the West if any. */
    string name;    /* The door name/number. */
    Direction wall; /* Which direction the the door is on. */
  
  public:
    Vertex();
    Vertex(const string &_name, Direction _wall);
    void setEdge(Vertex &_dest, Direction _direction);
    string getName();
    Direction getWall();
    Vertex *getVertex(Direction dir);
    int getConnectivity();
    void printVertex();
};

class BuildingMap {
  private:
    map<string, Vertex> vertexList; /* A map containing all the vertices in this. */

  public:
    void addVertex(const string &name, Direction dir);
    void setEdge(string _x, string _y, Direction _dir);
    Vertex getVertex(string name);
    Vertex *getNextVertex(string name, Direction dir);
    bool contains(string key);
    void printMap();
};

BuildingMap init_map();
vector<string> generate_prompt(string start, Direction orientation, BuildingMap map);

#endif