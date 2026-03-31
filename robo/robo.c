
//A/Z ombro esquerdo
//S/X cotovelo esquerdo
//K/M ombro direito
//L/, cotovelo direito
//T/G quadril esquerdo
//Y/H joelho esquerdo
//U/J quadril direito
//I/N joelho direito


#define GLFW_INCLUDE_NONE
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

// articulações
static GLfloat shoulderLeft = -20.0f;
static GLfloat shoulderRight = 20.0f;
static GLfloat elbowLeft = -20.0f;
static GLfloat elbowRight = 20.0f;

static GLfloat hipLeft = 0.0f;
static GLfloat hipRight = 0.0f;
static GLfloat kneeLeft = 0.0f;
static GLfloat kneeRight = 0.0f;

//rotação com o mouse
double lastX = 400, lastY = 300;
int firstMouse = 1;
float sensitivity = 0.1f;


/* function to reset our viewport after a window resize */
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
    gluPerspective(60.0f, fAspect, 1.0, 40.0);  
  
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

    glEnable(GL_NORMALIZE);
  
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
  
    // lavanda 
    glClearColor(0.9f, 0.9f, 0.98f, 1.0f);  

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

    if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        xRot -= 1.0f;

    if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        xRot += 1.0f;

    // braço esquerdo
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) shoulderLeft += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) shoulderLeft -= 1.0f;
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) elbowLeft += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) elbowLeft -= 1.0f;

    // braço direito
    if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) shoulderRight -= 1.0f;
    if(glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) shoulderRight += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) elbowRight -= 1.0f;
    if(glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) elbowRight += 1.0f;

    // perna esquerda
    if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) hipLeft += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) hipLeft -= 1.0f;
    if(glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) kneeLeft += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) kneeLeft -= 1.0f;

    // perna direita
    if(glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) hipRight += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) hipRight -= 1.0f;
    if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) kneeRight += 1.0f;
    if(glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) kneeRight -= 1.0f;

    yRot = (GLfloat)((int)yRot % 360);
    xRot = (GLfloat)((int)xRot % 360);

    // limites simples
    if (shoulderLeft > 90) shoulderLeft = 90;
    if (shoulderLeft < -90) shoulderLeft = -90;
    if (shoulderRight > 90) shoulderRight = 90;
    if (shoulderRight < -90) shoulderRight = -90;

    if (elbowLeft > 120) elbowLeft = 120;
    if (elbowLeft < -10) elbowLeft = -10;
    if (elbowRight > 120) elbowRight = 120;
    if (elbowRight < -10) elbowRight = -10;

    if (hipLeft > 60) hipLeft = 60;
    if (hipLeft < -60) hipLeft = -60;
    if (hipRight > 60) hipRight = 60;
    if (hipRight < -60) hipRight = -60;

    if (kneeLeft > 120) kneeLeft = 120;
    if (kneeLeft < 0) kneeLeft = 0;
    if (kneeRight > 120) kneeRight = 120;
    if (kneeRight < 0) kneeRight = 0;
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
    float yoffset = lastY - ypos; // invertido (movimento natural)

    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yRot += xoffset; // esquerda/direita
    xRot += yoffset; // cima/baixo

    // Limitar rotação vertical (evita virar de cabeça pra baixo)
    if (xRot > 89.0f) xRot = 89.0f;
    if (xRot < -89.0f) xRot = -89.0f;
}

void drawRightArm(GLUquadricObj *pObj)
{
    glColor3f(0.47f, 0.53f, 0.6f);

    glPushMatrix();
        // pivô no ombro direito
        glTranslatef(0.5f, 1.65f, 0.0f);
        glRotatef(shoulderRight, 0.0f, 0.0f, 1.0f);

        // junta do ombro
        glPushMatrix();
            glScalef(0.9f, 1.1f, 0.9f);
            gluSphere(pObj, 0.12f, 20, 20);
        glPopMatrix();

        // braço superior
        glPushMatrix();
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            gluCylinder(pObj, 0.09f, 0.09f, 0.75f, 20, 20);
        glPopMatrix();

        // cotovelo
        glPushMatrix();
            glTranslatef(0.75f, 0.0f, 0.0f);
            gluSphere(pObj, 0.10f, 20, 20);

            glRotatef(elbowRight, 0.0f, 0.0f, 1.0f);

            // antebraço
            glPushMatrix();
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
                gluCylinder(pObj, 0.08f, 0.08f, 0.60f, 20, 20);
            glPopMatrix();

            // mão
            glPushMatrix();
                glTranslatef(0.59f, 0.0f, 0.0f);
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
                gluCylinder(pObj, 0.09f, 0.2f, 0.25f, 20, 20);
                glTranslatef(0.0f, 0.0f, 0.25f);
                gluDisk(pObj, 0.0f, 0.2f, 20, 20);
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();
}

void drawLeftArm(GLUquadricObj *pObj)
{
    glColor3f(0.47f, 0.53f, 0.6f);

    glPushMatrix();
        // pivô no ombro esquerdo
        glTranslatef(-0.5f, 1.65f, 0.0f);
        glRotatef(-shoulderLeft, 0.0f, 0.0f, 1.0f);

        // junta do ombro
        glPushMatrix();
            glScalef(0.9f, 1.1f, 0.9f);
            gluSphere(pObj, 0.12f, 20, 20);
        glPopMatrix();

        // braço superior
        glPushMatrix();
            glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
            gluCylinder(pObj, 0.09f, 0.09f, 0.75f, 20, 20);
        glPopMatrix();

        // cotovelo
        glPushMatrix();
            glTranslatef(-0.75f, 0.0f, 0.0f);
            gluSphere(pObj, 0.10f, 20, 20);

            glRotatef(-elbowLeft, 0.0f, 0.0f, 1.0f);

            // antebraço
            glPushMatrix();
                glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
                gluCylinder(pObj, 0.08f, 0.08f, 0.60f, 20, 20);
            glPopMatrix();

            // mão
            glPushMatrix();
                glTranslatef(-0.59f, 0.0f, 0.0f);
                glRotatef(-90.0f, 0.0f, 1.0f, 0.0f);
                gluCylinder(pObj, 0.09f, 0.2f, 0.25f, 20, 20);
                glTranslatef(0.0f, 0.0f, 0.25f);
                gluDisk(pObj, 0.0f, 0.2f, 20, 20);
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();
}

void drawRightLeg(GLUquadricObj *pObj)
{
    glColor3f(0.47f, 0.53f, 0.6f);

    glPushMatrix();
        // pivô no quadril direito
        glTranslatef(0.25f, 0.45f, 0.0f);
        glRotatef(hipRight, 1.0f, 0.0f, 0.0f);

        // coxa/perna superior
        glPushMatrix();
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            gluCylinder(pObj, 0.09f, 0.09f, 0.60f, 20, 20);
        glPopMatrix();

        // joelho
        glPushMatrix();
            glTranslatef(0.0f, -0.3f, 0.0f);
            gluSphere(pObj, 0.09f, 20, 20);

            glRotatef(kneeRight, 1.0f, 0.0f, 0.0f);

            // canela
            glPushMatrix();
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                gluCylinder(pObj, 0.08f, 0.08f, 0.45f, 20, 20);
            glPopMatrix();

            // pé
            glPushMatrix();
                glTranslatef(0.0f, -0.3f, 0.0f);
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                gluCylinder(pObj, 0.2f, 0.09f, 0.25f, 20, 20);

                glPushMatrix();
                    glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
                    gluDisk(pObj, 0.0f, 0.2f, 20, 20);
                glPopMatrix();
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();
}

void drawLeftLeg(GLUquadricObj *pObj)
{
    glColor3f(0.47f, 0.53f, 0.6f);

    glPushMatrix();
        // pivô no quadril esquerdo
        glTranslatef(-0.25f, 0.45f, 0.0f);
        glRotatef(hipLeft, 1.0f, 0.0f, 0.0f);

        // coxa/perna superior
        glPushMatrix();
            glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
            gluCylinder(pObj, 0.09f, 0.09f, 0.60f, 20, 20);
        glPopMatrix();

        // joelho
        glPushMatrix();
            glTranslatef(0.0f, -0.3f, 0.0f);
            gluSphere(pObj, 0.09f, 20, 20);

            glRotatef(kneeLeft, 1.0f, 0.0f, 0.0f);

            // canela
            glPushMatrix();
                glRotatef(-90.0f, 0.7f, 0.0f, 0.0f);
                gluCylinder(pObj, 0.08f, 0.08f, 0.45f, 20, 20);
            glPopMatrix();

            // pé
            glPushMatrix();
                glTranslatef(0.0f, -0.3f, 0.0f);
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                gluCylinder(pObj, 0.2f, 0.09f, 0.25f, 20, 20);

                glPushMatrix();
                    glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
                    gluDisk(pObj, 0.0f, 0.2f, 20, 20);
                glPopMatrix();
            glPopMatrix();
        glPopMatrix();
    glPopMatrix();
}

/* Here goes our drawing code */
void drawGLScene(GLFWwindow* window)
{
    GLUquadricObj *pObj;    // Quadric Object  
      
    // Clear the window with current clearing color  
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
  
    // Save the matrix state and do the rotations  
    glPushMatrix();

	// Move object back and do in place rotation  
	glTranslatef(0.0f, -1.0f, -5.0f);  
	glRotatef(yRot, 0.0f, 1.0f, 0.0f);  
    glRotatef(xRot, 1.0f, 0.0f, 0.0f);

	// Draw something  
	pObj = gluNewQuadric();  
	gluQuadricNormals(pObj, GLU_SMOOTH);  

    //cinza ardosia claro
	glColor3f(0.47f, 0.53f, 0.6f);
	// Main Body  
    glPushMatrix();
        glTranslatef(0.0f, 0.8f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);

        // cilindro
        gluCylinder(pObj, 0.4f, 0.5f, 1.0f, 20, 20);

        // base (parte de baixo)
        glPushMatrix();
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f); // vira o disco
            gluDisk(pObj, 0.0f, 0.4f, 20, 20);
        glPopMatrix();

    glPopMatrix();

    //"ombros"
    glColor3f(0.47f, 0.53f, 0.6f);
    glPushMatrix(); // save transform matrix state
		glTranslatef(0.0f, 1.8f, 0.0f);
        glScalef(1.25f, 0.56f, 1.25f);
		gluSphere(pObj, 0.4f, 39, 19.5);
	glPopMatrix(); // restore transform matrix state

    //cabeça
    glColor3f(0.47f, 0.53f, 0.6f);
    glPushMatrix();
        glTranslatef(0.0f, 1.9f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(pObj, 0.25f, 0.25f, 0.68f, 20, 20);
    glPopMatrix();

    glColor3f(0.47f, 0.53f, 0.6f);
    glPushMatrix(); // save transform matrix state
		glTranslatef(0.0f, 2.45f, 0.0f);
        glScalef(0.9f, 1.1f, 0.9f);
		gluSphere(pObj, 0.29f, 39, 19.5);
	glPopMatrix(); // restore transform matrix state

    //antena
    glColor3f(0.47f, 0.53f, 0.6f);
    glPushMatrix();
        glTranslatef(0.0f, 2.6f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(pObj, 0.09f, 0.0f, 0.46f, 20, 20);
    glPopMatrix();

    glColor3f(0.47f, 0.53f, 0.6f);
    glPushMatrix(); 
		glTranslatef(0.0f, 2.72f, 0.0f);
        glScalef(1.0f, 0.9f, 1.0f);
		gluSphere(pObj, 0.1f, 39, 19.5);
	glPopMatrix(); 

    glColor3f(0.47f, 0.53f, 0.6f);
    glPushMatrix(); 
		glTranslatef(0.0f, 3.0f, 0.0f);
		gluSphere(pObj, 0.049f, 39, 19.5);
	glPopMatrix(); 

    //rosto
    glColor3f(0.47f, 0.53f, 0.6f);
    glPushMatrix();
        glTranslatef(0.0f, 2.43f, 0.15f);
        glScalef(0.7f, 0.4f, 1.0f);
        gluCylinder(pObj, 0.33f, 0.33f, 0.15f, 20, 20);

        glPushMatrix();
            //glRotatef(180.0f, 1.0f, 0.0f, 0.0f);
            gluDisk(pObj, 0.0f, 0.33f, 20, 20);
        glPopMatrix();

    glPopMatrix();
    //olhos
    //esquerdo
    glColor3f(1.0f, 1.0f, 0.88f); 
    glPushMatrix(); 
		glTranslatef(-0.09f, 2.42f, 0.2f);
        glScalef(1.0f, 0.8f, 1.0f);
		gluSphere(pObj, 0.12f, 39, 19.5);
	glPopMatrix(); 
    //direito
    glColor3f(1.0f, 1.0f, 0.88f); 
    glPushMatrix(); 
		glTranslatef(0.09f, 2.42f, 0.2f);
        glScalef(1.0f, 0.8f, 1.0f);
		gluSphere(pObj, 0.12f, 39, 19.5);
	glPopMatrix();

    glColor3f(0.1f, 0.1f, 0.1f);
    // pupila esquerda
    glPushMatrix();
        glTranslatef(-0.085f, 2.42f, 0.33f);
        gluSphere(pObj, 0.015f, 20, 20);
    glPopMatrix();

    // pupila direita
    glPushMatrix();
        glTranslatef(0.085f, 2.42f, 0.33f);
        gluSphere(pObj, 0.015f, 20, 20);
    glPopMatrix();

    //boca
    glColor3f(1.0f, 1.0f, 0.88f);
    glPushMatrix();
        glTranslatef(0.0f, 1.93f, 0.075f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(pObj, 0.2f, 0.2f, 0.29f, 20, 20);
    glPopMatrix();

    drawLeftArm(pObj);
    drawRightArm(pObj);
    drawLeftLeg(pObj);
    drawRightLeg(pObj);

    // Restore the matrix state  
    glPopMatrix();  
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Robozinho", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to open GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resizeWindow);
    glfwSwapInterval(1);
    
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    resizeWindow(window, SCREEN_WIDTH, SCREEN_HEIGHT);
    setupRC();
    //rotação mouse
    glfwSetCursorPosCallback(window, mouse_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        handleKeyPress(window);

        // render
        drawGLScene(window);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}
