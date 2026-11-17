#if !defined(RENDERER_HEADER)
#define RENDERER_HEADER

#include "scene.h"
#include "math.h"

typedef enum {
    RENDERING_METHOD_SOFTWARE   = 0, // HW1-2
    RENDERING_METHOD_OPENGL     = 1, // HW3
    RENDERING_METHOD_RAYTRACING = 2, // HW4
    
    RENDERING_METHOD_COUNT,
} RenderingMethod;

// RenderSettings is pointed by the renderer and not stored on it directly,
// so we can share it between different renderers.
typedef struct RenderSettings {
    int width, height;
    
    bool showDebugLayer;
    Vec4 tint;
    int problemX, problemY;
    
    RenderingMethod renderingMethod;
} RenderSettings;

typedef struct Renderer {
    RenderSettings *settings;
    Vec4 *pixels;
} Renderer;

void RenderScene(Scene *scene, Renderer *renderer, double time);

#endif // RENDERER_HEADER


// Renderer Implementation
#if defined(RENDERER_IMPLEMENTATION)
#if !defined(RENDERER_IMPLEMENTED)
#define RENDERER_IMPLEMENTED

#include "stb_ds.h" // For dynamic arrays

static void PlotPixel(Renderer *r, int x, int y) {
    RenderSettings *s = r->settings;
    if (x < 0 || x >= s->width || y < 0 || y >= s->height) return;
    r->pixels[y*s->width + x] = (Vec4){1.f, 1.f, 1.f, 1.f};
}

static void DrawLineBasic(Renderer *r, int x0, int y0, int x1, int y1) {
    int dx = x1 - x0, dy = y1 - y0;
    int steps = MAX(ABS(dx), ABS(dy));
    if (steps == 0) {
        PlotPixel(r, x0, y0); return;
    }

    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / (float)steps;
        PlotPixel(r, (int)(x0 + t*dx), (int)(y0 + t*dy));
    }
}

static void DrawLineBresenham(Renderer *r, int x0, int y0, int x1, int y1) {
    int t;
    bool steep = ABS(y1 - y0) > ABS(x1 - x0);
    if (steep) { // when 0 < slope < 1
        t = x0;
        x0 = y0;
        y0 = t;
        t = x1;
        x1 = y1;
        y1 = t;
    }
    
    if (x0 > x1) { // always go from left to righ
        t = x0;
        x0 = x1;
        x1 = t;
        t = y0;
        y0 = y1;
        y1 = t;
    }

    int dx = x1 - x0;
    int dy = ABS(y1 - y0);
    int yStep = (y0 < y1) ? 1 : -1;

    int d = 2*dy - dx;
    int y = y0;
    for (int x = x0; x <= x1; ++x) {
        if (steep) {
            PlotPixel(r, y, x);
        }
        else {   
            PlotPixel(r, x, y);
        }

        if (d < 0) {
            d += 2*dy;
        } else {
            y += yStep;
            d += 2*dy - 2*dx;
        }
    }
}

void RenderScene(Scene *scene, Renderer *renderer, double time) {
    // Note that 'TextFormat' uses global storage for the strings it returns, so don't save pointers
    // to those directly if you need to store them long-term, copy the result into a persistant buffer.
    DrawText(TextFormat("Drawing scene with %d models", arrlen(scene->models)), 400, 100, 10, BLACK);    
    RenderSettings *settings = renderer->settings;

    // Clear the frame (this has to happen before drawing anything into the buffer)
    for (int y = 0; y < settings->height; ++y) {
        for (int x = 0; x < settings->width; ++x) {
            Vec4 *pixel = &renderer->pixels[y*settings->width + x];
            *pixel = (Vec4){0};
        }
    }


    Camera cam = scene->cameras[scene->activeCameraIndex];
    cam.aspect = (float)settings->width / (float)settings->height;

    Mat4 fromWorldToCamera  = GetView(&cam);
    Mat4 fromCameraToScreen = GetProjection(&cam);
    Mat4 fromWorldToScreen  = M4MulM4(fromCameraToScreen, fromWorldToCamera);


    for (int modelIndex = 0; modelIndex < arrlen(scene->models); ++modelIndex) {
        Model model = scene->models[modelIndex];
        if (model.hidden) {   
            continue;
        }
        Mat4 fromModelToWorld  = GetModelMatrix(&model);
        Mat4 fromModelToScreen = M4MulM4(fromWorldToScreen, fromModelToWorld);

        // temp buffers because triangles may reference already computed pixels
        int *px = NULL, *py = NULL;
        bool *ok = NULL;                  // behind the camera
        arrsetlen(px, model.vertexCount);
        arrsetlen(py, model.vertexCount);
        arrsetlen(ok, model.vertexCount);

        for (int i = 0; i < model.vertexCount; i++) {
            Vec3 v = model.vertices[i];
            Vec4 point = M4MulV4(fromModelToScreen, (Vec4){v.x, v.y, v.z, 1.f});
            ok[i] = point.w > EPSILON && point.z >= -point.w;
            if (!ok[i]) continue;

            px[i] = (int)((point.x / point.w + 1.f) / 2.f * (settings->width  - 1));
            py[i] = (int)((point.y / point.w + 1.f) / 2.f * (settings->height - 1));
        }

        for (int t = 0; t < arrlen(model.triangles); ++t) {
            Triangle tri = model.triangles[t];
            for (int k = 0; k < 3; ++k) {
                int i = tri.vertices[k] - 1; // 1-based indices
                int j = tri.vertices[(k + 1) % 3] - 1;
                if (i < 0 || j < 0 || i >= model.vertexCount || j >= model.vertexCount) {
                    continue;
                }
                if (!ok[i] || !ok[j]) {
                    continue;
                }
                DrawLineBresenham(renderer, px[i], py[i], px[j], py[j]);
            }
        }

        arrfree(px);
        arrfree(py);
        arrfree(ok);
    }
}

#endif // RENDERER_IMPLEMENTED
#endif // RENDERER_IMPLEMENTATION
