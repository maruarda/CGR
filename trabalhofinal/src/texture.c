#include "texture.h"
#include <glad/glad.h>
#include <stdio.h>

/* STB_IMAGE_IMPLEMENTATION deve aparecer em EXATAMENTE um arquivo .c
 * do projeto, aqui. Isso faz o compilador gerar o código da
 * biblioteca (não só as declarações) uma única vez. */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int texture_load(const char *path) {
    int width, height, channels;

    /* stbi_set_flip_vertically_on_load: PNG/JPG guardam a primeira
     * linha como o TOPO da imagem, mas OpenGL espera (0,0) no canto
     * INFERIOR esquerdo da textura, sem isso, a textura aparece de
     * cabeça para baixo. */
    //stbi_set_flip_vertically_on_load(1);

    unsigned char *data = stbi_load(path, &width, &height, &channels, 0);
    if (!data) {
        fprintf(stderr, "[texture] Falha ao carregar '%s': %s\n", path, stbi_failure_reason());
        return 0;
    }

    GLenum format = (channels == 4) ? GL_RGBA : (channels == 3) ? GL_RGB : GL_RED;

    unsigned int tex_id;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    /* GL_REPEAT: essencial para o rótulo da lata "dar a volta" sem
     * costura visível, e para o donut tilear na direção da volta grande. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return tex_id;
}