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
    bool showBoundingBox;
    bool showFaceNormals;
    bool showVertexNormals;
    Vec4 tint;
    int problemX, problemY;
    bool showBoundingBox;
    bool showFaceNormals;
    
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

#define RENDER_COLOR_WHITE   (Vec4){1.f, 1.f, 1.f, 1.f}
#define RENDER_COLOR_GREEN   (Vec4){0.f, 1.f, 0.f, 1.f}
#define RENDER_COLOR_YELLOW  (Vec4){1.f, 1.f, 0.f, 1.f}
#define RENDER_COLOR_PURPLE  (Vec4){1.f, 0.f, 1.f, 1.f}
#define RENDER_COLOR_RED     (Vec4){1.f, 0.f, 0.f, 1.f}

static void PlotPixel(Renderer *r, int x, int y, Vec4 color) {
    RenderSettings *s = r->settings;
    if (x < 0 || x >= s->width || y < 0 || y >= s->height) {
        return;
    }
    r->pixels[y*s->width + x] = color;
}

static void DrawLineBasic(Renderer *r, int x0, int y0, int x1, int y1, Vec4 color) {
    int dx = x1 - x0, dy = y1 - y0;
    int steps = MAX(ABS(dx), ABS(dy));
    if (steps == 0) {
        PlotPixel(r, x0, y0, color); return;
    }

    for (int i = 0; i <= steps; ++i) {
        float t = (float)i / (float)steps;
        PlotPixel(r, (int)(x0 + t*dx), (int)(y0 + t*dy), color);
    }
}

static void DrawLineBresenham(Renderer *r, int x0, int y0, int x1, int y1, Vec4 color) {
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
            PlotPixel(r, y, x, color);
        }
        else {
            PlotPixel(r, x, y, color);
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
    for (int c = 0; c < arrlen(scene->cameras); ++c) {
        if (c == scene->activeCameraIndex) {
            continue;
        }

        Vec3 p = scene->cameras[c].position;
        Vec4 q = M4MulV4(fromWorldToScreen, (Vec4){p.x, p.y, p.z, 1.f});
        if (q.w <= EPSILON || q.z < -q.w) {
            continue;
        }

        int cx = (int)((q.x / q.w + 1.f) / 2.f * (settings->width  - 1));
        int cy = (int)((q.y / q.w + 1.f) / 2.f * (settings->height - 1));

        DrawLineBresenham(renderer, cx - 10, cy, cx + 10, cy, RENDER_COLOR_RED);
        DrawLineBresenham(renderer, cx, cy - 10, cx, cy + 10, RENDER_COLOR_RED);
    }

    for (int modelIndex = 0; modelIndex < arrlen(scene->models); ++modelIndex) {
        Model model = scene->models[modelIndex];
        if (model.hidden) {
            TraceLog(LOG_INFO, "Model modelIndex==%d is hidden", modelIndex);
            continue;
        }

        if (model.vertexCount <= 0) {
            TraceLog(LOG_ERROR, "Model modelIndex==%d has no vertices!! model.vertexCount==%d",
                     modelIndex, model.vertexCount);
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
                DrawLineBresenham(renderer, px[i], py[i], px[j], py[j], RENDER_COLOR_WHITE);
            }
        }

        arrfree(px);
        arrfree(py);
        arrfree(ok);

        if (settings->showFaceNormals) {
            for (int t = 0; t < arrlen(model.triangles); ++t) {
                Triangle tri = model.triangles[t];
                Vec3 a = model.vertices[tri.vertices[0] - 1];
                Vec3 b = model.vertices[tri.vertices[1] - 1];
                Vec3 c = model.vertices[tri.vertices[2] - 1];
                Vec3 n = V3Normalize(V3Cross(V3Sub(b, a), V3Sub(c, a)));

                Vec3 ends[2];
                ends[0] = V3DivS(V3Add(V3Add(a, b), c), 3.f);
                ends[1] = V3Add(ends[0], V3MulS(n, 0.2f));

                int sx[2], sy[2];
                bool visible = true;
                for (int e = 0; e < 2; ++e) {
                    Vec4 q = M4MulV4(fromModelToScreen, (Vec4){ends[e].x, ends[e].y, ends[e].z, 1.f});
                    if (q.w <= EPSILON || q.z < -q.w) { visible = false; break; }
                    sx[e] = (int)((q.x / q.w + 1.f) / 2.f * (settings->width  - 1));
                    sy[e] = (int)((q.y / q.w + 1.f) / 2.f * (settings->height - 1));
                }
                if (visible) {
                    DrawLineBresenham(renderer, sx[0], sy[0], sx[1], sy[1], RENDER_COLOR_GREEN);
                }
            }
        }

        if (settings->showVertexNormals && arrlen(model.normals) > 0) {
            for (int t = 0; t < arrlen(model.triangles); ++t) {
                Triangle tri = model.triangles[t];
                for (int k = 0; k < 3; ++k) {
                    int vi = tri.vertices[k] - 1;
                    int ni = tri.normals[k] - 1;
                    if (vi < 0 || vi >= model.vertexCount || ni < 0 || ni >= arrlen(model.normals)) {
                        continue;
                    }

                    Vec3 ends[2];
                    ends[0] = model.vertices[vi];
                    ends[1] = V3Add(ends[0], V3MulS(model.normals[ni], 0.2f));

                    int sx[2], sy[2];
                    bool visible = true;
                    for (int e = 0; e < 2; ++e) {
                        Vec4 q = M4MulV4(fromModelToScreen, (Vec4){ends[e].x, ends[e].y, ends[e].z, 1.f});
                        if (q.w <= EPSILON || q.z < -q.w) { visible = false; break; }
                        sx[e] = (int)((q.x / q.w + 1.f) / 2.f * (settings->width  - 1));
                        sy[e] = (int)((q.y / q.w + 1.f) / 2.f * (settings->height - 1));
                    }
                    if (visible) {
                        DrawLineBresenham(renderer, sx[0], sy[0], sx[1], sy[1], RENDER_COLOR_YELLOW);
                    }
                        
                }
            }
        } else if (arrlen(model.normals) <= 0) {
            TraceLog(LOG_ERROR, "showVertexNormals can't be enabled: modelIndex == %d has %d vertex normals",
                     modelIndex, arrlen(model.normals));
        }


        if (settings->showBoundingBox) {
            Vec3 lo = model.vertices[0], hi = model.vertices[0];
            for (int i = 1; i < model.vertexCount; i++) {
                Vec3 v = model.vertices[i];
                lo.x = MIN(lo.x, v.x); lo.y = MIN(lo.y, v.y); lo.z = MIN(lo.z, v.z);
                hi.x = MAX(hi.x, v.x); hi.y = MAX(hi.y, v.y); hi.z = MAX(hi.z, v.z);
            }

            Vec3 box[2] = {lo, hi};
            int bx[8], by[8];
            bool bok[8];
            for (int i = 0; i < 2; i++) {
                for (int j = 0; j < 2; j++) {
                    for (int k = 0; k < 2; k++) {
                        int c = 4*i + 2*j + k;
                        Vec4 q = M4MulV4(fromModelToScreen, (Vec4){box[i].x, box[j].y, box[k].z, 1.f});
                        bok[c] = q.w > EPSILON && q.z >= -q.w;
                        bx[c] = (int)((q.x / q.w + 1.f) / 2.f * (settings->width  - 1));
                        by[c] = (int)((q.y / q.w + 1.f) / 2.f * (settings->height - 1));
                    }
                }
            }

            int edges[12][2] = {
                {0,1},{2,3},{4,5},{6,7},
                {0,2},{1,3},{4,6},{5,7},
                {0,4},{1,5},{2,6},{3,7}
            };
            for (int e = 0; e < 12; e++) {
                int a = edges[e][0], b = edges[e][1];
                if (bok[a] && bok[b]) {
                    DrawLineBresenham(renderer, bx[a], by[a], bx[b], by[b], RENDER_COLOR_PURPLE);
                }
            }
        }
    
    }
}

#endif // RENDERER_IMPLEMENTED
#endif // RENDERER_IMPLEMENTATION
