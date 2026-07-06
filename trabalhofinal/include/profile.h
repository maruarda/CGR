#ifndef PROFILE_H
#define PROFILE_H

/* ============================================================
 * profile.h
 *
 * O "perfil" é a curva 2D que será varrida (sweep) para gerar
 * o sólido 3D. Cada ponto é (raio, altura): raio = distância
 * ao eixo de rotação, altura = posição no eixo Y.
 *
 * ============================================================ */

typedef struct {
    float radius; /* distância ao eixo de rotação (eixo Y) */
    float height; /* posição ao longo do eixo Y              */
} ProfilePoint;

/* Retorna um perfil de exemplo (silhueta de vaso) e preenche
 * out_count com o número de pontos. Este perfil é "aberto"
 * (vai da base ao topo) — use profile_closed = 0 no sweep. */
const ProfilePoint *profile_get_vase(int *out_count);

/* Gera um perfil circular fechado (um pequeno círculo deslocado
 * do eixo de rotação), usado para criar um torus/rosquinha ao
 * ser varrido. 'major_radius' é a distância do centro do tubo
 * até o eixo Y (raio do "buraco" do donut); 'minor_radius' é o
 * raio do próprio tubo (espessura do donut).
 * Retorna um array ALOCADO DINAMICAMENTE (malloc) — quem chamar
 * é responsável por dar free() nele depois de usar. Use
 * profile_closed = 1 no sweep, já que este perfil é uma volta
 * fechada. */
ProfilePoint *profile_generate_torus_ring(int subdivisions, float major_radius, float minor_radius);

#endif /* PROFILE_H */