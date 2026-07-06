#include "profile.h"
#include <stddef.h>

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