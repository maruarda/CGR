#ifndef TEXTURE_H
#define TEXTURE_H

/* ============================================================
 * texture.h
 *
 * Carrega uma imagem (PNG/JPG) do disco e cria uma textura OpenGL
 * 2D pronta pra usar. Depende do stb_image.h 
 * ============================================================ */

/* Carrega a imagem em 'path' e retorna o ID da textura OpenGL já
 * configurada (wrap repeat, mipmaps, filtro linear). Retorna 0
 * em caso de erro (arquivo não encontrado, formato inválido). */
unsigned int texture_load(const char *path);

#endif /* TEXTURE_H */