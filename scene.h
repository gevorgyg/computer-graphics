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
void AddCamera(Scene *scene, Camera cam);
void RemoveCamera(Scene* scene, int camIndex);
Mat4 GetView(const Camera* cam);
Mat4 GetProjection(const Camera* cam);
void SyncCameraFromOrientation(Camera* cam);
void SyncOrientationFromCamera(Camera* cam);
Mat3 CameraRotMat(Vec3 deg);
Vec3 CameraDegFromMat(Mat3 R);

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
        .aspect = 1.f,
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

void SyncCameraFromOrientation(Camera* cam) {
	Mat3 R = CameraRotMat(cam->orientation);

    Vec3 forward = M3MulV3(R, (Vec3){0.f,0.f,-1.f});
	cam->up = M3MulV3(R, (Vec3){0.f, 1.f, 0.f});
    cam->target = V3Add(cam->position, forward);
}

void SyncOrientationFromCamera(Camera* cam) {
	Vec3 dir = V3Normalize(V3Sub(cam->target, cam->position));
    cam->orientation.x = asin(CLAMP(dir.y, -1.f, 1.f)) * RAD2DEG;
    cam->orientation.y = atan2(-dir.x, -dir.z) * RAD2DEG;
    cam->orientation.z = 0.f;
}

Mat3 CameraRotMat(Vec3 deg) {
    Mat3 Rx = M3Rotate(deg.x * DEG2RAD, (Vec3){1.f, 0.f, 0.f});
    Mat3 Ry = M3Rotate(deg.y * DEG2RAD, (Vec3){0.f, 1.f, 0.f});
    Mat3 Rz = M3Rotate(deg.z * DEG2RAD, (Vec3){0.f, 0.f, 1.f});
    return M3MulM3(Ry, M3MulM3(Rx, Rz));
}

Vec3 CameraDegFromMat(Mat3 R) {
    float sx = CLAMP(-R.s12, -1.f, 1.f);
    float x  = asinf(sx);
    float y, z;
    if(fabsf(sx) < 0.9999f) {
        y = atan2(R.s02, R.s22);
        z = atan2(R.s10, R.s11);
    } else {
        y = atan2(-R.s20, R.s00);
        z = 0.f;
    }
    return (Vec3){x * RAD2DEG, y * RAD2DEG, z * RAD2DEG};
}

#endif // SCENE_IMPLEMENTED
#endif // SCENE_IMPLEMENTATION
