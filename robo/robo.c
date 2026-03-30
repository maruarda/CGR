
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

/* function to handle key press events */
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

    //cinza
	glColor3f(0.5f, 0.5f, 0.5f);
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
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix(); // save transform matrix state
		glTranslatef(0.0f, 1.8f, 0.0f);
        glScalef(1.25f, 0.56f, 1.25f);
		gluSphere(pObj, 0.4f, 39, 19.5);
	glPopMatrix(); // restore transform matrix state

    //cabeça
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
        glTranslatef(0.0f, 1.9f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(pObj, 0.25f, 0.25f, 0.68f, 20, 20);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix(); // save transform matrix state
		glTranslatef(0.0f, 2.45f, 0.0f);
        glScalef(0.9f, 1.1f, 0.9f);
		gluSphere(pObj, 0.29f, 39, 19.5);
	glPopMatrix(); // restore transform matrix state

    //pernas
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
        glTranslatef(0.25f, 0.0f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(pObj, 0.1f, 0.1f, 1.0f, 20, 20);
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
        glTranslatef(-0.25f, 0.0f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(pObj, 0.09f, 0.09f, 1.0f, 20, 20);
    glPopMatrix();

    //pés
    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
        glTranslatef(0.25f, -0.25f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(pObj, 0.2f, 0.09f, 0.25f, 20, 20);

        // base (parte de baixo)
        glPushMatrix();
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f); // vira o disco
            gluDisk(pObj, 0.0f, 0.2f, 20, 20);
        glPopMatrix();
    glPopMatrix();

    glColor3f(0.5f, 0.5f, 0.5f);
    glPushMatrix();
        glTranslatef(-0.25f, -0.25f, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        gluCylinder(pObj, 0.2f, 0.09f, 0.25f, 20, 20);

        // base (parte de baixo)
        glPushMatrix();
            glRotatef(180.0f, 1.0f, 0.0f, 0.0f); // vira o disco
            gluDisk(pObj, 0.0f, 0.2f, 20, 20);
        glPopMatrix();
    glPopMatrix();

    //braços

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

