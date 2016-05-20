#include "Camera.h"
#include "Model.h"
#include "SkyBox.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define ESCAPE 27
#define MSEC_TO_SEC 1000.0
#define MAX_SAMPLES 4

const std::vector<std::string> paths = {"/Users/rohansawhney/Desktop/developer/C++/walkthrough/loft/loft.txt",
                                        "/Users/rohansawhney/Desktop/developer/C++/walkthrough/campus/campus.txt"};
const std::string skyboxPath = "/Users/rohansawhney/Desktop/developer/C++/walkthrough/skybox/";

const std::string shaderPath = "/Users/rohansawhney/Desktop/developer/C++/walkthrough/shaders/";
const std::string modelVert = shaderPath + "model.vert";
const std::string modelFrag = shaderPath + "model.frag";
const std::string cullVert = shaderPath + "cull.vert";
const std::string cullGeom = shaderPath + "cull.geom";
const std::string normalVert = shaderPath + "normal.vert";
const std::string normalGeom = shaderPath + "normal.geom";
const std::string normalFrag = shaderPath + "normal.frag";
const std::string wireframeVert = shaderPath + "wireframe.vert";
const std::string wireframeGeom = shaderPath + "wireframe.geom";
const std::string wireframeFrag = shaderPath + "wireframe.frag";
const std::string skyboxVert = shaderPath + "skybox.vert";
const std::string skyboxFrag = shaderPath + "skybox.frag";

int gridX = 800;
int gridY = 600;
const float clipNear = 0.1;
const float clipFar = 1000;

Model model;
Skybox skybox;

Shader modelShader;
Shader cullShader;
Shader normalShader;
Shader wireframeShader;
Shader skyboxShader;

Camera camera;

int p = 0;
int frame = 0;
int elapsedTime = 0;
int baseTime = 0;
int lastTime = 0;
float dt = 0.0;
bool keys[256];
bool firstMouse = true;
float lastX = 0.0, lastY = 0.0;

GLuint transformUbo;
GLuint lightUbo;
GLuint fbo;

const Eigen::Vector3f lightPosition(0.0, 30.0, 30.0);
const Eigen::Vector3f lightColor(1.0, 1.0, 1.0);

bool success = true;
bool showNormals = false;
bool showWireframe = false;

void init()
{
    // enable depth test, blending and multisampling
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);

    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // create and bind framebuffers
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    // create a multisampled color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, gridX, gridY, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture, 0);
    
    // create a renderbuffer object for depth and stencil attachments
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MAX_SAMPLES, GL_DEPTH24_STENCIL8, gridX, gridY);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
}

void printInstructions()
{
    std::cerr << "→/←: switch between models\n"
              << "↑/↓ : scroll in/out\n"
              << "w/s: move in/out\n"
              << "a/d: move left/right\n"
              << "e/q: move up/down\n"
              << "n: toggle vertex normals\n"
              << "m: toggle wireframe\n"
              << "escape: exit program\n"
              << std::endl;
}

void setupTransformFeedback(const GLuint& program)
{
    const char *vars[] = { "modelMatrixRow1", "modelMatrixRow2", "modelMatrixRow3", "modelMatrixRow4" };
    glTransformFeedbackVaryings(program, 4, vars, GL_INTERLEAVED_ATTRIBS);
}

void setUniformBlocks()
{
    // 1) generate transform indices
    GLuint modelShaderIndex = glGetUniformBlockIndex(modelShader.program, "Transform");
    GLuint cullShaderIndex = glGetUniformBlockIndex(cullShader.program, "Transform");
    GLuint normalShaderIndex = glGetUniformBlockIndex(normalShader.program, "Transform");
    GLuint wireframeShaderIndex = glGetUniformBlockIndex(wireframeShader.program, "Transform");
    GLuint skyboxShaderIndex = glGetUniformBlockIndex(skyboxShader.program, "Transform");
    
    // bind
    glUniformBlockBinding(modelShader.program, modelShaderIndex, 0);
    glUniformBlockBinding(cullShader.program, cullShaderIndex, 0);
    glUniformBlockBinding(normalShader.program, normalShaderIndex, 0);
    glUniformBlockBinding(wireframeShader.program, wireframeShaderIndex, 0);
    glUniformBlockBinding(skyboxShader.program, skyboxShaderIndex, 0);
    
    // add transform data
    glGenBuffers(1, &transformUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, transformUbo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, transformUbo, 0, 2 * sizeof(glm::mat4));
    
    // 2) generate light index
    modelShaderIndex = glGetUniformBlockIndex(modelShader.program, "Light");
    
    // bind
    glUniformBlockBinding(modelShader.program, modelShaderIndex, 1);
    
    // add light data
    glGenBuffers(1, &lightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, lightUbo);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(Eigen::Vector4f), NULL, GL_STATIC_DRAW); // std140 alignment
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, lightUbo, 0, 2 * sizeof(Eigen::Vector4f));
    
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Eigen::Vector4f), lightPosition.data());
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Eigen::Vector4f), sizeof(Eigen::Vector4f), lightColor.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void updateUniformBlocks()
{
    // compute and set transformation matrices
    glm::mat4 projectionMatrix = glm::perspective(camera.fov, (float)gridX/(float)gridY, clipNear, clipFar);
    Eigen::Vector3f center = camera.pos + camera.dir;
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(camera.pos.x(), camera.pos.y(), camera.pos.z()),
                                       glm::vec3(center.x(), center.y(), center.z()),
                                       glm::vec3(camera.up.x(), camera.up.y(), camera.up.z()));
    
    glBindBuffer(GL_UNIFORM_BUFFER, transformUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMatrix));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewMatrix));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // set view position
    modelShader.use();
    glUniform3f(glGetUniformLocation(modelShader.program, "viewPosition"),
                camera.pos.x(), camera.pos.y(), camera.pos.z());
}

void updateTitle()
{
    frame++;
    elapsedTime = glutGet(GLUT_ELAPSED_TIME);
    dt = (elapsedTime - lastTime) / MSEC_TO_SEC;
    lastTime = elapsedTime;
    
    if (elapsedTime - baseTime > MSEC_TO_SEC) {
        std::string title = "FPS = " + std::to_string(frame * MSEC_TO_SEC / (elapsedTime - baseTime));
        glutSetWindowTitle(title.c_str());
        baseTime = elapsedTime;
        frame = 0;
    }
}

void display()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // update uniform blocks
    updateUniformBlocks();
        
    // draw skybox
    glDepthFunc(GL_LEQUAL);
    skybox.draw(skyboxShader);
    
    if (success) {
        // draw model
        glDepthFunc(GL_LESS);
        model.draw(modelShader, cullShader);
        
        // draw normals
        if (showNormals) model.draw(normalShader, cullShader);
        
        // draw wireframe
        if (showWireframe) model.draw(wireframeShader, cullShader);
    }
    
    // update title
    updateTitle();
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, gridX, gridY, 0, 0, gridX, gridY, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}

void keyboardPressed(unsigned char key, int x0, int y0)
{
    keys[key] = true;
    
    if (keys[ESCAPE]) {
        model.reset();
        skybox.reset();
        modelShader.reset();
        cullShader.reset();
        normalShader.reset();
        wireframeShader.reset();
        skyboxShader.reset();
        glDeleteBuffers(1, &transformUbo);
        glDeleteBuffers(1, &lightUbo);
        glDeleteFramebuffers(1, &fbo);
        exit(0);
        
    } else if (keys['a']) {
        camera.processKeyboard(LEFT, dt);
    
    } else if (keys['d']) {
        camera.processKeyboard(RIGHT, dt);
    
    } else if (keys['w']) {
        camera.processKeyboard(FORWARD, dt);
    
    } else if (keys['s']) {
        camera.processKeyboard(BACKWARD, dt);
        
    } else if (keys['e']) {
        camera.processKeyboard(UP, dt);
        
    } else if (keys['q']) {
        camera.processKeyboard(DOWN, dt);
        
    } else if (keys['n']) {
        showNormals = !showNormals;
    
    } else if (keys['m']) {
        showWireframe = !showWireframe;
    }
}

void keyboardReleased(unsigned char key, int x0, int y0)
{
    if (key != ESCAPE) keys[key] = false;
}

void mouse(int x, int y)
{
    if (firstMouse) {
        lastX = x;
        lastY = y;
        firstMouse = false;
    }
    
    float dx = x - lastX;
    float dy = lastY - y;
    
    lastX = x;
    lastY = y;
    
    camera.processMouse(dx, dy);
}

void special(int i, int x0, int y0)
{
    switch (i) {
        case GLUT_KEY_UP:
            camera.processScroll(1.0);
            break;
        case GLUT_KEY_DOWN:
            camera.processScroll(-1.0);
            break;
        case GLUT_KEY_LEFT:
            p--;
            if (p < 0) p = (int)paths.size()-1;
            model.reset();
            success = model.load(paths[p]);
            break;
        case GLUT_KEY_RIGHT:
            p++;
            if (p == (int)paths.size()) p = 0;
            model.reset();
            success = model.load(paths[p]);
            break;
    }
}

int main(int argc, char** argv)
{
    // TODO: occlusion culling
    // TODO: lods
    // TODO: weighted average transparency
    // TODO: shadows
    // TODO: ssao
    // TODO: normal mapping
    // TODO: parallex mapping
    // TODO: hdr
    // TODO: bloom
    // TODO: deferred shading
    
    InitializeMagick(*argv);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(gridX, gridY);
    
    std::stringstream title;
    title << "FPS = N/A";
    glutCreateWindow(title.str().c_str());
    
    init();
    printInstructions();
    
    modelShader.setup(modelVert, "", modelFrag); modelShader.link();
    cullShader.setup(cullVert, cullGeom, ""); setupTransformFeedback(cullShader.program); cullShader.link();
    normalShader.setup(normalVert, normalGeom, normalFrag); normalShader.link();
    wireframeShader.setup(wireframeVert, wireframeGeom, wireframeFrag); wireframeShader.link();
    skyboxShader.setup(skyboxVert, "", skyboxFrag); skyboxShader.link();
    setUniformBlocks();
    
    skybox.load(skyboxPath);
    success = model.load(paths[p]);
    
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboardPressed);
    glutKeyboardUpFunc(keyboardReleased);
    glutMotionFunc(mouse);
    glutSpecialFunc(special);
    glutMainLoop();
    
    return 0;
}
