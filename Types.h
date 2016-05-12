#ifndef TYPES_H
#define TYPES_H

#ifdef __APPLE_CC__
#include <OpenGL/gl3.h>
#include <OpenGL/gl3ext.h>
#define __gl_h_
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <string>
#include <algorithm>  
#include <vector>
#include <unordered_map>
#include <iostream>
#include <time.h>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Magick++.h>
#include "math.h"

using namespace Magick;

class Vertex;
class Face;
class Material;
class Texture;
class Mesh;
class Instance;
class WorldMesh;
class Model;
class SkyBox;
class MeshIO;
class Camera;
class Shader;
class RenderAttributes;

typedef std::vector<Vertex>::iterator VertexIter;
typedef std::vector<Vertex>::const_iterator VertexCIter;
typedef std::vector<Face>::iterator FaceIter;
typedef std::vector<Face>::const_iterator FaceCIter;

#endif
