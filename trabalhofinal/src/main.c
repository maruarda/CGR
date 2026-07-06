/* ============================================================
 * main.c
 *
 * Ponto de entrada. Responsável por:
 *   - abrir janela/contexto OpenGL (GLFW + glad)
 *   - tratar input (mouse orbita a câmera, scroll dá zoom,
 *     tecla W alterna wireframe)
 *   - gerar DOIS objetos lado a lado:
 *       - uma rosquinha (donut), via sweep ROTACIONAL
 *       - um "tubo" tipo lata de refrigerante, via sweep
 *         TRANSLACIONAL (extrusão ao longo de um caminho reto)
 *   - carregar as texturas de cada objeto
 *   - loop de render aplicando o shader Phong + textura
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
#include "texture.h"

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
    /* TODO: teclas +/- para aumentar/diminuir 'steps' e regenerar
     * a malha em tempo real (chamar mesh_free + sweep_generate_rotational
     * de novo com um valor diferente de steps). Bom "plus" para a nota. */
}

int main(void) {
    if (!glfwInit()) {
        fprintf(stderr, "Falha ao iniciar GLFW\n");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(1024, 768, "Sweep - Donut e Lata", NULL, NULL);
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

    /* --- monta o shader (compartilhado pelos dois objetos) --- */
    unsigned int shader = shader_load("shaders/vertex.glsl", "shaders/fragment.glsl");

    /* ============================================================
     * OBJETO 1: donut (rosquinha), sweep ROTACIONAL
     * Perfil = um pequeno círculo (o "tubo" do donut) deslocado do
     * eixo Y; ao girar em torno do eixo, gera a rosquinha inteira.
     * ============================================================ */
    int donut_profile_count = 32;                 /* suavidade do tubo do donut */
    ProfilePoint *donut_profile = profile_generate_torus_ring(donut_profile_count, 1.2f, 0.5f);
    int donut_steps = 48;                          /* suavidade da volta grande */
    Mesh donut_mesh = sweep_generate_rotational(donut_profile, donut_profile_count, donut_steps, /*profile_closed=*/1);
    free(donut_profile); /* já foi copiado para dentro da mesh, pode liberar */

    /* ============================================================
     * OBJETO 2: tubo tipo lata de refrigerante, sweep TRANSLACIONAL
     * Perfil = círculo simples centrado na origem (major_radius = 0),
     * usado como seção transversal; o caminho é reto, de baixo para
     * cima, "extrudando" esse círculo e formando o corpo da lata.
     * ============================================================ */
    int can_profile_count = 32;                    /* suavidade da circunferência da lata */
    ProfilePoint *can_profile = profile_generate_torus_ring(can_profile_count, 0.0f, 0.55f);
    vec3 can_path[2] = {
        vec3_make(0.0f, -1.0f, 0.0f), /* base da lata   */
        vec3_make(0.0f,  1.0f, 0.0f)  /* topo da lata   */
    };
    Mesh can_mesh = sweep_generate_translational(can_profile, can_profile_count, can_path, 2, /*add_caps=*/1);
    free(can_profile);

    /* --- carrega as texturas de cada objeto --- */
    unsigned int donut_texture = texture_load("assets/donut_texture.png");
    unsigned int can_texture = texture_load("assets/can_texture.png");

    camera_init(&g_camera);
    g_camera.radius = 7.0f;

    /* matrizes de modelo: os dois objetos lado a lado no eixo X.
     * O donut fica "deitado" (girado 90 graus em X) para mostrar o
     * "buraco" de frente para a câmera, como um donut de vitrine. */
    float donut_model[16];
    {
        /* rotaciona 90 graus em torno do eixo X para deitar o donut.
         * mathutils.h só tem rotate_y pronto, então aplicamos a
         * rotação em X manualmente aqui mesmo. */
        float rx[16];
        mat4_identity(rx);
        float c = cosf(1.5707963f), s = sinf(1.5707963f); /* 90 graus */
        rx[5] = c;  rx[9]  = -s;
        rx[6] = s;  rx[10] = c;

        float t[16];
        mat4_translate(t, vec3_make(-2.2f, 0.0f, 0.0f));
        mat4_multiply(donut_model, t, rx);
    }

    float can_model[16];
    mat4_translate(can_model, vec3_make(2.2f, 0.0f, 0.0f));

    while (!glfwWindowShouldClose(window)) {
        /* Cor de fundo (R, G, B, A), cada valor de 0.0 a 1.0. */
        glClearColor(0.55f, 0.65f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shader);

        float view[16], projection[16];
        camera_get_view_matrix(&g_camera, view);
        mat4_perspective(projection, 45.0f * (float)M_PI / 180.0f, 1024.0f / 768.0f, 0.1f, 100.0f);

        shader_set_mat4(shader, "view", view);
        shader_set_mat4(shader, "projection", projection);

        vec3 eye = camera_get_eye(&g_camera);
        shader_set_vec3(shader, "viewPos", eye.x, eye.y, eye.z);
        shader_set_vec3(shader, "lightPos", 4.0f, 5.0f, 4.0f);
        shader_set_vec3(shader, "lightColor", 1.0f, 1.0f, 1.0f);
        shader_set_int(shader, "diffuseTexture", 0); /* sempre usa a unidade de textura 0 */

        /* --- desenha o donut --- */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, donut_texture);
        shader_set_mat4(shader, "model", donut_model);
        mesh_draw(&donut_mesh);

        /* --- desenha a lata --- */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, can_texture);
        shader_set_mat4(shader, "model", can_model);
        mesh_draw(&can_mesh);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    mesh_free(&donut_mesh);
    mesh_free(&can_mesh);
    glfwTerminate();
    return 0;
}