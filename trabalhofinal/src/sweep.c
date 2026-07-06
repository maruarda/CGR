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
                               int closed, float *out_nr, float *out_ny) {
    float dr, dy;

    if (closed) {
        /* laço fechado: os "vizinhos" de qualquer ponto, incluindo as
         * pontas, sempre existem via módulo (wraparound). */
        int prev = (j - 1 + count) % count;
        int next = (j + 1) % count;
        dr = profile[next].radius - profile[prev].radius;
        dy = profile[next].height - profile[prev].height;
    } else if (j == 0) {
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

Mesh sweep_generate_rotational(const ProfilePoint *profile, int profile_count, int steps, int profile_closed) {
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
            profile_normal_2d(profile, profile_count, j, profile_closed, &nr, &ny);
            mesh.vertices[idx].nx = nr * s;
            mesh.vertices[idx].ny = ny;
            mesh.vertices[idx].nz = nr * c;

            /* UV: u percorre a volta grande (0..1), v percorre o perfil.
             * Se o perfil é fechado (torus), v também dá uma volta completa
             * (por isso dividimos por profile_count, sem o -1). */
            mesh.vertices[idx].u = (float)ring / (float)steps;
            mesh.vertices[idx].v = profile_closed
                ? (float)j / (float)profile_count
                : (float)j / (float)(profile_count - 1);
        }
    }

    /* --- Passo 2: gerar os índices (2 triângulos por "quad" da malha) ---
     * 'segments' é quantos "degraus" existem na direção do perfil: se o
     * perfil é aberto (vaso), são profile_count - 1 (não fecha nas pontas);
     * se é fechado (torus), são profile_count (o último ponto conecta de
     * volta ao primeiro). */
    int segments = profile_closed ? profile_count : (profile_count - 1);
    mesh.index_count = steps * segments * 6;
    mesh.indices = malloc(mesh.index_count * sizeof(unsigned int));

    int k = 0;
    for (int ring = 0; ring < steps; ring++) {
        int next_ring = (ring + 1) % steps; /* fecha o "cilindro" */

        for (int seg = 0; seg < segments; seg++) {
            int j = seg;
            int j_next = profile_closed ? (seg + 1) % profile_count : seg + 1;

            unsigned int a = vertex_index(ring, j, profile_count);
            unsigned int b = vertex_index(ring, j_next, profile_count);
            unsigned int c = vertex_index(next_ring, j, profile_count);
            unsigned int d = vertex_index(next_ring, j_next, profile_count);

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
 * SWEEP TRANSLACIONAL (extrusão ao longo de um caminho)
 * ============================================================ */

/* Rotaciona o vetor 'v' em torno do eixo unitário 'axis' por 'angle'
 * radianos, usando a fórmula de Rodrigues. Usado para "transportar"
 * o referencial local de um ponto do caminho para o próximo, sem
 * torcer a seção transversal. */
static vec3 rotate_around_axis(vec3 v, vec3 axis, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    vec3 term1 = vec3_scale(v, c);
    vec3 term2 = vec3_scale(vec3_cross(axis, v), s);
    vec3 term3 = vec3_scale(axis, vec3_dot(axis, v) * (1.0f - c));
    return vec3_add(vec3_add(term1, term2), term3);
}

/* Calcula, para cada ponto do caminho, um referencial local
 * ortonormal (tangent, normal, binormal) via "parallel transport":
 * a cada passo, gira o referencial anterior pela mesma rotação que
 * levou a tangente anterior até a nova tangente. Isso evita a torção
 * que apareceria se recalculássemos a normal do zero a cada ponto. */
static void compute_path_frames(const vec3 *path, int count,
                                 vec3 *out_tangent, vec3 *out_normal, vec3 *out_binormal) {
    for (int i = 0; i < count; i++) {
        if (i == 0) {
            out_tangent[i] = vec3_normalize(vec3_sub(path[1], path[0]));
        } else if (i == count - 1) {
            out_tangent[i] = vec3_normalize(vec3_sub(path[i], path[i - 1]));
        } else {
            out_tangent[i] = vec3_normalize(vec3_sub(path[i + 1], path[i - 1]));
        }
    }

    /* referencial inicial: escolhe um "up" de referência que não seja
     * quase-paralelo à tangente, para evitar produto vetorial degenerado */
    vec3 up = (fabsf(out_tangent[0].y) > 0.99f) ? vec3_make(1, 0, 0) : vec3_make(0, 1, 0);
    out_normal[0] = vec3_normalize(vec3_cross(up, out_tangent[0]));
    out_binormal[0] = vec3_cross(out_tangent[0], out_normal[0]);

    for (int i = 1; i < count; i++) {
        vec3 axis = vec3_cross(out_tangent[i - 1], out_tangent[i]);
        float axis_len = vec3_length(axis);

        if (axis_len < 1e-6f) {
            /* tangentes praticamente iguais (caminho reto): sem rotação */
            out_normal[i] = out_normal[i - 1];
        } else {
            axis = vec3_scale(axis, 1.0f / axis_len);
            float cos_angle = vec3_dot(out_tangent[i - 1], out_tangent[i]);
            if (cos_angle > 1.0f) cos_angle = 1.0f;
            if (cos_angle < -1.0f) cos_angle = -1.0f;
            float angle = acosf(cos_angle);
            out_normal[i] = rotate_around_axis(out_normal[i - 1], axis, angle);
        }
        out_binormal[i] = vec3_cross(out_tangent[i], out_normal[i]);
    }
}

/* Adiciona um "disco" (tampa) preenchendo o anel de índice 'ring',
 * usado para fechar as pontas do tubo. 'cap_normal' aponta para fora
 * do sólido (sentido oposto ao caminho, na ponta inicial; mesmo
 * sentido do caminho, na ponta final). */
static void add_disc_cap(Mesh *mesh, int *vertex_write, int *index_write,
                          const Vertex *ring_vertices, int profile_count,
                          vec3 center, vec3 cap_normal, int flip_winding) {
    int center_idx = *vertex_write;
    mesh->vertices[center_idx].px = center.x;
    mesh->vertices[center_idx].py = center.y;
    mesh->vertices[center_idx].pz = center.z;
    mesh->vertices[center_idx].nx = cap_normal.x;
    mesh->vertices[center_idx].ny = cap_normal.y;
    mesh->vertices[center_idx].nz = cap_normal.z;
    mesh->vertices[center_idx].u = 0.5f;
    mesh->vertices[center_idx].v = 0.5f;
    (*vertex_write)++;

    int rim_start = *vertex_write;
    for (int j = 0; j < profile_count; j++) {
        int idx = *vertex_write;
        mesh->vertices[idx].px = ring_vertices[j].px;
        mesh->vertices[idx].py = ring_vertices[j].py;
        mesh->vertices[idx].pz = ring_vertices[j].pz;
        /* normal da tampa é plana (mesma direção do caminho), não a
         * normal radial que o vértice tinha no corpo do tubo */
        mesh->vertices[idx].nx = cap_normal.x;
        mesh->vertices[idx].ny = cap_normal.y;
        mesh->vertices[idx].nz = cap_normal.z;
        /* UV simples em disco, usando as próprias coordenadas locais
         * do perfil (que já estão centradas em 0) */
        mesh->vertices[idx].u = 0.5f + (ring_vertices[j].px * 0.5f);
        mesh->vertices[idx].v = 0.5f + (ring_vertices[j].pz * 0.5f);
        (*vertex_write)++;
    }

    for (int j = 0; j < profile_count; j++) {
        int j_next = (j + 1) % profile_count;
        unsigned int a = (unsigned int)center_idx;
        unsigned int b = (unsigned int)(rim_start + j);
        unsigned int c = (unsigned int)(rim_start + j_next);

        if (!flip_winding) {
            mesh->indices[(*index_write)++] = a;
            mesh->indices[(*index_write)++] = b;
            mesh->indices[(*index_write)++] = c;
        } else {
            mesh->indices[(*index_write)++] = a;
            mesh->indices[(*index_write)++] = c;
            mesh->indices[(*index_write)++] = b;
        }
    }
}

Mesh sweep_generate_translational(const ProfilePoint *profile, int profile_count,
                                   const vec3 *path_points, int path_count,
                                   int add_caps) {
    Mesh mesh = {0};

    /* --- Passo 1: referenciais locais ao longo do caminho --- */
    vec3 *tangent = malloc(path_count * sizeof(vec3));
    vec3 *normal  = malloc(path_count * sizeof(vec3));
    vec3 *binormal = malloc(path_count * sizeof(vec3));
    compute_path_frames(path_points, path_count, tangent, normal, binormal);

    /* --- Passo 2: aloca vértices/índices (corpo do tubo + tampas) --- */
    int body_vertex_count = path_count * profile_count;
    int cap_vertex_count = add_caps ? 2 * (profile_count + 1) : 0;
    mesh.vertex_count = body_vertex_count + cap_vertex_count;
    mesh.vertices = malloc(mesh.vertex_count * sizeof(Vertex));

    int body_index_count = (path_count - 1) * profile_count * 6;
    int cap_index_count = add_caps ? 2 * profile_count * 3 : 0;
    mesh.index_count = body_index_count + cap_index_count;
    mesh.indices = malloc(mesh.index_count * sizeof(unsigned int));

    /* --- Passo 3: gera o corpo do tubo, seção por seção ---
     * O perfil (seção transversal) é tratado como um laço fechado
     * (ex: um círculo) — por isso profile_closed = 1 aqui sempre. */
    for (int i = 0; i < path_count; i++) {
        for (int j = 0; j < profile_count; j++) {
            int idx = vertex_index(i, j, profile_count);

            float local_x = profile[j].radius; /* coordenada local no referencial */
            float local_y = profile[j].height;

            vec3 offset = vec3_add(vec3_scale(normal[i], local_x),
                                    vec3_scale(binormal[i], local_y));
            vec3 world_pos = vec3_add(path_points[i], offset);

            mesh.vertices[idx].px = world_pos.x;
            mesh.vertices[idx].py = world_pos.y;
            mesh.vertices[idx].pz = world_pos.z;

            float nr, ny;
            profile_normal_2d(profile, profile_count, j, /*closed=*/1, &nr, &ny);
            vec3 world_normal = vec3_normalize(vec3_add(vec3_scale(normal[i], nr),
                                                         vec3_scale(binormal[i], ny)));
            mesh.vertices[idx].nx = world_normal.x;
            mesh.vertices[idx].ny = world_normal.y;
            mesh.vertices[idx].nz = world_normal.z;

            /* UV: u dá a volta na seção transversal (circunferência),
             * v acompanha a posição ao longo do caminho (altura) —
             * mapeamento pensado para rótulos tipo lata de refrigerante */
            mesh.vertices[idx].u = (float)j / (float)profile_count;
            mesh.vertices[idx].v = (float)i / (float)(path_count - 1);
        }
    }

    int k = 0;
    for (int i = 0; i < path_count - 1; i++) {
        for (int j = 0; j < profile_count; j++) {
            int j_next = (j + 1) % profile_count;

            unsigned int a = vertex_index(i, j, profile_count);
            unsigned int b = vertex_index(i, j_next, profile_count);
            unsigned int c = vertex_index(i + 1, j, profile_count);
            unsigned int d = vertex_index(i + 1, j_next, profile_count);

            mesh.indices[k++] = a;
            mesh.indices[k++] = c;
            mesh.indices[k++] = b;

            mesh.indices[k++] = b;
            mesh.indices[k++] = c;
            mesh.indices[k++] = d;
        }
    }

    /* --- Passo 4: tampas nas duas pontas (se pedido) --- */
    if (add_caps) {
        int vertex_write = body_vertex_count;
        int index_write = body_index_count;

        /* tampa inicial: normal aponta para TRÁS (sentido oposto ao caminho) */
        vec3 start_normal = vec3_scale(tangent[0], -1.0f);
        add_disc_cap(&mesh, &vertex_write, &index_write,
                     &mesh.vertices[vertex_index(0, 0, profile_count)], profile_count,
                     path_points[0], start_normal, /*flip_winding=*/1);

        /* tampa final: normal aponta para a FRENTE (mesmo sentido do caminho) */
        vec3 end_normal = tangent[path_count - 1];
        add_disc_cap(&mesh, &vertex_write, &index_write,
                     &mesh.vertices[vertex_index(path_count - 1, 0, profile_count)], profile_count,
                     path_points[path_count - 1], end_normal, /*flip_winding=*/0);
    }

    free(tangent);
    free(normal);
    free(binormal);

    mesh_upload(&mesh);
    return mesh;
}