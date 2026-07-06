#ifndef SWEEP_H
#define SWEEP_H

#include "mesh.h"
#include "profile.h"

/* ============================================================
 * sweep.h
 *
 * Núcleo do trabalho: gera uma Mesh 3D a partir de um perfil 2D
 * usando a técnica de sweep (varredura).
 *
 * Implementado agora:
 *   - sweep_generate_rotational(): superfície de revolução
 *     (gira o perfil em torno do eixo Y em 'steps' divisões).
 *
 * A FAZER (próxima etapa do trabalho):
 *   - sweep_generate_translational(): extrusão do perfil ao
 *     longo de um caminho (reto, ou uma polyline/curva).
 *   - Tampas (caps) no topo/base do sólido de revolução, caso
 *     o perfil não feche naturalmente no eixo (radius = 0).
 * ============================================================ */

/* Gera uma superfície de revolução a partir do perfil, com
 * 'steps' divisões angulares (quanto maior, mais suave o
 * arredondamento — e mais triângulos). */
Mesh sweep_generate_rotational(const ProfilePoint *profile, int profile_count, int steps);

#endif /* SWEEP_H */