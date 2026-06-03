// gcc novelo.c -I$HOME/local/include -L$HOME/local/lib -lglfw -lGL -lGLU -lm -o novelo && ./novelo novelo.obj textura.jpg

#define GLFW_INCLUDE_NONE
#define STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "stb_image.h"

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define MAX_VERTS     100000

/* ------------------------------------------------------------------ */
/*  Câmera orbital                                                      */
/* ------------------------------------------------------------------ */
float cam_yaw      = 0.0f;    /* rotação horizontal (graus) */
float cam_pitch    = 20.0f;   /* rotação vertical   (graus) */
float cam_distance = 4.0f;    /* distância ao modelo        */

/* estado do mouse */
int   mouse_dragging = 0;
double mouse_last_x  = 0.0;
double mouse_last_y  = 0.0;

#define YAW_SPEED     0.5f    /* sensibilidade mouse/seta horizontal */
#define PITCH_SPEED   0.5f    /* sensibilidade mouse/seta vertical   */
#define ZOOM_SPEED    0.3f    /* sensibilidade scroll                */
#define PITCH_MIN    -89.0f
#define PITCH_MAX     89.0f

/* ------------------------------------------------------------------ */
/*  Dados do modelo                                                     */
/* ------------------------------------------------------------------ */
typedef struct { float x, y, z; } Vec3;
typedef struct { float u, v; }    Vec2;

Vec3 positions[MAX_VERTS];
Vec3 normals[MAX_VERTS];
Vec2 uvs[MAX_VERTS];
int  npos = 0, nnorm = 0, nuv = 0;

typedef struct { Vec3 pos; Vec2 uv; Vec3 norm; } Vertex;
Vertex vertices[MAX_VERTS];
int    nvertices = 0;

GLuint texture_id = 0;

/* ------------------------------------------------------------------ */
GLuint LoadTexture(const char *path)
{
    int w, h, channels;
    stbi_set_flip_vertically_on_load(1);
    unsigned char *data = stbi_load(path, &w, &h, &channels, STBI_rgb_alpha);
    if (!data) { fprintf(stderr, "Erro ao carregar textura: %s\n", path); return 0; }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    printf("Textura carregada: %s (%dx%d)\n", path, w, h);
    return id;
}

/* ------------------------------------------------------------------ */
void LoadOBJ(const char *obj_path)
{
    FILE *f = fopen(obj_path, "r");
    if (!f) { fprintf(stderr, "Erro ao abrir %s\n", obj_path); return; }

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        if (strncmp(line, "v ", 2) == 0) {
            Vec3 p; sscanf(line+2, "%f %f %f", &p.x, &p.y, &p.z);
            positions[npos++] = p;
        } else if (strncmp(line, "vt ", 3) == 0) {
            Vec2 t; sscanf(line+3, "%f %f", &t.u, &t.v);
            uvs[nuv++] = t;
        } else if (strncmp(line, "vn ", 3) == 0) {
            Vec3 n; sscanf(line+3, "%f %f %f", &n.x, &n.y, &n.z);
            normals[nnorm++] = n;
        } else if (strncmp(line, "f ", 2) == 0) {
            int pi[4], ti[4], ni[4];
            int count = sscanf(line+2,
                "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
                &pi[0],&ti[0],&ni[0], &pi[1],&ti[1],&ni[1],
                &pi[2],&ti[2],&ni[2], &pi[3],&ti[3],&ni[3]);
            int nverts = count / 3;
            for (int t = 1; t < nverts-1; t++) {
                int idx[3] = {0, t, t+1};
                for (int k = 0; k < 3; k++) {
                    Vertex v;
                    v.pos  = positions[pi[idx[k]]-1];
                    v.uv   = uvs      [ti[idx[k]]-1];
                    v.norm = normals  [ni[idx[k]]-1];
                    vertices[nvertices++] = v;
                }
            }
        }
    }
    fclose(f);
    printf("Modelo carregado: %d vértices\n", nvertices);
}

/* ------------------------------------------------------------------ */
/*  Callbacks de input                                                  */
/* ------------------------------------------------------------------ */
void mouse_button_callback(GLFWwindow *win, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mouse_dragging = 1;
            glfwGetCursorPos(win, &mouse_last_x, &mouse_last_y);
        } else {
            mouse_dragging = 0;
        }
    }
}

void cursor_pos_callback(GLFWwindow *win, double xpos, double ypos)
{
    if (!mouse_dragging) return;

    float dx = (float)(xpos - mouse_last_x);
    float dy = (float)(ypos - mouse_last_y);
    mouse_last_x = xpos;
    mouse_last_y = ypos;

    cam_yaw   += dx * YAW_SPEED;
    cam_pitch -= dy * PITCH_SPEED;   /* invertido: arrastar pra cima = subir */

    if (cam_pitch < PITCH_MIN) cam_pitch = PITCH_MIN;
    if (cam_pitch > PITCH_MAX) cam_pitch = PITCH_MAX;
}

void scroll_callback(GLFWwindow *win, double xoffset, double yoffset)
{
    cam_distance -= (float)yoffset * ZOOM_SPEED;
    if (cam_distance < 0.5f)  cam_distance = 0.5f;
    if (cam_distance > 20.0f) cam_distance = 20.0f;
}

void resizeWindow(GLFWwindow *win, int w, int h)
{
    if (h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (float)w/h, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
}

/* ------------------------------------------------------------------ */
/*  Lê setas do teclado a cada frame                                   */
/* ------------------------------------------------------------------ */
void handleKeys(GLFWwindow *win)
{
    if (glfwGetKey(win, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(win, 1);
    }
    /* setas orbitam a câmera */
    if (glfwGetKey(win, GLFW_KEY_LEFT)  == GLFW_PRESS) cam_yaw   -= YAW_SPEED   * 2.0f;
    if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS) cam_yaw   += YAW_SPEED   * 2.0f;
    if (glfwGetKey(win, GLFW_KEY_UP)    == GLFW_PRESS) cam_pitch += PITCH_SPEED * 2.0f;
    if (glfwGetKey(win, GLFW_KEY_DOWN)  == GLFW_PRESS) cam_pitch -= PITCH_SPEED * 2.0f;
    /* +/- para zoom com teclado também */
    if (glfwGetKey(win, GLFW_KEY_EQUAL) == GLFW_PRESS) cam_distance -= 0.05f;
    if (glfwGetKey(win, GLFW_KEY_MINUS) == GLFW_PRESS) cam_distance += 0.05f;

    if (cam_pitch < PITCH_MIN) cam_pitch = PITCH_MIN;
    if (cam_pitch > PITCH_MAX) cam_pitch = PITCH_MAX;
    if (cam_distance < 0.5f)   cam_distance = 0.5f;
}

/* ------------------------------------------------------------------ */
void drawScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    /* Converte ângulos esféricos → posição cartesiana da câmera */
    float yaw_r   = cam_yaw   * M_PI / 180.0f;
    float pitch_r = cam_pitch * M_PI / 180.0f;

    float cx = cam_distance * cosf(pitch_r) * sinf(yaw_r);
    float cy = cam_distance * sinf(pitch_r);
    float cz = cam_distance * cosf(pitch_r) * cosf(yaw_r);

    /* modelo parado na origem; câmera orbita ao redor */
    gluLookAt(cx, cy, cz,   /* posição da câmera  */
              0,  0,  0,    /* olha para a origem */
              0,  1,  0);   /* vetor "up"          */

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLES);
    for (int i = 0; i < nvertices; i++) {
        glNormal3f(vertices[i].norm.x, vertices[i].norm.y, vertices[i].norm.z);
        glTexCoord2f(vertices[i].uv.u, vertices[i].uv.v);
        glVertex3f(vertices[i].pos.x, vertices[i].pos.y, vertices[i].pos.z);
    }
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

/* ------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    const char *obj_path = (argc > 1) ? argv[1] : "modelo.obj";
    const char *tex_path = (argc > 2) ? argv[2] : "textura.png";

    glfwInit();
    GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
                                          "Modelo com Textura", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resizeWindow);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSwapInterval(1);

    glClearColor(1.0f, 0.98f, 0.94f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat lpos[] = {2.0f, 4.0f, 3.0f, 1.0f};
    GLfloat ldif[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  ldif);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    resizeWindow(window, SCREEN_WIDTH, SCREEN_HEIGHT);
    LoadOBJ(obj_path);
    texture_id = LoadTexture(tex_path);

    printf("\nControles:\n");
    printf("  Mouse: clique e arraste para girar\n");
    printf("  Scroll: zoom\n");
    printf("  Setas: girar\n");
    printf("  +/-: zoom\n");
    printf("  ESC: sair\n\n");

    while (!glfwWindowShouldClose(window))
    {
        handleKeys(window);
        drawScene();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}