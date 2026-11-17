#if !defined(MODEL_HEADER)
#define MODEL_HEADER

#include "math.h"

typedef struct Triangle {
  int vertices[3];
  int textures[3];
  int normals[3];
} Triangle;


typedef struct Model {
  // Model geometry data
  int vertexCount;
  Vec3 *vertices;
  Vec3* textures;
  Vec3 *normals;
  Triangle* triangles;
  
  // Model transformation data
  Vec3 position;
  // rotations with respect to: x, y, z
  Vec3 rotation;
  Vec3 scale;
  bool hidden;


  // GPU handles (HW3)
  unsigned int gpuVertexBufferId;
  unsigned int gpuNormalBufferId;
    
  unsigned int gpuVertexArrayId;
} Model;

// only declare new models through this function!!!
Model NewModel();
void AddTriangle(Model *model, int a, int b, int c);
Model LoadModelFromObjFile(const char *filename);
void FreeModel(Model *model);
Mat4 GetModelMatrix(const Model *model);

#endif // MODEL_HEADER


// Model Implementation
#if defined(MODEL_IMPLEMENTATION)
#if !defined(MODEL_IMPLEMENTED)
#define MODEL_IMPLEMENTED

#include "stb_ds.h" // For dynamic arrays

//
// Provided helper functions
//
static int ParseNextInt(const char *str, int *start, int len) {
  int sign = 1;
  int result = 0;
    
  int i = *start;
  for(; i < len; ++i) {
    if(str[i] == '-') {
      sign = -1;
      break;
    } else if(str[i] >= '0' && str[i] <= '9') {
      break;
    } else if(str[i] == EOF || str[i] == '\n') {
      return 0;
    }
  }
    
  for(; i < len; ++i) {
    if(str[i] < '0' || str[i] > '9') break;
        
    result *= 10;
    result += (int)(str[i] - '0');
  }
    
  result *= sign;
  *start = i;
    
  return result;
}

static float ParseNextFloat(const char *str, int *start, int len) {
  bool parse_before_dot = true;
  int sign = 1;
    
  int i = *start;
  for(; i < len; ++i) {
    if(str[i] == '.') {
      ++i;
      parse_before_dot = false;
      break;
    } else if(str[i] == '-') {
      ++i;
      sign = -1;
      break;
    } else if(str[i] >= '0' && str[i] <= '9') {
      break;
    } else if(str[i] == EOF || str[i] == '\n') {
      return 0.f;
    }
  }
    
  int before_dot = 0;
  int after_dot = 0;
  int fraction_digits = 0;
    
  if(parse_before_dot) {
    for(; i < len; ++i) {
      if(str[i] < '0' || str[i] > '9') break;
            
      before_dot *= 10;
      before_dot += (int)(str[i] - '0');
    }
  }
  if(str[i] == '.') {
    ++i;
    for(; i < len; ++i) {
      if(str[i] < '0' || str[i] > '9') break;
            
      after_dot *= 10;
      after_dot += (int)(str[i] - '0');
      fraction_digits++;
    }
  }
    
  float result = (float)before_dot + (float)after_dot/pow(10, fraction_digits);
    
  result *= (float)sign;
  *start = i;
    
  return result;
}

static Vec2 ParseNextVec2(const char *str, int *start, int len) {
  Vec2 v;
  int i = *start;
  v.x = ParseNextFloat(str, &i, len);
  v.y = ParseNextFloat(str, &i, len);
  *start = i;
  return v;
}

static Vec3 ParseNextVec3(const char *str, int *start, int len) {
  Vec3 v;
  int i = *start;
  v.x = ParseNextFloat(str, &i, len);
  v.y = ParseNextFloat(str, &i, len);
  v.z = ParseNextFloat(str, &i, len);
  *start = i;
  return v;
}

static Vec3i ParseNextTriplet(const char *str, int *start, int len) {
  Vec3i t = {0, 0, 0};
  int i = *start;
  t.x = ParseNextInt(str, &i, len);       

  if (i < len && str[i] == '/') {
    i++;                                  
    if (i < len && str[i] != '/') {
      t.y = ParseNextInt(str, &i, len);   
    }
    if (i < len && str[i] == '/') {
      i++;                                
      t.z = ParseNextInt(str, &i, len);   
    }
  }

  *start = i;
  return t;
}

static Triangle ParseNextTriangle(const char *str, int *start, int len) {
  Triangle tri = {};
  int i = *start;
  Vec3i t;
  for (int k = 0; k < 3; k++) {
    t = ParseNextTriplet(str, &i, len);
    tri.vertices[k] = t.x;
    tri.textures[k] = t.y;
    tri.normals[k] = t.z;
  }
  *start = i;
  return tri;
}

// Helper functions end


void AddTriangle( Model *m, int a, int b, int c) {
    Vec3 e1 = V3Sub(m->vertices[b-1], m->vertices[a-1]);
    Vec3 e2 = V3Sub(m->vertices[c-1], m->vertices[a-1]);
    arrput(m->normals, V3Normalize(V3Cross(e1, e2)));
    int n = (int)arrlen(m->normals);
    arrput(m->triangles, ((Triangle){ .vertices = {a, b, c}, .normals = {n, n, n} }));
}

Model NewModel(void) {
    Model m = {};
    m.scale = (Vec3){1.f, 1.f, 1.f};
    return m;
}


// model->rotation holds angles in degrees about x, y, z
Mat4 GetModelMatrix(const Model *model) {
    float toRadians = PI / 180.f;
    Mat4 Rx = M4Rotate(model->rotation.x * toRadians, (Vec3){1.f, 0.f, 0.f});
    Mat4 Ry = M4Rotate(model->rotation.y * toRadians, (Vec3){0.f, 1.f, 0.f});
    Mat4 Rz = M4Rotate(model->rotation.z * toRadians, (Vec3){0.f, 0.f, 1.f});
    Mat4 R = M4MulM4(Rz, M4MulM4(Ry, Rx));
    Mat4 T = M4Translate(model->position);
    Mat4 S = M4ScaleV3(model->scale);

    return M4MulM4(T, M4MulM4(R, S));
}

Model LoadModelFromObjFile(const char *filename) {
    Model model = NewModel();
    
  int dataSize = -1;
  const char *data = (char *)LoadFileData(filename, &dataSize);
    
  int lineIndex = 0;
  for (int i = 0; i < dataSize; ++i) {

    // Skip whitespace
    while ((data[i] == ' '  || data[i] == '\t' || data[i] == '\r') && (i < dataSize)) ++i;
        
    switch (data[i]) {
    case '#': {
      // OBJ comment, ignore line
    } break;
            
    case 'v': {
      ++i;
      if (data[i] == ' ') {
        Vec3 v = ParseNextVec3(data, &i, dataSize);
        arrput(model.vertices, v);
      } else if (data[i] == 'n') {
        i++;
        Vec3 v = ParseNextVec3(data, &i, dataSize);
        arrput(model.normals, v);
      } else if (data[i] == 't') {
        i++;
        Vec3 v = ParseNextVec3(data, &i, dataSize);
        arrput(model.textures, v);
      } else {
        TraceLog(LOG_WARNING, "%s:%d: OBJ Loader: Unhandled character '%c' when parsing a vertex, skipping the rest of the line", filename, lineIndex, data[i]);
      }
    } break;
            
    case 'f': {
      ++i;
      Triangle t = ParseNextTriangle(data, &i, dataSize);
      arrput(model.triangles, t);
    } break;
            
    default: {
      TraceLog(LOG_WARNING, "%s:%d: OBJ Loader: Unhandled character '%c' when parsing a line, skipping the rest of the line", filename, lineIndex, data[i]);
    } break;
    }
        
    // Ignore the rest of the line
    while((data[i] != '\n') && (i < dataSize)) ++i;
        
    lineIndex += 1;
  }
  model.vertexCount = (int)arrlen(model.vertices);

  UnloadFileData((unsigned char*)data);
    
  return model;
}

void FreeModel(Model *model) {
  arrfree(model->vertices);
  arrfree(model->textures);
  arrfree(model->normals);
  arrfree(model->triangles);
}

#endif // MODEL_IMPLEMENTED
#endif // MODEL_IMPLEMENTATION
