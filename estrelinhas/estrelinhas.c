#define GLFW_INCLUDE_NONE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define NUM_PARTICLES 3000

typedef struct {
    float angle;        // ângulo atual
    float radius;       // raio do centro
    float speed;        // velocidade angular
    float brightness;   // brilho atual
    float pulseSpeed;   // velocidade do brilho
    float size;         // tamanho visual
} Particle;

Particle particles[NUM_PARTICLES];

static float sceneRot = 0.0f;

void resizeWindow(GLFWwindow* window, int width, int height)
{
    if (height == 0) height = 1;

    float ratio = (float)width / (float)height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void handleKeyPress(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

float randFloat(float min, float max)
{
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

void initParticles(void)
{
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // distribui ao redor do círculo
        particles[i].angle = ((float)i / (float)NUM_PARTICLES) * 2.0f * (float)M_PI;

        // anel principal com pequena variação
        particles[i].radius = 1.4f + randFloat(-0.25f, 0.45f);

        // velocidade angular baixa
        particles[i].speed = randFloat(0.01f, 0.03f) / particles[i].radius;

        // brilho inicial
        particles[i].brightness = randFloat(0.4f, 1.0f);

        // velocidade do "piscar"
        particles[i].pulseSpeed = randFloat(0.01f, 0.04f);

        // tamanho da estrela
        particles[i].size = randFloat(2.0f, 6.0f);
    }
}

void initOpenGL(void)
{
    glEnable(GL_DEPTH_TEST);

    // fundo escuro azulado
    glClearColor(0.01f, 0.01f, 0.03f, 1.0f);

    // blend para brilho
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // suaviza pontos
    glEnable(GL_POINT_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

void drawParticles(void)
{
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // rotação individual
        particles[i].angle += particles[i].speed;

        if (particles[i].angle > 2.0f * (float)M_PI) {
            particles[i].angle -= 2.0f * (float)M_PI;
        }

        // brilho pulsando
        particles[i].brightness += sinf((float)glfwGetTime() * particles[i].pulseSpeed * 10.0f + i) * 0.002f;

        if (particles[i].brightness < 0.25f) particles[i].brightness = 0.25f;
        if (particles[i].brightness > 1.0f)  particles[i].brightness = 1.0f;

        float x = particles[i].radius * cosf(particles[i].angle);
        float y = particles[i].radius * sinf(particles[i].angle);

        // cor branca-azulada
        float b = particles[i].brightness;

        glPointSize(particles[i].size);
        glBegin(GL_POINTS);
            glColor4f(1.0f, 0.8f, 0.2f, b);
            glVertex3f(x, y, 0.0f);
        glEnd();

        // glow fake: ponto maior e mais transparente
        glPointSize(particles[i].size + 3.0f);
        glBegin(GL_POINTS);
            float heat = 1.0f - (particles[i].radius - 1.1f) / 0.9f;
            if (heat < 0.0f) heat = 0.0f;
            if (heat > 1.0f) heat = 1.0f;

            float r = 1.0f;
            float g = 0.2f + 0.8f * heat;
            float bl = 0.05f + 0.15f * heat;
            glColor4f(r, g, bl, b);
            glVertex3f(x, y, 0.0f);
        glEnd();
    }
}

void drawGLScene(GLFWwindow* window)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // desenha o disco central
    glPushMatrix();
        glColor3f(0.0f, 0.0f, 0.0f);
        GLUquadric* p = gluNewQuadric();
        gluDisk(p, 0.0f, 0.9f, 40, 1);
        gluDeleteQuadric(p);
    glPopMatrix();

    //anel ao redor do centro escuro:
    glPushMatrix();
        glColor4f(1.0f, 0.8f, 0.3f, 0.12f);
        GLUquadric* q = gluNewQuadric();
        gluDisk(q, 0.95f, 1.15f, 50, 1);
        gluDeleteQuadric(q);
    glPopMatrix();

    // afasta a cena
    glTranslatef(0.0f, 0.0f, -7.0f);

    // rotação global lenta do círculo
    sceneRot += 0.03f;
    glRotatef(sceneRot, 0.0f, 0.0f, 1.0f);

    // leve inclinação 
    glRotatef(60.0f, 1.0f, 0.0f, 0.0f);

    drawParticles();
}

int main(void)
{
    srand((unsigned int)time(NULL));

    if (!glfwInit()) {
        fprintf(stderr, "Falha ao iniciar GLFW\n");
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Buraco negro", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr, "Falha ao criar janela GLFW\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resizeWindow);
    glfwSwapInterval(1);

    resizeWindow(window, SCREEN_WIDTH, SCREEN_HEIGHT);
    initOpenGL();
    initParticles();

    while (!glfwWindowShouldClose(window)) {
        handleKeyPress(window);
        drawGLScene(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}