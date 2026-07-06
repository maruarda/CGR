#ifndef CAMERA_H
#define CAMERA_H

#include "mathutils.h"

/* ============================================================
 * camera.h
 *
 * Câmera orbital (arcball simplificada): posição descrita em
 * coordenadas esféricas ao redor de um alvo (o objeto do sweep
 * fica sempre centralizado). Controlada por arraste do mouse
 * (azimute/elevação) e scroll (zoom/raio).
 * ============================================================ */

typedef struct {
    float azimuth;    /* rotação horizontal, radianos   */
    float elevation;  /* rotação vertical, radianos      */
    float radius;     /* distância até o alvo             */
    vec3 target;      /* ponto observado (origem, normalmente) */
} Camera;

void camera_init(Camera *cam);
vec3 camera_get_eye(const Camera *cam);
void camera_get_view_matrix(const Camera *cam, float *out_view);

/* Chamar a partir dos callbacks de mouse do GLFW */
void camera_process_drag(Camera *cam, float dx, float dy);
void camera_process_scroll(Camera *cam, float dy);

#endif /* CAMERA_H */