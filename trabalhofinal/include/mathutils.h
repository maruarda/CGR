#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <math.h>

/* ============================================================
 * mathutils.h
 *
 * Biblioteca mínima de álgebra linear para gráficos 3D.
 * Como o projeto é em C puro (não C++), não temos GLM disponível,
 * então implementamos aqui só o necessário: vec3, mat4 e as
 * operações clássicas do pipeline (translate, rotate, scale,
 * lookAt, perspective).
 *
 * Convenção: matrizes em column-major (igual ao OpenGL),
 * armazenadas como float[16].
 * ============================================================ */

typedef struct { float x, y, z; } vec3;

static inline vec3 vec3_make(float x, float y, float z) {
    vec3 v = { x, y, z };
    return v;
}

static inline vec3 vec3_sub(vec3 a, vec3 b) {
    return vec3_make(a.x - b.x, a.y - b.y, a.z - b.z);
}

static inline vec3 vec3_add(vec3 a, vec3 b) {
    return vec3_make(a.x + b.x, a.y + b.y, a.z + b.z);
}

static inline vec3 vec3_scale(vec3 a, float s) {
    return vec3_make(a.x * s, a.y * s, a.z * s);
}

static inline vec3 vec3_cross(vec3 a, vec3 b) {
    return vec3_make(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

static inline float vec3_dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float vec3_length(vec3 a) {
    return sqrtf(vec3_dot(a, a));
}

static inline vec3 vec3_normalize(vec3 a) {
    float len = vec3_length(a);
    if (len < 1e-6f) return vec3_make(0.0f, 0.0f, 0.0f);
    return vec3_scale(a, 1.0f / len);
}

/* ---------- Matriz 4x4 (column-major, float[16]) ---------- */

static inline void mat4_identity(float *m) {
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

/* out = a * b */
static inline void mat4_multiply(float *out, const float *a, const float *b) {
    float result[16];
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < 4; row++) {
            float sum = 0.0f;
            for (int k = 0; k < 4; k++) {
                sum += a[k * 4 + row] * b[col * 4 + k];
            }
            result[col * 4 + row] = sum;
        }
    }
    for (int i = 0; i < 16; i++) out[i] = result[i];
}

static inline void mat4_translate(float *m, vec3 t) {
    mat4_identity(m);
    m[12] = t.x;
    m[13] = t.y;
    m[14] = t.z;
}

static inline void mat4_scale(float *m, vec3 s) {
    mat4_identity(m);
    m[0] = s.x;
    m[5] = s.y;
    m[10] = s.z;
}

static inline void mat4_rotate_y(float *m, float radians) {
    mat4_identity(m);
    float c = cosf(radians), s = sinf(radians);
    m[0] = c;  m[8]  = s;
    m[2] = -s; m[10] = c;
}

/* Câmera "olhando de eye para center", com vetor up de referência */
static inline void mat4_lookat(float *m, vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_normalize(vec3_sub(center, eye));
    vec3 s = vec3_normalize(vec3_cross(f, up));
    vec3 u = vec3_cross(s, f);

    mat4_identity(m);
    m[0] = s.x; m[4] = s.y; m[8]  = s.z;
    m[1] = u.x; m[5] = u.y; m[9]  = u.z;
    m[2] = -f.x; m[6] = -f.y; m[10] = -f.z;
    m[12] = -vec3_dot(s, eye);
    m[13] = -vec3_dot(u, eye);
    m[14] =  vec3_dot(f, eye);
}

/* Projeção perspectiva clássica (fovy em radianos) */
static inline void mat4_perspective(float *m, float fovy, float aspect, float znear, float zfar) {
    for (int i = 0; i < 16; i++) m[i] = 0.0f;
    float tanHalfFovy = tanf(fovy / 2.0f);
    m[0] = 1.0f / (aspect * tanHalfFovy);
    m[5] = 1.0f / tanHalfFovy;
    m[10] = -(zfar + znear) / (zfar - znear);
    m[11] = -1.0f;
    m[14] = -(2.0f * zfar * znear) / (zfar - znear);
}

#endif /* MATHUTILS_H */