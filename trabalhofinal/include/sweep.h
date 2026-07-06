#ifndef SWEEP_H
#define SWEEP_H

#include "mesh.h"
#include "profile.h"
#include "mathutils.h"

/* ============================================================
 * sweep.h
 *
 * Núcleo do trabalho: gera uma Mesh 3D a partir de um perfil 2D
 * usando a técnica de sweep (varredura).
 *
 * Implementado agora:
 *   - sweep_generate_rotational(): superfície de revolução
 *     (gira o perfil em torno do eixo Y em 'steps' divisões).
 *   - sweep_generate_translational(): extrusão do perfil ao
 *     longo de um caminho 3D (path), usando referenciais locais
 *     transportados (parallel transport) para evitar torção.
 *   - Tampas (caps) nas duas pontas do sweep translacional.
 *
 * A FAZER (opcional):
 *   - Tampas no sweep rotacional (quando o perfil aberto não
 *     toca o eixo Y nas pontas, ex.: base do vaso).
 * ============================================================ */

/* Gera uma superfície de revolução a partir do perfil, com
 * 'steps' divisões angulares (quanto maior, mais suave o
 * arredondamento — e mais triângulos).
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

/* Gera um sólido por EXTRUSÃO: a mesma seção transversal 2D
 * (o perfil, tratado aqui como coordenadas locais x/y de uma
 * forma fechada, ex: um círculo) é repetida ao longo de um
 * caminho 3D (path_points), orientada por um referencial local
 * que "acompanha" a curva do caminho sem torcer.
 *
 * - profile: forma da seção transversal (normalmente fechada,
 *   ex: profile_generate_torus_ring(N, 0.0f, raio) gera um
 *   círculo simples centrado na origem — perfeito como seção
 *   de um tubo/lata).
 * - path_points: pontos 3D pelos quais o caminho passa (2 pontos
 *   = extrusão reta; mais pontos = caminho curvo).
 * - add_caps: se 1, fecha as duas pontas do sólido com um "disco"
 *   (necessário para a lata parecer sólida, não um tubo oco). */
Mesh sweep_generate_translational(const ProfilePoint *profile, int profile_count,
                                   const vec3 *path_points, int path_count,
                                   int add_caps);

#endif /* SWEEP_H */