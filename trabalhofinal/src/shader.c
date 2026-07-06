#include "shader.h"
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>

/* Lê um arquivo inteiro para uma string alocada dinamicamente.
 * Quem chamar é responsável por dar free() depois. */
static char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "[shader] Nao foi possivel abrir: %s\n", path);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);
    return buffer;
}

static unsigned int compile_stage(const char *source, GLenum stage_type, const char *label) {
    unsigned int id = glCreateShader(stage_type);
    glShaderSource(id, 1, &source, NULL);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetShaderInfoLog(id, 1024, NULL, log);
        fprintf(stderr, "[shader] Erro compilando %s:\n%s\n", label, log);
    }
    return id;
}

unsigned int shader_load(const char *vertex_path, const char *fragment_path) {
    char *vsrc = read_file(vertex_path);
    char *fsrc = read_file(fragment_path);
    if (!vsrc || !fsrc) {
        free(vsrc);
        free(fsrc);
        return 0;
    }

    unsigned int vs = compile_stage(vsrc, GL_VERTEX_SHADER, "vertex");
    unsigned int fs = compile_stage(fsrc, GL_FRAGMENT_SHADER, "fragment");

    unsigned int program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[1024];
        glGetProgramInfoLog(program, 1024, NULL, log);
        fprintf(stderr, "[shader] Erro linkando programa:\n%s\n", log);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    free(vsrc);
    free(fsrc);
    return program;
}

void shader_set_mat4(unsigned int program, const char *name, const float *matrix) {
    int loc = glGetUniformLocation(program, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, matrix);
}

void shader_set_vec3(unsigned int program, const char *name, float x, float y, float z) {
    int loc = glGetUniformLocation(program, name);
    glUniform3f(loc, x, y, z);
}

void shader_set_float(unsigned int program, const char *name, float value) {
    int loc = glGetUniformLocation(program, name);
    glUniform1f(loc, value);
}

void shader_set_int(unsigned int program, const char *name, int value) {
    int loc = glGetUniformLocation(program, name);
    glUniform1i(loc, value);
}