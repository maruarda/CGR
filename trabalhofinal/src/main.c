/* ============================================================
 * main.c
 *
 * Ponto de entrada. Responsável por:
 *   - abrir janela/contexto OpenGL (GLFW + glad)
 *   - tratar input (mouse orbita a câmera, scroll dá zoom,
 *     tecla W alterna wireframe)
 *   - gerar a malha via sweep_generate_rotational()
 *   - loop de render aplicando o shader Phong
 *
 * ============================================================ */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "mathutils.h"
#include "shader.h"
#include "camera.h"
#include "mesh.h"
#include "sweep.h"
#include "profile.h"

static Camera g_camera;
static int g_wireframe = 0;
static int g_dragging = 0;
static double g_last_x = 0.0, g_last_y = 0.0;

static void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    (void)mods;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            g_dragging = 1;
            glfwGetCursorPos(window, &g_last_x, &g_last_y);
        } else if (action == GLFW_RELEASE) {
            g_dragging = 0;
        }
    }
}

static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    (void)window;
    if (g_dragging) {
        float dx = (float)(xpos - g_last_x);
        float dy = (float)(ypos - g_last_y);
        camera_process_drag(&g_camera, dx, dy);
        g_last_x = xpos;
        g_last_y = ypos;
    }
}

static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    (void)window; (void)xoffset;
    camera_process_scroll(&g_camera, (float)yoffset);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    (void)scancode; (void)mods;
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, 1);
    } else if (key == GLFW_KEY_W) {
        g_wireframe = !g_wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, g_wireframe ? GL_LINE : GL_FILL);
    }
}

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Falha ao iniciar GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1024, 768, "Sweep", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Falha ao criar janela\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Falha ao iniciar GLAD\n");
        return 1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    glEnable(GL_DEPTH_TEST);

    /* --- monta o shader --- */
    unsigned int shader = shader_load("shaders/vertex.glsl", "shaders/fragment.glsl");

    /* --- gera a malha via sweep rotacional --- */

    /* OPÇÃO A: vaso (perfil aberto) 
    int profile_count;
    const ProfilePoint *profile = profile_get_vase(&profile_count); */
    //int steps = 48; /* divisões angulares: mais alto = mais suave */
    //Mesh mesh = sweep_generate_rotational(profile, profile_count, steps, /*profile_closed=*/0);

    /* OPÇÃO B: torus / rosquinha (perfil fechado) */

    int profile_count = 24;                 // suavidade do "tubo" do donut
    ProfilePoint *profile = profile_generate_torus_ring(profile_count, 1.2f, 0.5f);
    int steps = 48;                          // suavidade do laço grande
    Mesh mesh = sweep_generate_rotational(profile, profile_count, steps, 1);
    free(profile); // já foi copiado para dentro da mesh, pode liberar


    camera_init(&g_camera);
    g_camera.radius = 6.0f;

    /* modelo centralizado; sobe um pouco para ficar visível na tela */
    float model[16];
    mat4_translate(model, vec3_make(0.0f, -1.0f, 0.0f));

    while (!glfwWindowShouldClose(window)) {
        /* Cor de fundo (R, G, B, A), cada valor de 0.0 a 1.0.
         * Exemplos: cinza-claro (0.85, 0.85, 0.85), azul-céu (0.53, 0.81, 0.92),
         * branco (1.0, 1.0, 1.0), preto (0.0, 0.0, 0.0). */
        glClearColor(0.55f, 0.65f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        float view[16], projection[16];
        camera_get_view_matrix(&g_camera, view);
        mat4_perspective(projection, 45.0f * (float)M_PI / 180.0f, 1024.0f / 768.0f, 0.1f, 100.0f);

        shader_set_mat4(shader, "model", model);
        shader_set_mat4(shader, "view", view);
        shader_set_mat4(shader, "projection", projection);

        vec3 eye = camera_get_eye(&g_camera);
        shader_set_vec3(shader, "viewPos", eye.x, eye.y, eye.z);
        shader_set_vec3(shader, "lightPos", 4.0f, 5.0f, 4.0f);
        shader_set_vec3(shader, "lightColor", 1.0f, 1.0f, 1.0f);
        shader_set_vec3(shader, "objectColor", 0.75f, 0.55f, 0.35f);

        mesh_draw(&mesh);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    mesh_free(&mesh);
    glfwTerminate();
    return 0;
}
