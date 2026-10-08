#define FREEGLUT_STATIC
#include <GL/freeglut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string.h> 
#include <string>

using std::vector;
using std::string;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 1. GLOBAL CONFIG & STATE
int winWidth = 1500, winHeight = 750;
float btnX, btnY, btnW, btnH; 
bool btnHover = false;
bool showCyberHUD = true;     
bool isRaining = true;       

enum GameState {
    STATE_INTRO, STATE_ROOM, STATE_SCREEN_FLASH,
    STATE_TRANSITION, STATE_CYBER,
};
GameState currentState = STATE_INTRO;

bool isHyperWarp = false;
float warpFOV = 60.0f;      
float warpSpeedMult = 1.0f; 

enum TimeOfDay { TIME_DAY, TIME_SUNSET, TIME_NIGHT };
TimeOfDay currentTimeOfDay = TIME_NIGHT; 

float realTime = 0.0f;          
float introStartTime = 0.0f;
float transitionStartTime = 0.0f;
float cyberStartTime = 0.0f;

const float SCREEN_FLASH_DURATION = 0.75f;
const float TRANSITION_DURATION = 1.6f;


// 2. CAMERA & CONTROLS
bool isCloudMode = false;      
float cyberCamYaw = 0.0f;      
float cyberCamPitch = 10.0f;   
float camDist = 40.0f;         
float camBaseHeight = 6.f;    

float camYaw = 0.0f;
float camPitch = 0.0f;


// 3. SCENE CONFIGURATION
bool isTransparentFloor = false; 
float worldOffsetZ = 0.0f;     
float globalTimeCat = 0.0f;    
int dynamicLightIndex = 0;     

const float MAX_SPEED = 1.9f;
const float SEGMENT_SPACING = 60.0f;
const int SEGMENTS_TO_DRAW = 60;
const float LOOP_LEN = SEGMENT_SPACING * SEGMENTS_TO_DRAW;
const float SEGMENT_WIDTH_BASE = 30.0f;
const float ROAD_TOTAL_WIDTH = SEGMENT_WIDTH_BASE * 2.0f;
const float CULL_FRONT = 200.0f;
const float CULL_BACK = -400.0f;

float laneCenters[] = { -26.0f, -14.0f, 14.0f, 26.0f };
float stripPositions[] = { -35.0f, -20.0f, -8.0f, 8.0f, 20.0f, 35.0f };


// 4. PARTICLE SYSTEMS (Rain & Ripple)
struct RainDrop {
    float x, y, z;
    float speed;
    int type; 
    float randomOffset; 
};
vector<RainDrop> rainSystem;
const int MAX_RAIN = 1050;
struct Ripple {
    float x, z;
    float life; 
    float maxRadius;
};
vector<Ripple> ripples;

// 5. TEXTURE MANAGER
struct TextureManager {
    // Room Scene
    GLuint wall, keyboard, frameLeft, frameRight, floor, screen, flash, boxSide;

    // Cyber Scene
    GLuint vortex;
    GLuint bgDay, bgSunset, bgNight;
    GLuint skyTopDay, skyTopSunset, skyTopNight;
    GLuint floorDay, floorSunset, floorNight;

    GLuint cyberPavement;  
    GLuint skyTrain;      

    GLuint walls[4];
    GLuint roadMain;      
    GLuint galaxy;
    GLuint screen2;
} tex;


// 6. COLOR PALETTE
// Cyber Neon 
const GLfloat COLOR_CYBER_PURPLE[] = { 0.6f, 0.0f, 1.0f };
const GLfloat COLOR_CYBER_CYAN[] = { 0.0f, 0.9f, 1.0f };
const GLfloat COLOR_CYBER_PINK[] = { 1.0f, 0.0f, 0.6f };
const GLfloat COLOR_NEON_RED[] = { 1.0f, 0.0f, 0.2f };
const GLfloat COLOR_NEON_ORANGE[] = { 1.0f, 0.5f, 0.0f };
const GLfloat COLOR_NEON_YELLOW[] = { 0.9f, 1.0f, 0.1f };
const GLfloat COLOR_NEON_GREEN[] = { 0.1f, 1.0f, 0.2f };
const GLfloat COLOR_NEON_YELLOW_ALT[] = { 1.0f, 0.9f, 0.1f };
const GLfloat COLOR_NEON_GREEN_ALT[] = { 0.1f, 1.0f, 0.3f };
const GLfloat COLOR_NEON_CYAN_ALT[] = { 0.0f, 0.8f, 1.0f };
const GLfloat COLOR_NEON_BLUE_ALT[] = { 0.2f, 0.1f, 1.0f };

// Basic Colors 
const GLfloat COLOR_WHITE[] = { 1.0f, 1.0f, 1.0f };
const GLfloat COLOR_BLACK[] = { 0.0f, 0.0f, 0.0f };
const GLfloat COLOR_PURE_BLACK[] = { 0.1f, 0.1f, 0.1f };
const GLfloat COLOR_DARK_GREY[] = { 0.2f, 0.2f, 0.25f };
const GLfloat COLOR_DEEP_GREY[] = { 0.15f, 0.15f, 0.20f };
const GLfloat COLOR_CONCRETE[] = { 0.7f, 0.7f, 0.75f };
const GLfloat COLOR_SILVER[] = { 0.75f, 0.75f, 0.80f };
const GLfloat COLOR_WARM_GREY[] = { 0.75f, 0.65f, 0.6f };
const GLfloat COLOR_WARM_LIGHT[] = { 0.8f, 0.6f, 0.2f };

// Nature & Vehicle 
const GLfloat COLOR_TREE_GREEN[] = { 0.1f, 0.6f, 0.1f };
const GLfloat COLOR_GRASS_GREEN[] = { 0.2f, 0.7f, 0.2f };
const GLfloat COLOR_TRAIN_RED[] = { 1.0f, 0.1f, 0.1f };
const GLfloat COLOR_TRAIN_BLUE[] = { 0.1f, 0.3f, 1.0f };
const GLfloat COLOR_CYBER_GREEN_ALT[] = { 0.1f, 0.9f, 0.1f };

// Cyber Industrial Colors 
const GLfloat COLOR_BUILD_CONCRETE[] = { 0.65f, 0.65f, 0.67f }; 
const GLfloat COLOR_BUILD_STEEL[] = { 0.45f, 0.50f, 0.55f };   
const GLfloat COLOR_BUILD_DARK[] = { 0.20f, 0.20f, 0.23f };   
const GLfloat COLOR_BUILD_BRONZE[] = { 0.45f, 0.38f, 0.32f };   
const GLfloat COLOR_BUILD_NAVY[] = { 0.15f, 0.20f, 0.30f };    
const GLfloat COLOR_BUILD_WHITE[] = { 0.85f, 0.88f, 0.90f };    
const GLfloat COLOR_L_TITANIUM[] = { 0.92f, 0.92f, 0.94f };    
const GLfloat COLOR_L_PLATINUM[] = { 0.80f, 0.82f, 0.85f };    
const GLfloat COLOR_L_ICE_BLUE[] = { 0.70f, 0.85f, 0.95f };    
const GLfloat COLOR_L_CERAMIC[] = { 0.95f, 0.92f, 0.88f };      
const GLfloat COLOR_L_MINT[] = { 0.75f, 0.90f, 0.82f };         
const GLfloat COLOR_L_LAVENDER[] = { 0.85f, 0.80f, 0.92f };   

// UI Colors
const GLfloat COLOR_UI_BORDER[] = { 0.0f, 0.8f, 1.0f };
const GLfloat COLOR_UI_BG[] = { 0.0f, 0.1f, 0.3f };
const GLfloat COLOR_UI_YELLOW[] = { 1.0f, 0.9f, 0.1f };

// Color Collections
const GLfloat* RAINBOW_COLORS[] = {
    COLOR_NEON_RED, COLOR_NEON_ORANGE, COLOR_NEON_YELLOW_ALT, COLOR_NEON_GREEN_ALT, COLOR_NEON_CYAN_ALT, COLOR_NEON_BLUE_ALT
};

const GLfloat* BUILD_COLORS_MACARON[] = {
    COLOR_L_TITANIUM, COLOR_BUILD_STEEL, COLOR_BUILD_DARK, COLOR_L_MINT, COLOR_BUILD_NAVY, COLOR_L_ICE_BLUE,    
};

const char* CYBER_TEXTS[] = {
   "NEKO MART",        "SKY DINING",      "2001: A Space Odyssey",
    "STAR VOYAGE",         "CYBER CAFE",
    "FOX ICE CREAM",    "RABBIT POLICE",
    "Fear and Dreams",
    "Data Library"
};

// Basic Utilities
/**
 * @brief Loads a BMP texture.
 *
 * @param fileName Path to BMP file.
 * @param repeat Use GL_REPEAT if true, else GL_CLAMP.
 * @param linear Use GL_LINEAR if true, else GL_NEAREST.
 * @return Texture ID or 0 on error.
 */
GLuint LoadTexture(const char* fileName, bool repeat = false, bool linear = false) {
    FILE* pfile = nullptr;
    fopen_s(&pfile, fileName, "rb"); 

    if (!pfile) {
        printf("Error: Texture file not found: %s\n", fileName);
        return 0;
    }

    GLint width, height;
    fseek(pfile, 18, SEEK_SET);
    fread(&width, sizeof(width), 1, pfile);
    fread(&height, sizeof(height), 1, pfile);
    GLint lineBytes = width * 3;
    while (lineBytes % 4 != 0) lineBytes++;
    GLint totalBytes = lineBytes * height;
    GLubyte* pixeldata = (GLubyte*)malloc(totalBytes);
    if (!pixeldata) { fclose(pfile); return 0; }
    fseek(pfile, 54, SEEK_SET);
    fread(pixeldata, totalBytes, 1, pfile);
    fclose(pfile);

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    float filter = linear ? GL_LINEAR : GL_NEAREST;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);

    float wrap = repeat ? GL_REPEAT : GL_CLAMP;
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, pixeldata);
    free(pixeldata);
    printf("Loaded: %s\n", fileName);
    return textureID;
}

/**
 * @brief Initializes all application textures.
 */
void initAllTextures() {
    tex.wall = LoadTexture("wall_f.bmp");
    tex.keyboard = LoadTexture("keyboard.bmp");
    tex.frameLeft = LoadTexture("frame.bmp");
    tex.floor = LoadTexture("back.bmp", true); 
    tex.screen = LoadTexture("screen1.bmp");
    tex.screen2 = LoadTexture("screen2.bmp");
    tex.flash = LoadTexture("loading.bmp");
    tex.boxSide = LoadTexture("wall_rl.bmp");
    tex.vortex = LoadTexture("vortex.bmp", true, true);

    tex.bgDay = LoadTexture("day.bmp");
    tex.bgSunset = LoadTexture("sunset.bmp");
    tex.bgNight = LoadTexture("night.bmp");
    tex.skyTopSunset = LoadTexture("sunset_sky.bmp", false, true);
    tex.cyberPavement = LoadTexture("pavement.bmp", true);
    tex.skyTrain = LoadTexture("train.bmp");
    tex.walls[0] = LoadTexture("wall0.bmp");
    tex.walls[1] = LoadTexture("wall1.bmp");
    tex.walls[2] = LoadTexture("wall2.bmp");
    tex.walls[3] = LoadTexture("wall3.bmp");
    tex.roadMain = LoadTexture("road.bmp", true, true);
}
void setNeon(const GLfloat* color, float intensity = 1.0f) {
    GLfloat matEmit[] = { color[0] * intensity, color[1] * intensity, color[2] * intensity, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, matEmit); glColor3fv(color);
}
void setSolid(const GLfloat* color) {
    GLfloat noEmit[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmit);
    GLfloat matAmbDiff[] = { color[0], color[1], color[2], 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, matAmbDiff);
    glColor3fv(color);
}
void drawBox(float w, float h, float d) {
    glPushMatrix();
    glScalef(w, h, d);
    glutSolidCube(1.0f);
    glPopMatrix();
}
/**
 * @brief Renders a box with texture coordinates.
 *
 * @param w Width.
 * @param h Height.
 * @param d Depth.
 * @param texID Texture ID (0 to disable).
 */
void drawTexturedBox(float w, float h, float d, GLuint texID) {
    float hw = w / 2.0f;
    float hh = h / 2.0f;
    float hd = d / 2.0f;

    if (texID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texID);
        glColor3f(1.0f, 1.0f, 1.0f);
    }

    glBegin(GL_QUADS);

    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, -hh, hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, hd);

    glNormal3f(0.0f, 0.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-hw, -hh, -hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-hw, hh, -hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(hw, hh, -hd);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hw, -hh, -hd);

    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, -hh, -hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-hw, -hh, hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-hw, hh, hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, -hd);

    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hw, -hh, -hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, hh, -hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(hw, -hh, hd);

    glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-hw, hh, -hd);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-hw, hh, hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(hw, hh, hd);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(hw, hh, -hd);

    glNormal3f(0.0f, -1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-hw, -hh, -hd);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(hw, -hh, -hd);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(hw, -hh, hd);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-hw, -hh, hd);

    glEnd();

    if (texID != 0) glDisable(GL_TEXTURE_2D);
}
void initRain() {
    for (int i = 0; i < MAX_RAIN; i++) {
        RainDrop r;
        r.x = (rand() % 3000 / 10.0f) - 100.0f; 
        r.y = (rand() % 1000 / 5.0f);         
        r.z = (rand() % 3000 / 10.0f) - 100.0f;
        r.speed = 0.5f + (rand() % 100 / 100.0f) * 0.5f;
        r.type = rand() % 3;
        r.randomOffset = (float)(rand() % 100);

        int chance = rand() % 100;
        if (chance < 15) {
            r.type = 0;
        }
        else if (chance < 55) {
            r.type = 1;
        }
        else {
            r.type = 2;
        }
        rainSystem.push_back(r);
    }
}
// For both room and cyber
/**
 * @brief Draws an animated surveillance drone.
 * * Features rotating rotors, a scanning red eye, and day/night color adaptation.
 */
void drawDrone() {
    const GLfloat* bodyCol = (currentTimeOfDay == TIME_DAY) ? COLOR_WHITE : COLOR_DARK_GREY;
    const GLfloat* glowCol = COLOR_CYBER_CYAN;

    setSolid(bodyCol);
    glPushMatrix();
    glScalef(1.0f, 0.6f, 1.0f);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.6f);

    float scanAngle = sinf(realTime * 3.0f) * 30.0f;
    glRotatef(scanAngle, 0.0f, 1.0f, 0.0f);

    setSolid(COLOR_DARK_GREY);
    glutSolidSphere(0.25f, 10, 10);

    glTranslatef(0.0f, 0.0f, 0.2f);
    setNeon(COLOR_NEON_RED, 3.0f);
    glutSolidSphere(0.12f, 8, 8);

    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);

    if (currentTimeOfDay == TIME_DAY) glColor4f(1.0f, 0.0f, 0.0f, 0.6f);
    else glColor4f(1.0f, 0.0f, 0.0f, 0.3f);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -3.f);
    glScalef(1.0f, 1.0f, 2.0f);
    glutSolidCone(0.35f, 0.5f, 10, 2);
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glPopMatrix();

    setNeon(glowCol, 2.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex3f(-1.5f, 0.0f, -1.5f); glVertex3f(1.5f, 0.0f, 1.5f);
    glVertex3f(-1.5f, 0.0f, 1.5f); glVertex3f(1.5f, 0.0f, -1.5f);
    glEnd();

    float rotorSpeed = realTime * 2000.0f;

    for (int x = -1; x <= 1; x += 2) {
        for (int z = -1; z <= 1; z += 2) {
            glPushMatrix();
            glTranslatef(x * 1.5f, 0.1f, z * 1.5f);

            setNeon(COLOR_CYBER_CYAN, 2.0f);

            glPushMatrix();
            glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
            glutWireTorus(0.05f, 0.6f, 6, 20);
            glPopMatrix();

            setSolid(COLOR_DARK_GREY);
            glRotatef(rotorSpeed + x * z * 90.0f, 0.0f, 1.0f, 0.0f);
            glScalef(1.0f, 0.1f, 0.1f);
            glutSolidCube(1.0f);
            glPopMatrix();
        }
    }
}
/**
 * @brief Renders a holographic object in various styles.
 * Adapt color visibility based on the current time of day.
 *
 * @param size Scale factor.
 * @param style 0=Scanning Cube, 1=Atom Rings, 2=Inverted Pyramid, 3=Cyber Heart.
 */
void drawHologram(float size, int style) {
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);

    glPushMatrix();

    glRotatef(realTime * 40.0f, 0.0f, 1.0f, 0.0f);

    float pulse = 0.6f + 0.4f * sinf(realTime * 3.0f);

    float r, g, b, alpha;
    if (currentTimeOfDay == TIME_DAY) {
        if (style % 2 == 0) { r = 0.0f; g = 0.2f; b = 0.8f; }
        else { r = 0.4f; g = 0.0f; b = 0.6f; }
        alpha = 0.4f * pulse;
    }
    else if (currentTimeOfDay == TIME_SUNSET) {
        if (style % 2 == 0) { r = 1.0f; g = 0.5f; b = 0.0f; }
        else { r = 1.0f; g = 0.8f; b = 0.2f; }
        alpha = 0.4f * pulse;
    }
    else {
        if (style % 2 == 0) { r = 0.0f; g = 0.9f; b = 1.0f; }
        else { r = 1.0f; g = 0.0f; b = 0.6f; }
        alpha = 0.4f * pulse;
    }
    glColor4f(r, g, b, alpha);

    if (style == 0) {
        for (float y = -0.5f; y <= 0.5f; y += 0.2f) {
            glPushMatrix();
            glTranslatef(0.0f, y * size, 0.0f);
            glScalef(size, 0.1f, size);
            glutWireCube(1.0f);
            glPopMatrix();
        }
        glPushMatrix();
        glScalef(size * 0.4f, size * 0.4f, size * 0.4f);
        glRotatef(realTime * 100.0f, 1.0f, 1.0f, 0.0f);
        glutWireOctahedron();
        glPopMatrix();
    }
    else if (style == 1) {
        glutWireSphere(size * 0.2f, 10, 10);

        for (int i = 0; i < 3; i++) {
            glPushMatrix();
            glRotatef(i * 60.0f + realTime * 50.0f, 0.0f, 0.0f, 1.0f);
            glScalef(1.0f, 0.3f, 1.0f);
            glutWireTorus(0.05f * size, size * 0.8f, 5, 30);
            glPopMatrix();
        }
    }
    else if (style == 2) {
        glPushMatrix();
        glTranslatef(0.0f, size / 2.0f - 17.f, 0.0f);
        glRotatef(-90.0f + sinf(realTime) * 20.0f, 1.0f, 0.0f, 0.0f);
        glutWireCone(size * 0.6f, size * 1.5f, 4, 10);
        glPopMatrix();
    }
    else {
        glPushMatrix();
        glScalef(size, size, size);
        glPushMatrix(); glTranslatef(-0.35f, 0.f, 0.0f); glutWireSphere(0.35f, 8, 8); glPopMatrix();
        glPushMatrix(); glTranslatef(0.35f, 0.f, 0.0f); glutWireSphere(0.35f, 8, 8); glPopMatrix();
        glPushMatrix(); glTranslatef(0.0f, -0.3f, 0.0f); glRotatef(90, 1, 0, 0); glutWireCone(0.48f, 0.9f, 10, 5); glPopMatrix();
        glPopMatrix();
    }

    glPopMatrix();

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}
/**
 * @brief Renders a holographic "cyber" screen.
 * Adapts brightness and blending for day/night cycles.
 *
 * @param w Screen width.
 * @param h Screen height.
 * @param color Base RGB neon color.
 * @param type 0=2D Geom, 1=Text, 2=3D Geom.
 * @param fixedTextIdx Optional index for fixed text display (default -1).
 */
void drawCyberScreen(float w, float h, const GLfloat* color, int type, int fixedTextIdx = -1) {
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    float bgAlpha, fgAlpha;
    GLfloat drawCol[3] = { color[0], color[1], color[2] };

    if (currentTimeOfDay == TIME_DAY) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        drawCol[0] *= 1.2f;
        drawCol[1] *= 1.2f;
        drawCol[2] *= 1.2f;

        bgAlpha = 0.3f;
        fgAlpha = 0.8f;
    }
    else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        bgAlpha = 0.20f;
        fgAlpha = 0.8f;
    }

    glPushMatrix();

    glColor4f(drawCol[0], drawCol[1], drawCol[2], bgAlpha);

    glBegin(GL_QUADS);
    glVertex3f(-w / 2, -h / 2, 0); glVertex3f(w / 2, -h / 2, 0);
    glVertex3f(w / 2, h / 2, 0);   glVertex3f(-w / 2, h / 2, 0);
    glEnd();

    float time = realTime * 0.15f;
    int numScanlines = 7;

    glColor4f(drawCol[0], drawCol[1], drawCol[2], 0.5f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < numScanlines; i++) {
        float offset = (float)i / numScanlines;
        float yRaw = fmod(time + offset, 1.0f);
        float yPos = -h / 2.0f + yRaw * h;
        glVertex3f(-w / 2, yPos, 0.01f);  glVertex3f(w / 2, yPos, 0.01f);
    }
    glEnd();

    glColor4f(drawCol[0], drawCol[1], drawCol[2], fgAlpha);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(-w / 2, -h / 2, 0); glVertex3f(w / 2, -h / 2, 0);
    glVertex3f(w / 2, h / 2, 0);   glVertex3f(-w / 2, h / 2, 0);
    glEnd();

    if (type == 1) {
        glPushMatrix();
        float textScale = w * 0.0007f;
        glLineWidth(2.0f);
        glColor4f(drawCol[0], drawCol[1], drawCol[2], fgAlpha);

        int textIdx;
        if (fixedTextIdx != -1) {
            textIdx = fixedTextIdx;
        }
        else {
            textIdx = (int)(w * 10) % 9;
        }

        if (textIdx < 0) textIdx = 0;
        if (textIdx > 8) textIdx = 8;

        const char* str = CYBER_TEXTS[textIdx];

        float totalTextW = strlen(str) * 90.0f * textScale;
        glTranslatef(-totalTextW / 2.0f, -h / 4.0f, 0.0f);
        glScalef(textScale, textScale, 1.0f);

        for (const char* c = str; *c; c++) glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
        glPopMatrix();
    }
    else if (type == 0) {
        glPushMatrix();
        glTranslatef(0, 0, 0.01f);
        int geoStyle = (int)(w) % 3;
        glColor4f(drawCol[0], drawCol[1], drawCol[2], fgAlpha);

        if (geoStyle == 0) {
            glScalef(0.8f, 0.8f, 1.0f);
            glutWireTorus(w * 0.025f, w * 0.45f, 5, 30);
            glutWireTorus(w * 0.02f, w * 0.25f, 5, 20);
        }
        else if (geoStyle == 1) {
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i < 15; i++) {
                float x = -w / 2 + (w / 15) * i;
                float y = (rand() % 100 / 100.0f - 0.5f) * h * 0.8f;
                glVertex3f(x, y, 0);
            }
            glEnd();
        }
        glPopMatrix();
    }
    else if (type == 2) {
        glPushMatrix();
        glTranslatef(0, 0, h * 0.1f);
        glRotatef(realTime * 60.0f, 0.5f, 1.0f, 0.0f);
        int shape = (int)w % 2;

        glLineWidth(2.0f);
        glColor4f(drawCol[0], drawCol[1], drawCol[2], 0.40f);

        if (shape == 0) {
            glRotatef(realTime * 40.0f, 1, 1, 0);
            glutWireCube(h * 0.6f);
            glutWireCube(h * 0.45f);
            glScalef(0.4f, 0.4f, 0.4f);
            glColor4f(drawCol[0], drawCol[1], drawCol[2], 0.6f);
            glutSolidCube(h * 0.7f);
        }
        else {
            glScalef(h * 0.6f, h * 0.6f, h * 0.8f);
            glutWireTetrahedron();
            glScalef(0.5f, 0.5f, 0.5f);
            glColor4f(drawCol[0], drawCol[1], drawCol[2], 0.20f);
            glutSolidTetrahedron();
        }
        glPopMatrix();
    }
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}
/**
 * @brief Renders ambient dust particles inside the room.
 */
void drawRoomDust() {
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); 
    glEnable(GL_POINT_SMOOTH);         

    float pulse = 1.5f + 0.5f * sinf(realTime * 3.0f);
    glPointSize(2.0f * pulse);      

    glColor4f(1.f, 0.3f, 0.6f, 0.8f);

    srand(50);
    for (int i = 0; i < 800; i++) {
        float rx = (rand() % 1000 / 100.0f) - 5.0f;
        float ry = (rand() % 500 / 100.0f);
        float rz = (rand() % 1000 / 100.0f) - 5.0f;

        float hover = sinf(realTime * 0.5f + i) * 0.1f;

        glBegin(GL_POINTS);
        glVertex3f(rx, ry + hover, rz);
        glEnd();
    }

    glDisable(GL_POINT_SMOOTH);
    glPointSize(1.0f);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

/**
 * @brief Renders environmental particles with day/night colors and warp effects.
 * Changes from points to streaks when hyper warp is active.
 */
void drawFloatingParticles() {
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    const GLfloat colsDay[3][3] = {
        { 1.0f, 1.0f, 1.0f },
        { 0.2f, 0.6f, 1.0f },
        { 0.4f, 1.0f, 0.5f }
    };

    const GLfloat colsSunset[3][3] = {
        { 1.0f, 0.9f, 0.1f },
        { 1.0f, 0.6f, 0.0f },
        { 1.0f, 0.2f, 0.2f }
    };

    const GLfloat colsNight[3][3] = {
        { 0.7f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 0.8f }
    };

    srand(123);
    for (int i = 0; i < 1350; i++) {
        float range = 350.0f;
        float x = (rand() % 1000 / 1000.0f - 0.5f) * range;
        float y = (rand() % 1000 / 1000.0f) * 75.0f;
        float zBase = (rand() % 1000 / 1000.0f - 0.5f) * range;

        float speed = ((rand() % 100 / 100.0f) * 5.0f + 2.0f) * (isHyperWarp ? 5.0f : 1.0f);
        float zMove = zBase + realTime * speed;

        float zRel = zMove - worldOffsetZ;
        while (zRel > range / 2) zRel -= range;
        while (zRel < -range / 2) zRel += range;
        glPushMatrix();
        glTranslatef(x, y, zRel);
        const GLfloat* useCol;
        int colorIdx = i % 3;
        if (currentTimeOfDay == TIME_DAY) {
            useCol = colsDay[colorIdx];
        }
        else if (currentTimeOfDay == TIME_SUNSET) {
            useCol = colsSunset[colorIdx];
        }
        else {
            useCol = colsNight[colorIdx];
        }
        glColor4f(useCol[0], useCol[1], useCol[2], 0.7f);

        if (isHyperWarp) {
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            glVertex3f(0, 0, 20.0f);
            glEnd();
        }
        else {
            glPointSize(2.0f);
            glBegin(GL_POINTS);
            glVertex3f(0, 0, 0);
            glEnd();
        }
        glPopMatrix();
    }

    glDepthMask(GL_TRUE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}
/**
 * @brief Renders a 2D string using GLUT bitmap fonts.
 *
 * @param str The string to display.
 * @param x Screen X position.
 * @param y Screen Y position.
 */
void drawBitmapText(const char* str, float x, float y) {
    glRasterPos2f(x, y);
    int len = (int)strlen(str);
    for (int i = 0; i < len; i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
    }
}

/**
 * @brief Renders a 2D HUD welcome box with animated stroke text.
 * Switches to orthogonal projection for UI rendering.
 */
void drawWelcomeBox() {
    glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, winWidth, 0, winHeight);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float cx = winWidth / 2.0f;
    float cy = winHeight / 2.0f + 150.0f;
    float w = 720.0f;
    float h = 150.0f;

    glColor4f(0.0f, 0.1f, 0.3f, 0.6f);
    glBegin(GL_QUADS); glVertex2f(cx - w / 2, cy - h / 2); glVertex2f(cx + w / 2, cy - h / 2); glVertex2f(cx + w / 2, cy + h / 2); glVertex2f(cx - w / 2, cy + h / 2); glEnd();

    glColor4f(0.0f, 0.8f, 1.0f, 0.8f); glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP); glVertex2f(cx - w / 2, cy - h / 2); glVertex2f(cx + w / 2, cy - h / 2); glVertex2f(cx + w / 2, cy + h / 2); glVertex2f(cx - w / 2, cy + h / 2); glEnd();

    glColor4f(0.0f, 0.6f, 0.8f, 0.9f); glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP); glVertex2f(cx - w / 2 - 4.f, cy - h / 2 - 4.f); glVertex2f(cx + w / 2 + 4.f, cy - h / 2 - 4.f); glVertex2f(cx + w / 2 + 4.f, cy + h / 2 + 4.f); glVertex2f(cx - w / 2 - 4.f, cy + h / 2 + 4.f); glEnd();

    const char* title = "Have a Magical Day!";
    const char* sub = "Cyber World Connected";

    glPushMatrix();
    glColor3f(0.2f, 1.0f, 1.0f);
    glTranslatef(cx - 310.0f, cy + 10.0f, 0.0f);
    glScalef(0.40f, 0.30f, 0.40f);
    glLineWidth(4.5f);
    float titleSpacing = 15.0f;
    for (const char* c = title; *c; c++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
        glTranslatef(titleSpacing, 0.0f, 0.0f);
    }
    glPopMatrix();

    if ((int)(realTime * 4) % 2 == 0) {
        glPushMatrix();
        glColor3f(1.0f, 1.0f, 1.0f);
        glTranslatef(cx - 210.0f, cy - 40.0f, 0.0f);
        glScalef(0.25f, 0.2f, 0.25f);
        glLineWidth(3.0f);
        float subSpacing = 9.0f;
        for (const char* c = sub; *c; c++) {
            glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
            glTranslatef(subSpacing, 0.0f, 0.0f);
        }
        glPopMatrix();
    }

    glLineWidth(1.0f);
    glDisable(GL_BLEND); glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix(); glEnable(GL_LIGHTING);
}
/**
 * @brief Renders a technological HUD overlay with text.
 * Supports alignment using '|' separator.
 *
 * @param textLines Array of strings to display.
 * @param lineCount Number of lines in the array.
 */
void drawHUD(const char* textLines[], int lineCount) {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winWidth, 0, winHeight); // (0,0) is Bottom-Left

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float boxW = 380.0f;
    float boxH = 36.0f * lineCount;
    float posX = 20.0f;
    float posY = winHeight - 20.0f;

    glColor4f(0.0f, 0.05f, 0.15f, 0.8f);
    glBegin(GL_QUADS);
    glVertex2f(posX, posY - boxH);        // Bottom-Left
    glVertex2f(posX + boxW, posY - boxH); // Bottom-Right
    glVertex2f(posX + boxW, posY);        // Top-Right
    glVertex2f(posX, posY);               // Top-Left
    glEnd();

    glLineWidth(2.4f);
    glColor3f(0.0f, 0.8f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(posX, posY - boxH);
    glVertex2f(posX + boxW, posY - boxH);
    glVertex2f(posX + boxW, posY);
    glVertex2f(posX, posY);
    glEnd();

    float corner = 20.0f;
    glColor4f(0.0f, 0.8f, 1.0f, 0.8f);
    glBegin(GL_TRIANGLES);
    glVertex2f(posX, posY - corner); // 下一点
    glVertex2f(posX + corner, posY); // 右一点
    glVertex2f(posX, posY);          // 角点
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);

    float textStartX = posX + 25.0f;
    float columnOffset = 120.0f;

    for (int i = 0; i < lineCount; i++) {
        float textY = posY - 25.0f - (i * 35.0f);

        glRasterPos2f(textStartX, textY);

        const char* p = textLines[i];
        while (*p) {
            if (*p == '|') {
                glRasterPos2f(textStartX + columnOffset, textY);
            }
            else {
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
            }
            p++;
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_LIGHTING);
}

// For the first environment: Room
/**
 * @brief Configures lighting settings for the room scene.
 * Disables fog and extra lights; sets up a directional blue light.
 */
void setupRoomLighting() {
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_FOG);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);

    GLfloat lightPos[] = { 5.0f, 6.0f, -1.0f, 0.0f };
    GLfloat diffuseBlue[] = { 0.3f, 0.5f, 1.0f, 1.0f };
    GLfloat ambientBlue[] = { 0.05f, 0.08f, 0.15f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseBlue);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientBlue);
}
/**
 * @brief Renders a glowing grid pattern on the ceiling.
 */
void drawSciFiCeiling() {
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);

    glColor3fv(COLOR_CYBER_CYAN);

    for (float x = -5.0f; x <= 5.0f; x += 2.0f) {
        glBegin(GL_LINES);
        glVertex3f(x, 4.95f, -5.0f);
        glVertex3f(x, 4.95f, 5.0f);
        glEnd();
    }
    for (float z = -5.0f; z <= 5.0f; z += 2.0f) {
        glBegin(GL_LINES);
        glVertex3f(-5.0f, 4.95f, z);
        glVertex3f(5.0f, 4.95f, z);
        glEnd();
    }

    glEnable(GL_LIGHTING);
}
/**
 * @brief Renders a server tower with neon edges and blinking status lights.
 *
 * @param x World X position.
 * @param z World Z position.
 */
void drawServerTower(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    setSolid(COLOR_PURE_BLACK);
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 0.0f);
    glScalef(1.2f, 5.0f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    setNeon(COLOR_NEON_GREEN, 1.5f);
    glLineWidth(2.0f);
    glDisable(GL_LIGHTING);
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 0.0f);
    glScalef(1.22f, 5.02f, 1.22f);
    glutWireCube(1.0f);
    glPopMatrix();
    glEnable(GL_LIGHTING);

    glDisable(GL_LIGHTING);
    glPointSize(3.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i < 20; i++) {
        float flash = sinf(realTime * (5.0f + i) + i);
        if (flash > 0.0f) glColor3f(0.0f, 1.0f, 0.0f);
        else glColor3f(0.0f, 0.2f, 0.0f);

        float ly = 0.5f + i * 0.2f;
        glVertex3f(0.0f, ly, 0.61f);
        glVertex3f(0.2f, ly, 0.61f);
    }
    glEnd();
    glEnable(GL_LIGHTING);

    glPopMatrix();
}
/**
 * @brief Renders a VR headset prop.
 *
 * @param x World X position.
 * @param y World Y position.
 * @param z World Z position.
 */
void drawVRHeadset(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(-30.0f, 0, 1, 0);
    glScalef(0.75f, 0.75f, 0.75f);

    setSolid(COLOR_DARK_GREY);
    glPushMatrix();
    glScalef(0.5f, 0.25f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();

    setNeon(COLOR_CYBER_PINK, 2.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.16f);
    glScalef(0.4f, 0.05f, 0.01f);
    glutSolidCube(1.0f);
    glPopMatrix();

    setSolid(COLOR_PURE_BLACK);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -0.2f);
    glRotatef(90.0f, 1, 0, 0);
    glutSolidTorus(0.05f, 0.25f, 8, 16);
    glPopMatrix();

    glPopMatrix();
}
/**
 * @brief Renders a wall-mounted tech panel with a holographic overlay.
 *
 * @param x World X position.
 * @param y World Y position.
 * @param z World Z position.
 * @param onSideWall Rotates panel 90 degrees if true.
 */
void drawWallTechPanel(float x, float y, float z, bool onSideWall) {
    glPushMatrix();
    glTranslatef(x, y, z);
    if (onSideWall) glRotatef(90.0f, 0, 1, 0);

    setSolid(COLOR_DEEP_GREY);
    glPushMatrix();
    glScalef(2.0f, 1.2f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.06f);
    drawCyberScreen(1.8f, 1.0f, COLOR_CYBER_CYAN, 0);
    glPopMatrix();

    glPopMatrix();
}
/**
 * @brief Renders a computer mouse on the desk surface using a clipping plane.
 *
 * @param x World X position.
 * @param y World Y position (desk surface level).
 * @param z World Z position.
 */
void drawMouseOnDesk(float x, float y, float z) {
    GLdouble planeEq[4] = { 0.0, 1.0, 0.0, -y };
    glPushMatrix();
    glClipPlane(GL_CLIP_PLANE0, planeEq);
    glEnable(GL_CLIP_PLANE0);
    glTranslatef(x, y + 0.1f, z);
    glScalef(1.0f, 0.6f, 1.4f);
    glColor3f(0.18f, 0.18f, 0.18f);
    glutSolidSphere(0.10f, 32, 16);
    glDisable(GL_CLIP_PLANE0);
    glPopMatrix();
}
/**
 * @brief Renders a potted succulent plant with a neon wireframe accent.
 *
 * @param x World X position.
 * @param y World Y position.
 * @param z World Z position.
 */
void drawSucculent(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x, y + 0.06f, z);

    // Pot
    glColor3f(0.6f, 0.7f, 0.8f);
    drawBox(0.20f, 0.40f, 0.25f);

    // Neon outline
    setNeon(COLOR_CYBER_CYAN, 0.8f);
    glLineWidth(1.0f);
    glPushMatrix();
    glScalef(0.21f, 0.41f, 0.26f);
    glutWireCube(1.0f);
    glPopMatrix();

    // Plant
    glTranslatef(0.0f, 0.17f, 0.0f);
    setSolid(COLOR_NEON_GREEN);
    glutSolidSphere(0.10f, 24, 16);
    glPopMatrix();
}
/**
 * @brief Renders a simple cylinder cup.
 *
 * @param x World X position.
 * @param y World Y position.
 * @param z World Z position.
 */
void drawCup(float x, float y, float z) {
    glPushMatrix();
    glTranslatef(x - 0.2f, y, z - 0.5f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glColor3f(0.92f, 0.92f, 0.97f);
    glutSolidCylinder(0.10f, 0.26f, 32, 8);
    glPopMatrix();
}
/**
 * @brief Renders the entire room interior including floor, ceiling, walls, and props.
 * Props include server towers, tech panels, and framed art.
 */
void drawRoom() {
    // 1. Floor
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.floor);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 1.0f, 0.0f);
    float repeat = 4.0f;
    glTexCoord2f(0.0f, 0.0f);     glVertex3f(-5.0f, 0.0f, -5.0f);
    glTexCoord2f(repeat, 0.0f);   glVertex3f(5.0f, 0.0f, -5.0f);
    glTexCoord2f(repeat, repeat); glVertex3f(5.0f, 0.0f, 5.0f);
    glTexCoord2f(0.0f, repeat);   glVertex3f(-5.0f, 0.0f, 5.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // 2. Ceiling
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.15f, 0.15f, 0.18f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-5.0f, 5.0f, -5.0f);
    glVertex3f(5.0f, 5.0f, -5.0f);
    glVertex3f(5.0f, 5.0f, 5.0f);
    glVertex3f(-5.0f, 5.0f, 5.0f);
    glEnd();

    drawSciFiCeiling();

    // 3. Back Wall
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.wall);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, 0.0f, -5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, 0.0f, -5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, 5.0f, -5.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, 5.0f, -5.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // 4. Left Wall
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.boxSide);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-5.0f, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-5.0f, 0.0f, -5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-5.0f, 5.0f, -5.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-5.0f, 5.0f, 5.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // 5. Right Wall
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.boxSide);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(-1.0f, 0.0f, 0.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(5.0f, 0.0f, -5.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(5.0f, 0.0f, 5.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(5.0f, 5.0f, 5.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(5.0f, 5.0f, -5.0f);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    // 6. High-Tech Props
    // Server Towers
    drawServerTower(-4.2f, -4.2f);
    drawServerTower(4.2f, -4.2f);

    // Tech Panels
    glPushMatrix();
    glTranslatef(4.f, 0.5f, -3.9f);
    drawWallTechPanel(-4.9f, 3.5f, -1.0f, false);
    glPopMatrix();

    drawWallTechPanel(4.9f, 3.5f, 1.0f, true);

    // 7. Wall Frames 
    float rightX = 1.7f;
    float rightY = 3.1f;
    float rightZ = -4.8f;

    // Frame Box
    glPushMatrix();
    glColor3f(0.4f, 0.25f, 0.15f);
    glTranslatef(rightX, rightY, rightZ);
    drawBox(1.8f, 1.2f, 0.1f);

    setNeon(COLOR_NEON_YELLOW, 0.8f);
    glLineWidth(1.0f);
    glPushMatrix();
    glScalef(1.85f, 1.25f, 0.11f);
    glutWireCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    // Frame Texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.frameLeft);
    glColor3f(1.0f, 1.0f, 1.0f);

    glPushMatrix();
    glTranslatef(rightX, rightY, rightZ + 0.06f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.8f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(0.8f, -0.5f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(0.8f, 0.5f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.8f, 0.5f, 0.0f);
    glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}
/**
 * @brief Renders the main desk setup including monitors, PC, and peripherals.
 * Features a transparent PC case, dual monitors (landscape + portrait), and holographic elements.
 */
void drawDeskAndPC() {
    // 1. Mega Desk
    glPushMatrix();
    setSolid(COLOR_DARK_GREY);
    glTranslatef(0.0f, 0.8f, -2.0f);
    drawBox(6.0f, 0.1f, 2.0f);

    setNeon(COLOR_CYBER_PURPLE, 0.8f);
    glLineWidth(1.0f);
    glPushMatrix(); glScalef(6.02f, 0.12f, 2.02f); glutWireCube(1.0f); glPopMatrix();

    glColor3f(0.3f, 0.18f, 0.1f);
    float legH = 0.8f;
    float legOffX = 2.8f;
    float legOffZ = 0.9f;
    glPushMatrix();
    glTranslatef(-legOffX, -legH / 2.0f, -legOffZ); drawBox(0.1f, legH, 0.1f);
    glTranslatef(2 * legOffX, 0.0f, 0.0f); drawBox(0.1f, legH, 0.1f);
    glTranslatef(0.0f, 0.0f, 2 * legOffZ); drawBox(0.1f, legH, 0.1f);
    glTranslatef(-2 * legOffX, 0.0f, 0.0f); drawBox(0.1f, legH, 0.1f);
    glPopMatrix();
    glPopMatrix();

    // 2. Main Monitor (Curved)
    float monW = 3.f;
    float monH = 1.2f;

    glPushMatrix();
    glTranslatef(0.0f, 1.6f, -2.5f);

    setSolid(COLOR_L_TITANIUM);
    drawBox(monW, monH, 0.1f);

    setNeon(COLOR_CYBER_CYAN, 0.8f);
    glPushMatrix(); glScalef(monW + 0.05f, monH + 0.05f, 0.12f); glutWireCube(1.0f); glPopMatrix();

    // Screen Texture
    float halfW = (monW - 0.1f) / 2.0f;
    float halfH = (monH - 0.1f) / 2.0f;
    float zFront = 0.06f + 0.001f;

    GLuint screenTex = (currentState == STATE_SCREEN_FLASH) ? tex.flash : tex.screen;
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, screenTex);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.2f, 0.2f); glVertex3f(-halfW, -halfH, zFront);
    glTexCoord2f(0.8f, 0.2f); glVertex3f(halfW, -halfH, zFront);
    glTexCoord2f(0.8f, 0.8f); glVertex3f(halfW, halfH, zFront);
    glTexCoord2f(0.2f, 0.8f); glVertex3f(-halfW, halfH, zFront);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // Monitor Stand
    glPushMatrix();
    glTranslatef(0.0f, 1.0f, -2.6f);
    glColor3f(0.12f, 0.12f, 0.12f);
    drawBox(0.6f, 0.4f, 0.3f);
    glTranslatef(0.0f, -0.3f, 0.0f);
    drawBox(1.4f, 0.1f, 0.8f);
    glPopMatrix();

    // 3. Vertical Monitor
    glPushMatrix();
    glTranslatef(-2.f, 1.8f, -2.f);
    glRotatef(45.0f, 0.0f, 1.0f, 0.0f);

    float vMonW = 0.8f;
    float vMonH = 1.6f;

    setSolid(COLOR_L_TITANIUM);
    drawBox(vMonW, vMonH, 0.1f);

    setNeon(COLOR_CYBER_PINK, 0.8f);
    glPushMatrix(); glScalef(vMonW + 0.05f, vMonH + 0.05f, 0.12f); glutWireCube(1.0f); glPopMatrix();

    // Vertical Screen Texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.screen2);
    glColor3f(1.0f, 1.0f, 1.0f);

    float vHalfW = (vMonW - 0.1f) / 2.0f;
    float vHalfH = (vMonH - 0.1f) / 2.0f;

    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-vHalfW, -vHalfH, zFront);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(vHalfW, -vHalfH, zFront);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(vHalfW, vHalfH, zFront);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-vHalfW, vHalfH, zFront);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Stand
    glTranslatef(0.0f, -0.6f, -0.1f);
    setSolid(COLOR_DARK_GREY);
    drawBox(0.2f, 0.6f, 0.2f);
    glPopMatrix();

    // 4. Keyboard
    glPushMatrix();
    glTranslatef(0.0f, 0.9f, -1.6f);
    glColor3f(0.15f, 0.15f, 0.15f);
    drawBox(1.4f, 0.05f, 0.4f);
    setNeon(COLOR_CYBER_PINK, 0.8f);
    glPushMatrix(); glScalef(1.42f, 0.06f, 0.42f); glutWireCube(1.0f); glPopMatrix();

    // Keyboard Texture
    float halfX = 1.4f / 2.0f;
    float halfZ = 0.4f / 2.0f;
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D, tex.keyboard); glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS); glNormal3f(0.0f, 1.0f, 0.0f);
    glTexCoord2f(0.1f, 0.9f); glVertex3f(-halfX, 0.03f, -halfZ);
    glTexCoord2f(0.9f, 0.9f); glVertex3f(halfX, 0.03f, -halfZ);
    glTexCoord2f(0.9f, 0.1f); glVertex3f(halfX, 0.03f, halfZ);
    glTexCoord2f(0.1f, 0.1f); glVertex3f(-halfX, 0.03f, halfZ);
    glEnd(); glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // 5. PC Case (Transparent)
    glPushMatrix();
    glTranslatef(2.6f, 1.45f, -2.0f);
    float caseW = 0.6f, caseH = 1.2f, caseD = 1.3f;

    // Internal Fans
    for (int i = 0; i < 2; i++) {
        glPushMatrix();
        float fanY = (i == 0) ? -0.3f : 0.3f;
        glTranslatef(0.0f, fanY, 0.4f);
        glRotatef(realTime * 300.0f, 0, 0, 1);

        setNeon(COLOR_CYBER_CYAN, 2.0f);
        glutWireTorus(0.05f, 0.25f, 8, 16);
        setSolid(COLOR_WHITE);
        glScalef(0.05f, 0.5f, 0.05f); glutSolidCube(1.0f);
        glScalef(1.0f, 0.2f, 10.0f); glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Glass Case Shell
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4f(0.1f, 0.1f, 0.1f, 0.7f);

    glPushMatrix();
    glScalef(caseW, caseH, caseD);
    glutSolidCube(1.0f);
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // Frame
    setNeon(COLOR_NEON_GREEN, 1.0f);
    glLineWidth(2.0f);
    glPushMatrix(); glScalef(caseW, caseH, caseD); glutWireCube(1.0f); glPopMatrix();

    // Drone Stand
    glPushMatrix();
    glTranslatef(0.0f, caseH / 2.0f + 0.05f, 0.2f);
    setNeon(COLOR_NEON_RED, 0.5f);
    glPushMatrix(); glScalef(0.5f, 0.05f, 0.5f); glutSolidCube(1.0f); glPopMatrix();

    glTranslatef(-0.3f, 0.3f, 0.3f);
    glScalef(0.14f, 0.14f, 0.14f);
    glRotatef(realTime * 15.0f, 0, 1, 0);
    drawDrone();
    glPopMatrix();

    glPopMatrix(); // End PC


    // 6. Accessories
    drawVRHeadset(-2.0f, 0.95f, -1.8f);
    drawSucculent(-1.2f, 0.8f, -2.2f);
    drawMouseOnDesk(1.0f, 0.8f, -1.6f);
    drawCup(1.5f, 0.8f, -1.8f);

    // 7. Holographic Overlays
    glPushMatrix();
    glTranslatef(1.8f, 3.8f, -3.7f);
    glRotatef(5.0f, 1, 0, 0);
    drawCyberScreen(2.2f, 0.3f, COLOR_NEON_YELLOW, 1, 7);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-2.2f, 3.0f, -2.4f);
    glRotatef(15.0f, 0, 1, 0);
    drawCyberScreen(1.0f, 0.8f, COLOR_CYBER_CYAN, 0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-1.8f, 0.9f, -1.2f);
    glScalef(0.3f, 0.2f, 0.3f);
    drawHologram(0.9f, 0);
    glPopMatrix();
}

// For the second environment: Cyber World
/**
 * @brief Renders a circular shadow using multiply blending.
 *
 * @param size Shadow radius.
 * @param darkness Darkness level (0.0 = black, 1.0 = invisible).
 */
void drawShadowBlob(float size, float darkness) {
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    glBegin(GL_TRIANGLE_FAN);

    glColor3f(darkness, darkness, darkness);
    glVertex3f(0.0f, 0.05f, 0.0f);

    glColor3f(1.0f, 1.0f, 1.0f);

    for (int i = 0; i <= 16; i++) {
        float angle = i * 2.0f * M_PI / 16;
        glVertex3f(cosf(angle) * size, 0.05f, sinf(angle) * size);
    }
    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}
/**
 * @brief Renders a rectangular shadow for buildings.
 *
 * @param w Width of the object.
 * @param d Depth of the object.
 * @param darkness Darkness level.
 */
void drawSquareShadow(float w, float d, float darkness) {
    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);

    float y = 0.1f;
    float ext = 8.0f;

    glBegin(GL_QUADS);

    glColor3f(darkness, darkness, darkness);

    glVertex3f(-w / 2 - 2 * ext, y, -d / 2 - ext);
    glVertex3f(w / 2 + 2 * ext, y, -d / 2 - ext);
    glVertex3f(w / 2 + 2 * ext, y, d / 2 + ext);
    glVertex3f(-w / 2 - 2 * ext, y, d / 2 + ext);

    glEnd();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}

//Cyber Environment
/**
 * @brief Renders a building-like box with neon edges and illuminated windows.
 * Window lighting is randomized based on the time of day.
 *
 * @param w Width of the box.
 * @param h Height of the box.
 * @param d Depth of the box.
 * @param neonColor Color array for the neon outline.
 */
void drawNeonBox(float w, float h, float d, const GLfloat* neonColor) {
    setSolid(COLOR_DARK_GREY);
    glPushMatrix();
    glScalef(w, h, d);
    glutSolidCube(1.0f);
    glPopMatrix();

    glDisable(GL_LIGHTING);
    glColor3fv(neonColor);
    glLineWidth(2.0f);
    glPushMatrix();
    glScalef(w * 1.01f, h * 1.01f, d * 1.01f);
    glutWireCube(1.0f);
    glPopMatrix();

    glPushMatrix();
    float numWindowsY = (int)floor(h / 8.0f);
    float winHeight = h / numWindowsY;

    for (int i = 0; i < numWindowsY; i++) {
        float winY = -h / 2.0f + winHeight * (i + 0.5f);

        if (currentTimeOfDay != TIME_DAY) {
            glColor3f(0.3f, 0.3f, 0.3f);
        }
        else if (rand() % 3 != 0 || i == 0) {
            GLfloat neonWinColor[] = { 0.8f * (rand() % 10 / 10.0f), 0.8f * (rand() % 10 / 10.0f), 1.0f, 1.0f };
            glColor3fv(neonWinColor);
            setNeon(neonWinColor, 0.8f);
        }
        else {
            continue;
        }

        for (int side = -1; side <= 1; side += 2) {
            glPushMatrix();
            glTranslatef(0.0f, winY, side * d / 2.0f);

            for (int j = 0; j < 3; j++) {
                float winX = -w / 2.0f + w * (j + 1) / 4.0f;
                float sizeX = w / 10.0f;
                float sizeY = winHeight / 4.0f;

                glBegin(GL_QUADS);
                glNormal3f(0.0f, 0.0f, side * 1.0f);
                glVertex3f(winX + sizeX, sizeY, side * 0.02f);
                glVertex3f(winX - sizeX, sizeY, side * 0.02f);
                glVertex3f(winX - sizeX, -sizeY, side * 0.02f);
                glVertex3f(winX + sizeX, -sizeY, side * 0.02f);
                glEnd();

                glDisable(GL_LIGHTING);
                glColor3f(1.0f, 1.0f, 1.0f);
                glLineWidth(1.0f);

                glPushMatrix();
                glTranslatef(winX, 0.0f, side * 0.03f);
                glScalef(sizeX * 2.0f, sizeY * 2.0f, 0.01f);
                glutWireCube(1.0f);
                glPopMatrix();

                glEnable(GL_LIGHTING);
            }
            glPopMatrix();
        }
    }
    glPopMatrix();
    glEnable(GL_LIGHTING);
}
/**
 * @brief Renders a detailed building unit with textured geometry and dynamic windows.
 * Adapts lighting, window colors, and decorative strokes based on the time of day.
 *
 * @param w Width of the unit.
 * @param h Height of the unit.
 * @param d Depth of the unit.
 * @param baseColor Base color array for the walls.
 * @param isGroundFloor True if this is the ground floor (adds storefronts).
 * @param floorIndex Vertical index of the floor (affects procedural generation).
 * @param texID Texture ID for the wall surface.
 */
void drawRichBuildingUnit(float w, float h, float d, const GLfloat* baseColor,
    bool isGroundFloor, int floorIndex, GLuint texID) {

    // 1. Main Structure (Textured Box)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    setSolid(baseColor);

    glPushMatrix();
    drawTexturedBox(w * 0.99f, h * 0.99f, d * 0.99f, texID);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    // 2. Decorations & Strokes
    int neonIdx = (floorIndex + (int)w) % 6;
    const GLfloat* currentNeonColor = RAINBOW_COLORS[neonIdx];

    // Decoration Color
    if (currentTimeOfDay == TIME_DAY) {
        GLfloat dayDecor[] = { 0.0f, 0.8f, 1.0f };
        setSolid(dayDecor);
    }
    else {
        setSolid(COLOR_DARK_GREY);
    }

    // Geometric Patterns
    int patternType = (floorIndex * 7 + (int)w) % 3;

    if (patternType == 1) { // Vertical Strips
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, d / 2.0f + 0.05f);
        glScalef(w * 0.08f, h * 0.8f, 0.01f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    else if (patternType == 2) { // Horizontal Rings
        glPushMatrix();
        glScalef(w * 1.02f, h * 0.1f, d * 1.02f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Edge Strokes
    if (currentTimeOfDay == TIME_DAY) {
        GLfloat dayStroke[] = { 0.2f, 0.8f, 0.7f };
        setSolid(dayStroke);
        glLineWidth(2.f);
    }
    else {
        setNeon(currentNeonColor, 2.0f);
        glLineWidth(2.5f);
    }

    glPushMatrix();
    glScalef(w * 1.01f, h * 1.01f, d * 1.01f);
    glutWireCube(1.0f);
    glPopMatrix();

    // 3. Windows & Storefronts
    if (isGroundFloor) {
        // Storefront Frame
        setSolid(COLOR_DARK_GREY);
        glPushMatrix(); glTranslatef(0.0f, -h / 2.0f + 4.0f, d / 2.0f + 0.1f);
        glScalef(w * 0.9f, 10.0f, 0.1f); glutSolidCube(1.0f); glPopMatrix();

        // Glass
        if (currentTimeOfDay != TIME_DAY) {
            if (currentTimeOfDay == TIME_SUNSET) setNeon(COLOR_WARM_LIGHT, 1.5f);
            else setNeon(COLOR_NEON_YELLOW, 2.0f);
        }
        else {
            GLfloat glassDay[] = { 0.7f, 0.8f, 0.9f };
            setSolid(glassDay);
        }

        glPushMatrix(); glTranslatef(0.0f, -h / 2.0f + 4.0f, d / 2.0f + 0.15f);
        glScalef(w * 0.85f, 9.5f, 0.1f); glutSolidCube(1.0f); glPopMatrix();

        // Side Signage
        if ((int)w % 3 == 0) {
            glPushMatrix(); glTranslatef(w / 2.0f + 2.0f, 0.0f, 0.0f);
            if (currentTimeOfDay != TIME_DAY) setNeon(COLOR_CYBER_PINK, 2.0f); else setSolid(COLOR_WHITE);
            glScalef(0.5f, h * 0.6f, 6.0f); glutSolidCube(1.0f); glPopMatrix();
        }
    }
    else if (!isGroundFloor) {
        // Upper Windows
        int cols = (int)(w / 5.0f); if (cols < 1) cols = 1;
        int rows = (int)(h / 4.0f); if (rows < 1) rows = 1;
        float winW = w / (float)cols * 0.7f;
        float winH = h / (float)rows * 0.6f;

        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                if (rand() % 100 > 45) { 
                    float wx = -w / 2.0f + (w / (float)cols) * (c + 0.5f);
                    float wy = -h / 2.0f + (h / (float)rows) * (r + 0.5f);

                    glPushMatrix();
                    glTranslatef(wx, wy, d / 2.0f);

                    // Window Frame 
                    glDisable(GL_LIGHTING);
                    glDepthMask(GL_FALSE);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_DST_COLOR, GL_ZERO);
                    glColor3f(0.8f, 0.8f, 0.8f);

                    glPushMatrix();
                    glScalef(winW * 1.2f, winH * 1.3f, 0.05f);
                    glutSolidCube(1.0f);
                    glPopMatrix();

                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glDepthMask(GL_TRUE);
                    glEnable(GL_LIGHTING);

                    // Window Glass
                    glTranslatef(0.0f, 0.0f, 0.05f);

                    if (currentTimeOfDay == TIME_DAY) {
                        GLfloat winCol[] = { 0.6f, 0.8f, 0.95f }; setSolid(winCol);
                    }
                    else if (currentTimeOfDay == TIME_NIGHT) {
                        int colorType = (r + c + floorIndex) % 4;
                        if (colorType == 0) setNeon(COLOR_CYBER_CYAN, 2.0f);
                        else if (colorType == 1) setNeon(COLOR_CYBER_PINK, 2.0f);
                        else if (colorType == 2) setNeon(COLOR_NEON_YELLOW, 2.0f);
                        else setNeon(COLOR_WHITE, 2.0f);
                    }
                    else { 
                        int colorType = (r * c) % 3;
                        if (colorType == 0) setNeon(COLOR_WARM_LIGHT, 1.5f);
                        else if (colorType == 1) setNeon(COLOR_NEON_ORANGE, 1.2f);
                        else { GLfloat darkRed[] = { 0.8f, 0.2f, 0.1f }; setNeon(darkRed, 1.0f); }
                    }

                    glScalef(winW, winH, 0.05f);
                    glutSolidCube(1.0f);

                    glPopMatrix();
                }
            }
        }
    }
}
/**
 * @brief Renders random colorful glowing spheres (berries).
 *
 * @param num Number of berries to draw.
 * @param range Spatial range for random positioning.
 */
void drawCyberBerries(int num, float range) {
    for (int i = 0; i < num; i++) {
        glPushMatrix();

        int colorType = rand() % 3;
        if (colorType == 0) setNeon(COLOR_CYBER_PINK, 2.0f);
        else if (colorType == 1) setNeon(COLOR_NEON_YELLOW, 2.0f);
        else setNeon(COLOR_CYBER_CYAN, 2.0f);

        float bx = ((rand() % 200) / 100.0f - 1.0f) * range;
        float by = ((rand() % 200) / 100.0f - 1.0f) * range;
        float bz = ((rand() % 200) / 100.0f - 1.0f) * range;

        glTranslatef(bx, by, bz);
        glutSolidSphere(0.12f, 6, 6);
        glPopMatrix();
    }
}
/**
 * @brief Renders a stylized cybernetic tree with a rotating crown and glowing berries.
 *
 * @param h Height of the tree.
 */
void drawCyberTree(float h) {
    float trunkRatio = 0.5f;
    float trunkWidth = 1.3f;
    float crownScale = 3.0f;
    float crownHover = 0.5f;

    float trunkHeight = h * trunkRatio;

    setSolid(COLOR_DARK_GREY);
    glPushMatrix();
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(trunkWidth, trunkWidth, trunkHeight / 2.0f);
    glutSolidCone(1.0f, 2.0f, 12, 2);
    glPopMatrix();

    if (currentTimeOfDay == TIME_NIGHT) setNeon(COLOR_TREE_GREEN, 0.6f);
    else if (currentTimeOfDay == TIME_SUNSET) setSolid(COLOR_GRASS_GREEN);
    else setSolid(COLOR_NEON_GREEN);

    glPushMatrix();
    glTranslatef(0.0f, trunkHeight + crownHover, 0.0f);
    glScalef(crownScale, crownScale, crownScale);
    glRotatef(realTime * 10.0f, 0.0f, 1.0f, 0.0f);

    glutSolidDodecahedron();

    drawCyberBerries(7 + rand() % 4, 0.9f);

    glPopMatrix();
}

/**
 * @brief Renders a bush composed of randomized geometric shapes and glowing berries.
 */
void drawCyberBush() {
    if (currentTimeOfDay == TIME_NIGHT) setNeon(COLOR_GRASS_GREEN, 0.3f);
    else setSolid(COLOR_GRASS_GREEN);

    glPushMatrix();

    float globalScale = 3.0f;
    glScalef(globalScale, globalScale, globalScale);

    int numParts = 5 + rand() % 3;
    for (int i = 0; i < numParts; i++) {
        glPushMatrix();
        float offsetX = (rand() % 200 / 100.0f) - 1.0f;
        float offsetZ = (rand() % 200 / 75.0f) - 1.0f;
        float size = 0.5f + (rand() % 100 / 200.0f);

        glTranslatef(offsetX, 0.0f, offsetZ);
        glScalef(size, size, size);

        int shapeType = rand() % 3;
        if (shapeType == 0) glutSolidDodecahedron();
        else if (shapeType == 1) {
            glRotatef(45, 0, 1, 0);
            glutSolidCube(0.8f);
        }
        else glutSolidIcosahedron();
        glPopMatrix();
    }

    int numBerries = 5 + rand() % 3;
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, 0.0f);
    drawCyberBerries(numBerries, 1.0f);
    glPopMatrix();

    glPopMatrix();
}

/**
 * @brief Renders a detailed parametric street lamp.
 *
 * @param side Direction (-1 for left, 1 for right).
 * @param h Height of the lamp pole.
 */
void drawRichLamp(float side, float h) {
    const GLfloat* lightColor;
    if (currentTimeOfDay == TIME_NIGHT) lightColor = COLOR_CYBER_CYAN;
    else if (currentTimeOfDay == TIME_SUNSET) lightColor = COLOR_WARM_LIGHT;
    else lightColor = COLOR_WHITE;

    glPushMatrix();

    // Ambient Shadow
    glPushMatrix();
    glTranslatef(0.0f, 0.1f, 0.0f);
    glScalef(3.5f, 1.0f, 5.5f);
    drawShadowBlob(2.5f, 0.6f);
    glPopMatrix();

    // Base
    setSolid(COLOR_CONCRETE);
    glPushMatrix();
    glScalef(4.0f, 2.0f, 4.0f);
    glTranslatef(0.0f, 0.5f, 0.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Pole
    setSolid(COLOR_DARK_GREY);
    glPushMatrix();
    glTranslatef(0.0f, h / 2.0f, 0.0f);
    glScalef(1.2f, h, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Decorative Rings
    setNeon(lightColor, 0.5f);
    int numRings = 6;
    float step = (h / 1.5f) / numRings;
    for (int i = 1; i <= numRings; i++) {
        glPushMatrix();
        glTranslatef(0.0f, i * step + 2.0f, 0.0f);
        glScalef(1.5f, 0.2f, 1.5f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }

    // Anchor to pole top
    glTranslatef(0.0f, h - 1.0f, 0.0f);

    if (side > 0) {
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    }

    // Arm
    setSolid(COLOR_DARK_GREY);
    glPushMatrix();
    glTranslatef(6.0f, 1.0f, 0.0f);
    glScalef(12.0f, 1.5f, 1.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Lamp Head
    glPushMatrix();
    glTranslatef(11.0f, 0.5f, 0.0f);

    setSolid(COLOR_DARK_GREY);
    glPushMatrix();
    glScalef(3.0f, 1.0f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Light Panel
    glTranslatef(0.0f, -0.6f, 0.0f);
    if (currentTimeOfDay == TIME_DAY) setSolid(COLOR_WHITE);
    else setNeon(lightColor, 2.0f);

    glPushMatrix();
    glScalef(2.5f, 0.2f, 1.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Volumetric Beam
    if (currentTimeOfDay != TIME_DAY) {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        GLfloat beamColor[4];
        if (currentTimeOfDay == TIME_NIGHT) {
            beamColor[0] = 0.0f; beamColor[1] = 0.9f; beamColor[2] = 1.0f; beamColor[3] = 0.15f;
        }
        else {
            beamColor[0] = 1.0f; beamColor[1] = 0.8f; beamColor[2] = 0.4f; beamColor[3] = 0.15f;
        }

        glMaterialfv(GL_FRONT, GL_EMISSION, beamColor);
        glColor4fv(beamColor);

        float beamH = h + 8.0f;
        float beamBaseR = 6.5f;

        glPushMatrix();
        glTranslatef(0.0f, -beamH, 0.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidCone(beamBaseR, beamH, 16, 2);
        glPopMatrix();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }

    // Holographic Decor
    glPushMatrix();
    glTranslatef(6.0f, 1.0f, 0.0f);
    int holoStyle = ((int)h / 5) % 2 == 0 ? 1 : 3;
    drawHologram(2.5f, holoStyle);
    glPopMatrix();

    glPopMatrix();
    glPopMatrix();
}

/**
 * @brief Renders massive overhead cyberpunk pipes traversing the city.
 * Features customizable density, connecting rings, tech boxes, cables, and energy pulses.
 */
void drawMegaPipes() {
    // Configuration
    float pipeSpacing = 600.0f; 
    int numPipes = 6;         

    float pipeRadius = 5.0f;
    float pipeLen = 300.0f;
    float pipeHeight = 90.0f;

    // Pipe Material
    if (currentTimeOfDay == TIME_DAY) {
        GLfloat pipeDay[] = { 0.8f, 0.85f, 0.9f };
        setSolid(pipeDay);
    }
    else {
        setSolid(COLOR_DARK_GREY);
        GLfloat dimBlue[] = { 0.1f, 0.15f, 0.2f };
        setNeon(dimBlue, 0.3f);
    }

    for (int i = 0; i < numPipes; i++) {
        // Position Calculation
        float pipeZ = (i * pipeSpacing) - worldOffsetZ;

        while (pipeZ > LOOP_LEN / 2) pipeZ -= LOOP_LEN;
        while (pipeZ < -LOOP_LEN / 2) pipeZ += LOOP_LEN;

        // Culling
        if (pipeZ < CULL_BACK || pipeZ > CULL_FRONT) continue;

        glPushMatrix();
        glTranslatef(0.0f, pipeHeight, pipeZ);

        // Main Cylinder
        glPushMatrix();
        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
        glTranslatef(0.0f, 0.0f, -pipeLen / 2.0f);
        glScalef(1.0f, 0.9f, 1.0f);
        glutSolidCylinder(pipeRadius, pipeLen, 24, 2);
        glPopMatrix();

        // Details: Rings, Tech Boxes, Cables
        const GLfloat* ringColor;
        if (currentTimeOfDay == TIME_DAY) {
            static GLfloat dayRing[] = { 0.2f, 0.6f, 0.5f };
            ringColor = dayRing;
        }
        else {
            ringColor = COLOR_NEON_ORANGE;
        }

        for (int j = -2; j <= 5; j++) {
            float nodeX = j * 35.0f;
            glPushMatrix();
            glTranslatef(nodeX, 0.0f, 0.0f);

            // Connector Rings
            if (currentTimeOfDay == TIME_DAY) setSolid(ringColor);
            else setNeon(ringColor, 2.0f);

            glPushMatrix();
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            glutSolidTorus(0.8f, pipeRadius * 0.95f, 10, 32);
            glPopMatrix();

            // Tech Boxes
            if (j % 2 == 0) {
                glPushMatrix();
                glTranslatef(0.0f, -pipeRadius - 1.0f, 0.0f);

                if (currentTimeOfDay == TIME_DAY) setSolid(COLOR_DARK_GREY);
                else setNeon(COLOR_CYBER_CYAN, 1.5f);

                glScalef(4.0f, 2.0f, 3.0f);
                glutSolidCube(1.0f);

                if (currentTimeOfDay != TIME_DAY) {
                    setNeon(COLOR_NEON_RED, 3.0f);
                    glTranslatef(0.0f, -0.6f, 0.0f);
                    glutSolidSphere(0.2f, 6, 6);
                }
                glPopMatrix();
            }

            // Suspension Cables
            if (abs(j) == 1) {
                setSolid(COLOR_NEON_RED);
                glLineWidth(3.0f);
                glBegin(GL_LINES);
                glVertex3f(0.0f, pipeRadius, 0.0f);
                glVertex3f(0.0f, 300.0f, 0.0f);
                glEnd();
            }
            glPopMatrix();
        }

        // Effects: Energy Pulses
        if (currentTimeOfDay != TIME_DAY) {
            float pulseSpeed = 200.0f;
            int numPulses = 4;

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);

            for (int k = 0; k < numPulses; k++) {
                float spacing = (pipeLen / numPulses) * k;
                float rawPos = realTime * pulseSpeed + i * 130.0f + spacing;
                float headPos = fmod(rawPos, pipeLen) - pipeLen / 2.0f;

                int trailLength = 15;
                for (int t = 0; t < trailLength; t++) {
                    float trailFactor = (float)t / trailLength;
                    float alpha = 1.0f - trailFactor;
                    float scale = 1.0f - trailFactor * 0.7f;

                    if (t == 0) glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
                    else glColor4f(0.0f, 1.0f, 1.0f, alpha * 0.8f);

                    glPushMatrix();
                    float currentPos = headPos - t * 2.5f;
                    if (currentPos > -pipeLen / 2.0f) {
                        glTranslatef(currentPos, 0.0f, 0.0f);
                        glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
                        glutSolidTorus(0.5f * scale, pipeRadius * 1.15f, 8, 30);
                    }
                    glPopMatrix();
                }
            }
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        glPopMatrix();
    }
}
/**
 * @brief Renders the rain system.
 * Displays standard rain lines during the day/sunset and "Matrix" style digital rain at night.
 */
void drawRainSystem() {
    if (!isRaining) return;
    if (rainSystem.empty()) initRain();

    glDisable(GL_LIGHTING);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    float speedNight = 3.f; 
    float speedDay = 3.5f; 
    float widthNight = 2.5f; 
    float widthDay = 2.0f; 
    float lenNight = 5.8f; 
    float lenDay = 4.5f; 

    for (auto& r : rainSystem) {
        float mult = (currentTimeOfDay == TIME_NIGHT) ? speedNight : speedDay;
        float fallSpeed = r.speed * mult;
        float currentY = 80.0f - fmod(realTime * 20.0f * fallSpeed + r.randomOffset, 80.0f);

        if (currentTimeOfDay == TIME_NIGHT) {
            float alpha = 0.6f + 0.4f * sinf(realTime * 10.0f + r.randomOffset);
            glColor4f(0.0f, 1.0f, 0.2f, alpha);

            if (r.type == 0) {
                glLineWidth(widthNight); 
                glBegin(GL_LINES);
                glVertex3f(r.x, currentY, r.z);
                glVertex3f(r.x, currentY + lenNight, r.z);
                glEnd();
            }
            else {
                glRasterPos3f(r.x, currentY, r.z);
                char matrixChar = (r.type == 1) ? '0' : '1';
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, matrixChar);

                glColor4f(0.0f, 0.8f, 0.1f, alpha * 0.3f);
                glRasterPos3f(r.x, currentY + 2.8f, r.z); // Tail character
                glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, matrixChar);
            }
        }
        else {
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            if (currentTimeOfDay == TIME_SUNSET) glColor4f(1.0f, 0.6f, 0.2f, 0.3f); // Sunset Orange
            else glColor4f(0.7f, 0.8f, 1.0f, 0.25f); // Day Blue

            glLineWidth(widthDay); // Apply Width
            glBegin(GL_LINES);
            glVertex3f(r.x, currentY, r.z);
            glVertex3f(r.x, currentY + lenDay, r.z); // Apply Length
            glEnd();
        }
    }

    glDepthMask(GL_TRUE);
    glEnable(GL_LIGHTING);
}
/**
 * @brief Renders expanding ripples on the ground during rain.
 * Includes logic for spawning new ripples and updating existing ones.
 */
void drawRipples() {
    if (!isRaining) return;

    srand(glutGet(GLUT_ELAPSED_TIME));

    // 1. Spawn New Ripples
    for (int i = 0; i < 2; i++) {
        if (rand() % 100 > 45) continue;

        Ripple r;
        r.x = (rand() % 700 / 10.0f) - 35.0f;
        r.z = worldOffsetZ + (rand() % 3000 / 10.0f) - 150.0f;
        r.life = 1.0f;
        r.maxRadius = 2.0f + (rand() % 40 / 10.0f);
        ripples.push_back(r);
    }

    // 2. Render , Update
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glLineWidth(2.0f);

    for (auto it = ripples.begin(); it != ripples.end(); ) {
        it->life -= 0.012f;

        if (it->life <= 0) {
            it = ripples.erase(it);
            continue;
        }

        float drawZ = it->z - worldOffsetZ;

        if (drawZ < CULL_BACK || drawZ > CULL_FRONT) {
            it = ripples.erase(it);
            continue;
        }

        float baseAlpha = it->life;

        glPushMatrix();
        glTranslatef(it->x, 0.15f, drawZ);
        glScalef(1.0f, 0.1f, 1.0f);

        if (currentTimeOfDay == TIME_NIGHT) glColor4f(0.2f, 1.0f, 0.6f, baseAlpha * 0.8f);
        else glColor4f(0.7f, 0.8f, 1.0f, baseAlpha * 0.8f);

        // Animate Concentric Circles
        float progress = 1.0f - it->life;

        for (int ring = 0; ring < 3; ring++) {
            float ringStart = ring * 0.15f;

            if (progress > ringStart) {
                float ringP = (progress - ringStart) * 1.5f;
                if (ringP > 1.0f) ringP = 1.0f;

                float r = it->maxRadius * ringP;
                float a = baseAlpha * (1.0f - ringP);

                if (currentTimeOfDay == TIME_NIGHT) {
                    glColor4f(0.2f, 1.0f, 0.6f, a);
                }
                else if (currentTimeOfDay == TIME_SUNSET) {
                    glColor4f(1.0f, 0.6f, 0.1f, a);
                }
                else {
                    glColor4f(0.7f, 0.8f, 1.0f, a);
                }

                glBegin(GL_LINE_LOOP);
                for (int k = 0; k < 24; k++) {
                    float ang = k * 2.0f * M_PI / 24;
                    glVertex3f(cosf(ang) * r, 0, sinf(ang) * r);
                }
                glEnd();
            }
        }

        glPopMatrix();
        ++it;
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LIGHTING);
}
/**
 * @brief Configures lighting, fog, and background colors for the cyberpunk scene.
 * Adapts light positions and colors based on day, sunset, and night modes.
 */
void setupCyberLighting() {
    glEnable(GL_LIGHTING);
    glDisable(GL_LIGHT0);

    glEnable(GL_FOG);
    glFogi(GL_FOG_MODE, GL_EXP2);

    GLfloat voidColor[] = { 0.15f, 0.15f, 0.20f, 1.0f };

    if (currentTimeOfDay == TIME_DAY) {
        glEnable(GL_LIGHT1);
        GLfloat sunPos[] = { 50.0f, 100.0f, 50.0f, 0.0f };
        GLfloat sunDiff[] = { 1.0f, 0.98f, 0.95f, 1.0f };
        GLfloat sunAmb[] = { 0.6f, 0.6f, 0.7f, 1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, sunPos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, sunDiff);
        glLightfv(GL_LIGHT1, GL_AMBIENT, sunAmb);
        glDisable(GL_LIGHT2); glDisable(GL_LIGHT3);

        GLfloat fogColor[] = { 0.5f, 0.6f, 0.9f, 1.0f };
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, 0.0020f);

        glClearColor(voidColor[0], voidColor[1], voidColor[2], 1.0f);
    }
    else if (currentTimeOfDay == TIME_SUNSET) {
        glEnable(GL_LIGHT1);
        GLfloat sunPos[] = { 0.0f, 20.0f, -100.0f, 0.0f };
        GLfloat sunDiff[] = { 1.0f, 0.6f, 0.2f, 1.0f };
        GLfloat sunAmb[] = { 0.4f, 0.3f, 0.3f, 1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, sunPos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, sunDiff);
        glLightfv(GL_LIGHT1, GL_AMBIENT, sunAmb);
        glEnable(GL_LIGHT2);

        GLfloat streetPos[] = { 0.0f, 10.0f, 0.0f, 1.0f };
        GLfloat streetCol[] = { 1.0f, 0.6f, 0.3f, 1.0f };
        glLightfv(GL_LIGHT2, GL_POSITION, streetPos);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, streetCol);
        glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.05f);
        glDisable(GL_LIGHT3);

        GLfloat fogColor[] = { 0.8f, 0.4f, 0.2f, 1.0f };
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, 0.00335f);

        glClearColor(voidColor[0], voidColor[1], voidColor[2], 1.0f);
    }
    else {
        glEnable(GL_LIGHT1);
        GLfloat moonPos[] = { 0.0f, 100.0f, -50.0f, 0.0f };
        GLfloat moonDiff[] = { 0.1f, 0.1f, 0.25f, 1.0f };
        GLfloat moonAmb[] = { 0.05f, 0.05f, 0.1f, 1.0f };
        glLightfv(GL_LIGHT1, GL_POSITION, moonPos);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, moonDiff);
        glLightfv(GL_LIGHT1, GL_AMBIENT, moonAmb);

        glEnable(GL_LIGHT2);
        float lightTime = realTime * 0.8f;
        GLfloat dynPos[] = { sinf(lightTime) * 40.0f, 20.0f, cosf(lightTime) * 40.0f - 20.0f, 1.0f };
        GLfloat dynColor[] = { 0.5f + 0.5f * sinf(lightTime), 0.3f, 0.5f + 0.5f * cosf(lightTime), 1.0f };
        glLightfv(GL_LIGHT2, GL_POSITION, dynPos);
        glLightfv(GL_LIGHT2, GL_DIFFUSE, dynColor);
        glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.02f);

        glEnable(GL_LIGHT3);
        GLfloat shipPos[] = { 0.0f, 15.0f, 10.0f, 1.0f };
        GLfloat shipCol[] = { 0.0f, 0.8f, 1.0f, 1.0f };
        glLightfv(GL_LIGHT3, GL_POSITION, shipPos);
        glLightfv(GL_LIGHT3, GL_DIFFUSE, shipCol);
        glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.01f);

        GLfloat fogColor[] = { 0.1f, 0.02f, 0.2f, 1.0f };
        glFogfv(GL_FOG_COLOR, fogColor);
        glFogf(GL_FOG_DENSITY, 0.0035f);

        glClearColor(0.15f, 0.05f, 0.25f, 1.0f);
    }
}
/**
 * @brief Renders the environment skybox.
 * Supports different textures for sides, top, and bottom depending on the time of day.
 * Disables fog during rendering to ensure the sky is visible.
 */
void drawEnvironmentBox() {
    float heightOffset = 350.0f;
    float dist = 480.0f;
    float size = 480.0f;
    GLuint sideTex = 0;

    // Select Side Texture
    if (currentTimeOfDay == TIME_DAY)       sideTex = tex.bgDay;
    else if (currentTimeOfDay == TIME_SUNSET) sideTex = tex.bgSunset;
    else                                      sideTex = tex.bgNight;

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_TEXTURE_2D);

    // A. Render Sides
    if (sideTex != 0) {
        glBindTexture(GL_TEXTURE_2D, sideTex);
        float squashY = 0.75f;

        // Front
        glPushMatrix(); glTranslatef(0.0f, heightOffset, -dist); glScalef(size, size * squashY, 1.0f);
        glBegin(GL_QUADS); glTexCoord2f(0, 0); glVertex3f(-1, -1, 0); glTexCoord2f(1, 0); glVertex3f(1, -1, 0); glTexCoord2f(1, 1); glVertex3f(1, 1, 0); glTexCoord2f(0, 1); glVertex3f(-1, 1, 0); glEnd(); glPopMatrix();

        // Left
        glPushMatrix(); glTranslatef(-dist, heightOffset, 0.0f); glRotatef(90.0f, 0.0f, 1.0f, 0.0f); glScalef(size, size * squashY, 1.0f);
        glBegin(GL_QUADS); glTexCoord2f(0, 0); glVertex3f(-1, -1, 0); glTexCoord2f(1, 0); glVertex3f(1, -1, 0); glTexCoord2f(1, 1); glVertex3f(1, 1, 0); glTexCoord2f(0, 1); glVertex3f(-1, 1, 0); glEnd(); glPopMatrix();

        // Right
        glPushMatrix(); glTranslatef(dist, heightOffset, 0.0f); glRotatef(-90.0f, 0.0f, 1.0f, 0.0f); glScalef(size, size * squashY, 1.0f);
        glBegin(GL_QUADS); glTexCoord2f(0, 0); glVertex3f(-1, -1, 0); glTexCoord2f(1, 0); glVertex3f(1, -1, 0); glTexCoord2f(1, 1); glVertex3f(1, 1, 0); glTexCoord2f(0, 1); glVertex3f(-1, 1, 0); glEnd(); glPopMatrix();

        // Back
        glPushMatrix(); glTranslatef(0.0f, heightOffset, dist); glRotatef(180.0f, 0.0f, 1.0f, 0.0f); glScalef(size, size * squashY, 1.0f);
        glBegin(GL_QUADS); glTexCoord2f(0, 0); glVertex3f(-1, -1, 0); glTexCoord2f(1, 0); glVertex3f(1, -1, 0); glTexCoord2f(1, 1); glVertex3f(1, 1, 0); glTexCoord2f(0, 1); glVertex3f(-1, 1, 0); glEnd(); glPopMatrix();
    }

    // B. Render Top
    glPushMatrix();
    float skyDrop = 120.0f;
    glTranslatef(0.0f, heightOffset + size - skyDrop, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(size, size, 1.0f);

    if (currentTimeOfDay == TIME_SUNSET) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex.skyTopSunset);
        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex3f(-1, -1, 0);
        glTexCoord2f(1, 0); glVertex3f(1, -1, 0);
        glTexCoord2f(1, 1); glVertex3f(1, 1, 0);
        glTexCoord2f(0, 1); glVertex3f(-1, 1, 0);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }
    else {
        glDisable(GL_TEXTURE_2D);

        if (currentTimeOfDay == TIME_DAY) {
            glColor3f(15.0f / 255.0f, 98.0f / 255.0f, 204.0f / 255.0f);
        }
        else {
            glColor3f(2.0f / 255.0f, 10.0f / 255.0f, 30.0f / 255.0f);
        }

        glBegin(GL_QUADS);
        glVertex3f(-1, -1, 0);
        glVertex3f(1, -1, 0);
        glVertex3f(1, 1, 0);
        glVertex3f(-1, 1, 0);
        glEnd();
    }
    glPopMatrix();

    // C. Render Bottom
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.1f, 0.1f, 0.1f);

    glPushMatrix();
    glTranslatef(0.0f, heightOffset - size, 0.0f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(size, size, 1.0f);
    glBegin(GL_QUADS); glVertex3f(-1, -1, 0); glVertex3f(1, -1, 0); glVertex3f(1, 1, 0); glVertex3f(-1, 1, 0); glEnd();
    glPopMatrix();

    glEnable(GL_FOG);
}

// Cyber Vehicles & Traffic
/**
 * @brief Renders flying paper planes (Daytime only).
 * Animated with simple flight paths and cycling colors.
 */
void drawPaperPlanes() {
    if (currentTimeOfDay != TIME_DAY) return;

    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    const GLfloat planeColors[3][3] = {
        {1.0f, 1.0f, 1.0f},
        {0.4f, 0.7f, 1.0f},
        {0.4f, 1.0f, 0.5f}
    };

    for (int i = 0; i < 15; i++) {
        float t = realTime * 0.5f + i * 20.0f;
        float px = cosf(t) * 60.0f;
        float py = 80.0f + sinf(t * 1.5f) * 20.0f;
        float pz = sinf(t) * 100.0f - 50.0f;

        glPushMatrix();
        glTranslatef(px, py, pz);

        float dx = -sinf(t);
        float dz = cosf(t);
        float angle = atan2f(dx, dz) * 180.0f / M_PI;
        glRotatef(angle, 0, 1, 0);

        glColor3fv(planeColors[i % 3]);

        glScalef(2.0f, 2.0f, 2.0f);
        glBegin(GL_TRIANGLES);
        glVertex3f(0, 0, 1); glVertex3f(-0.5, 0, -1); glVertex3f(0, -0.2, -0.5);
        glVertex3f(0, 0, 1); glVertex3f(0.5, 0, -1);  glVertex3f(0, -0.2, -0.5);

        glVertex3f(0, 0, 0.5); glVertex3f(-1.5, 0.2, -1); glVertex3f(0, 0, -1);
        glVertex3f(0, 0, 0.5); glVertex3f(1.5, 0.2, -1);  glVertex3f(0, 0, -1);
        glEnd();

        glPopMatrix();
    }
    glEnable(GL_LIGHTING);
}
/**
 * @brief Renders a futuristic hovering vehicle (Type 1).
 * Features day/night adaptation, ground shadows, and neon thrusters.
 *
 * @param isForward If true, faces forward; otherwise, rotates 180 degrees.
 * @param colorIndex Index for selecting the neon color scheme (0-5).
 */
void drawHoverCar(bool isForward, int colorIndex) {
    if (currentTimeOfDay == TIME_DAY) {
        const GLfloat* macCol = RAINBOW_COLORS[colorIndex % 6];
        GLfloat brightCol[] = { macCol[0] * 0.5f + 0.5f, macCol[1] * 0.5f + 0.5f, macCol[2] * 0.5f + 0.5f };
        setSolid(brightCol);
    }
    else {
        setSolid(COLOR_DEEP_GREY);
    }

    glPushMatrix();
    glScalef(2.f, 2.f, 2.f);

    glPushMatrix();
    glTranslatef(0.0f, 0.1f, 0.0f);
    glScalef(1.2f, 1.0f, 2.5f);
    drawShadowBlob(1.8f, 0.6f);
    glPopMatrix();

    glTranslatef(0.0f, 2.5f, 0.0f);

    if (!isForward) glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glScalef(2.2f, 0.8f, 4.5f);
    glutSolidSphere(1.0f, 32, 32);
    glPopMatrix();

    setSolid(COLOR_DARK_GREY);
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, -0.5f);
    glScalef(1.8f, 0.6f, 2.5f);
    glutSolidSphere(1.0f, 24, 24);
    glPopMatrix();

    if (currentTimeOfDay != TIME_DAY) {
        const GLfloat* neonCol = RAINBOW_COLORS[colorIndex % 6];
        setNeon(neonCol, 1.5f);

        glDisable(GL_LIGHTING);
        glLineWidth(2.0f);
        glPushMatrix();
        glScalef(2.3f, 0.85f, 4.6f);
        glutWireSphere(1.0f, 16, 16);
        glPopMatrix();
        glEnable(GL_LIGHTING);

        glPushMatrix();
        glTranslatef(0.0f, -0.8f, 0.0f);
        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
        glutSolidTorus(0.3f, 2.5f, 16, 32);
        glPopMatrix();

        setNeon(COLOR_NEON_CYAN_ALT, 2.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, 2.5f);
        glScalef(1.5f, 0.5f, 2.5f);
        glutSolidSphere(1.f, 16, 16);
        glPopMatrix();
    }

    glPopMatrix();
}

/**
 * @brief Renders a futuristic ground vehicle (Type 2).
 * Features angular design, glowing wheels, and directional lights.
 *
 * @param isForward If true, faces forward; otherwise, rotates 180 degrees.
 * @param colorIndex Index for selecting the vehicle color scheme.
 */
void drawGroundCarFuture(bool isForward, int colorIndex) {
    if (currentTimeOfDay == TIME_DAY) {
        const GLfloat* macCol = RAINBOW_COLORS[(colorIndex + 2) % 6];
        GLfloat pastelCol[] = { macCol[0] * 0.6f + 0.4f, macCol[1] * 0.6f + 0.4f, macCol[2] * 0.6f + 0.4f };
        setSolid(pastelCol);
    }
    else {
        setSolid(COLOR_DEEP_GREY);
    }

    glPushMatrix();
    glScalef(2.3f, 2.5f, 2.5f);
    glTranslatef(0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glTranslatef(0.0f, -0.9f, 0.0f);
    glScalef(1.4f, 1.0f, 2.8f);
    drawShadowBlob(1.8f, 0.6f);
    glPopMatrix();

    if (!isForward) glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

    glPushMatrix();
    glScalef(2.5f, 1.2f, 5.0f);
    glutSolidCube(1.0f);
    glPopMatrix();

    setSolid(COLOR_DARK_GREY);
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, -1.5f);
    glRotatef(-30.0f, 1.0f, 0.0f, 0.0f);
    glScalef(2.4f, 0.2f, 1.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    if (currentTimeOfDay != TIME_DAY) {
        const GLfloat* neonCol = RAINBOW_COLORS[colorIndex % 6];
        setNeon(neonCol, 2.0f);

        float wheelZ[] = { -1.8f, 1.8f };
        float wheelX[] = { -1.3f, 1.3f };
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                glPushMatrix();
                glTranslatef(wheelX[j], -0.2f, wheelZ[i]);
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
                glutSolidTorus(0.15f, 0.6f, 12, 24);
                glPopMatrix();
            }
        }

        setNeon(COLOR_WHITE, 3.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.2f, -2.55f);
        glScalef(2.2f, 0.1f, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();

        setNeon(COLOR_NEON_RED, 3.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.2f, 2.55f);
        glScalef(2.2f, 0.1f, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();
    }
    else {
        GLfloat rimCol[] = { 0.0f, 0.8f, 0.6f };
        setSolid(rimCol);

        float wheelZ[] = { -1.8f, 1.8f };
        float wheelX[] = { -1.3f, 1.3f };
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                glPushMatrix();
                glTranslatef(wheelX[j], -0.2f, wheelZ[i]);
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
                glutSolidTorus(0.2f, 0.5f, 12, 24);
                glPopMatrix();
            }
        }
    }

    glPopMatrix();
}
/**
 * @brief Renders a UFO with movement logic, scaling, and dynamic thruster effects.
 *
 * @param x World X position.
 * @param y World Y position.
 * @param zOffset Initial Z offset for animation.
 * @param speed Movement speed along the Z-axis.
 * @param scale Uniform scale factor (default 1.0).
 */
void drawUFO(float x, float y, float zOffset, float speed, float scale = 1.0f) {
    float moveZ = realTime * speed;
    float absZ = zOffset + moveZ;
    float relativeZ = absZ - worldOffsetZ;

    while (relativeZ > LOOP_LEN / 2.0f) relativeZ -= LOOP_LEN;
    while (relativeZ < -LOOP_LEN / 2.0f) relativeZ += LOOP_LEN;

    if (relativeZ < -500 || relativeZ > 200) return;

    glPushMatrix();
    glTranslatef(x, y, relativeZ);
    glScalef(scale, scale, scale);

    float hover = sinf(realTime * 2.0f) * 0.5f;
    glTranslatef(0.0f, hover, 0.0f);
    glRotatef(realTime * 60.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(10.0f, 1.0f, 0.0f, 0.0f);

    // Body
    if (currentTimeOfDay == TIME_DAY) setSolid(COLOR_SILVER);
    else setSolid(COLOR_DARK_GREY);
    glPushMatrix(); glScalef(1.0f, 0.2f, 1.0f); glutSolidSphere(3.0f, 32, 32); glPopMatrix();

    // Cockpit
    glEnable(GL_BLEND); glDepthMask(GL_FALSE);
    if (currentTimeOfDay == TIME_DAY) glColor4f(0.0f, 0.6f, 0.8f, 0.6f);
    else glColor4f(0.0f, 0.9f, 1.0f, 0.4f);
    glPushMatrix(); glTranslatef(0.0f, 0.8f, 0.0f); glutSolidSphere(1.2f, 24, 24); glPopMatrix();
    glDepthMask(GL_TRUE); glDisable(GL_BLEND);

    // Energy Ring
    setNeon(COLOR_CYBER_PINK, 2.0f);
    glPushMatrix(); glRotatef(realTime * -120.0f, 0.0f, 1.0f, 0.0f); glRotatef(90.0f, 1.0f, 0.0f, 0.0f); glutWireTorus(0.07f, 4.f, 8, 40); glPopMatrix();

    // Thruster (Volumetric Beam)
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    GLfloat beamColor[4];
    if (currentTimeOfDay == TIME_NIGHT) {
        beamColor[0] = 0.0f; beamColor[1] = 0.9f; beamColor[2] = 1.0f; beamColor[3] = 0.5f;
    }
    else if (currentTimeOfDay == TIME_SUNSET) {
        beamColor[0] = 1.0f; beamColor[1] = 0.5f; beamColor[2] = 0.0f; beamColor[3] = 0.5f;
    }
    else {
        beamColor[0] = 0.8f; beamColor[1] = 0.9f; beamColor[2] = 1.0f; beamColor[3] = 0.6f;
    }

    glMaterialfv(GL_FRONT, GL_EMISSION, beamColor);
    glColor4fv(beamColor);
    glPushMatrix();
    glTranslatef(0.0f, -5.5f, 0.0f);
    float thrustFlicker = 1.0f + sinf(realTime * 25.0f) * 0.1f;
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glScalef(1.0f, 1.0f, thrustFlicker);
    glutSolidCone(1.8f, 6.0f, 24, 10);
    glPopMatrix();

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, noEmission);
    glPopMatrix();
}
/**
 * @brief Renders a single train car with textures and conditional lighting.
 * Adapts colors for day, sunset, and night modes.
 *
 * @param neonColor Neon color used for night mode details.
 */
void drawTrainCar(const GLfloat* neonColor) {
    const GLfloat* bodyCol;
    const GLfloat* winCol;

    if (currentTimeOfDay == TIME_DAY) {
        bodyCol = COLOR_WHITE;
        static GLfloat dayWin[] = { 0.0f, 0.8f, 0.7f };
        winCol = dayWin;
    }
    else if (currentTimeOfDay == TIME_SUNSET) {
        static GLfloat duskBody[] = { 1.0f, 0.8f, 0.8f };
        bodyCol = duskBody;
        static GLfloat duskWin[] = { 1.0f, 0.6f, 0.0f };
        winCol = duskWin;
    }
    else {
        bodyCol = COLOR_DARK_GREY;
        winCol = neonColor;
    }

    // 1. Train Body (Textured)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.skyTrain);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    if (currentTimeOfDay == TIME_NIGHT) setSolid(bodyCol);
    else setSolid(bodyCol);

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    drawTexturedBox(15.0f, 9.0f, 10.0f, tex.skyTrain);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);

    // 2. Top Connector
    glPushMatrix();
    glTranslatef(0.0f, 5.0f, 0.0f);
    setSolid(COLOR_DARK_GREY);
    glScalef(0.2f, 1.0f, 0.2f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // 3. Side Decorations & Windows
    for (int side = -1; side <= 1; side += 2) {
        float zPos = side * 5.05f;

        // Side Strips
        if (currentTimeOfDay != TIME_DAY) setNeon(winCol, 1.5f);
        else setSolid(winCol);

        glPushMatrix();
        glTranslatef(0.0f, 2.5f, zPos);
        glScalef(14.5f, 0.5f, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(0.0f, -2.5f, zPos);
        glScalef(14.5f, 0.5f, 0.1f);
        glutSolidCube(1.0f);
        glPopMatrix();

        // Round Windows
        for (int i = -1; i <= 1; i++) {
            glPushMatrix();
            glTranslatef(i * 4.0f, 0.0f, zPos);

            if (currentTimeOfDay == TIME_DAY) setSolid(winCol);
            else if (currentTimeOfDay == TIME_SUNSET) setNeon(winCol, 1.5f);
            else setNeon(COLOR_NEON_YELLOW, 2.0f);

            glScalef(1.2f, 1.2f, 0.2f);
            glutSolidSphere(1.0f, 16, 16);
            glPopMatrix();
        }
    }

    if (currentTimeOfDay != TIME_DAY) glEnable(GL_LIGHTING);
}
/**
 * @brief Renders multiple trains on different tracks with varying speeds and offsets.
 * Includes sinusoidal movement logic and dynamic ground shadow projection.
 */
void drawTrain() {
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);

    struct TrainConfig {
        float x, y, speed;
        float offset;
        const GLfloat* color;
        bool reverse;
    };

    TrainConfig trains[] = {
        { -25.0f, 43.0f, 40.0f, 0.0f,    COLOR_TRAIN_RED,    false },
        { -25.0f, 45.0f, 40.0f, 1800.0f, COLOR_TRAIN_RED,    false },

        {  25.0f, 50.0f, 35.0f, 0.0f,    COLOR_TRAIN_BLUE,   true  },
        {  25.0f, 48.0f, 35.0f, 240.0f,  COLOR_TRAIN_BLUE,   true  },
        {  25.0f, 45.0f, 35.0f, 520.0f,  COLOR_TRAIN_BLUE,   true  },

        { -30.0f, 58.0f, 60.0f, 150.0f,  COLOR_CYBER_PURPLE, false },
        { -30.0f, 58.0f, 60.0f, 1000.0f, COLOR_CYBER_PURPLE, false },

        {  20.0f, 70.0f, 45.0f, 1000.0f, COLOR_NEON_GREEN,   false },
        {  20.0f, 70.0f, 45.0f, 250.0f,  COLOR_NEON_GREEN,   false }
    };

    int numTrains = sizeof(trains) / sizeof(trains[0]);

    for (int t = 0; t < numTrains; t++) {
        float moveDist = (realTime * trains[t].speed) + trains[t].offset;

        float absZ = moveDist;
        if (trains[t].reverse) absZ = -moveDist;

        float relativeZ = absZ - worldOffsetZ;

        while (relativeZ > LOOP_LEN / 2.0f) relativeZ -= LOOP_LEN;
        while (relativeZ < -LOOP_LEN / 2.0f) relativeZ += LOOP_LEN;

        if (relativeZ < CULL_BACK || relativeZ > CULL_FRONT) continue;

        const GLfloat* col = trains[t].color;
        if (currentTimeOfDay == TIME_DAY) col = COLOR_DARK_GREY;

        glPushMatrix();

        float wave = relativeZ * 0.03f;
        float offsetY = sinf(wave) * 5.0f;
        float offsetX = cosf(wave) * 3.0f;

        glTranslatef(trains[t].x + offsetX, trains[t].y + offsetY, relativeZ);

        float bankAngle = -sinf(wave) * 12.0f;
        if (trains[t].reverse) bankAngle = -bankAngle;

        glRotatef(bankAngle, 0.0f, 0.0f, 1.0f);
        if (trains[t].reverse) glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

        for (int i = 0; i < 8; i++) {
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, -i * 15.0f);

            // Ground Shadow Projection
            glPushMatrix();
            float currentHeight = trains[t].y + offsetY;
            glTranslatef(0.0f, -currentHeight + 2.0f, 0.0f);
            glScalef(0.3f, 0.3f, 2.0f);
            drawSquareShadow(3.5f, 0.80f, 0.96f);
            glPopMatrix();

            drawTrainCar(col);
            glPopMatrix();
        }
        glPopMatrix();
    }
}
/**
 * @brief Renders a standalone traffic spaceship.
 * Features a solid body, a rotating neon ring based on the color index, and wireframe wings.
 *
 * @param colorIdx Index (0-5) to select the neon ring color.
 */
void drawTrafficShip(int colorIdx) {
    if (currentTimeOfDay == TIME_DAY) setSolid(COLOR_SILVER);
    else setSolid(COLOR_DARK_GREY);

    glPushMatrix();
    glScalef(1.0f, 0.4f, 1.5f);
    glutSolidSphere(3.0f, 20, 20);
    glPopMatrix();

    const GLfloat* neonCol = RAINBOW_COLORS[colorIdx % 6];

    setNeon(neonCol, 2.0f);

    glPushMatrix();
    glScalef(1.f, 0.2f, 1.f);
    glRotatef(realTime * 200.0f, 0, 0, 1);
    glutWireTorus(0.5f, 7.0f, 10, 30);
    glPopMatrix();

    setNeon(COLOR_CYBER_CYAN, 2.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex3f(-4.0f, 0.0f, 2.0f); glVertex3f(-6.0f, 0.0f, -4.0f);
    glVertex3f(4.0f, 0.0f, 2.0f); glVertex3f(6.0f, 0.0f, -4.0f);
    glEnd();
}
/**
 * @brief Renders the entire traffic system, including ground vehicles, trains, drones, and UFOs.
 * Orchestrates movement, spawning, and culling for all traffic types.
 */
void drawTrafficSystem() {

    // PART A: Ground & Low Altitude Vehicles
    float laneXCoords[] = { laneCenters[0], laneCenters[1], laneCenters[2], laneCenters[3] };
    bool isHoverType[] = { true, false, false, true };
    bool isMovingForward[] = { true, true, false, false };
    float baseSpeeds[] = { 120.0f, 80.0f, 80.0f, 120.0f };
    int colorIndices[] = { 0, 1, 4, 5 };

    int carsPerLane = 3;
    float trafficSpan = LOOP_LEN * 1.5f;

    for (int lane = 0; lane < 4; lane++) {
        for (int i = 0; i < carsPerLane; i++) {
            float startOffset = -trafficSpan / 2.0f + i * (trafficSpan / carsPerLane);
            float speedDir = isMovingForward[lane] ? -1.0f : 1.0f;
            float finalSpeed = baseSpeeds[lane] + (rand() % 20);
            float randomOffset = (rand() % 500);

            float moveDist = realTime * finalSpeed * speedDir;
            float absZ = startOffset + moveDist + randomOffset;
            float relativeZ = absZ - worldOffsetZ;

            while (relativeZ > LOOP_LEN / 2.0f) relativeZ -= LOOP_LEN;
            while (relativeZ < -LOOP_LEN / 2.0f) relativeZ += LOOP_LEN;

            if (relativeZ < CULL_BACK || relativeZ > CULL_FRONT) continue;

            glPushMatrix();
            glTranslatef(laneXCoords[lane], 0.0f, relativeZ);
            if (isHoverType[lane]) {
                drawHoverCar(isMovingForward[lane], colorIndices[lane]);
            }
            else {
                drawGroundCarFuture(isMovingForward[lane], colorIndices[lane]);
            }
            glPopMatrix();
        }
    }

    // PART B: Sky Trains
    drawTrain();

    // PART C: Drone Swarm
    for (int i = 0; i < 8; i++) {
        float droneSpeed = 20.0f;
        float startZ = -i * 100.0f;
        float absZ = startZ + realTime * droneSpeed;
        float relZ = absZ - worldOffsetZ;
        while (relZ > 200) relZ -= LOOP_LEN;
        while (relZ < -LOOP_LEN) relZ += LOOP_LEN;

        glPushMatrix();
        float angle = realTime * 2.0f + i;
        float droneX = cosf(angle) * 30.0f;
        float droneY = 40.0f + sinf(angle) * 10.0f;

        glPushMatrix();
        glTranslatef(droneX, 0.1f, relZ);
        float shadowScale = 1.0f + (droneY - 30.0f) * 0.05f;
        drawShadowBlob(2.2f * shadowScale, 0.85f);
        glPopMatrix();

        glTranslatef(droneX, droneY, relZ);
        glRotatef(angle * 50.0f, 0, 1, 0);
        drawDrone();
        glPopMatrix();
    }


    // PART D: Giant UFOs
    // UFO 1: Figure-8 Hover
    float ufo1Time = realTime * 0.5f;
    float ufo1X = sinf(ufo1Time) * 80.0f;
    float ufo1Z = sinf(ufo1Time * 2.0f) * 40.0f;
    float ufo1BaseZ = -100.0f;

    drawUFO(ufo1X, 95.0f, ufo1BaseZ + ufo1Z + 30.f, 0.0f, 1.5f);
    drawUFO(ufo1X, 105.0f, ufo1BaseZ + ufo1Z, 0.0f, 1.5f);

    // UFO 2: Giant Cruiser
    float ufo2Time = realTime * 0.3f;
    float ufo2X = cosf(ufo2Time) * 200.0f;
    float ufo2Z = sinf(ufo2Time) * 200.0f - 300.0f;

    drawUFO(ufo2X, 90.0f, ufo2Z, 0.0f, 4.0f);

    // UFO 3: High Altitude Fleet
    for (int k = 0; k < 6; k++) {
        float fleetSpeed = 150.0f;
        float startZ = k * 500.0f;
        float currentZ = startZ + realTime * fleetSpeed;

        float relZ = currentZ - worldOffsetZ;
        while (relZ > LOOP_LEN / 2.0f) relZ -= LOOP_LEN;
        while (relZ < -LOOP_LEN / 2.0f) relZ += LOOP_LEN;

        if (relZ < CULL_BACK || relZ > CULL_FRONT) continue;

        glPushMatrix();
        float fleetX = (k - 1) * 120.0f;
        float fleetY = 90.0f + sinf(realTime + k) * 10.0f;

        drawUFO(fleetX, fleetY, relZ, 0.0f, 1.5f);
        glPopMatrix();
    }



    // UFO4: Low Altitude Patrol
    int ufoLanes[] = { 1, 2 };
    float ufoHeight = 25.0f;

    for (int k = 0; k < 2; k++) {
        int laneIdx = ufoLanes[k];
        float laneX = laneCenters[laneIdx];
        bool isForward = isMovingForward[laneIdx];
        float speedDir = isForward ? -1.0f : 1.0f;
        float speed = 60.0f;

        for (int i = 0; i < 6; i++) {
            float startOffset = i * 800.0f;
            float moveDist = realTime * speed * speedDir;
            float absZ = startOffset + moveDist;

            float relZ = absZ - worldOffsetZ;
            while (relZ > LOOP_LEN / 2.0f) relZ -= LOOP_LEN;
            while (relZ < -LOOP_LEN / 2.0f) relZ += LOOP_LEN;

            if (relZ < CULL_BACK || relZ > CULL_FRONT) continue;

            glPushMatrix();
            float bobbing = sinf(realTime * 3.0f + i) * 2.0f;
            drawUFO(laneX, ufoHeight + bobbing, relZ, 0.0f, 1.2f);
            glPopMatrix();
        }
    }
    // PART E: Free-Flying Ships
    for (int i = 0; i < 8; i++) {
        float speed = 80.0f + (i * 20.0f);
        float offset = i * 500.0f;
        float moveDist = (realTime * speed) + offset;

        bool reverse = (i % 2 != 0);
        if (reverse) moveDist = -moveDist;

        float relZ = moveDist - worldOffsetZ;
        while (relZ > LOOP_LEN / 2.0f) relZ -= LOOP_LEN;
        while (relZ < -LOOP_LEN / 2.0f) relZ += LOOP_LEN;

        if (relZ < CULL_BACK || relZ > CULL_FRONT) continue;

        glPushMatrix();

        float xPos = ((i % 5) - 2) * 60.0f;
        float yPos = 20.0f + (i % 3) * 30.0f;

        glTranslatef(xPos, yPos, relZ);
        if (reverse) glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

        drawTrafficShip(i);
        glPopMatrix();
    }
}
/**
 * @brief Renders a simplified Shinkansen-style train.
 * Features a streamlined head, modular body carriages, and tail lights.
 * Appearance adapts to day/night cycles.
 *
 * @param isRight If true, rotates the train 180 degrees.
 * @param length Number of carriage units (recommended 3-8).
 */
void drawShinkansen(bool isRight, int length) {
    glPushMatrix();

    const GLfloat* bodyColor;
    const GLfloat* glowColor;
    const GLfloat* windowColor;

    if (currentTimeOfDay == TIME_DAY) {
        static GLfloat macaronWhite[] = { 0.95f, 0.98f, 1.0f };
        static GLfloat mintGreen[] = { 0.4f, 1.0f, 0.8f };
        static GLfloat darkWin[] = { 0.2f, 0.2f, 0.3f };
        bodyColor = macaronWhite;
        glowColor = mintGreen;
        windowColor = darkWin;
    }
    else {
        bodyColor = COLOR_SILVER;
        glowColor = isRight ? COLOR_NEON_CYAN_ALT : COLOR_NEON_BLUE_ALT;
        windowColor = COLOR_BLACK;
    }

    glScalef(12.0f, 12.0f, 12.0f);
    if (isRight) glRotatef(180.0f, 0.0f, 1.0f, 0.0f);

    float carLen = 8.0f;
    float totalLen = length * carLen;

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -totalLen / 2.0f - 2.0f);

    setSolid(bodyColor);
    glPushMatrix();
    glScalef(1.2f, 1.0f, 3.0f);
    glutSolidSphere(0.8f, 16, 16);
    glPopMatrix();

    setNeon(COLOR_NEON_YELLOW, 3.0f);
    glPushMatrix();
    glTranslatef(0.4f, -0.2f, -2.5f); glutSolidSphere(0.2f, 8, 8);
    glTranslatef(-0.8f, 0.0f, 0.0f);  glutSolidSphere(0.2f, 8, 8);
    glPopMatrix();
    glPopMatrix();

    // Body Cars
    for (int i = 0; i < length; i++) {
        float zPos = -totalLen / 2.0f + carLen / 2.0f + i * carLen;

        glPushMatrix();
        glTranslatef(0.0f, 0.0f, zPos);

        setSolid(bodyColor);
        glPushMatrix();
        glScalef(1.2f, 1.0f, carLen - 0.2f);
        glutSolidCube(1.0f);
        glPopMatrix();

        if (currentTimeOfDay != TIME_DAY) setNeon(glowColor, 1.5f);
        else setSolid(windowColor);

        for (int w = 0; w < 3; w++) {
            float winZ = (w - 1) * (carLen / 3.5f);

            glPushMatrix(); glTranslatef(-0.61f, 0.2f, winZ);
            glScalef(0.1f, 0.4f, 1.5f); glutSolidCube(1.0f); glPopMatrix();

            glPushMatrix(); glTranslatef(0.61f, 0.2f, winZ);
            glScalef(0.1f, 0.4f, 1.5f); glutSolidCube(1.0f); glPopMatrix();
        }

        setSolid(COLOR_DARK_GREY);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, carLen / 2.0f);
        glScalef(1.1f, 0.9f, 0.2f);
        glutSolidCube(1.0f);
        glPopMatrix();

        glPopMatrix();
    }

    // Tail
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, totalLen / 2.0f + 2.0f);
    setSolid(bodyColor);
    glPushMatrix();
    glScalef(1.2f, 1.0f, 2.0f);
    glutSolidSphere(0.7f, 16, 16);
    glPopMatrix();

    setNeon(COLOR_NEON_RED, 3.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 1.5f);
    glScalef(0.8f, 0.1f, 0.1f);
    glutSolidCube(1.0f);
    glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}
/**
 * @brief Renders the underground train traffic system.
 * Manages two high-speed trains moving in opposite directions with positional looping.
 */
void drawUndergroundTraffic() {
    float trainSpeed = 200.0f;

    // 1. Left Train (Forward)
    float moveLeft = realTime * trainSpeed;
    float relZLeft = -moveLeft - worldOffsetZ;

    while (relZLeft > 1000.0f) relZLeft -= 2000.0f;
    while (relZLeft < -1000.0f) relZLeft += 2000.0f;

    if (relZLeft > CULL_BACK && relZLeft < CULL_FRONT) {
        glPushMatrix();
        glTranslatef(-35.0f, -40.0f, relZLeft);
        drawShinkansen(false, 5);
        glPopMatrix();
    }

    // 2. Right Train (Backward)
    float moveRight = realTime * trainSpeed + 1000.0f;
    float relZRight = moveRight - worldOffsetZ;

    while (relZRight > 1000.0f) relZRight -= 2000.0f;
    while (relZRight < -1000.0f) relZRight += 2000.0f;

    if (relZRight > -800.0f && relZRight < 400.0f) {
        glPushMatrix();
        glTranslatef(35.0f, -40.0f, relZRight);
        drawShinkansen(true, 5);
        glPopMatrix();
    }
}

//City Generation
/**
 * @brief Renders a complete city segment including roads, sidewalks, buildings, and street furniture.
 * Incorporates procedural generation for buildings, holographic ads, vegetation, and lighting.
 *
 * @param index Segment index for procedural randomization.
 */
void drawCitySegment(int index) {
    srand(index * 12345);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    // 1. Base & Pavement
    setSolid(COLOR_DEEP_GREY);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.cyberPavement);
    glColor3f(0.6f, 0.6f, 0.65f);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float baseDepth = SEGMENT_SPACING;
    float baseHeight = 30.0f;
    float roadWidth = 620.0f;
    float gapWidth = 74.0f;

    // Left Base
    glPushMatrix();
    glTranslatef(-(roadWidth / 4.0f + gapWidth / 2.0f) - 10.0f, -baseHeight / 2.0f, 0.0f);
    drawTexturedBox(roadWidth / 2.0f + 10.0f, baseHeight, baseDepth, tex.cyberPavement);
    glPopMatrix();

    // Right Base
    glPushMatrix();
    glTranslatef((roadWidth / 4.0f + gapWidth / 2.0f) + 10.0f, -baseHeight / 2.0f, 0.0f);
    drawTexturedBox(roadWidth / 2.0f + 10.0f, baseHeight, baseDepth, tex.cyberPavement);
    glPopMatrix();

    // 2. Road Surface
    if (currentTimeOfDay == TIME_NIGHT) {
        GLfloat wetSpec[] = { 0.8f, 0.8f, 0.8f, 1.0f };
        glMaterialfv(GL_FRONT, GL_SPECULAR, wetSpec);
        glMaterialf(GL_FRONT, GL_SHININESS, 100.0f);
    }
    else {
        GLfloat drySpec[] = { 0.1f, 0.1f, 0.1f, 1.0f };
        glMaterialfv(GL_FRONT, GL_SPECULAR, drySpec);
        glMaterialf(GL_FRONT, GL_SHININESS, 10.0f);
    }

    if (isTransparentFloor) {
        glEnable(GL_BLEND);
        glDepthMask(GL_FALSE);
        glColor4f(0.0f, 0.8f, 1.0f, 0.2f);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-40.0f, 0.0f, -SEGMENT_SPACING / 2);
        glVertex3f(-40.0f, 0.0f, SEGMENT_SPACING / 2);
        glVertex3f(40.0f, 0.0f, SEGMENT_SPACING / 2);
        glVertex3f(40.0f, 0.0f, -SEGMENT_SPACING / 2);
        glEnd();

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
    else {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, tex.roadMain);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        if (currentTimeOfDay == TIME_DAY) glColor3f(1.0f, 1.0f, 1.0f);
        else glColor3f(0.6f, 0.6f, 0.6f);

        glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);

        float repX = 2.0f;
        float repZ = 1.0f;

        glTexCoord2f(0.0f, 0.0f);   glVertex3f(-40.0f, 0.0f, -SEGMENT_SPACING / 2);
        glTexCoord2f(0.0f, repZ);   glVertex3f(-40.0f, 0.0f, SEGMENT_SPACING / 2);
        glTexCoord2f(repX, repZ);   glVertex3f(40.0f, 0.0f, SEGMENT_SPACING / 2);
        glTexCoord2f(repX, 0.0f);   glVertex3f(40.0f, 0.0f, -SEGMENT_SPACING / 2);
        glEnd();

        glDisable(GL_TEXTURE_2D);
    }

    // 3. Road Markings (Neon Lines)
    glEnable(GL_BLEND);
    glDisable(GL_LIGHTING);
    glLineWidth(3.0f);

    for (int i = 0; i < 6; i++) {
        if (currentTimeOfDay == TIME_NIGHT) glColor3fv(RAINBOW_COLORS[i]);
        else glColor3f(RAINBOW_COLORS[i][0] * 0.7f, RAINBOW_COLORS[i][1] * 0.7f, RAINBOW_COLORS[i][2] * 0.7f);

        float lineX = stripPositions[i];
        float lineWidth = 1.0f;

        glBegin(GL_QUADS);
        glVertex3f(lineX - lineWidth / 2.0f, 0.15f, -SEGMENT_SPACING / 2);
        glVertex3f(lineX - lineWidth / 2.0f, 0.15f, SEGMENT_SPACING / 2);
        glVertex3f(lineX + lineWidth / 2.0f, 0.15f, SEGMENT_SPACING / 2);
        glVertex3f(lineX + lineWidth / 2.0f, 0.15f, -SEGMENT_SPACING / 2);
        glEnd();
    }

    // Crosswalk (Zebra Crossing)
    {
        const GLfloat* walkColor = RAINBOW_COLORS[abs(index) % 6];
        if (currentTimeOfDay == TIME_NIGHT) glColor3fv(walkColor);
        else glColor3f(walkColor[0] * 0.8f, walkColor[1] * 0.8f, walkColor[2] * 0.8f);

        float swWidth = 10.0f;
        float stripeW = 3.0f;
        float gap = 2.0f;
        float startZ = -SEGMENT_SPACING / 2.0f + 2.0f;

        for (float z = startZ; z < SEGMENT_SPACING / 2.0f; z += (stripeW + gap)) {
            glBegin(GL_QUADS);
            glVertex3f(-swWidth / 2, 0.1f, z);
            glVertex3f(-swWidth / 2, 0.1f, z + stripeW);
            glVertex3f(swWidth / 2, 0.1f, z + stripeW);
            glVertex3f(swWidth / 2, 0.1f, z);
            glEnd();
        }
    }
    glEnable(GL_LIGHTING);

    // 4. Side Elements (Buildings, Trees, Lamps)
    for (int side = -1; side <= 1; side += 2) {
        float lampX = side * 37.0f;
        float greenX = side * 42.0f;
        float buildX = side * 65.0f;

        // Holographic Ad (Empty Space)
        if (rand() % 3 == 0) {
            glPushMatrix();
            float sideOffset = (float)side;
            float holoX = sideOffset * (45.0f + (rand() % 10));
            float holoZ = (float)(rand() % 40 - 30);

            glTranslatef(holoX, 8.0f, holoZ);
            drawHologram(6.0f, (rand() % 2 == 0) ? 0 : 2);
            glPopMatrix();
        }

        // Vegetation
        bool isTreeSpot = (rand() % 10 < 7);
        if (isTreeSpot) {
            glPushMatrix();
            glTranslatef(greenX, 0.0f, (float)(rand() % 40 - 20));
            glRotatef((rand() % 10 - 5.0f), 0, 0, 1);
            drawCyberTree(15.0f + (rand() % 15));
            glPopMatrix();

            if (rand() % 2 == 0) {
                glPushMatrix();
                glTranslatef(greenX - side * 2.0f, 0.0f, (float)(rand() % 40 - 20));
                glScalef(0.6f, 0.6f, 0.6f);
                drawCyberBush();
                glPopMatrix();
            }
        }
        else {
            glPushMatrix();
            glTranslatef(greenX, 0.0f, -10.0f);
            drawCyberBush();
            glPopMatrix();
            glPushMatrix();
            glTranslatef(greenX, 0.0f, 10.0f);
            drawCyberBush();
            glPopMatrix();
        }

        // Buildings
        int stacks = 2 + rand() % 3;
        float currentH = 0.0f;
        int colIdx = rand() % 6;
        const GLfloat* mainColor = BUILD_COLORS_MACARON[colIdx];

        float baseW = 20.0f + rand() % 15;
        float baseD = 20.0f + rand() % 20;

        // Building Shadow
        glPushMatrix();
        glTranslatef(buildX, 0.1f, 0.0f);
        drawSquareShadow(baseW * 1.2f, baseD * 1.2f, 0.7f);
        glPopMatrix();

        // Building Stack Loop
        for (int k = 0; k < stacks; k++) {
            float w = (k == 0) ? baseW : (20.0f + rand() % 15);
            float d = (k == 0) ? baseD : (20.0f + rand() % 20);
            float h = 30.0f + rand() % 50;

            int wallRand = rand() % 4;
            GLuint currentWallTex = tex.walls[wallRand];

            glPushMatrix();
            glTranslatef(buildX, currentH + h / 2.0f, 0.0f);
            float shiftX = (rand() % 5 - 2.5f);
            glTranslatef(shiftX, 0.0f, 0.0f);

            drawRichBuildingUnit(w, h, d, mainColor, k == 0, k, currentWallTex);
            glPopMatrix();

            // Background Buildings
            float backBuildX = side * 185.0f;
            float backH = 75.0f + rand() % 100;
            float backW = 30.0f;
            float backD = 30.0f;
            float zOffset = (rand() % 20) - 10.0f;

            glPushMatrix();
            glTranslatef(backBuildX, backH / 2.0f + 0.1f, zOffset);

            GLfloat bodyColor[3];
            GLfloat outlineColor[3];
            int colorType = rand() % 2;

            if (currentTimeOfDay == TIME_DAY) {
                if (colorType == 0) { bodyColor[0] = 0.85f; bodyColor[1] = 0.85f; bodyColor[2] = 0.85f; }
                else { bodyColor[0] = 0.40f; bodyColor[1] = 0.45f; bodyColor[2] = 0.50f; }
                outlineColor[0] = 0.2f; outlineColor[1] = 0.2f; outlineColor[2] = 0.3f;
            }
            else if (currentTimeOfDay == TIME_SUNSET) {
                if (colorType == 0) { bodyColor[0] = 0.6f; bodyColor[1] = 0.3f; bodyColor[2] = 0.0f; }
                else { bodyColor[0] = 0.4f; bodyColor[1] = 0.1f; bodyColor[2] = 0.1f; }
                outlineColor[0] = 1.0f; outlineColor[1] = 0.6f; outlineColor[2] = 0.0f;
            }
            else {
                if (colorType == 0) { bodyColor[0] = 0.15f; bodyColor[1] = 0.15f; bodyColor[2] = 0.20f; }
                else { bodyColor[0] = 0.35f; bodyColor[1] = 0.0f;  bodyColor[2] = 0.45f; }
                outlineColor[0] = 0.0f; outlineColor[1] = 0.9f; outlineColor[2] = 1.0f;
            }

            setSolid(bodyColor);
            glScalef(backW, backH, backD);
            glutSolidCube(1.0f);

            glDisable(GL_LIGHTING);
            glLineWidth(1.5f);
            glColor3fv(outlineColor);

            glPushMatrix();
            glScalef(1.02f, 1.02f, 1.02f);
            glutWireCube(1.0f);
            glPopMatrix();

            glEnable(GL_LIGHTING);
            glPopMatrix();

            // Wall Screen Ads
            if (rand() % 10 < 4) {
                glPushMatrix();
                float screenX = buildX - side * (w / 2.0f + 1.2f);
                float screenY = currentH + h / 2.0f + rand() % 10;

                glTranslatef(screenX, screenY, 0.0f);
                glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

                const GLfloat* screenCol = RAINBOW_COLORS[rand() % 6];
                int screenType = rand() % 2;

                drawCyberScreen(d * 0.8f, h * 0.6f, screenCol, screenType);
                glPopMatrix();
            }

            // Cross-Street Banner
            if (rand() % 20 == 0) {
                glPushMatrix();
                glTranslatef(0.f, 40.0f, 0.0f);
                glTranslatef(0.0f, currentH + 20.f, 0.0f);

                const GLfloat* bannerCol = (rand() % 2 == 0) ? COLOR_NEON_RED : COLOR_NEON_YELLOW;
                float randomWidth = 80.0f + (rand() % 20);

                drawCyberScreen(randomWidth, 12.0f, bannerCol, 1);
                glPopMatrix();
            }

            currentH += h;

            // Rooftop Ads
            if (k == stacks - 1 && rand() % 10 < 3) {
                glPushMatrix();
                glTranslatef(buildX, currentH + h / 2.0f - 24.0f, 0.0f);
                glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
                const GLfloat* topCol = RAINBOW_COLORS[rand() % 6];
                drawCyberScreen(30.0f, 30.0f, topCol, 2);
                glPopMatrix();
            }

            // Street Level Ads
            if (rand() % 6 == 0) {
                glPushMatrix();
                float holoX = side * 40.0f;
                float holoZ = (float)(rand() % 40 - 20);
                float holoY = 12.0f;

                glTranslatef(holoX, holoY, holoZ);

                int style = (rand() % 2 == 0) ? 0 : 2;
                drawHologram(6.0f, style);
                glPopMatrix();
            }
        }

        // Street Lamps
        glPushMatrix();
        glTranslatef(lampX, 0.0f, 0.0f);
        drawRichLamp(side, 32.0f + (rand() % 5));
        glPopMatrix();
    }
}

//Character & FX
/**
 * @brief Renders a floating mechanical cloud.
 * Features breathing animations, rotating rings, and a bottom jet effect.
 */
void drawMechanicalCloud() {
    float breath = sinf(realTime * 2.0f) * 0.1f;
    float scaleBreath = 1.0f + sinf(realTime * 3.0f) * 0.05f;

    glPushMatrix();
    glTranslatef(0.0f, breath, 0.0f);

    glScalef(4.0f, 4.0f, 4.0f);
    glScalef(scaleBreath, scaleBreath, scaleBreath);

    // A. Cloud Body
    if (currentTimeOfDay == TIME_NIGHT) {
        setNeon(COLOR_WHITE, 0.6f);
    }
    else if (currentTimeOfDay == TIME_SUNSET) {
        static GLfloat warmCloud[] = { 1.0f, 0.9f, 0.8f };
        setSolid(warmCloud);
    }
    else {
        setSolid(COLOR_WHITE);
    }

    glPushMatrix(); glScalef(2.0f, 0.8f, 1.5f); glutSolidSphere(1.0f, 20, 20); glPopMatrix();
    glPushMatrix(); glTranslatef(1.2f, 0.2f, -0.5f); glutSolidSphere(0.8f, 16, 16); glPopMatrix();
    glPushMatrix(); glTranslatef(-1.2f, 0.3f, 0.5f); glutSolidSphere(0.9f, 16, 16); glPopMatrix();
    glPushMatrix(); glTranslatef(0.5f, -0.3f, 1.0f); glutSolidSphere(0.7f, 16, 16); glPopMatrix();

    // B. Cyber Ring
    if (currentTimeOfDay == TIME_DAY) {
        static GLfloat dayRingBlue[] = { 0.2f, 0.5f, 1.0f };
        setNeon(dayRingBlue, 1.5f);
    }
    else if (currentTimeOfDay == TIME_SUNSET) {
        setNeon(COLOR_NEON_ORANGE, 1.2f);
    }
    else {
        setNeon(COLOR_CYBER_CYAN, 0.8f);
    }

    glPushMatrix();
    glRotatef(realTime * 100.0f, 0.2f, 1.0f, 0.2f);
    glTranslatef(0.0f, -1.2f, 0.0f);
    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
    glutWireTorus(0.08f, 2.0f, 12, 50);
    glPopMatrix();

    // C. Bottom Jet Effect
    const GLfloat* jetColor;
    if (currentTimeOfDay == TIME_DAY) jetColor = COLOR_CYBER_CYAN;
    else if (currentTimeOfDay == TIME_SUNSET) jetColor = COLOR_NEON_RED;
    else jetColor = COLOR_CYBER_PINK;

    setNeon(jetColor, 2.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glColor4f(jetColor[0], jetColor[1], jetColor[2], 0.35f);

    glPushMatrix();
    glTranslatef(0.0f, -4.3f, 0.0f);

    glScalef(2.5f + breath * 10.0f, 1.8f, 2.f);
    glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
    glutSolidCone(0.6f, 2.5f, 12, 10);
    glPopMatrix();

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPopMatrix();
}

/**
 * @brief Helper function to render a cat's leg with articulated joints.
 * Adapts appearance for day/night (white fur/pink joints vs dark fur/neon joints).
 *
 * @param x Local X position.
 * @param y Local Y position.
 * @param z Local Z position.
 * @param angle Rotation angle for animation.
 * @param isRight True if it's a right leg (affects joint offset).
 */
void drawCatLeg(float x, float y, float z, float angle, bool isRight) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(angle, 1.0f, 0.0f, 0.0f);

    // Thigh
    if (currentTimeOfDay == TIME_DAY) {
        GLfloat whiteCat[] = { 0.95f, 0.95f, 1.0f };
        setSolid(whiteCat);
    }
    else {
        setSolid(COLOR_DARK_GREY);
    }

    glPushMatrix();
    glTranslatef(0.0f, -0.6f, 0.0f);
    glScalef(0.4f, 1.2f, 0.5f);
    glutSolidCube(1.0f);
    glPopMatrix();

    // Knee Joint
    if (currentTimeOfDay == TIME_DAY) {
        GLfloat pinkJoint[] = { 1.0f, 0.4f, 0.7f };
        setSolid(pinkJoint);
    }
    else {
        setNeon(COLOR_CYBER_PURPLE);
    }

    glPushMatrix();
    glTranslatef(isRight ? 0.22f : -0.22f, 0.0f, 0.0f);
    glutSolidSphere(0.15f, 8, 8);
    glPopMatrix();

    // Calf
    setSolid(COLOR_CYBER_PURPLE);

    glPushMatrix();
    glTranslatef(0.0f, -1.2f, 0.2f);
    glScalef(0.3f, 0.8f, 0.3f);
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}

/**
 * @brief Main function to render the Cyber Cat character.
 * Handles locomotion states (walking, cloud riding, hyper warp UFO).
 * Adapts materials for day/night cycles.
 */
void drawCyberCat() {
    glPushMatrix();

    float catBaseY = 2.5f;

    // 1. Locomotion Mode Logic
    if (isHyperWarp) {
        // Mode 1: Hyper Warp (UFO)
        float warpHeight = 15.0f;
        float hover = sinf(realTime * 5.0f) * 0.5f;
        glTranslatef(0.0f, warpHeight + hover, 0.0f);

        // Draw UFO attached to player
        drawUFO(0.0f, 0.0f, worldOffsetZ - 20.0f, 0.0f, 2.3f);

        // Position cat inside cockpit
        glTranslatef(0.0f, 3.5f, -12.5f);
        glScalef(0.40f, 0.40f, 0.40f);
    }
    else if (isCloudMode) {
        // Mode 2: Cloud Ride
        float flyHeight = 20.0f;
        float globalBreath = sinf(realTime * 2.0f) * 0.5f;
        glTranslatef(0.0f, flyHeight + globalBreath, 0.0f);

        drawMechanicalCloud();
        glTranslatef(0.0f, 5.5f, 0.0f);
    }
    else {
        // Mode 3: Walking
        glPushMatrix();
        glTranslatef(0.0f, 0.1f, 0.0f);
        drawShadowBlob(3.8f, 0.6f);
        glPopMatrix();

        glTranslatef(0.0f, catBaseY, 0.0f);
    }

    // 2. Render Cat Body
    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
    glScalef(1.5f, 1.5f, 1.5f);

    GLfloat catWhite[] = { 0.95f, 0.95f, 1.0f };
    GLfloat catOutline[] = { 0.3f, 0.5f, 1.0f };

    // Body
    glPushMatrix();
    if (currentTimeOfDay == TIME_DAY) setSolid(catWhite);
    else setSolid(COLOR_DARK_GREY);

    glScalef(1.2f, 1.0f, 2.2f);
    glutSolidCube(1.0f);

    // Wireframe Outline
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    if (currentTimeOfDay == TIME_DAY) glColor3fv(catOutline);
    else glColor3fv(COLOR_CYBER_CYAN);

    glPushMatrix();
    glScalef(1.02f, 1.02f, 1.02f);
    glutWireCube(1.0f);
    glPopMatrix();
    glEnable(GL_LIGHTING);
    glPopMatrix();

    // Head
    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 1.2f);

    if (currentTimeOfDay == TIME_DAY) setSolid(catWhite);
    else setSolid(COLOR_DARK_GREY);
    glutSolidSphere(0.7f, 16, 16);

    // Ears
    if (currentTimeOfDay == TIME_DAY) setSolid(COLOR_CYBER_PINK);
    else setNeon(COLOR_CYBER_PURPLE);

    glPushMatrix(); glTranslatef(-0.4f, 0.6f, 0.0f); glRotatef(-20, 0, 0, 1); glRotatef(90, 1, 0, 0); glutSolidCone(0.3f, 0.7f, 10, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(0.4f, 0.6f, 0.0f); glRotatef(20, 0, 0, 1); glRotatef(90, 1, 0, 0); glutSolidCone(0.3f, 0.7f, 10, 10); glPopMatrix();

    glPopMatrix();

    // Legs (Animation)
    float angFL, angBR, angFR, angBL;
    if (isCloudMode || isHyperWarp) {
        angFL = -20.0f; angFR = -20.0f;
        angBL = 20.0f;  angBR = 20.0f;
    }
    else {
        float walkSpeed = globalTimeCat * 1.2f;
        float amp = 35.0f;
        angFL = sinf(walkSpeed) * amp;
        angBR = sinf(walkSpeed + 0.2f) * amp;
        angFR = sinf(walkSpeed + M_PI) * amp;
        angBL = sinf(walkSpeed + M_PI + 0.2f) * amp;
    }

    drawCatLeg(-0.4f, -0.4f, 0.9f, angFL, false);
    drawCatLeg(0.4f, -0.4f, 0.9f, angFR, true);
    drawCatLeg(-0.4f, -0.4f, -0.9f, angBL, false);
    drawCatLeg(0.4f, -0.4f, -0.9f, angBR, true);

    // Tail (Animation)
    glPushMatrix();
    glTranslatef(0.0f, 0.3f, -1.1f);

    if (currentTimeOfDay == TIME_DAY) setSolid(catOutline);
    else setNeon(COLOR_CYBER_PURPLE);

    float tailSpeed = isCloudMode ? 1.5f : 1.0f;
    float tailBaseAngle = sinf(globalTimeCat * tailSpeed) * 0.2f;
    glRotatef(tailBaseAngle * 180.0f / M_PI, 0.0f, 1.0f, 0.0f);

    for (int i = 0; i < 10; i++) {
        float sway = sinf(globalTimeCat * tailSpeed - i * 0.5f) * 15.0f;
        glRotatef(sway, 0.0f, 1.0f, 0.0f);
        glRotatef(10.0f, 1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, 0.0f, -0.3f);
        float size = 0.25f - i * 0.02f;

        if (i % 2 == 0) {
            if (currentTimeOfDay == TIME_DAY) setSolid(catWhite);
            else setSolid(COLOR_DARK_GREY);
        }
        else {
            if (currentTimeOfDay == TIME_DAY) setSolid(COLOR_CYBER_PINK);
            else setNeon(COLOR_CYBER_CYAN);
        }

        glutSolidSphere(size, 8, 8);
    }
    glPopMatrix();
    glPopMatrix();
}
/**
 * @brief Renders a 2D vortex transition effect covering the screen.
 * Uses a rotating texture that scales based on the alpha value.
 *
 * @param alpha Transparency level (1.0 = opaque, lower values expand the vortex).
 */
void drawVortexTransition(float alpha) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winWidth, 0, winHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, tex.vortex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    float baseSize = (winWidth > winHeight ? winWidth : winHeight);
    float size = baseSize * (1.0f + (1.0f - alpha) * 2.0f);
    float half = size / 2.0f;

    glColor4f(1.0f, 1.0f, 1.0f, alpha);

    glPushMatrix();
    glTranslatef(winWidth / 2.0f, winHeight / 2.0f, 0.0f);
    glRotatef(realTime * 180.0f, 0.0f, 0.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(-half, -half);
    glTexCoord2f(1, 0); glVertex2f(half, -half);
    glTexCoord2f(1, 1); glVertex2f(half, half);
    glTexCoord2f(0, 1); glVertex2f(-half, half);
    glEnd();
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

//Core Loop & Entry
/**
 * @brief Main display callback function.
 * Manages rendering for different application states: Intro, Room, Transition, and Cyber City.
 */
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // STATE: INTRO
    if (currentState == STATE_INTRO) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); gluOrtho2D(0, winWidth, 0, winHeight);
        glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        float cx = winWidth / 2.0f;
        float cy = winHeight / 2.0f + 50.0f;
        float w = 600.0f;
        float h = 200.0f;

        glColor3f(0.0f, 0.8f, 1.0f); glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP); glVertex2f(cx - w / 2, cy - h / 2); glVertex2f(cx + w / 2, cy - h / 2); glVertex2f(cx + w / 2, cy + h / 2); glVertex2f(cx - w / 2, cy + h / 2); glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawBitmapText("In the future...", cx - 60, cy + 40);
        drawBitmapText("People can travel into videos!", cx - 110, cy);
        drawBitmapText("Let's experience Cyber Travel~!", cx - 120, cy - 40);

        // Start Button
        btnW = 200.0f;
        btnH = 50.0f;
        btnX = cx;
        btnY = cy - h / 2.0f - 60.0f;

        if (btnHover) glColor3f(0.2f, 0.9f, 1.0f);
        else glColor3f(0.0f, 0.4f, 0.8f);

        glBegin(GL_QUADS);
        glVertex2f(btnX - btnW / 2, btnY - btnH / 2);
        glVertex2f(btnX + btnW / 2, btnY - btnH / 2);
        glVertex2f(btnX + btnW / 2, btnY + btnH / 2);
        glVertex2f(btnX - btnW / 2, btnY + btnH / 2);
        glEnd();

        glColor3f(1.0f, 0.9f, 0.1f); glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(btnX - btnW / 2, btnY - btnH / 2);
        glVertex2f(btnX + btnW / 2, btnY - btnH / 2);
        glVertex2f(btnX + btnW / 2, btnY + btnH / 2);
        glVertex2f(btnX - btnW / 2, btnY + btnH / 2);
        glEnd();

        glColor3f(1.0f, 1.0f, 1.0f);
        drawBitmapText("YES! I'M READY", btnX - 65.0f, btnY - 5.0f);

        glDisable(GL_BLEND); glEnable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix(); glEnable(GL_LIGHTING);
    }

    // STATE: ROOM
    if (currentState == STATE_ROOM) {
        glClearColor(0.9f, 0.85f, 0.8f, 1.0f);

        float eyeX = 0.0f, eyeY = 1.7f, eyeZ = 0.6f;
        float yawRad = camYaw * M_PI / 180.0f;
        float pitchRad = camPitch * M_PI / 180.0f;

        float baseDirX = 0.0f; float baseDirY = -0.3f; float baseDirZ = -1.9f;
        float len = sqrtf(baseDirX * baseDirX + baseDirY * baseDirY + baseDirZ * baseDirZ);
        baseDirX /= len; baseDirY /= len; baseDirZ /= len;

        float cosY = cosf(yawRad), sinY = sinf(yawRad);
        float dirX = baseDirX * cosY + baseDirZ * sinY;
        float dirY = baseDirY;
        float dirZ = -baseDirX * sinY + baseDirZ * cosY;

        float cosP = cosf(pitchRad), sinP = sinf(pitchRad);
        float dirY2 = dirY * cosP - dirZ * sinP;
        float dirZ2 = dirY * sinP + dirZ * cosP;

        gluLookAt(eyeX, eyeY, eyeZ, eyeX + dirX, eyeY + dirY2, eyeZ + dirZ2, 0.0f, 1.0f, 0.0f);

        setupRoomLighting();
        drawRoom();
        drawDeskAndPC();
        drawRoomDust();
        const char* msgs[] = {
             "[Mouse]|Look Around",           // 在冒号后加 |
             "[SPACE]|Enter Cyber World"    // 在冒号后加 |
        };
        drawHUD(msgs, 2);
    }

    // STATE: SCREEN FLASH
    else if (currentState == STATE_SCREEN_FLASH) {
        glClearColor(0.9f, 0.85f, 0.8f, 1.0f);

        float eyeX = 0.0f, eyeY = 1.7f, eyeZ = 0.6f;
        float yawRad = camYaw * M_PI / 180.0f;
        float pitchRad = camPitch * M_PI / 180.0f;

        float baseDirX = 0.0f; float baseDirY = -0.3f; float baseDirZ = -1.9f;
        float len = sqrtf(baseDirX * baseDirX + baseDirY * baseDirY + baseDirZ * baseDirZ);
        baseDirX /= len; baseDirY /= len; baseDirZ /= len;

        float cosY = cosf(yawRad), sinY = sinf(yawRad);
        float dirX = baseDirX * cosY + baseDirZ * sinY;
        float dirY = baseDirY;
        float dirZ = -baseDirX * sinY + baseDirZ * cosY;

        float cosP = cosf(pitchRad), sinP = sinf(pitchRad);
        float dirY2 = dirY * cosP - dirZ * sinP;
        float dirZ2 = dirY * sinP + dirZ * cosP;

        gluLookAt(eyeX, eyeY, eyeZ, eyeX + dirX, eyeY + dirY2, eyeZ + dirZ2, 0.0f, 1.0f, 0.0f);

        setupRoomLighting();
        drawRoom();
        drawDeskAndPC();
        drawRoomDust();

        float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        if (now - transitionStartTime > SCREEN_FLASH_DURATION) {
            currentState = STATE_TRANSITION;
            transitionStartTime = now;
        }
    }

    // STATE: TRANSITION
    else if (currentState == STATE_TRANSITION) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float t = (now - transitionStartTime) / TRANSITION_DURATION;

        float alpha = 0.0f;
        if (t < 0.5f) {
            alpha = t / 0.5f;
        }
        else {
            alpha = 1.0f - (t - 0.5f) / 0.5f;
        }

        drawVortexTransition(alpha);

        if (t >= 1.0f) {
            currentState = STATE_CYBER;
            cyberStartTime = now;
        }
    }

    // STATE: CYBER CITY
    else if (currentState == STATE_CYBER) {
        float targetFOV = isHyperWarp ? 75.0f : 60.0f;
        if (warpFOV < targetFOV) warpFOV += 3.0f;
        if (warpFOV > targetFOV) warpFOV -= 3.0f;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(warpFOV, (float)winWidth / winHeight, 1.0, 3000.0);
        glMatrixMode(GL_MODELVIEW);

        float shakeX = 0, shakeY = 0;
        if (isHyperWarp) {
            shakeX = (rand() % 100 / 1000.0f) * 0.5f;
            shakeY = (rand() % 100 / 1000.0f) * 0.5f;
        }

        float radYaw = cyberCamYaw * M_PI / 180.0f;
        float radPitch = cyberCamPitch * M_PI / 180.0f;

        float lookAtY = 34.0f + tanf(radPitch) * 33.0f;
        float lookAtX = sinf(radYaw) * 50.0f;

        float cX = 0.0f;
        float cY = camBaseHeight + (camDist * 0.5f);
        float cZ = camDist;

        gluLookAt(cX, cY, cZ, lookAtX, lookAtY, -50.0f, 0.0f, 1.0f, 0.0f);

        setupCyberLighting();
        drawEnvironmentBox();

        if (isTransparentFloor) {
            drawUndergroundTraffic();
        }

        int startSeg = (int)floor(worldOffsetZ / SEGMENT_SPACING);
        int segmentsToDraw = (int)(abs(CULL_BACK) / SEGMENT_SPACING) + 5;

        for (int i = startSeg - 5; i < startSeg + segmentsToDraw; i++) {
            float zPos = i * SEGMENT_SPACING;
            float drawZ = zPos - worldOffsetZ;

            if (drawZ > CULL_FRONT || drawZ < CULL_BACK - 1500.f) continue;

            glPushMatrix();
            glTranslatef(0.0f, 0.0f, drawZ);
            drawCitySegment(i);
            glPopMatrix();
        }

        drawTrafficSystem();
        drawPaperPlanes();
        drawCyberCat();
        drawRainSystem();
        drawRipples();
        drawMegaPipes();
        drawFloatingParticles();
        drawTrafficSystem();
        drawCyberCat();

        float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

        if (now - cyberStartTime < 2.5f) {
            drawWelcomeBox();
        }
        else {
            if (showCyberHUD) {
                const char* guide[] = {
                    "[C]|Hide Interface",
                    "[R]|Stop Rain",
                    "[Mouse]|Look Around",             
                    "[1/2/3]|Day / Sunset / Night",   
                    "[W / S]|Move Forward / Back",
                    "[Z / X]|Camera Zoom In / Out",
                    "[SPACE]|Ride Cloud (Speed Up)",
                    "[H]|Ride UFO (Speed Up Up)",
                    "[T]|Glass Floor (Subway)",
                    "[ESC]|Exit"
                };
                drawHUD(guide, 10);
            }
        }
    }

    glutSwapBuffers();
}
/**
 * @brief Background idle function for updating simulation state.
 * Handles global time updates and hyper-warp auto-scrolling logic.
 */
void idle() {
    realTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    // Hyper-Warp Auto-Pilot Logic
    if (currentState == STATE_CYBER && isHyperWarp) {
        float warpSpeed = MAX_SPEED * 3.f * 1.2f;
        worldOffsetZ -= warpSpeed;
        globalTimeCat += 0.3f;
        if (worldOffsetZ < -LOOP_LEN) worldOffsetZ += LOOP_LEN;
        if (worldOffsetZ > LOOP_LEN) worldOffsetZ -= LOOP_LEN;
    }
    glutPostRedisplay();
}
/**
 * @brief Handles keyboard input events.
 * Controls application state transitions, character movement, camera zoom,
 * environment settings, and special effects toggles.
 */
void keyboard(unsigned char key, int x, int y) {
    if (key == 27) { // ESC
        exit(0);
    }

    if (currentState == STATE_ROOM) {
        if (key == ' ') {
            currentState = STATE_SCREEN_FLASH;
            transitionStartTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        }
    }
    else if (currentState == STATE_CYBER) {
        float currentMaxSpeed = isCloudMode ? MAX_SPEED * 3.5f : MAX_SPEED;

        switch (key) {
        case 'H': case 'h':
            isHyperWarp = !isHyperWarp;
            break;

        case ' ':
            isCloudMode = !isCloudMode;
            break;

        case 'c': case 'C':
            showCyberHUD = !showCyberHUD;
            break;

        case 'w': case 'W':
            worldOffsetZ -= currentMaxSpeed;
            globalTimeCat += 0.2f;
            break;

        case 's': case 'S':
            worldOffsetZ += currentMaxSpeed;
            globalTimeCat += 0.2f;
            break;

        case 'r': case 'R':
            isRaining = !isRaining;
            break;

        case '1': currentTimeOfDay = TIME_DAY;    break;
        case '2': currentTimeOfDay = TIME_SUNSET; break;
        case '3': currentTimeOfDay = TIME_NIGHT;  break;

        case 'z': case 'Z':
            camDist -= 1.0f;
            if (camDist < 15.0f) camDist = 10.0f;
            break;

        case 'x': case 'X':
            camDist += 1.0f;
            if (camDist > 150.0f) camDist = 150.0f;
            break;

        case 't': case 'T':
            isTransparentFloor = !isTransparentFloor;
            break;
        }

        if (worldOffsetZ < -LOOP_LEN) worldOffsetZ += LOOP_LEN;
        if (worldOffsetZ > LOOP_LEN) worldOffsetZ -= LOOP_LEN;
    }
}

/**
 * @brief Handles mouse click events.
 * Detects interactions with the "Start" button in the Intro state.
 */
void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (currentState == STATE_INTRO) {
            float glY = winHeight - y;

            if (abs(x - btnX) < btnW / 2.0f && abs(glY - btnY) < btnH / 2.0f) {
                currentState = STATE_ROOM;
                printf("Button Clicked! Entering Room...\n");
            }
        }
    }
}

/**
 * @brief Handles passive mouse motion (movement without clicking).
 * Updates UI hover states in the Intro and camera orientation in Room/Cyber states.
 */
void passiveMotion(int x, int y) {
    if (winWidth <= 0 || winHeight <= 0) return;

    float tx = 2.0f * x / (float)winWidth - 1.0f;
    float ty = 1.0f - 2.0f * y / (float)winHeight;
    float glY = winHeight - y;

    if (currentState == STATE_INTRO) {
        if (abs(x - btnX) < btnW / 2.0f && abs(glY - btnY) < btnH / 2.0f) {
            btnHover = true;
        }
        else {
            btnHover = false;
        }
    }
    else if (currentState == STATE_ROOM) {
        float maxAngle = 22.0f;
        float sensitivity = 1.1f;
        camYaw = tx * maxAngle * sensitivity;
        camPitch = -ty * maxAngle * sensitivity;
    }
    else if (currentState == STATE_CYBER) {
        float maxCyberYaw = 60.0f;
        float maxCyberPitch = 60.0f;
        float sensitivity = 1.2f;

        cyberCamYaw = tx * maxCyberYaw * sensitivity;
        cyberCamPitch = ty * maxCyberPitch * sensitivity;
    }

    glutPostRedisplay();
}
/**
 * @brief Handles window resize events.
 * Adjusts the viewport and perspective projection matrix to match the new window dimensions.
 *
 * @param w New window width.
 * @param h New window height.
 */
void reshape(int w, int h) {
    if (h == 0) h = 1;
    winWidth = w;
    winHeight = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / h, 1.0, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

/**
 * @brief Main entry point of the application.
 * Initializes GLUT, OpenGL settings, textures, and registers input/rendering callbacks.
 * @return Exit status.
 */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("Reality Link: Cyber Travel");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initAllTextures();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(passiveMotion);
    glutMouseFunc(mouseClick);
    glutIdleFunc(idle);

    introStartTime = 0.0f;

    glutMainLoop();
    return 0;
}