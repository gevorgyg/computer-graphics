#if !defined(PRIMITIVES_HEADER)
#define PRIMITIVES_HEADER

#include "model.h"

// cube with edge of len 1
Model MakeCube() {
  Model cube = {};
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

  Triangle cover[12] = {
    // -x
    { .vertices = {1, 2, 4} }, { .vertices = {1, 4, 3} },
    // +x
    { .vertices = {5, 7, 8} }, { .vertices = {5, 8, 6} },
    // -y
    { .vertices = {1, 5, 6} }, { .vertices = {1, 6, 2} },
    // +y
    { .vertices = {3, 4, 8} }, { .vertices = {3, 8, 7} },
    // -z
    { .vertices = {1, 3, 7} }, { .vertices = {1, 7, 5} },
    // +z
    { .vertices = {2, 6, 8} }, { .vertices = {2, 8, 4} },
  };

  arrsetlen(cube.triangles, 12);
  memcpy(cube.triangles, cover, sizeof(cover));

  return cube;
}

// TODO
Model MakeSphere() {
  Model sphere = {};
}

#endif //PRIMITIVES_HEADER
