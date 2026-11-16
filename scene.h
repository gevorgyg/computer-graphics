#if !defined(SCENE_HEADER)
#define SCENE_HEADER

#include "model.h"
#include "math.h"

typedef enum {
    PROJ_PERSPECTIVE,
    PROJ_ORTHOGRAPHIC
} ProjectionType;

typedef struct Camera {
    Vec3 position;
    Vec3 target;
    Vec3 up;

    ProjectionType projection;

    float fov;
    float aspect;
    float zNear;
    float zFar;

    float orthoSize;
} Camera;

typedef struct Scene {
    Model *models;
    Camera *cameras;
	
	int activeModelIndex;
	int activeCameraIndex;
    
    bool advanceTime;
    float step;
} Scene;

void AddModel(Scene *scene, Model model);
void RemoveModel(Scene *scene, int modelIndex);
Mat4 GetView(Camera* cam);
Mat4 GetProjection(Camera* cam);

#endif // SCENE_HEADER

// Scene Implementation
#if defined(SCENE_IMPLEMENTATION) 
#if !defined(SCENE_IMPLEMENTED)
#define SCENE_IMPLEMENTED

#include "stb_ds.h" // For dynamic arrays

Mat4 GetView(Camera* cam) {
    return M4LookAt(cam->position, cam->target, cam->up);
}

Mat4 GetProjection(Camera* cam) {
    if(cam->projection == PROJ_ORTHOGRAPHIC) {
        float halfH = cam->orthoSize * 0.5f;
        float halfW = halfH * cam->aspect;
        return M4Orthographic(-halfW, halfW, -halfH, halfH, cam->zNear, cam->zFar);
    }
    return M4Perspective(cam->fov, cam->aspect, cam->zNear, cam->zFar);
}

void AddModel(Scene *scene, Model model) {
    arrput(scene->models, model);
}

void RemoveModel(Scene *scene, int modelIndex) {
    arrdel(scene->models, modelIndex);
}

#endif // SCENE_IMPLEMENTED
#endif // SCENE_IMPLEMENTATION
