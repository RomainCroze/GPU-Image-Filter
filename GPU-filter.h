#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/glfw.h>
#include <glm/glm.hpp>
#include <time.h>
#include <math.h>
#include <unistd.h>
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

// Functions used in the program
char* LoadSource(const char *filename);
GLuint LoadShader(GLenum type, const char *filename);
int verif_link(GLuint program);
GLuint create_shaders(const char* vertex_file_path, const char* fragment_file_path);
GLuint BMPToTexture(const char * imagepath);
int BMP_height(const char * imagepath);
int BMP_width(const char * imagepath);
GLuint renderToTexture();
int initializeWindow(float width,float height, const char* title);
void prepare_data(GLuint* vertexbuffer,const GLfloat* g_vertex_buffer_data,const GLfloat* g_txcoord_buffer_data,int vertex_data_size,int txcoord_data_size);
void create_program(const char * shaderpath,GLuint* programID,int GLSL_used,GLuint* timeID,GLuint* heightID,GLuint* widthID,GLuint* directionID,GLuint* blurSizeID,GLuint* gammaID,GLuint* TextureID1,GLuint* TextureID2,GLuint* keypressedID,GLuint* mouseposxID,GLuint* mouseposyID);
void set_values(GLuint Texture1, GLuint TextureID1, GLuint Texture2, GLuint TextureID2, GLuint keypressedID, GLuint mouseposxID, GLuint mouseposyID, GLuint widthID, GLuint heightID, float width, float height, GLuint directionID, GLuint blurSizeID, GLuint gammaID, int start, GLuint timeID);
void draw_triangles(GLuint vertexbuffer, int g_vertex_buffer_size);
GLuint frameBuffer(float width, float height);
void renderToTexture(GLuint Framebuffer);

static const GLfloat g_vertex_buffer_data[] = {
   -1.0f, -1.0f, 0.0f,
   1.0f, -1.0f, 0.0f,
   -1.0f,  1.0f, 0.0f,
   -1.0,1.0,0.0,
   1.0,-1.0,0.0,
   1.0,1.0,0.0,

};

// Give the texture coordinates of the triangles

static const GLfloat g_txcoord_buffer_data[] = {
   0.0f, 0.0f,
   1.0f, 0.0f,
   0.0f, 1.0f,
   0.0f, 1.0f,
   1.0f, 0.0f,
   1.0f, 1.0f,
};

GLuint vertexbuffer;

GLuint programID, timeID,heightID,widthID,directionID,blurSizeID,gammaID,TextureID1,TextureID2,keypressedID,mouseposxID,mouseposyID;

GLuint Texture1, Texture2;
