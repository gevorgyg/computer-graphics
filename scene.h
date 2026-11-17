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
    Vec3 orientation;
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

Camera DefaultCamera();
void AddModel(Scene *scene, Model model);
void RemoveModel(Scene *scene, int modelIndex);
Mat4 GetView(const Camera* cam);
Mat4 GetProjection(const Camera* cam);
void SyncCameraWithOrientation(Camera* cam);

#endif // SCENE_HEADER

// Scene Implementation
#if defined(SCENE_IMPLEMENTATION) 
#if !defined(SCENE_IMPLEMENTED)
#define SCENE_IMPLEMENTED

#include "stb_ds.h" // For dynamic arrays

Mat4 GetView(const Camera* cam) {
    return M4LookAt(cam->position, cam->target, cam->up);
}

Mat4 GetProjection(const Camera* cam) {
    if(cam->projection == PROJ_ORTHOGRAPHIC) {
        float halfH = cam->orthoSize * 0.5f;
        float halfW = halfH * cam->aspect;
        return M4Orthographic(-halfW, halfW, -halfH, halfH, cam->zNear, cam->zFar);
    }
    return M4Perspective(cam->fov, cam->aspect, cam->zNear, cam->zFar);
}

Camera DefaultCamera() {
	return (Camera){
    	.position = {0.f,0.f,5.f},
        .orientation = {0.f,0.f,0.f},
        .target = {0.f,0.f,0.f},
        .up = {0.f,1.f,0.f},
        .projection = PROJ_PERSPECTIVE,
        .fov = 60.f * DEG2RAD,
        .aspect = (float)settings->width/(float)settings->height,
        .zNear = 0.1f,
        .zFar = 100.f,
        .orthoSize = 4.f
    };
}

void AddModel(Scene *scene, Model model) {
    arrput(scene->models, model);
}

void RemoveModel(Scene *scene, int modelIndex) {
    arrdel(scene->models, modelIndex);
}

void AddCamera(Scene *scene, Camera cam) {
    arrput(scene->cameras, cam);
}

void RemoveCamera(Scene* scene, int camIndex) {
    arrdel(scene->cameras, camIndex);
}

void SyncCameraWithOrientation(Camera* cam) {
	Mat3 Rx = M3Rotate(cam->orientation.x * DEG2RAD, (Vec3){1.f,0.f,0.f});
    Mat3 Ry = M3Rotate(cam->orientation.y * DEG2RAD, (Vec3){0.f,1.f,0.f});
    Mat3 Rz = M3Rotate(cam->orientation.z * DEG2RAD, (Vec3){0.f,0.f,1.f});
    Mat3 R = M3MulM3(Ry, M3MULM3(Rx, Rz));

    Vec3 forward = M3MulM3(R, (Vec3){0.f,0.f,-1.f};
	cam->up = M3MulM3(R, (Vec3){0.f, 1.f, 0.f};
    cam->target = V3Add(cam->position, forward);
}

#endif // SCENE_IMPLEMENTED
#endif // SCENE_IMPLEMENTATION
