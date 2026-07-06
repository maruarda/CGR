#include "sweep.h"
#include "mathutils.h"
#include <stdlib.h>
#include <math.h>

/* Índice do vértice do "anel" i (divisão angular), ponto j do perfil,
 * dentro do array linear de vértices. */
static int vertex_index(int ring, int point, int profile_count) {
    return ring * profile_count + point;
}

/* Calcula a normal 2D (no plano raio-altura) da curva do perfil no
 * ponto 'j', usando diferença central (ou forward/backward nas pontas).
 * A normal da curva 2D é a tangente rotacionada 90 graus. */
static void profile_normal_2d(const ProfilePoint *profile, int count, int j,
                               float *out_nr, float *out_ny) {
    float dr, dy;

    if (j == 0) {
        dr = profile[1].radius - profile[0].radius;
        dy = profile[1].height - profile[0].height;
    } else if (j == count - 1) {
        dr = profile[j].radius - profile[j - 1].radius;
        dy = profile[j].height - profile[j - 1].height;
    } else {
        dr = profile[j + 1].radius - profile[j - 1].radius;
        dy = profile[j + 1].height - profile[j - 1].height;
    }

    /* tangente = (dr, dy); normal (apontando para fora) = (dy, -dr) */
    float nr = dy;
    float ny = -dr;
    float len = sqrtf(nr * nr + ny * ny);
    if (len < 1e-6f) { nr = 1.0f; ny = 0.0f; len = 1.0f; }

    *out_nr = nr / len;
    *out_ny = ny / len;
}

Mesh sweep_generate_rotational(const ProfilePoint *profile, int profile_count, int steps) {
    Mesh mesh = {0};

    mesh.vertex_count = steps * profile_count;
    mesh.vertices = malloc(mesh.vertex_count * sizeof(Vertex));

    /* --- Passo 1: gerar os vértices, anel por anel ---
     * Cada "anel" é o perfil inteiro rotacionado por um ângulo.
     * Não duplicamos o anel final (ring == steps) porque ele
     * fecha exatamente sobre o anel 0 — conectamos via módulo
     * na etapa de índices abaixo. */
    for (int ring = 0; ring < steps; ring++) {
        float angle = ring * (2.0f * (float)M_PI / (float)steps);
        float s = sinf(angle);
        float c = cosf(angle);

        for (int j = 0; j < profile_count; j++) {
            int idx = vertex_index(ring, j, profile_count);

            float r = profile[j].radius;
            float y = profile[j].height;

            /* posição: gira o ponto (r, y) do perfil em torno do eixo Y */
            mesh.vertices[idx].px = r * s;
            mesh.vertices[idx].py = y;
            mesh.vertices[idx].pz = r * c;

            /* normal: pega a normal 2D da curva no plano (r,y) e aplica
             * a mesma rotação angular à componente radial */
            float nr, ny;
            profile_normal_2d(profile, profile_count, j, &nr, &ny);
            mesh.vertices[idx].nx = nr * s;
            mesh.vertices[idx].ny = ny;
            mesh.vertices[idx].nz = nr * c;
        }
    }

    /* --- Passo 2: gerar os índices (2 triângulos por "quad" da malha) --- */
    mesh.index_count = steps * (profile_count - 1) * 6;
    mesh.indices = malloc(mesh.index_count * sizeof(unsigned int));

    int k = 0;
    for (int ring = 0; ring < steps; ring++) {
        int next_ring = (ring + 1) % steps; /* fecha o "cilindro" */

        for (int j = 0; j < profile_count - 1; j++) {
            unsigned int a = vertex_index(ring, j, profile_count);
            unsigned int b = vertex_index(ring, j + 1, profile_count);
            unsigned int c = vertex_index(next_ring, j, profile_count);
            unsigned int d = vertex_index(next_ring, j + 1, profile_count);

            /* triângulo 1: a, c, b */
            mesh.indices[k++] = a;
            mesh.indices[k++] = c;
            mesh.indices[k++] = b;

            /* triângulo 2: b, c, d */
            mesh.indices[k++] = b;
            mesh.indices[k++] = c;
            mesh.indices[k++] = d;
        }
    }

    mesh_upload(&mesh);
    return mesh;
}

/* ============================================================
 * TODO (próxima etapa do trabalho): sweep_generate_translational()
 *
 * Ideia geral para implementar depois:
 *   1. Definir um "caminho" (path) como lista de pontos 3D
 *      (reto: apenas 2 pontos; curvo: uma polyline ou spline
 *      Catmull-Rom com vários pontos de controle).
 *   2. Para cada ponto do caminho, calcular um referencial local
 *      (tangente, normal, binormal) — os "frames" de Frenet, ou
 *      o método mais simples de "parallel transport" para evitar
 *      torções.
 *   3. Em cada ponto do caminho, copiar o perfil 2D transformado
 *      pelo referencial local (como uma seção transversal).
 *   4. Conectar seções consecutivas exatamente como fizemos aqui
 *      entre anéis (mesma lógica de índices), mas sem fechar em
 *      loop no final (a menos que o caminho seja fechado).
 * ============================================================ */