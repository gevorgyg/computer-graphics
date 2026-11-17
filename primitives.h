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

// sphere with diameter 1, the poles are on the y axis
Model MakeSphere() {
    Model sphere = NewModel();
    int rings = 8;
    int segments = 16;
    float radius = 0.5f;

    arrput(sphere.vertices, ((Vec3){0.f, radius, 0.f}));
    for (int r = 1; r <= rings; r++) {
        float phi = PI * r / (rings + 1);
        for (int s = 0; s < segments; s++) {
            float theta = 2.f * PI * s / segments;
            arrput(sphere.vertices, ((Vec3){radius * sin(phi) * cos(theta),
                                            radius * cos(phi),
                                            radius * sin(phi) * sin(theta)}));
        }
    }
    arrput(sphere.vertices, ((Vec3){0.f, -radius, 0.f}));

    int south = (int)arrlen(sphere.vertices);
    int lastRing = 2 + (rings - 1) * segments;
    for (int s = 0; s < segments; s++) {
        int next = (s + 1) % segments;
        arrput(sphere.triangles, ((Triangle){ .vertices = {1, 2 + next, 2 + s} }));
        arrput(sphere.triangles, ((Triangle){ .vertices = {south, lastRing + s, lastRing + next} }));
    }
    for (int r = 0; r < rings - 1; r++) {
        for (int s = 0; s < segments; s++) {
            int next = (s + 1) % segments;
            int a = 2 + r * segments + s, b = 2 + r * segments + next;
            int c = a + segments, d = b + segments;
            arrput(sphere.triangles, ((Triangle){ .vertices = {a, b, c} }));
            arrput(sphere.triangles, ((Triangle){ .vertices = {b, d, c} }));
        }
    }

    for (int i = 0; i < arrlen(sphere.vertices); i++) {
        arrput(sphere.normals, V3Normalize(sphere.vertices[i]));
    }
    for (int t = 0; t < arrlen(sphere.triangles); t++) {
        for (int k = 0; k < 3; k++) {
            sphere.triangles[t].normals[k] = sphere.triangles[t].vertices[k];
        }
    }

    sphere.vertexCount = (int)arrlen(sphere.vertices);
    return sphere;
}

#endif //PRIMITIVES_HEADER
