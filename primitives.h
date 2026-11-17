#if !defined(PRIMITIVES_HEADER)
#define PRIMITIVES_HEADER

#include "model.h"

// cube with edge of len 1
Model MakeCube() {
    Model cube = NewModel();
  float coords[2] = {-0.5, 0.5};
  Vec3 t;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      for (int k = 0; k < 2; k++) {
        t.x = coords[i];
        t.y = coords[j];
        t.z = coords[k];
        arrput(cube.vertices, t);
      }
    }
  }

  AddTriangle(&cube, 1, 2, 4); AddTriangle(&cube, 1, 4, 3);
  AddTriangle(&cube, 5, 7, 8); AddTriangle(&cube, 5, 8, 6);
  AddTriangle(&cube, 1, 5, 6); AddTriangle(&cube, 1, 6, 2);
  AddTriangle(&cube, 3, 4, 8); AddTriangle(&cube, 3, 8, 7);
  AddTriangle(&cube, 1, 3, 7); AddTriangle(&cube, 1, 7, 5);
  AddTriangle(&cube, 2, 6, 8); AddTriangle(&cube, 2, 8, 4);

  cube.vertexCount = (int)arrlen(cube.vertices);

  return cube;
}

// TODO
Model MakeSphere() {
    Model sphere = NewModel();
    return sphere;
}

#endif //PRIMITIVES_HEADER
