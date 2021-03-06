#include "Model.h"
#include "SkyBox.h"
#include "Camera.h"

#define ESCAPE 27
#define MSEC_TO_SEC 1000.0
#define MAX_SAMPLES 4
#define NONE 0
#define FRUSTUM 1
#define OCCLUSION 2
#define DIGIT_OFFSET 48
#define MAX_ANGLE 180.0

const std::string path = "/Users/rohansawhney/Desktop/developer/C++/walkthrough";
const std::vector<std::string> paths = {path + "/loft/loft.txt", path + "/campus/campus.txt"};
const std::string skyboxPath = path + "/skybox/";
const std::string shaderPath = path + "/shaders/";

int gridX = 800;
int gridY = 600;

Model model;
Skybox skybox;
Camera camera;

Shader skyboxShader(shaderPath);
Shader hiZShader(shaderPath);
Shader cullShader(shaderPath);
Shader modelShader(shaderPath);
Shader normalShader(shaderPath);
Shader wireframeShader(shaderPath);
Shader depthShader(shaderPath);
Shader screenShader(shaderPath);

int p = 0;
int frame = 0;
int elapsedTime = 0;
int baseTime = 0;
int lastTime = 0;
int cullMode = FRUSTUM;
float mipLevel = 0;
float dt = 0.0;
float lastX = 0.0, lastY = 0.0;
bool keys[256];
bool firstMouse = true;

GLuint subroutineIndex[3];
GLuint transformUbo;
GLuint lightUbo;
GLuint screenVao;
GLuint screenVbo;
GLuint fbo;
GLuint screenFbo;
GLuint screenColorTexture;
GLuint screenDepthTexture;

double angleX = 0.0;
double angleZ = 0.0;
Eigen::Vector3f lightDirection(0.0, -1.0, 0.0);
const Eigen::Vector3f lightPosition(0.0, 50.0, 0.0);
const Eigen::Vector3f lightColor(1.0, 1.0, 1.0);

bool success = true;
bool showNormals = false;
bool showWireframe = false;
bool showDepth = false;

void setupMultisampledFramebuffer()
{
    // create multisampled color buffer texture
    GLuint colorTexture;
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MAX_SAMPLES, GL_RGBA8, gridX, gridY, GL_TRUE);
    
    // create multisampled depth buffer texture
    GLuint depthTexture;
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTexture);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MAX_SAMPLES, GL_DEPTH_COMPONENT24, gridX, gridY, GL_TRUE);
    
    // create multisampled framebuffer object
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupScreenFramebuffer()
{
    // create color buffer texture
    glGenTextures(1, &screenColorTexture);
    glBindTexture(GL_TEXTURE_2D, screenColorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gridX, gridY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // create depth buffer texture
    glGenTextures(1, &screenDepthTexture);
    glBindTexture(GL_TEXTURE_2D, screenDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, gridX, gridY, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // create screen fbo
    glGenFramebuffers(1, &screenFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, screenFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, screenDepthTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void setupTransformFeedback(const GLuint& program)
{
    const char *vars[] = {"modelMatrixRow1", "modelMatrixRow2", "modelMatrixRow3", "modelMatrixRow4"};
    glTransformFeedbackVaryings(program, 4, vars, GL_INTERLEAVED_ATTRIBS);
}

void initShaders()
{
    skyboxShader.setup("Skybox.vert", "", "Skybox.frag"); skyboxShader.link();
    hiZShader.setup("Screen.vert", "", "Hi-z.frag"); hiZShader.link();
    cullShader.setup("Cull.vert", "Cull.geom", ""); setupTransformFeedback(cullShader.program); cullShader.link();
    modelShader.setup("Model.vert", "", "Model.frag"); modelShader.link();
    normalShader.setup("Normal.vert", "Normal.geom", "Normal.frag"); normalShader.link();
    wireframeShader.setup("Normal.vert", "Wireframe.geom", "Wireframe.frag"); wireframeShader.link();
    depthShader.setup("Screen.vert", "", "Depth.frag"); depthShader.link();
    screenShader.setup("Screen.vert", "", "Screen.frag"); screenShader.link();
}

void setUniformBlocks()
{
    // 1) generate transform indices
    GLuint cullShaderIndex = glGetUniformBlockIndex(cullShader.program, "Transform");
    GLuint modelShaderIndex = glGetUniformBlockIndex(modelShader.program, "Transform");
    GLuint normalShaderIndex = glGetUniformBlockIndex(normalShader.program, "Transform");
    GLuint wireframeShaderIndex = glGetUniformBlockIndex(wireframeShader.program, "Transform");
    GLuint skyboxShaderIndex = glGetUniformBlockIndex(skyboxShader.program, "Transform");
    
    // bind
    glUniformBlockBinding(cullShader.program, cullShaderIndex, 0);
    glUniformBlockBinding(modelShader.program, modelShaderIndex, 0);
    glUniformBlockBinding(normalShader.program, normalShaderIndex, 0);
    glUniformBlockBinding(wireframeShader.program, wireframeShaderIndex, 0);
    glUniformBlockBinding(skyboxShader.program, skyboxShaderIndex, 0);
    
    // add transform data
    Eigen::Vector2f viewport(gridX, gridY);
    glGenBuffers(1, &transformUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, transformUbo);
    glBufferData(GL_UNIFORM_BUFFER, 2*sizeof(Eigen::Matrix4f)+sizeof(Eigen::Vector2f), NULL,
                 GL_DYNAMIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(Eigen::Matrix4f), sizeof(Eigen::Vector2f), &viewport);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, transformUbo);
    
    // 2) generate light index
    modelShaderIndex = glGetUniformBlockIndex(modelShader.program, "Light");
    
    // bind
    glUniformBlockBinding(modelShader.program, modelShaderIndex, 1);
    
    // add light data
    glGenBuffers(1, &lightUbo);
    glBindBuffer(GL_UNIFORM_BUFFER, lightUbo);
    glBufferData(GL_UNIFORM_BUFFER, 2*sizeof(Eigen::Vector4f), NULL, GL_STATIC_DRAW); // std140 alignment
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, lightUbo);
    
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Eigen::Vector4f), lightPosition.data());
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Eigen::Vector4f), sizeof(Eigen::Vector4f), lightColor.data());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void setupScreen()
{
    GLfloat screen[] = {
        // positions   // tex coords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
        1.0f, -1.0f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };
    
    // generate buffers
    glGenVertexArrays(1, &screenVao);
    glGenBuffers(1, &screenVbo);
    
    // bind and enable attributes
    glBindVertexArray(screenVao);
    glBindBuffer(GL_ARRAY_BUFFER, screenVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(screen), &screen, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);
}

void printInstructions()
{
    std::cerr << "space: change cull mode\n"
              << "→/←: switch between models\n"
              << "↑/↓ : scroll in/out\n"
              << "w/s: move in/out\n"
              << "a/d: move left/right\n"
              << "e/q: move up/down\n"
              << "b: toggle vertex normals\n"
              << "n: toggle wireframe\n"
              << "m: toggle depth\n"
              << "o/p: increment/decrement depth map mip level\n"
              << "escape: exit program\n"
              << std::endl;
}

void init()
{
    // enable depth test, blending and multisampling
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_BLEND);
    glEnable(GL_MULTISAMPLE);
    
    glClearColor(0, 0, 0, 0);
    glClearDepth(1.0);
    glDepthFunc(GL_LEQUAL);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    setupMultisampledFramebuffer();
    setupScreenFramebuffer();
    initShaders();
    setUniformBlocks();
    setupScreen();
    
    skybox.load(skyboxPath);
    success = model.load(paths[p]);
    if (success) {
        cullShader.use();
        subroutineIndex[0] = glGetSubroutineIndex(cullShader.program, GL_VERTEX_SHADER, "passThrough");
        subroutineIndex[1] = glGetSubroutineIndex(cullShader.program, GL_VERTEX_SHADER, "frustumCulling");
        subroutineIndex[2] = glGetSubroutineIndex(cullShader.program, GL_VERTEX_SHADER, "hiZOcclusionCulling");
    }
    
    printInstructions();
}

void updateUniformBlocks()
{
    // set transformation matrices
    glBindBuffer(GL_UNIFORM_BUFFER, transformUbo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4),
                    glm::value_ptr(camera.projectionMatrix(gridX, gridY)));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4),
                    glm::value_ptr(camera.viewMatrix()));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    // set view position
    modelShader.use();
    glUniform3f(glGetUniformLocation(modelShader.program, "viewPosition"),
                camera.pos.x(), camera.pos.y(), camera.pos.z());
}

void createHiZMap()
{
    hiZShader.use();
    glBindVertexArray(screenVao);
    
    // disable color buffer to render only depth image
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenDepthTexture);
    
    // disable depth testing but allow depth writes
    glDepthFunc(GL_ALWAYS);
    
    // calculate the number of mipmap levels for NPOT texture
    int numLevels = 1 + (int)floorf(log2f(fmaxf(gridX, gridY)));
    int currentWidth = gridX;
    int currentHeight = gridY;
    for (int i = 1; i < numLevels; i++) {
        glUniform2i(glGetUniformLocation(hiZShader.program, "lastMipSize"), currentWidth, currentHeight);
        
        // calculate next viewport size
        currentWidth /= 2;
        currentHeight /= 2;
        
        // ensure that the viewport size is always at least 1x1
        currentWidth = currentWidth > 0 ? currentWidth : 1;
        currentHeight = currentHeight > 0 ? currentHeight : 1;
        glViewport(0, 0, currentWidth, currentHeight);
        
        // bind next level for rendering but first restrict fetches only to previous level
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, i-1);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, i-1);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, screenDepthTexture, i);
        
        // draw command
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    // reset mipmap level range for the depth image
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numLevels-1);
    
    // reset the framebuffer configuration
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenColorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, screenDepthTexture, 0);
    
    // reenable color buffer writes, reset viewport and reenable depth test
    glDepthFunc(GL_LEQUAL);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glViewport(0, 0, gridX, gridY);
}

void drawScreen()
{
    glDisable(GL_DEPTH_TEST);
    
    if (showDepth) {
        depthShader.use();
        glUniform1f(glGetUniformLocation(depthShader.program, "mipLevel"), mipLevel);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenDepthTexture);
    
    } else {
        screenShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, screenColorTexture);
    }
    
    glBindVertexArray(screenVao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glEnable(GL_DEPTH_TEST);
}

void updateTitle()
{
    frame++;
    elapsedTime = glutGet(GLUT_ELAPSED_TIME);
    dt = (elapsedTime - lastTime) / MSEC_TO_SEC;
    lastTime = elapsedTime;
    
    if (elapsedTime - baseTime > MSEC_TO_SEC) {
        std::string title = "Fps: " + std::to_string(frame * MSEC_TO_SEC / (elapsedTime - baseTime)) +
                            "    Cull Mode: " + (cullMode == NONE ? "None" :
                                                (cullMode == FRUSTUM ? "Frustum" : "Occlusion")) +
                            "    Cull Ratio: " + std::to_string(model.cullRatio());
        if (showDepth && cullMode == OCCLUSION) title += ("    Mip Level: " + std::to_string((int)mipLevel));
        
        glutSetWindowTitle(title.c_str());
        baseTime = elapsedTime;
        frame = 0;
    }
}

void display()
{
    if (success) {
        // update uniform blocks
        updateUniformBlocks();
    
        // create hiZ map
        if (cullMode == OCCLUSION) {
            glBindFramebuffer(GL_FRAMEBUFFER, screenFbo);
            createHiZMap();
        }
    
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // cull
        model.cull(cullShader, subroutineIndex[cullMode]);
        
        // draw model
        model.draw(modelShader);
        if (showNormals) model.draw(normalShader, false);
        if (showWireframe) model.draw(wireframeShader, false);
        
        // draw skybox
        skybox.draw(skyboxShader);
    
        // blit multisampled buffer to normal color buffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFbo);
        glBlitFramebuffer(0, 0, gridX, gridY, 0, 0, gridX, gridY,
                          GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        // NOTE: not able to blit depth buffer for unknown reason
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // draw screen
        drawScreen();

        // update title
        updateTitle();
    
        glutSwapBuffers();
    }
}

void idle()
{
    glutPostRedisplay();
}

void resetShaders()
{
    skyboxShader.reset();
    hiZShader.reset();
    cullShader.reset();
    modelShader.reset();
    normalShader.reset();
    wireframeShader.reset();
    depthShader.reset();
    screenShader.reset();
}

void reset()
{
    model.reset();
    skybox.reset();
    resetShaders();
    glDeleteBuffers(1, &transformUbo);
    glDeleteBuffers(1, &lightUbo);
    glDeleteVertexArrays(1, &screenVao);
    glDeleteBuffers(1, &screenVbo);
    glDeleteTextures(1, &screenColorTexture);
    glDeleteTextures(1, &screenDepthTexture);
    glDeleteFramebuffers(1, &fbo);
    glDeleteFramebuffers(1, &screenFbo);
}

void keyboardPressed(unsigned char key, int x0, int y0)
{
    keys[key] = true;
    
    if (keys[' ']) {
        mipLevel = 0;
        cullMode = (cullMode + 1) % 3;
        
    } else  if (keys[ESCAPE]) {
        reset();
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
        
    } else if (keys['o']) {
        if (showDepth && cullMode == OCCLUSION && mipLevel > 0) mipLevel--;
        
    } else if (keys['p']) {
        if (showDepth && cullMode == OCCLUSION && mipLevel < floorf(log2f(fmaxf(gridX, gridY)))) mipLevel++;
        
    } else if (keys['k']) {
        p--;
        if (p < 0) p = (int)paths.size()-1;
        model.reset();
        success = model.load(paths[p]);
        
    } else if (keys['l']) {
        p++;
        if (p == (int)paths.size()) p = 0;
        model.reset();
        success = model.load(paths[p]);
        
    } else if (keys[DIGIT_OFFSET + 1]) {
        showNormals = !showNormals;
        
    } else if (keys[DIGIT_OFFSET + 2]) {
        showWireframe = !showWireframe;
        
    } else if (keys[DIGIT_OFFSET + 3]) {
        showDepth = !showDepth;
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

void rotateLightDir()
{
    Eigen::Matrix3f rotZ, rotX;
    rotZ << cos(angleX*DEG_TO_RAD), -sin(angleX*DEG_TO_RAD), 0,
    sin(angleX*DEG_TO_RAD), cos(angleX*DEG_TO_RAD), 0,
    0, 0, 1;
    rotX << 1, 0, 0,
    0, cos(angleZ*DEG_TO_RAD), -sin(angleZ*DEG_TO_RAD),
    0, sin(angleZ*DEG_TO_RAD), cos(angleZ*DEG_TO_RAD);
    
    lightDirection = rotZ * rotX * Eigen::Vector3f(0, -1, 0);
}

void special(int i, int x0, int y0)
{
    switch (i) {
        case GLUT_KEY_UP:
            if (angleX < MAX_ANGLE) angleX += 1.0;
            break;
        case GLUT_KEY_DOWN:
            if (angleX > -MAX_ANGLE) angleX -= 1.0;
            break;
        case GLUT_KEY_LEFT:
            if (angleZ > -MAX_ANGLE) angleZ -= 1.0;
            break;
        case GLUT_KEY_RIGHT:
            if (angleZ < MAX_ANGLE) angleZ += 1.0;
            break;
    }
    
    rotateLightDir();
}

int main(int argc, char** argv)
{
    // TODO: performance optimizations
    // 1) bug fix hiZ -> correctness
    // 2) minimize CPU GPU sync points
    // 3) lods
    
    // TODO: graphics
    // 1) shadows
    // 2) normal mapping
    // 3) parallex mapping
    // 4) hdr
    // 5) bloom
    // 6) deferred rendering
    // 7) ssao
    // 8) weighted average transparency
    
    InitializeMagick(*argv);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_3_2_CORE_PROFILE);
    glutInitWindowSize(gridX, gridY);
    
    std::stringstream title;
    title << ("Fps: N/A    Shadows: On    Cull Mode: Frustum    Cull Ratio: 1.0");
    glutCreateWindow(title.str().c_str());
    
    init();
    
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutKeyboardFunc(keyboardPressed);
    glutKeyboardUpFunc(keyboardReleased);
    glutMotionFunc(mouse);
    glutSpecialFunc(special);
    glutMainLoop();
    
    return 0;
}
