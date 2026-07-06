#ifndef MESH_H
#define MESH_H

/* ============================================================
 * mesh.h
 *
 * Estrutura genérica de malha: vértices (posição + normal) e
 * índices de triângulos, mais os IDs de VAO/VBO/EBO do OpenGL.
 * Qualquer algoritmo (sweep rotacional, translacional, etc.)
 * só precisa preencher os arrays e chamar mesh_upload().
 * ============================================================ */

typedef struct {
    float px, py, pz; /* posição */
    float nx, ny, nz; /* normal  */
    float u, v;        /* coordenada de textura (0..1) */
} Vertex;

typedef struct {
    Vertex *vertices;
    int vertex_count;

    unsigned int *indices;
    int index_count;

    unsigned int vao, vbo, ebo;
} Mesh;

/* Envia os dados de 'vertices'/'indices' já preenchidos para a GPU
 * e configura os atributos do vertex shader (posição=0, normal=1, uv=2). */
void mesh_upload(Mesh *mesh);

void mesh_draw(const Mesh *mesh);

void mesh_free(Mesh *mesh); /* libera VAO/VBO/EBO e memória da CPU */

#endif /* MESH_H */