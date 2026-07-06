#ifndef PROFILE_H
#define PROFILE_H

/* ============================================================
 * profile.h
 *
 * O "perfil" é a curva 2D que será varrida (sweep) para gerar
 * o sólido 3D. Cada ponto é (raio, altura): raio = distância
 * ao eixo de rotação, altura = posição no eixo Y.
 *
 * Por enquanto os pontos são fixos (hardcoded), representando
 * a silhueta de um vaso/cálice. Como próximo passo, dá pra:
 *   - carregar de um arquivo texto
 *   - deixar o usuário desenhar clicando na tela
 *   - suavizar com uma curva Catmull-Rom / Bézier em vez de
 *     segmentos retos entre os pontos de controle
 * ============================================================ */

typedef struct {
    float radius; /* distância ao eixo de rotação (eixo Y) */
    float height; /* posição ao longo do eixo Y              */
} ProfilePoint;

/* Retorna um perfil de exemplo (silhueta de vaso) e preenche
 * out_count com o número de pontos. */
const ProfilePoint *profile_get_vase(int *out_count);

#endif /* PROFILE_H */