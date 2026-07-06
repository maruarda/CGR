#ifndef SHADER_H
#define SHADER_H

/* ============================================================
 * shader.h
 *
 * Carrega, compila e linka um par de shaders (vertex + fragment)
 * a partir de arquivos .glsl. Retorna o ID do programa OpenGL
 * pronto para uso com glUseProgram().
 * ============================================================ */

unsigned int shader_load(const char *vertex_path, const char *fragment_path);

/* Helpers para setar uniforms sem precisar chamar glGetUniformLocation
 * toda hora no main.c */
void shader_set_mat4(unsigned int program, const char *name, const float *matrix);
void shader_set_vec3(unsigned int program, const char *name, float x, float y, float z);
void shader_set_float(unsigned int program, const char *name, float value);
void shader_set_int(unsigned int program, const char *name, int value);

#endif /* SHADER_H */