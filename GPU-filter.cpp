#include "GPU-filter.h"

int main(int argc,char *argv[])
{

// Choose mode: C (GLSL_used=0) or Shaders (GLSL_used= 1)
    int GLSL_used=1;

// Image size
    char* imagePath = argv[1];

    float width = BMP_width(imagePath)/2;
    float height = BMP_height(imagePath)/2;

// Initialize window: if returns 0 = ok, if returns -1 error
    initializeWindow(width,height, "Image filtering with GPU");

    /***** Prepare the data sent to the shaders *****/

// Make 2 triangles that fit in the window

    prepare_data(&vertexbuffer,g_vertex_buffer_data,g_txcoord_buffer_data,sizeof(g_vertex_buffer_data),sizeof(g_txcoord_buffer_data));

    /***** Create Program *****/

    create_program("Shaders/BD_frag.txt", &programID,GLSL_used, &timeID, &heightID, &widthID, &directionID, &blurSizeID, &gammaID, &TextureID1, &TextureID2, &keypressedID, &mouseposxID,&mouseposyID);

// Initialize textures
    glEnable(GL_TEXTURE_2D);

    Texture1 = BMPToTexture(imagePath);
    Texture2 = BMPToTexture(imagePath);


    /***** Display loop *****/

    int start =clock();

    do
    {

        /***** Set values uniform *****/
        if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            glDeleteTextures(1,&Texture1);
            Texture1=frameBuffer(width, height);
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        set_values(Texture1, TextureID1, Texture2, TextureID2, keypressedID, mouseposxID, mouseposyID, widthID, heightID, width, height, directionID, blurSizeID, gammaID, start, timeID);

        /***** Draw triangles ******/
        usleep(1000);
        draw_triangles(vertexbuffer, sizeof(g_vertex_buffer_data));

    }

// Check if the ESC key was pressed or the window was closed
    while( (glfwGetKey( GLFW_KEY_ESC ) != GLFW_PRESS &&
            glfwGetWindowParam( GLFW_OPENED ) ) );

    // Cleanup VBO
    glDeleteBuffers(1, &vertexbuffer);

// Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;


}


GLuint create_shaders(const char* vertex_file_path, const char* fragment_file_path)
{
    // Textures and program ID
    GLuint pixel_shader;
    GLuint vertex_shader;
    GLuint program;

    // Load shaders
    pixel_shader = LoadShader(GL_FRAGMENT_SHADER,fragment_file_path);
    vertex_shader = LoadShader(GL_VERTEX_SHADER,vertex_file_path);

    // Create program
    program = glCreateProgram();

    // Attach shaders to program
    glAttachShader(program,pixel_shader);
    glAttachShader(program,vertex_shader);

    // Bind it
    glBindAttribLocation(program, 0, "in_Vertex");
    glBindAttribLocation(program, 1, "in_TexCoord");

    // Link program
    glLinkProgram(program);
    verif_link(program);

    // Delete shaders
    glDeleteShader(pixel_shader);
    glDeleteShader(vertex_shader);

    return program;
}

char* LoadSource(const char *filename)
{
    //Shader source code
    char *src ;
    FILE *fp ;
    long size;
    long i;

    // Open file
    fp = fopen(filename, "rb");

    // Check if it worked
    if(fp == NULL)
    {
        fprintf(stderr, "Impossible to open file '%s'\n", filename);
        return NULL;
    }

    // File length
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);

    // Start of the file
    rewind(fp);

    //
    src = (char*)malloc(sizeof(char) * size+1);
    if(src == NULL)
    {
        fclose(fp);
        fprintf(stderr, "Memory allocation error!\n");
        return NULL;
    }

    // Read file
    fread (src, sizeof(char), size, fp);

    src[size] = '\0';

    fclose(fp);

    return src;
}

GLuint LoadShader(GLenum type, const char *filename)
{

    GLuint shader = 0;
    GLsizei logsize = 0;
    GLint compile_status = GL_TRUE;
    char *log = NULL;
    char *src = NULL;

    // Create shader
    shader = glCreateShader(type);
    if(shader == 0)
    {
        fprintf(stderr, "impossible de creer le shader\n");
        return 0;
    }

    // Load source
    src = LoadSource(filename);
    if(src == NULL)
    {
        glDeleteShader(shader);
        return 0;
    }

    // Create shader from source
    glShaderSource(shader, 1, (const GLchar**)&src, NULL);

    // Compile shader
    glCompileShader(shader);

    // Free memory
    free(src);
    src = NULL;

    // Check compilation success
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE)
    {
        // Compilation error
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);

        log = (char*)malloc(logsize + 1);
        if(log == NULL)
        {
            fprintf(stderr, "Impossible to allocate memory !\n");
            return 0;
        }

        memset(log, '\0', logsize + 1);

        glGetShaderInfoLog(shader, logsize, &logsize, log);
        fprintf(stderr, "Compilation error in %s: \n %s ",
                filename, log);

        // Free memory
        free(log);
        glDeleteShader(shader);

        return 0;
    }

    return shader;
}

int verif_link(GLuint program)
{

    GLsizei logsize = 0;
    GLint link_status = GL_TRUE;
    char *log = NULL;

    // Check linking success
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if(link_status != GL_TRUE)
    {

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logsize);

        log = (char*)malloc(logsize + 1);
        if(log == NULL)
        {
            fprintf(stderr, "Memory allocation impossible !\n");
            return 0;
        }
        memset(log, '\0', logsize + 1);

        glGetProgramInfoLog(program, logsize, &logsize, log);
        fprintf(stderr, "impossible de linker %s", log);

        // Free memory
        free(log);
        return 0;

    }
    return 0;
}

GLuint BMPToTexture(const char * imagepath)
{

    printf("Reading image %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;


    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file)
    {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
        return 0;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file)!=54 )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    // Make sure this is a 24bpp file
    if ( *(int*)&(header[0x1E])!=0  )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    if ( *(int*)&(header[0x1C])!=24 )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }

    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);

    // Some BMP files are misformatted, guess missing information
    if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
    if (dataPos==0)      dataPos=54; // The BMP header is done that way

    // Create a buffer
    data = new unsigned char [imageSize];

    // Read the actual data from the file into the buffer
    fread(data,1,imageSize,file);

    // Everything is in memory now, the file wan be closed
    fclose (file);




    glEnable(GL_TEXTURE_2D);

    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has now copied the data. Free our own version
    delete [] data;


    // Trilinear filtering.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Return the ID of the texture we just created
    return textureID;
}

int BMP_height(const char * imagepath)
{

    printf("Get height of %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file)
    {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
        return 0;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file)!=54 )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    // Make sure this is a 24bpp file
    if ( *(int*)&(header[0x1E])!=0  )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    if ( *(int*)&(header[0x1C])!=24 )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }

    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);
    return height;

}


int BMP_width(const char * imagepath)
{

    printf("Get width of %s\n", imagepath);

    // Data read from the header of the BMP file
    unsigned char header[54];
    unsigned int dataPos;
    unsigned int imageSize;
    unsigned int width, height;
    // Actual RGB data
    unsigned char * data;

    // Open the file
    FILE * file = fopen(imagepath,"rb");
    if (!file)
    {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
        return 0;
    }

    // Read the header, i.e. the 54 first bytes

    // If less than 54 bytes are read, problem
    if ( fread(header, 1, 54, file)!=54 )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    // A BMP files always begins with "BM"
    if ( header[0]!='B' || header[1]!='M' )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    // Make sure this is a 24bpp file
    if ( *(int*)&(header[0x1E])!=0  )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }
    if ( *(int*)&(header[0x1C])!=24 )
    {
        printf("Not a correct BMP file\n");
        return 0;
    }

    // Read the information about the image
    dataPos    = *(int*)&(header[0x0A]);
    imageSize  = *(int*)&(header[0x22]);
    width      = *(int*)&(header[0x12]);
    height     = *(int*)&(header[0x16]);
    return width;
}


GLuint frameBuffer(float width, float height)
{


// The texture we're going to render to

    GLuint renderedTexture;
    glGenTextures(1, &renderedTexture);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,renderedTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glCopyTexImage2D (GL_TEXTURE_2D,0,GL_RGB,0,0,width,height,0);
    glFlush();

    return renderedTexture;


}

int initializeWindow(float width, float height, const char * title)
{
    /***** Initialize GLFW *****/

    if (!glfwInit() )
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

// Ensure we can capture the escape key
    glfwEnable( GLFW_STICKY_KEYS );

// 4x antialiasing
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 4);

// Open a window and create its OpenGL context fitting the size of the image
    if( !glfwOpenWindow( width, height, 0,0,0,0, 32,0, GLFW_WINDOW ) )
    {
        fprintf( stderr, "Failed to open GLFW window\n" );
        glfwTerminate();
        return -1;
    }

// Change window title
    glfwSetWindowTitle( title );

// Specify clear values for the color buffers: Grey
    glClearColor(0.43f, 0.43f, 0.43f, 0.0f);

    /***** Initialize GLEW *****/
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    return 0;

}

void prepare_data(GLuint* vertexbuffer,const GLfloat* g_vertex_buffer_data,const GLfloat* g_txcoord_buffer_data,int vertex_data_size,int txcoord_data_size)
{

// Create Vertex Buffer Object
    glGenBuffers(1, vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, *vertexbuffer);

// Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, vertex_data_size+txcoord_data_size, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,vertex_data_size, g_vertex_buffer_data);
    glBufferSubData(GL_ARRAY_BUFFER,vertex_data_size ,txcoord_data_size, g_txcoord_buffer_data);

}

void create_program(const char * shaderpath,GLuint* programID,int GLSL_used,GLuint* timeID,GLuint* heightID,GLuint* widthID,GLuint* directionID,GLuint* blurSizeID,GLuint* gammaID,GLuint* TextureID1,GLuint* TextureID2,GLuint* keypressedID,GLuint* mouseposxID,GLuint* mouseposyID)
{

// If we use shaders, we apply filters
    if (GLSL_used==1)
    {
        *programID = create_shaders( "Shaders/texture_vert.txt",shaderpath );
    }

// If we don't use shaders, we only display the image already filtered by the CPU
    else
    {
        *programID = create_shaders( "Shaders/texture_vert.txt","Shaders/texture_frag.txt" );
    }

    glUseProgram(*programID);

// Get the location of uniform variables
    *timeID = glGetUniformLocation(*programID, "time");
    *heightID = glGetUniformLocation(*programID, "height");
    *widthID = glGetUniformLocation(*programID, "width");
    *directionID = glGetUniformLocation(*programID, "direction");
    *blurSizeID = glGetUniformLocation(*programID, "blurSize");
    *gammaID = glGetUniformLocation(*programID, "gamma");
    *TextureID1  = glGetUniformLocation(*programID, "texture1");
    *TextureID2  = glGetUniformLocation(*programID, "texture2");
    *keypressedID = glGetUniformLocation(*programID, "keypressed");
    *mouseposxID = glGetUniformLocation(*programID, "mouseposx");
    *mouseposyID = glGetUniformLocation(*programID, "mouseposy");

}

void set_values(GLuint Texture1, GLuint TextureID1, GLuint Texture2, GLuint TextureID2, GLuint keypressedID, GLuint mouseposxID, GLuint mouseposyID, GLuint widthID, GLuint heightID, float width, float height, GLuint directionID, GLuint blurSizeID, GLuint gammaID, int start, GLuint timeID)
{

    // Link textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture1);
    glUniform1i(TextureID1, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, Texture2);
    glUniform1i(TextureID2, 1);

    // Specify the value of uniform variables

    // Indicates if mouse is clicked
    if (glfwGetMouseButton( GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS)
    {
        glUniform1f(keypressedID, 1.0);
    }

    else
    {
        glUniform1f(keypressedID, 0.0);
    }

    // Mouse position
    int xpos, ypos;
    glfwGetMousePos(&xpos, &ypos);
    glUniform1f(mouseposxID, xpos);
    glUniform1f(mouseposyID, ypos);

    // Size of the image
    glUniform1f(widthID, width);
    glUniform1f(heightID, height);
    // Blur info
    glUniform1f(directionID, 1.5);
    glUniform1f(blurSizeID, 1/512.0);
    // Gamma info
    glUniform1f(gammaID, 2.2);
    // actual time since loop start
    glUniform1f(timeID, clock() - start);

}

void draw_triangles(GLuint vertexbuffer, int g_vertex_buffer_size)
{
    // 1rst attribute buffer : Vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

// 2nd attribute buffer : Texture
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        1,                  // attribute 1
        2,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        BUFFER_OFFSET(g_vertex_buffer_size)            // array buffer offset

    );

// Draw the triangles
    glDrawArrays(GL_TRIANGLES, 0, 6); // Starting from vertex 0; 6 vertices total -> 2 triangles

// Disable vertex attribute arrays
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

// Swap buffers
    glfwSwapBuffers();

}

