// linux: gcc chuva.c -I$HOME/local/include -L$HOME/local/lib -lglfw -lGL -lGLU -lm -o chuva && ./chuva

#define GLFW_INCLUDE_NONE
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <time.h>

void processInput(GLFWwindow *window);

#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480

#define NUM_PARTICLES 80
#define NUM_STARS     200  

#define RAIN_VX  0.003f   /* deriva lateral                             */
#define RAIN_VY -0.019f   /* queda vertical — mais rápido = zoom in     */

#define POINT_SIZE 4.0f

struct s_pf {
    float x, y;
    float veloc_x, veloc_y;
    float lifetime;   
} particles[NUM_PARTICLES];

struct s_star {
    float x, y;
    float brightness;   /* brilho base  0..1          */
    float twinkle;      /* fase atual do pulso         */
    float twinkle_spd;  /* velocidade do pulso         */
} stars[NUM_STARS];

float aspect = (float)SCREEN_WIDTH / SCREEN_HEIGHT;

/* Reseta UMA partícula individualmente */
void ResetParticle(int i, int scattered)
{
    /* X cobre toda a largura da tela no espaço de mundo */
    particles[i].x = ((float)rand() / RAND_MAX) * 2.0f * aspect - aspect;

    if (scattered)
        /* espalhado pela tela inteira */
        particles[i].y = ((float)rand() / RAND_MAX) * 2.4f - 1.1f;
    else
        /* reinício normal: nasce acima da tela */
        particles[i].y = 1.1f + ((float)rand() / RAND_MAX) * 0.3f;

    /* pequena variação de velocidade para parecer menos uniforme */
    float jitter = 0.85f + ((float)rand() / RAND_MAX) * 0.3f;
    particles[i].veloc_x = RAIN_VX * jitter;
    particles[i].veloc_y = RAIN_VY * jitter;

    /* lifetime proporcional à posição Y: quem está mais abaixo tem menos vida
     * para evitar que partículas recém-nascidas no topo "cruzem" a área vazia */
    if (scattered)
        particles[i].lifetime = 0.3f + ((float)rand() / RAND_MAX) * 0.7f;
    else
        particles[i].lifetime = 0.7f + ((float)rand() / RAND_MAX) * 0.3f;
}

void InitStars()
{
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].x           = ((float)rand() / RAND_MAX) * 2.0f * aspect - aspect;
        stars[i].y           = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        stars[i].brightness  = 0.3f + ((float)rand() / RAND_MAX) * 0.7f;
        stars[i].twinkle     = ((float)rand() / RAND_MAX) * 6.28f; /* fase aleatória */
        stars[i].twinkle_spd = 0.01f + ((float)rand() / RAND_MAX) * 0.04f;
    }
}
 
void InitParticles()
{
    srand((unsigned)time(NULL));
    for (int i = 0; i < NUM_PARTICLES; i++)
        ResetParticle(i, 1); /* espalhado pela tela toda no início */
    InitStars();
}

void resizeWindow(GLFWwindow* window, int width, int height)
{
    if (height == 0) height = 1;
    aspect = (float)width / (float)height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-aspect, aspect, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void handleKeyPress(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwTerminate();
        exit(EXIT_SUCCESS);
    }
}

void drawGLScene(GLFWwindow* window)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_POINT_SMOOTH);

    /* --- Desenha o fundo com alpha para criar efeito de rastro --- */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);          /* coordenadas NDC simples      */
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor4f(0.0f, 0.02f, 0.05f, 0.18f);  /* mesma cor do fundo + alpha   */
    glBegin(GL_QUADS);
        glVertex2f(-1, -1);
        glVertex2f( 1, -1);
        glVertex2f( 1,  1);
        glVertex2f(-1,  1);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    /* --- Desenha as estrelas no fundo --- */
    glPointSize(1.5f);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_STARS; i++) {
        stars[i].twinkle += stars[i].twinkle_spd;
 
        /* pulso senoidal: brilho oscila entre 40% e 100% do valor base */
        float pulse = 0.7f + 0.3f * sinf(stars[i].twinkle);
        float a     = stars[i].brightness * pulse;
 
        /* cor levemente azulada/branca, variando por brilho */
        glColor4f(0.8f + 0.2f * a, 0.85f + 0.15f * a, 1.0f, a);
        glVertex2f(stars[i].x, stars[i].y);
    }
    glEnd();
 
    /* --- Desenha as partículas de chuva --- */
    glPointSize(POINT_SIZE);
    glBegin(GL_POINTS);
    for (int i = 0; i < NUM_PARTICLES; i++) {

        /* desconta vida; se esgotou, reinicia no topo */
        particles[i].lifetime -= 0.008f;   /* decay maior combina com velocidade maior */
        if (particles[i].lifetime <= 0.0f ||
            particles[i].y < -1.1f        ||
            particles[i].x >  aspect + 0.1f ||
            particles[i].x < -aspect - 0.1f)
        {
            ResetParticle(i, 0);
        }

        /* atualiza posição */
        particles[i].x += particles[i].veloc_x;
        particles[i].y += particles[i].veloc_y;

        /* alpha = vida restante */
        float alpha = particles[i].lifetime;
        glColor4f(0.96f, 0.96f, 0.86f, alpha);
        glVertex2f(particles[i].x, particles[i].y);
    }
    glEnd();
}

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chuva", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resizeWindow);
    glfwSwapInterval(1);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    glClearColor(0.0f, 0.02f, 0.05f, 1.0f); /* fundo azul-noite */
    glPointSize(POINT_SIZE);

    InitParticles();
    resizeWindow(window, SCREEN_WIDTH, SCREEN_HEIGHT);

    while (!glfwWindowShouldClose(window))
    {
        handleKeyPress(window);
        drawGLScene(window);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}