#include "profile.h"
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

/* Perfil de exemplo: silhueta de um vaso, de baixo para cima.
 * (raio, altura) — ajuste esses valores à vontade para testar
 * outras formas (garrafa, cálice, engrenagem simplificada etc). */
static const ProfilePoint vase_points[] = {
    { 0.00f, 0.00f },
    { 0.55f, 0.05f },
    { 0.70f, 0.35f },
    { 0.45f, 0.75f },
    { 0.30f, 1.10f },
    { 0.40f, 1.45f },
    { 0.65f, 1.60f },
    { 0.60f, 1.90f },
    { 0.35f, 2.00f },
};

const ProfilePoint *profile_get_vase(int *out_count) {
    *out_count = (int)(sizeof(vase_points) / sizeof(vase_points[0]));
    return vase_points;
}

ProfilePoint *profile_generate_torus_ring(int subdivisions, float major_radius, float minor_radius) {
    ProfilePoint *points = malloc(subdivisions * sizeof(ProfilePoint));

    /* Percorre um círculo completo (0 a 2*PI) no plano (raio, altura):
     * cada ponto fica a 'minor_radius' de distância do centro do tubo,
     * que por sua vez está a 'major_radius' do eixo Y. */
    for (int i = 0; i < subdivisions; i++) {
        float t = i * (2.0f * (float)M_PI / (float)subdivisions);
        points[i].radius = major_radius + minor_radius * cosf(t);
        points[i].height = minor_radius * sinf(t);
    }

    return points;
}
