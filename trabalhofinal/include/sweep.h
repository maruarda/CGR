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
 * ============================================================ */

/* Gera uma superfície de revolução a partir do perfil, com
 * 'steps' divisões angulares (quanto maior, mais suave o
 * arredondamento e mais triângulos).
 *
 * 'profile_closed': o perfil é um segmento aberto (vaso, ponta
 * a ponta) ou um laço fechado (círculo)?
 *   - 0 (aberto):  usado para perfis tipo vaso/garrafa, onde o
 *     primeiro e o último ponto NÃO se conectam entre si.
 *   - 1 (fechado): usado para o torus (rosquinha) e qualquer
 *     perfil que seja um laço fechado (ex: círculo, elipse) —
 *     o último ponto se conecta de volta ao primeiro, fechando
 *     o "tubo" também nessa direção (além da direção da rotação,
 *     que já fecha sempre). */
Mesh sweep_generate_rotational(const ProfilePoint *profile, int profile_count, int steps, int profile_closed);

#endif /* SWEEP_H */