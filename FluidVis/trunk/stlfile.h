#ifndef STLFILE_H
#define STLFILE_H

#include <iostream>
#include <fstream>
#include <exception>

#include "vector.h"
#include "GLMaterials.h"

#define qMax(a, b) a > b ? a : b;
#define qMin(a, b) a < b ? a : b;

enum GLMaterial {DEFAULT,ALUMINIUM, BRASS, BRONZE, P_BRONZE, CHROME, COPPER, P_COPPER, GOLD, P_GOLD, PEWTER, SILVER, P_SILVER, STEEL,
				EMERALD, JADE, OBSIDIAN, PEARL, RUBY, TURQUOISE,
				PLASTIC, RUBBER};//Material ¼Ó¼ºµé

class StlFile {
 public:
	 char STLFileName[200];
	 char STLFileTitle[200];
	 int itsMaterial;
	 int itsShadeRed;
	int itsShadeGreen;
	int itsShadeBlue;


  class wrong_header_size : public ::std::exception {};
  class error_opening_file : public ::std::exception {};
  enum Format {
    ASCII,
    BINARY
  };
  typedef struct {
    float x;
    float y;
    float z;
  } Normal;
  typedef char Extra[2];
  typedef struct {
    Normal normal;
 Vector vector[3];
    Extra extra;
  } Facet;
  typedef struct {
    ::std::string   header;
    Format          type;
    int             numFacets;
    int             numPoints;
    Vector          max;
    Vector          min;
    Vector          size;
    float           boundingDiameter;
    float           shortestEdge;
    float           volume;
    float           surface;
  } Stats;
  StlFile();
  ~StlFile();
  void open(char* fileName, char* FileTitle);
  void close();
  void setFormat(const int format);
  Stats getStats() const { return stats; };
  Facet* getFacets() const { return facets; };

 private:
  void initialize(char* fileName);
  void allocate();
  void readData(int, int);
  int readIntFromBytes(::std::ifstream&);
  float readFloatFromBytes(::std::ifstream&);
  int getNumPoints();
  float getVolume();
  float getSurface();
  float getArea(Facet *facet);
  void calculateNormal(float normal[], Facet *facet);
  void normalizeVector(float v[]);
  ::std::ifstream file;
  Facet *facets;
  Stats stats;
};

#endif  // STLFILE_H