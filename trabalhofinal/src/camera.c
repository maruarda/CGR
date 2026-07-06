#include "camera.h"

void camera_init(Camera *cam) {
    cam->azimuth = 0.0f;
    cam->elevation = 0.4f;
    cam->radius = 6.0f;
    cam->target = vec3_make(0.0f, 0.0f, 0.0f);
}

vec3 camera_get_eye(const Camera *cam) {
    /* Coordenadas esféricas -> cartesianas.
     * elevation limitada para não "virar de cabeça pra baixo". */
    float x = cam->radius * cosf(cam->elevation) * sinf(cam->azimuth);
    float y = cam->radius * sinf(cam->elevation);
    float z = cam->radius * cosf(cam->elevation) * cosf(cam->azimuth);
    return vec3_add(cam->target, vec3_make(x, y, z));
}

void camera_get_view_matrix(const Camera *cam, float *out_view) {
    vec3 eye = camera_get_eye(cam);
    vec3 up = vec3_make(0.0f, 1.0f, 0.0f);
    mat4_lookat(out_view, eye, cam->target, up);
}

void camera_process_drag(Camera *cam, float dx, float dy) {
    const float sensitivity = 0.005f;
    cam->azimuth   += dx * sensitivity;
    cam->elevation += dy * sensitivity;

    /* trava para evitar gimbal lock / câmera invertida */
    const float limit = 1.5f; /* ~85 graus */
    if (cam->elevation > limit) cam->elevation = limit;
    if (cam->elevation < -limit) cam->elevation = -limit;
}

void camera_process_scroll(Camera *cam, float dy) {
    cam->radius -= dy * 0.5f;
    if (cam->radius < 1.5f) cam->radius = 1.5f;
    if (cam->radius > 30.0f) cam->radius = 30.0f;
}