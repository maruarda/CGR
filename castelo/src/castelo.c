#define GLFW_INCLUDE_NONE
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>  // Header File For The GLFW Library
#include <GL/gl.h>       // Header File For The OpenGL32 Library
#include <GL/glu.h>      // Header File For The GLu32 Library

void processInput(GLFWwindow *window);

// settings
#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

// Rotation
static GLfloat xRot = 0.0f;
static GLfloat yRot = 0.0f;

//rotação com o mouse
double lastX = 400, lastY = 300;
int firstMouse = 1;
float sensitivity = 0.1f;


void resizeWindow(GLFWwindow* window, int w, int h )
{
    GLfloat fAspect;  
  
    // Prevent a divide by zero  
    if(h == 0)  
        h = 1;  
  
    // Set Viewport to window dimensions  
    glViewport(0, 0, w, h);  
  
    fAspect = (GLfloat)w/(GLfloat)h;  
  
    // Reset coordinate system  
    glMatrixMode(GL_PROJECTION);  
    glLoadIdentity();  
  
    // Produce the perspective projection  
    gluPerspective(35.0f, fAspect, 1.0, 40.0);  
  
    glMatrixMode(GL_MODELVIEW);  
    glLoadIdentity();  
}

// This function does any needed initialization on the rendering context.  Here it sets up and initializes the lighting for the scene.  
void setupRC(){  

    // Light values and coordinates  
    GLfloat  whiteLight[]  = { 0.2f, 0.2f, 0.2f, 1.0f }; 
    GLfloat  sourceLight[] = { 0.4f, 0.4f, 0.4f, 1.0f };
    GLfloat  lightPos[] = { -10.f, 5.0f, 5.0f, 1.0f };  
  
    glEnable(GL_DEPTH_TEST);    // Hidden surface removal  
    glFrontFace(GL_CCW);        // Counter clock-wise polygons face out  
    glEnable(GL_CULL_FACE);     // Do not calculate inside  
  
    // Enable lighting  
    glEnable(GL_LIGHTING);  
  
    // Setup and enable light 0  
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,whiteLight);  
    glLightfv(GL_LIGHT0,GL_AMBIENT,sourceLight);  
    glLightfv(GL_LIGHT0,GL_DIFFUSE,sourceLight);  
    glLightfv(GL_LIGHT0,GL_POSITION,lightPos);  
    glEnable(GL_LIGHT0);  
  
    // Enable color tracking  
    glEnable(GL_COLOR_MATERIAL);  
      
    // Set Material properties to follow glColor values  
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);  
  
    // Black blue background  
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);  

}  

void handleKeyPress(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwTerminate();
        exit(EXIT_SUCCESS);
    }

    if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) 
        yRot -= 1.0f;  
  
    if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)  
        yRot += 1.0f; 
        
     // cima / baixo
    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        xRot -= 1.0f;

    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        xRot += 1.0f;

    yRot = (GLfloat)((const int)yRot % 360);  
    xRot = (GLfloat)((int)xRot % 360);

}

//rotação com o mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = 0;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // invertido 

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yRot += xoffset; // esquerda/direita
    xRot += yoffset; // cima/baixo

    // Limitar rotação vertical 
    if (xRot > 89.0f) xRot = 89.0f;
    if (xRot < -89.0f) xRot = -89.0f;
}

GLuint carregarTextura(const char* caminho) {
    GLuint texturaID;
    glGenTextures(1, &texturaID);
    glBindTexture(GL_TEXTURE_2D, texturaID);

    // Parâmetros de filtragem
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int largura, altura, canais;
    stbi_set_flip_vertically_on_load(true); 
    unsigned char* dados = stbi_load(caminho, &largura, &altura, &canais, 0);

    if (dados) {
        printf("Sucesso: Imagem %s carregada (%dx%d)\n", caminho, largura, altura);
        GLenum formato = (canais == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, formato, largura, altura, 0, formato, GL_UNSIGNED_BYTE, dados);
        
    } else {
        printf("Erro ao carregar textura: %s\n", caminho);
    }

    stbi_image_free(dados);
    return texturaID;
}

void desenharFundo(GLuint id) {
    glDisable(GL_DEPTH_TEST); 
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); 
    glLoadIdentity(); 
    gluOrtho2D(0, 800, 0, 600); 

    glMatrixMode(GL_MODELVIEW); 
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f);

    glEnable(GL_TEXTURE_2D); 
    glBindTexture(GL_TEXTURE_2D, id);

    glBegin(GL_QUADS); 
        glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(800.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(800.0f, 600.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 600.0f);
    glEnd(); 

    glDisable(GL_TEXTURE_2D);
    glPopMatrix(); 
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    
    glEnable(GL_DEPTH_TEST); 
}

void desenharCubo(float w, float h, float d) {
    w /= 2.0f;
    d /= 2.0f;

    glBegin(GL_QUADS);
        // Frente
        glNormal3f(0,0,1);
        glVertex3f(-w, 0, d);
        glVertex3f(w, 0, d);
        glVertex3f(w, h, d);
        glVertex3f(-w, h, d);

        // Trás
        glNormal3f(0,0,-1);
        glVertex3f(-w, 0, -d);
        glVertex3f(-w, h, -d);
        glVertex3f(w, h, -d);
        glVertex3f(w, 0, -d);

        // Esquerda
        glNormal3f(-1,0,0);
        glVertex3f(-w, 0, -d);
        glVertex3f(-w, 0, d);
        glVertex3f(-w, h, d);
        glVertex3f(-w, h, -d);

        // Direita
        glNormal3f(1,0,0);
        glVertex3f(w, 0, -d);
        glVertex3f(w, h, -d);
        glVertex3f(w, h, d);
        glVertex3f(w, 0, d);

        // Topo
        glNormal3f(0,1,0);
        glVertex3f(-w, h, d);
        glVertex3f(w, h, d);
        glVertex3f(w, h, -d);
        glVertex3f(-w, h, -d);

        // Base
        glNormal3f(0,-1,0);
        glVertex3f(-w, 0, d);
        glVertex3f(-w, 0, -d);
        glVertex3f(w, 0, -d);
        glVertex3f(w, 0, d);
    glEnd();
}

void desenharTorre(GLUquadricObj *pObj, float x, float y, float z) {
    glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f); 

        // Corpo da Torre 
        glColor3f(1.0f, 0.75f, 0.8f);
        gluCylinder(pObj, 0.3f, 0.3f, 1.5f, 26, 13);
        gluDisk(pObj, 0.0f, 0.3f, 26, 1);

        // Telhado
        glPushMatrix();
            glTranslatef(0.0f, 0.0f, 1.49f);
            glColor3f(0.58f, 0.46f, 1.05f);
            gluDisk(pObj, 0.0f, 0.45f, 26, 1);
            gluCylinder(pObj, 0.45f, 0.0f, 0.75f, 26, 13);
        glPopMatrix();
    glPopMatrix();
}

void desenharMuralha(GLUquadricObj *pObj, float x, float y, float z, float rotY) {
    glPushMatrix();
        glTranslatef(x, y, z);
        glRotatef(rotY, 0.0f, 1.0f, 0.0f);

        float largura = 3.0f;
        float altura = 1.0f;
        float espessura = 0.4f;

        // CORPO DA MURALHA 
        glColor3f(1.0f, 0.75f, 0.8f);
        desenharCubo(largura, altura, espessura);

        // PORTA 
        if(z == 1.2f){
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, espessura/2 + 0.01f);

                glColor3f(0.65f, 0.16f, 0.16f);

                desenharCubo(0.6f, 0.7f, 0.1f);
            glPopMatrix();
        }

        float larguraDente = 0.2f;
        float alturaDente = 0.2f;

        for(float i = -largura/2; i < largura/2; i += larguraDente * 2) {
            glPushMatrix();
                glTranslatef(i + larguraDente/2, altura, 0.0f);

                glColor3f(1.0f, 0.75f, 0.8f);

                desenharCubo(larguraDente, alturaDente, espessura);
            glPopMatrix();
        }

    glPopMatrix();
}

/* Função principal de renderização */
void drawGLScene(GLFWwindow* window, GLuint id)
{
    // Limpa os buffers de cor e profundidade 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

    desenharFundo(id);

    // 2. Prepara o objeto de quádricas 
    GLUquadricObj *pObj = gluNewQuadric();
    gluQuadricNormals(pObj, GLU_SMOOTH);  

    // 3. Inicia as transformações 3D 
    glPushMatrix(); // Salva o estado da matriz atual 

        // Posicionamento da cena e controle de rotação pelo usuário 
        glTranslatef(0.0f, -1.0f, -7.0f); 
        glRotatef(yRot, 0.0f, 1.0f, 0.0f);
        glRotatef(xRot, 1.0f, 0.0f, 0.0f);

        // --- Gramado ---
         glColor3f(0.68f, 0.8f, 0.46f);
         glPushMatrix();
             glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
             gluDisk(pObj, 0.0f, 5.0f, 32, 1);
         glPopMatrix();

        // --- Desenho das 4 Torres ---
         desenharTorre(pObj, -1.4f, 0.0f, -1.4f);
         desenharTorre(pObj,  1.4f, 0.0f, -1.4f);
         desenharTorre(pObj, -1.4f, 0.0f,  1.4f);
         desenharTorre(pObj,  1.4f, 0.0f,  1.4f);

        // --- Muralhas conectando as torres ---
        desenharMuralha(pObj, 0.0f, 0.0f,  1.2f, 0.0f);
        desenharMuralha(pObj, 0.0f, 0.0f, -1.2f, 0.0f);   
        desenharMuralha(pObj, -1.2f, 0.0f, 0.0f, 90.0f);
        desenharMuralha(pObj, 1.2f, 0.0f, 0.0f, 90.0f);


    glPopMatrix(); // Restaura a matriz ao estado anterior 

    // Libera a memória do objeto de quádrica
    gluDeleteQuadric(pObj); 

    glFlush(); 
}

int main()
{
    glfwInit();

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Castelo", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resizeWindow);
    glfwSwapInterval(1);
    
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    resizeWindow(window, SCREEN_WIDTH, SCREEN_HEIGHT);
    setupRC();

    //rotação mouse
    //glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLuint fundoID = carregarTextura("fundo.jpg");

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        handleKeyPress(window);

        // render
        drawGLScene(window, fundoID);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

