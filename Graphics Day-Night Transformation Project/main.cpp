#include <windows.h>
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const float PI = 3.142f;
float balloonScale = 1.0f;
float balloonOffset = 0.0f;
float balloonSpeed = 0.05f;
float balloonAngle = 0.0f;
float balloonX = -30.0f;        // <- ADD: horizontal position
float balloonXSpeed = 0.05f;    // <- ADD: left to right speed
float balloonScaleDir = 1.0f;   // <- ADD: 1=growing, -1=shrinking
float car1X = 15.0f;
float car1Speed = -0.5f;
float treeSway = 0.0f;
float treeSwayDir = 1.0f;
//float treeSway     = 0.0f;
//float treeSwayDir  = 1.0f;
float rainWindDir  = 1.0f;
float rainWindTimer = 0.0f;
bool lightningOn = false;
int lightningFrames = 0;
int lightningCooldown = 45;
//float balloonScaleDir = 1.0f;
float snowSize[150];  // each flake has its own size
bool vehiclePaused = false;  // pause all vehicles

// Car 2 (small yellow car, moves right)
float car2X = -15.0f;
float car2Speed = 0.5f;

// Car 3 (small blue car, moves left)
float car3X = 5.0f;
float car3Speed = -1.2f;

// Scenario 1 variables
int currentScenario = 1;
bool isDay = true;
bool winterMode = false;
bool rainOn = false;
float rainX[200], rainY[200];
bool snowOn = false;
float snowX[150], snowY[150];
float X = 35.0f;
float Y = -35.0f;
float truckSpeed = -1.0f;
float BusSpeed = 1.0f;
float starX[50], starY[50];
int starsInitialized = 0;
float cloudX = -20.0f;
float cloudSpeed = 0.10f;

void drawCircle(float cx, float cy, float r) {
    int num_segments = 200;
    glBegin(GL_POLYGON);
    for (int i = 0; i < num_segments; i++) {
        float theta = 2.0f * PI * i / num_segments;
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(x + cx, y + cy);
    }
    glEnd();
}

void drawSemiCircleFromPoints(float x1, float y1, float x2, float y2, int up) {
    float cx = (x1 + x2) / 2.0f;
    float cy = (y1 + y2) / 2.0f;
    float dx = (x2 - x1);
    float dy = (y2 - y1);
    float r = sqrt(dx * dx + dy * dy) / 2.0f;

    int num_segments = 200;
    glBegin(GL_POLYGON);
    glVertex2f(x1, y1);
    for (int i = 0; i <= num_segments; i++) {
        float theta = up ? PI * i / num_segments : -PI * i / num_segments;
        float x = r * cosf(theta);
        float y = r * sinf(theta);
        glVertex2f(cx + x, cy + y);
    }
    glVertex2f(x2, y2);
    glEnd();
}


void drawRain() {     // Rain
    if (!rainOn) return;

    glColor3ub(173, 216, 230);
    glBegin(GL_LINES);
    for (int i = 0; i < 200; i++) {
        glVertex2f(rainX[i], rainY[i]);
        glVertex2f(rainX[i], rainY[i] - 0.5f);
    }
    glEnd();
}

void drawLightning() {
    if (!rainOn || !lightningOn) return;

    // Brief sky flash
    glColor4f(0.95f, 0.97f, 1.0f, 0.35f);
    glBegin(GL_QUADS);
    glVertex2f(-30.0f, -20.0f);
    glVertex2f(30.0f, -20.0f);
    glVertex2f(30.0f, 26.774f);
    glVertex2f(-30.0f, 26.774f);
    glEnd();

    // Small lightning bolt in the sky
    glColor3ub(255, 255, 210);
    glLineWidth(3.0f);
    glBegin(GL_LINE_STRIP);
    glVertex2f(-4.0f, 25.0f);
    glVertex2f(-1.8f, 20.5f);
    glVertex2f(-3.0f, 20.5f);
    glVertex2f(0.2f, 15.5f);
    glEnd();
    glLineWidth(1.0f);
}

void updateRain(int value) {   //rainupdate
    if (rainOn) {
        // Slow, smooth tree sway while raining
        treeSway += 0.05f * treeSwayDir;
        if (treeSway >  3.0f) treeSwayDir = -1.0f;
        if (treeSway < -3.0f) treeSwayDir =  1.0f;

        // Random lightning pulse every few moments
        if (lightningFrames > 0) {
            lightningFrames--;
            if (lightningFrames == 0) lightningOn = false;
        } else if (lightningCooldown > 0) {
            lightningCooldown--;
        } else if (rand() % 100 < 4) {
            lightningOn = true;
            lightningFrames = 3 + (rand() % 3);
            lightningCooldown = 40 + (rand() % 90);
        }

        // wind direction switch every 3 seconds


        for (int i = 0; i < 200; i++) {
            rainY[i] -= 0.5f;                    // fall down


            if (rainY[i] < -20) {
                rainY[i] = 25 + (rand() % 10);
                rainX[i] = (rand() % 60) - 30;
            }
        }
    } else {
        lightningOn = false;
        lightningFrames = 0;
        lightningCooldown = 45;

        // Let trees settle back gently when rain stops
        if (treeSway > 0.03f) treeSway -= 0.03f;
        else if (treeSway < -0.03f) treeSway += 0.03f;
        else treeSway = 0.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(30, updateRain, 0);
}

// Snow-------------------------------------->

void drawSnow() {
    if (!snowOn) return;

    glColor3ub(255, 255, 255);  //White snowflakes
    for (int i = 0; i < 150; i++) {
        // Draw snowflake as small circle
        drawCircle(snowX[i], snowY[i], 0.15f);
    }
}

void updateSnow(int value) {   //snowupdate
    if (snowOn) {\
        for (int i = 0; i < 150; i++) {
            snowY[i] -= 0.15f;                          // fall down slowly
            snowX[i] += sinf(snowY[i] * 0.5f) * 0.03f; // gentle side drift

            if (snowY[i] < -20) {
                snowY[i]    = 27 + (rand() % 10);
                snowX[i]    = (rand() % 60) - 30;
                snowSize[i] = 0.1f + (rand() % 10) * 0.05f;
            }
        }
    }

    glutPostRedisplay();
    glutTimerFunc(40, updateSnow, 0);
}

void drawSnowOnRoofs() {

    (void)winterMode;
}

void drawSnowOnTrees() {
    if (!winterMode) return;

    glColor3ub(255, 255, 255);//white

    //tree 1 left

    drawCircle(-19.0f, -1.5f, 1.2f);
    drawCircle(-21.3f, -1.2f, 1.0f);
    drawCircle(-22.5f, -3.5f, 0.9f);
    drawCircle(-19.5f, -2.0f, 0.8f);
    drawCircle(-20.5f, -0.8f, 0.7f);

    //tree 2 right tree near palace

    drawCircle(20.2f, -1.0f, 1.0f);
    drawCircle(19.5f, 0.5f, 0.8f);
    drawCircle(17.0f, 0.3f, 0.7f);
    drawCircle(19.4f, 1.0f, 0.6f);
    drawCircle(15.2f, -2.8f, 0.5f);

    // tree 3
    drawCircle(14.7f, -8.0f, 0.5f);
    drawCircle(14.3f, -7.2f, 0.4f);
    drawCircle(13.0f, -7.3f, 0.35f);

}

void drawSnowOnGround() {
    if (!winterMode) return;

    // ground-----
    glColor3ub(255, 255, 255);  // White snow layer
    glBegin(GL_QUADS);
    glVertex2f(-30.0f, -18.1f);
    glVertex2f(30.0f, -18.1f);
    glVertex2f(30.0f, -18.8f);
    glVertex2f(-30.0f, -18.8f);
    glEnd();

    //snow bumps on ground
    glColor3ub(250, 250, 250);
    for (float x = -30.0f; x < 30.0f; x += 3.0f) {
        drawCircle(x, -18.1f, 0.4f);
        drawCircle(x + 1.5f, -18.2f, 0.3f);
    }
}

void drawSnowOnBenchesAndLampsScenario1() {
    if (!winterMode) return;
    glColor3ub(255, 255, 255);

    // Snow on bench 1 (left)
    float scale1 = 3.0f;
    float dx1 = -15.0f;
    float dy1 = -14.20f;
    glBegin(GL_QUADS);
    glVertex2f((-1.1f * scale1) + dx1, (0.55f * scale1) + dy1);
    glVertex2f((1.1f * scale1) + dx1, (0.55f * scale1) + dy1);
    glVertex2f((1.1f * scale1) + dx1, (0.70f * scale1) + dy1);
    glVertex2f((-1.1f * scale1) + dx1, (0.70f * scale1) + dy1);
    glEnd();

    // Snow on bench 2 (right)
    float dx2 = 23.0f;
    glBegin(GL_QUADS);
    glVertex2f((-1.1f * scale1) + dx2, (0.55f * scale1) + dy1);
    glVertex2f((1.1f * scale1) + dx2, (0.55f * scale1) + dy1);
    glVertex2f((1.1f * scale1) + dx2, (0.70f * scale1) + dy1);
    glVertex2f((-1.1f * scale1) + dx2, (0.70f * scale1) + dy1);
    glEnd();

    // === snow on lamp post===
    float lampScale = 6.7f;

    // Snow caps on each lamp post
    float lampPositions[] = { -28.0f, -18.0f, -8.0f, 8.0f, 18.0f, 28.0f };
    for (int i = 0; i < 6; i++) {
        float x = lampPositions[i];
        float y = -11.90f;

        // Snow on lamp post top
        drawCircle(x, y + (0.43f * lampScale), 0.25f * lampScale);

        // Snow on lamp post pole
        glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2f(x - 0.02f * lampScale, y + 0.4f * lampScale);
        glVertex2f(x + 0.02f * lampScale, y + 0.4f * lampScale);
        glVertex2f(x + 0.02f * lampScale, y - 0.3f * lampScale);
        glVertex2f(x - 0.02f * lampScale, y - 0.3f * lampScale);
        glEnd();

        glColor3ub(255, 255, 255);
    }
}

//=========================snow er kaz sesh=========================================


// ==================================================================
void updateTruck(int value) {
    if (currentScenario == 1 && !vehiclePaused) {
        X += truckSpeed;
        if (X < -60.0f) X = 35.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(40, updateTruck, 0);
}
void updateBus(int value) {
    if (currentScenario == 1 && !vehiclePaused) {
        Y += BusSpeed;
        if (Y > 60.0f) Y = -35.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(40, updateBus, 0);
}
void updateBalloon(int value) {
       if (currentScenario == 1) {
        balloonX += balloonXSpeed;
        if (balloonX > 35.0f) balloonX = -35.0f;

        // grow and shrink
        balloonScale += 0.005f * balloonScaleDir;
        if (balloonScale >= 1.8f) balloonScaleDir = -1.0f;
        if (balloonScale <= 0.5f) balloonScaleDir =  1.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(30, updateBalloon, 0);
}

void updateCloud(int value) {   //cloudypdate
    if (currentScenario == 1) {
        cloudX += cloudSpeed;
        if (cloudX > 55.0f) {
            cloudX = -55.0f;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(2, updateCloud, 0);
}

void drawMoon(float cx, float cy, float r) {    //Moon1
    glColor3ub(245, 245, 245);
    drawCircle(cx, cy, r);
    glColor3ub(0, 0, 31);
    drawCircle(cx + r * 0.3, cy + r * 0.1, r);
}
void drawStars() {
    glColor3ub(255, 255, 255);

    // First time e random position generate hobe
    if (!starsInitialized) {
        for (int i = 0; i < 50; i++) {
            starX[i] = (rand() % 60) - 30;  // X position
            starY[i] = (rand() % 30);       // Y position
        }
        starsInitialized = 1;
    }

    // Protibar draw er age position update
    for (int i = 0; i < 50; i++) {     //starsupdate
        starX[i] += 0.02f;

        if (starX[i] > 30) {
            starX[i] = -30;
        }

        drawCircle(starX[i], starY[i], 0.1f);
    }
}

void drawSun(float cx, float cy, float r) {
    glColor3ub(255, 223, 0);
    drawCircle(6.287f, 20.220f, 3.018f);  //Sun1

}
void drawCloud() {
    glPushMatrix();
    glTranslatef(cloudX, 0.0f, 0.0f);
    glColor3f(1.0f, 1.0f, 0.9f);


    drawSemiCircleFromPoints(-24.388f, 12.058f, -20.138f, 12.058f, 1);

    drawSemiCircleFromPoints(-21.138f, 12.058f, -15.0f, 12.058f, 1);   //Cloud1


    drawSemiCircleFromPoints(-15.584, 12.058f, -12.280f, 12.058f, 1);

    drawSemiCircleFromPoints(-2.343, 17.081f, 1.028f, 17.081f, 1);
    drawSemiCircleFromPoints(0.294f, 17.081f, 5.265f, 17.081f, 1);   //Cloud2
    drawSemiCircleFromPoints(4.832, 17.081f, 7.454f, 17.081f, 1);

    drawSemiCircleFromPoints(-5.723, 6.966f, -1.302, 6.966f, 1);
    drawSemiCircleFromPoints(-2.314f, 6.966f, 4.373f, 6.966f, 1);   //Cloud3
    drawSemiCircleFromPoints(3.831, 6.966f, 7.245f, 6.966f, 1);

    drawSemiCircleFromPoints(13.665f, 9.0f, 20.0f, 9.0f, 1);   //Cloud4
    drawSemiCircleFromPoints(19.394f, 9.0f, 23.0f, 9.0f, 1);

    glPopMatrix();
}

//left House1------------>

void house1() {
    glColor3ub(188, 166, 97);
    glBegin(GL_POLYGON);
    glVertex2f(20.267f, 8.220f); //
    glVertex2f(25.795f, 8.220f); //
    glVertex2f(28.613f, 8.220f); //
    glVertex2f(30.0f, 8.220f); //
    glVertex2f(30.0f, -15.7f); //
    glVertex2f(20.267f, -15.7f); //

    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(25.795f, 8.220f);
    glVertex2f(25.795f, 10.316f);
    glVertex2f(28.613f, 10.316f);
    glVertex2f(28.613f, 8.220f);

    glEnd();

    // Roofline detail on tower
    glColor3ub(160, 140, 80);
    glBegin(GL_POLYGON);
    glVertex2f(25.4f, 10.316f);
    glVertex2f(28.95f, 10.316f);
    glVertex2f(28.95f, 10.7f);
    glVertex2f(25.4f, 10.7f);
    glEnd();

    // Horizontal facade bands
    glColor3ub(170, 150, 85);
    glBegin(GL_POLYGON);
    glVertex2f(20.267f, 4.0f);
    glVertex2f(25.795f, 4.0f);
    glVertex2f(25.795f, 3.8f);
    glVertex2f(20.267f, 3.8f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(20.267f, -2.0f);
    glVertex2f(30.0f, -2.0f);
    glVertex2f(30.0f, -2.2f);
    glVertex2f(20.267f, -2.2f);
    glEnd();

    // Small balcony on tower
    glColor3ub(180, 160, 90);
    glBegin(GL_POLYGON);
    glVertex2f(25.795f, 6.5f);
    glVertex2f(28.613f, 6.5f);
    glVertex2f(28.613f, 6.3f);
    glVertex2f(25.795f, 6.3f);
    glEnd();

    // Balcony railings
    glColor3ub(140, 120, 70);
    glLineWidth(1.5f);
    glBegin(GL_LINES);
    glVertex2f(26.0f, 6.5f);
    glVertex2f(26.0f, 7.2f);
    glVertex2f(26.7f, 6.5f);
    glVertex2f(26.7f, 7.2f);
    glVertex2f(27.4f, 6.5f);
    glVertex2f(27.4f, 7.2f);
    glVertex2f(28.1f, 6.5f);
    glVertex2f(28.1f, 7.2f);
    glEnd();
    glLineWidth(1.0f);

    // Windows - Left column, Window 1
    glColor3ub(255, 255, 255);
    glBegin(GL_POLYGON);
    glVertex2f(22.778f, 5.082f);
    glVertex2f(24.227f, 5.092f);
    glVertex2f(24.227f, 3.013f);
    glVertex2f(22.778f, 3.013f);
    glEnd();

    // Windows - Left column, Window 2
    glBegin(GL_POLYGON);
    glVertex2f(22.778f, 1.082f);
    glVertex2f(24.227f, 1.092f);
    glVertex2f(24.227f, -0.987f);
    glVertex2f(22.778f, -0.987f);
    glEnd();

    // Windows - Left column, Window 3
    glBegin(GL_POLYGON);
    glVertex2f(22.778f, -2.918f);
    glVertex2f(24.227f, -2.908f);
    glVertex2f(24.227f, -4.987f);
    glVertex2f(22.778f, -4.987f);
    glEnd();

    // Windows - Left column, Window 4
    glBegin(GL_POLYGON);
    glVertex2f(22.778f, -6.918f);
    glVertex2f(24.227f, -6.908f);
    glVertex2f(24.227f, -8.987f);
    glVertex2f(22.778f, -8.987f);
    glEnd();

    // Windows - Left column, Window 5
    glBegin(GL_POLYGON);
    glVertex2f(22.778f, -10.918f);
    glVertex2f(24.227f, -10.908f);
    glVertex2f(24.227f, -12.987f);
    glVertex2f(22.778f, -12.987f);
    glEnd();

    // Windows - Right column, Window 1
    glBegin(GL_POLYGON);
    glVertex2f(26.099f, 5.092f);
    glVertex2f(27.686f, 5.092f);
    glVertex2f(27.686f, 3.013f);
    glVertex2f(26.099f, 3.013f);
    glEnd();

    // Windows - Right column, Window 2
    glBegin(GL_POLYGON);
    glVertex2f(26.099f, 1.092f);
    glVertex2f(27.686f, 1.092f);
    glVertex2f(27.686f, -0.987f);
    glVertex2f(26.099f, -0.987f);
    glEnd();

    // Windows - Right column, Window 3
    glBegin(GL_POLYGON);
    glVertex2f(26.099f, -2.908f);
    glVertex2f(27.686f, -2.908f);
    glVertex2f(27.686f, -4.987f);
    glVertex2f(26.099f, -4.987f);
    glEnd();

    // Windows - Right column, Window 4
    glBegin(GL_POLYGON);
    glVertex2f(26.099f, -6.908f);
    glVertex2f(27.686f, -6.908f);
    glVertex2f(27.686f, -8.987f);
    glVertex2f(26.099f, -8.987f);
    glEnd();

    // Windows - Right column, Window 5
    glBegin(GL_POLYGON);
    glVertex2f(26.099f, -10.908f);
    glVertex2f(27.686f, -10.908f);
    glVertex2f(27.686f, -12.987f);
    glVertex2f(26.099f, -12.987f);
    glEnd();

    // Window frames - Left column, Window 1
    glColor3ub(140, 120, 70);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(22.778f, 5.082f);
    glVertex2f(24.227f, 5.092f);
    glVertex2f(24.227f, 3.013f);
    glVertex2f(22.778f, 3.013f);
    glEnd();

    // Window frames - Right column, Window 1
    glBegin(GL_LINE_LOOP);
    glVertex2f(26.099f, 5.092f);
    glVertex2f(27.686f, 5.092f);
    glVertex2f(27.686f, 3.013f);
    glVertex2f(26.099f, 3.013f);
    glEnd();

    // Window frames - Left column, Window 2
    glBegin(GL_LINE_LOOP);
    glVertex2f(22.778f, 1.082f);
    glVertex2f(24.227f, 1.092f);
    glVertex2f(24.227f, -0.987f);
    glVertex2f(22.778f, -0.987f);
    glEnd();

    // Window frames - Right column, Window 2
    glBegin(GL_LINE_LOOP);
    glVertex2f(26.099f, 1.092f);
    glVertex2f(27.686f, 1.092f);
    glVertex2f(27.686f, -0.987f);
    glVertex2f(26.099f, -0.987f);
    glEnd();

    // Window frames - Left column, Window 3
    glBegin(GL_LINE_LOOP);
    glVertex2f(22.778f, -2.918f);
    glVertex2f(24.227f, -2.908f);
    glVertex2f(24.227f, -4.987f);
    glVertex2f(22.778f, -4.987f);
    glEnd();

    // Window frames - Right column, Window 3
    glBegin(GL_LINE_LOOP);
    glVertex2f(26.099f, -2.908f);
    glVertex2f(27.686f, -2.908f);
    glVertex2f(27.686f, -4.987f);
    glVertex2f(26.099f, -4.987f);
    glEnd();

    // Window frames - Left column, Window 4
    glBegin(GL_LINE_LOOP);
    glVertex2f(22.778f, -6.918f);
    glVertex2f(24.227f, -6.908f);
    glVertex2f(24.227f, -8.987f);
    glVertex2f(22.778f, -8.987f);
    glEnd();

    // Window frames - Right column, Window 4
    glBegin(GL_LINE_LOOP);
    glVertex2f(26.099f, -6.908f);
    glVertex2f(27.686f, -6.908f);
    glVertex2f(27.686f, -8.987f);
    glVertex2f(26.099f, -8.987f);
    glEnd();

    // Window frames - Left column, Window 5
    glBegin(GL_LINE_LOOP);
    glVertex2f(22.778f, -10.918f);
    glVertex2f(24.227f, -10.908f);
    glVertex2f(24.227f, -12.987f);
    glVertex2f(22.778f, -12.987f);
    glEnd();

    // Window frames - Right column, Window 5
    glBegin(GL_LINE_LOOP);
    glVertex2f(26.099f, -10.908f);
    glVertex2f(27.686f, -10.908f);
    glVertex2f(27.686f, -12.987f);
    glVertex2f(26.099f, -12.987f);
    glEnd();

}

//right House2------------>

void house2() {
    glColor3ub(122, 110, 59);
    glBegin(GL_POLYGON);
    glVertex2f(-29.262f, 8.595f); //
    glVertex2f(-19.914f, 8.600f); //
    glVertex2f(-19.914f, -15.7f);  //
    glVertex2f(-29.262f, -15.7f); //

    glEnd();

    // Roofline detail
    glColor3ub(100, 90, 50);
    glBegin(GL_POLYGON);
    glVertex2f(-29.5f, 8.595f);
    glVertex2f(-19.6f, 8.600f);
    glVertex2f(-19.6f, 9.0f);
    glVertex2f(-29.5f, 9.0f);
    glEnd();

    // Horizontal facade bands
    glColor3ub(110, 100, 55);
    glBegin(GL_POLYGON);
    glVertex2f(-29.262f, 2.0f);
    glVertex2f(-19.914f, 2.0f);
    glVertex2f(-19.914f, 1.8f);
    glVertex2f(-29.262f, 1.8f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-29.262f, -4.0f);
    glVertex2f(-19.914f, -4.0f);
    glVertex2f(-19.914f, -4.2f);
    glVertex2f(-29.262f, -4.2f);
    glEnd();

    // Ground level door
    glColor3ub(80, 60, 40);
    glBegin(GL_POLYGON);
    glVertex2f(-25.5f, -9.0f);
    glVertex2f(-23.7f, -9.0f);
    glVertex2f(-23.7f, -15.7f);
    glVertex2f(-25.5f, -15.7f);
    glEnd();

    // Door frame
    glColor3ub(60, 50, 30);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-25.5f, -9.0f);
    glVertex2f(-23.7f, -9.0f);
    glVertex2f(-23.7f, -15.7f);
    glVertex2f(-25.5f, -15.7f);
    glEnd();
    glLineWidth(1.0f);

    // Door handle
    glColor3ub(180, 160, 100);
    glBegin(GL_POLYGON);
    glVertex2f(-24.0f, -11.5f);
    glVertex2f(-23.8f, -11.5f);
    glVertex2f(-23.8f, -12.0f);
    glVertex2f(-24.0f, -12.0f);
    glEnd();

    // Windows - left column
    glColor3ub(255, 255, 255);
    glBegin(GL_POLYGON);
    glVertex2f(-27.376f, 6.824f);
    glVertex2f(-25.537f, 6.824f);
    glVertex2f(-25.537f, 3.835f);
    glVertex2f(-27.376f, 3.835f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-27.376f, 2.624f);
    glVertex2f(-25.537f, 2.624f);
    glVertex2f(-25.537f, -0.365f);
    glVertex2f(-27.376f, -0.365f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-27.376f, -1.576f);
    glVertex2f(-25.537f, -1.576f);
    glVertex2f(-25.537f, -4.565f);
    glVertex2f(-27.376f, -4.565f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-27.376f, -5.776f);
    glVertex2f(-25.537f, -5.776f);
    glVertex2f(-25.537f, -8.765f);
    glVertex2f(-27.376f, -8.765f);
    glEnd();

    // Windows - right column
    glBegin(GL_POLYGON);
    glVertex2f(-23.870f, 6.824f);
    glVertex2f(-21.594f, 6.824f);
    glVertex2f(-21.594f, 3.835f);
    glVertex2f(-23.870f, 3.835f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-23.870f, 2.624f);
    glVertex2f(-21.594f, 2.624f);
    glVertex2f(-21.594f, -0.365f);
    glVertex2f(-23.870f, -0.365f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-23.870f, -1.576f);
    glVertex2f(-21.594f, -1.576f);
    glVertex2f(-21.594f, -4.565f);
    glVertex2f(-23.870f, -4.565f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-23.870f, -5.776f);
    glVertex2f(-21.594f, -5.776f);
    glVertex2f(-21.594f, -8.765f);
    glVertex2f(-23.870f, -8.765f);
    glEnd();

    // Window frames
    glColor3ub(100, 90, 50);
    glLineWidth(1.0f);

    // Left column frames
    glBegin(GL_LINE_LOOP);
    glVertex2f(-27.376f, 6.824f);
    glVertex2f(-25.537f, 6.824f);
    glVertex2f(-25.537f, 3.835f);
    glVertex2f(-27.376f, 3.835f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-27.376f, 2.624f);
    glVertex2f(-25.537f, 2.624f);
    glVertex2f(-25.537f, -0.365f);
    glVertex2f(-27.376f, -0.365f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-27.376f, -1.576f);
    glVertex2f(-25.537f, -1.576f);
    glVertex2f(-25.537f, -4.565f);
    glVertex2f(-27.376f, -4.565f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-27.376f, -5.776f);
    glVertex2f(-25.537f, -5.776f);
    glVertex2f(-25.537f, -8.765f);
    glVertex2f(-27.376f, -8.765f);
    glEnd();

    // Right column frames
    glBegin(GL_LINE_LOOP);
    glVertex2f(-23.870f, 6.824f);
    glVertex2f(-21.594f, 6.824f);
    glVertex2f(-21.594f, 3.835f);
    glVertex2f(-23.870f, 3.835f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-23.870f, 2.624f);
    glVertex2f(-21.594f, 2.624f);
    glVertex2f(-21.594f, -0.365f);
    glVertex2f(-23.870f, -0.365f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-23.870f, -1.576f);
    glVertex2f(-21.594f, -1.576f);
    glVertex2f(-21.594f, -4.565f);
    glVertex2f(-23.870f, -4.565f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-23.870f, -5.776f);
    glVertex2f(-21.594f, -5.776f);
    glVertex2f(-21.594f, -8.765f);
    glVertex2f(-23.870f, -8.765f);
    glEnd();
}

//Center house Building--------->
void palace() {

    //Main building body
    glColor3ub(175, 155, 135);  // Original tan color
    glBegin(GL_POLYGON);
    glVertex2f(-12.0f, 11.0f);
    glVertex2f(12.0f, 11.0f);
    glVertex2f(12.0f, -15.7f);   //ground level
    glVertex2f(-12.0f, -15.7f);  //ground level
    glEnd();

    //Rooftop
    glColor3ub(140, 120, 100);  //brown roof color
    glBegin(GL_POLYGON);
    glVertex2f(-12.3f, 11.0f);
    glVertex2f(12.3f, 11.0f);
    glVertex2f(12.3f, 11.5f);
    glVertex2f(-12.3f, 11.5f);
    glEnd();

    //Rooftop edge darker accent
    glColor3ub(120, 100, 80);  // Darker brown
    glBegin(GL_POLYGON);
    glVertex2f(-12.3f, 11.5f);
    glVertex2f(12.3f, 11.5f);
    glVertex2f(12.0f, 11.8f);
    glVertex2f(-12.0f, 11.8f);
    glEnd();

    //Vertical accent bands left side
    glColor3ub(160, 140, 120);  // Lighter tan
    glBegin(GL_POLYGON);
    glVertex2f(-12.0f, 11.0f);
    glVertex2f(-11.2f, 11.0f);
    glVertex2f(-11.2f, -15.7f);   //ground level
    glVertex2f(-12.0f, -15.7f);   //ground level
    glEnd();

    //Vertical accent bands right side
    glBegin(GL_POLYGON);
    glVertex2f(11.2f, 11.0f);
    glVertex2f(12.0f, 11.0f);
    glVertex2f(12.0f, -15.7f);    //ground level
    glVertex2f(11.2f, -15.7f);    //ground level
    glEnd();

    //Horizontal bands for modern segmentation
    glColor3ub(165, 145, 125);  // Medium tan
    glBegin(GL_POLYGON);
    glVertex2f(-12.0f, 6.8f);
    glVertex2f(12.0f, 6.8f);
    glVertex2f(12.0f, 7.0f);
    glVertex2f(-12.0f, 7.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-12.0f, 2.5f);
    glVertex2f(12.0f, 2.5f);
    glVertex2f(12.0f, 2.7f);
    glVertex2f(-12.0f, 2.7f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-12.0f, -1.8f);
    glVertex2f(12.0f, -1.8f);
    glVertex2f(12.0f, -1.6f);
    glVertex2f(-12.0f, -1.6f);
    glEnd();

    // light blue color
    glColor3ub(135, 200, 235);

    // Top floor windows
    glBegin(GL_POLYGON);
    glVertex2f(-9.5f, 10.0f);
    glVertex2f(-7.3f, 10.0f);
    glVertex2f(-7.3f, 7.3f);
    glVertex2f(-9.5f, 7.3f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-3.9f, 10.0f);
    glVertex2f(-1.7f, 10.0f);
    glVertex2f(-1.7f, 7.3f);
    glVertex2f(-3.9f, 7.3f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(1.7f, 10.0f);
    glVertex2f(3.9f, 10.0f);
    glVertex2f(3.9f, 7.3f);
    glVertex2f(1.7f, 7.3f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(7.3f, 10.0f);
    glVertex2f(9.5f, 10.0f);
    glVertex2f(9.5f, 7.3f);
    glVertex2f(7.3f, 7.3f);
    glEnd();

    // Second floor windows
    glBegin(GL_POLYGON);
    glVertex2f(-9.5f, 6.5f);
    glVertex2f(-7.3f, 6.5f);
    glVertex2f(-7.3f, 3.0f);
    glVertex2f(-9.5f, 3.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-3.9f, 6.5f);
    glVertex2f(-1.7f, 6.5f);
    glVertex2f(-1.7f, 3.0f);
    glVertex2f(-3.9f, 3.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(1.7f, 6.5f);
    glVertex2f(3.9f, 6.5f);
    glVertex2f(3.9f, 3.0f);
    glVertex2f(1.7f, 3.0f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(7.3f, 6.5f);
    glVertex2f(9.5f, 6.5f);
    glVertex2f(9.5f, 3.0f);
    glVertex2f(7.3f, 3.0f);
    glEnd();

    // Third floor windows
    glBegin(GL_POLYGON);
    glVertex2f(-9.5f, 2.2f);
    glVertex2f(-7.3f, 2.2f);
    glVertex2f(-7.3f, -1.3f);
    glVertex2f(-9.5f, -1.3f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-3.9f, 2.2f);
    glVertex2f(-1.7f, 2.2f);
    glVertex2f(-1.7f, -1.3f);
    glVertex2f(-3.9f, -1.3f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(1.7f, 2.2f);
    glVertex2f(3.9f, 2.2f);
    glVertex2f(3.9f, -1.3f);
    glVertex2f(1.7f, -1.3f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(7.3f, 2.2f);
    glVertex2f(9.5f, 2.2f);
    glVertex2f(9.5f, -1.3f);
    glVertex2f(7.3f, -1.3f);
    glEnd();

    // Fourth floor windows
    glBegin(GL_POLYGON);
    glVertex2f(-9.5f, -2.0f);
    glVertex2f(-7.3f, -2.0f);
    glVertex2f(-7.3f, -5.5f);
    glVertex2f(-9.5f, -5.5f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-3.9f, -2.0f);
    glVertex2f(-1.7f, -2.0f);
    glVertex2f(-1.7f, -5.5f);
    glVertex2f(-3.9f, -5.5f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(1.7f, -2.0f);
    glVertex2f(3.9f, -2.0f);
    glVertex2f(3.9f, -5.5f);
    glVertex2f(1.7f, -5.5f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(7.3f, -2.0f);
    glVertex2f(9.5f, -2.0f);
    glVertex2f(9.5f, -5.5f);
    glVertex2f(7.3f, -5.5f);
    glEnd();



    //Ground level entrance door
    glColor3ub(101, 67, 33);  //brown door color
    glBegin(GL_POLYGON);
    glVertex2f(-2.2f, -7.5f);   //Top of door
    glVertex2f(2.2f, -7.5f);
    glVertex2f(2.2f, -15.7f);   //Bottom at ground level
    glVertex2f(-2.2f, -15.7f);  //ground level
    glEnd();

    //Door frame outline
    glColor3ub(80, 60, 40);  //Dark brown frame
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-2.2f, -7.5f);
    glVertex2f(2.2f, -7.5f);
    glVertex2f(2.2f, -15.7f);  //ground level
    glVertex2f(-2.2f, -15.7f);  //ground level
    glEnd();
    glLineWidth(1.0f);

    //door panels two vertical sections
    glColor3ub(90, 70, 45);  // Lighter brown for panels
    glBegin(GL_POLYGON);
    glVertex2f(-1.9f, -8.0f);
    glVertex2f(-0.3f, -8.0f);
    glVertex2f(-0.3f, -15.2f);  //ground level
    glVertex2f(-1.9f, -15.2f);  //ground level
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(0.3f, -8.0f);
    glVertex2f(1.9f, -8.0f);
    glVertex2f(1.9f, -15.2f);  //ground level
    glVertex2f(0.3f, -15.2f);  //ground level
    glEnd();

    // Door handle
    glColor3ub(180, 160, 100);  //Gold/brass handle
    glBegin(GL_POLYGON);
    glVertex2f(1.4f, -10.5f);
    glVertex2f(1.7f, -10.5f);
    glVertex2f(1.7f, -11.2f);
    glVertex2f(1.4f, -11.2f);
    glEnd();

    // Window frames - dark outline
    glColor3ub(80, 60, 40);
    glLineWidth(1.0f);

    // Top floor frames
    glBegin(GL_LINE_LOOP);
    glVertex2f(-9.5f, 10.0f);
    glVertex2f(-7.3f, 10.0f);
    glVertex2f(-7.3f, 7.3f);
    glVertex2f(-9.5f, 7.3f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-3.9f, 10.0f);
    glVertex2f(-1.7f, 10.0f);
    glVertex2f(-1.7f, 7.3f);
    glVertex2f(-3.9f, 7.3f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(1.7f, 10.0f);
    glVertex2f(3.9f, 10.0f);
    glVertex2f(3.9f, 7.3f);
    glVertex2f(1.7f, 7.3f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(7.3f, 10.0f);
    glVertex2f(9.5f, 10.0f);
    glVertex2f(9.5f, 7.3f);
    glVertex2f(7.3f, 7.3f);
    glEnd();

    // Second floor frames
    glBegin(GL_LINE_LOOP);
    glVertex2f(-9.5f, 6.5f);
    glVertex2f(-7.3f, 6.5f);
    glVertex2f(-7.3f, 3.0f);
    glVertex2f(-9.5f, 3.0f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-3.9f, 6.5f);
    glVertex2f(-1.7f, 6.5f);
    glVertex2f(-1.7f, 3.0f);
    glVertex2f(-3.9f, 3.0f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(1.7f, 6.5f);
    glVertex2f(3.9f, 6.5f);
    glVertex2f(3.9f, 3.0f);
    glVertex2f(1.7f, 3.0f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(7.3f, 6.5f);
    glVertex2f(9.5f, 6.5f);
    glVertex2f(9.5f, 3.0f);
    glVertex2f(7.3f, 3.0f);
    glEnd();

    // Third floor frames
    glBegin(GL_LINE_LOOP);
    glVertex2f(-9.5f, 2.2f);
    glVertex2f(-7.3f, 2.2f);
    glVertex2f(-7.3f, -1.3f);
    glVertex2f(-9.5f, -1.3f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-3.9f, 2.2f);
    glVertex2f(-1.7f, 2.2f);
    glVertex2f(-1.7f, -1.3f);
    glVertex2f(-3.9f, -1.3f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(1.7f, 2.2f);
    glVertex2f(3.9f, 2.2f);
    glVertex2f(3.9f, -1.3f);
    glVertex2f(1.7f, -1.3f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(7.3f, 2.2f);
    glVertex2f(9.5f, 2.2f);
    glVertex2f(9.5f, -1.3f);
    glVertex2f(7.3f, -1.3f);
    glEnd();

    // Fourth floor frames
    glBegin(GL_LINE_LOOP);
    glVertex2f(-9.5f, -2.0f);
    glVertex2f(-7.3f, -2.0f);
    glVertex2f(-7.3f, -5.5f);
    glVertex2f(-9.5f, -5.5f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(-3.9f, -2.0f);
    glVertex2f(-1.7f, -2.0f);
    glVertex2f(-1.7f, -5.5f);
    glVertex2f(-3.9f, -5.5f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(1.7f, -2.0f);
    glVertex2f(3.9f, -2.0f);
    glVertex2f(3.9f, -5.5f);
    glVertex2f(1.7f, -5.5f);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex2f(7.3f, -2.0f);
    glVertex2f(9.5f, -2.0f);
    glVertex2f(9.5f, -5.5f);
    glVertex2f(7.3f, -5.5f);
    glEnd();

    // Building outline
    glColor3ub(80, 60, 40);  //Dark brown outline
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-12.0f, 11.0f);
    glVertex2f(12.0f, 11.0f);
    glVertex2f(12.0f, -15.7f);  //ground level
    glVertex2f(-12.0f, -15.7f);  //ground level
    glEnd();

    glLineWidth(1.0f);

}


void window1(float tx, float ty, float s) {
    // Window glass fill
    glColor3ub(185, 220, 240);
    glBegin(GL_POLYGON);
    glVertex2f(tx + (-12.836f * s), ty + (7.475f * s));
    glVertex2f(tx + (-11.831f * s), ty + (7.471f * s));
    glVertex2f(tx + (-11.831f * s), ty + (5.122f * s));
    glVertex2f(tx + (-12.836f * s), ty + (5.122f * s));
    glEnd();

    // Window frame outline
    glColor3ub(80, 60, 40);
    glBegin(GL_LINE_LOOP);
    glVertex2f(tx + (-12.836f * s), ty + (7.475f * s));
    glVertex2f(tx + (-11.831f * s), ty + (7.471f * s));
    glVertex2f(tx + (-11.831f * s), ty + (5.122f * s));
    glVertex2f(tx + (-12.836f * s), ty + (5.122f * s));
    glEnd();
}



void window2(float tx, float ty, float s) {
    // Window glass fill
    glColor3ub(185, 220, 240);
    glBegin(GL_POLYGON);
    glVertex2f(tx + (-14.238f * s), ty + (3.090f * s));
    glVertex2f(tx + (-12.861f * s), ty + (3.086f * s));
    glVertex2f(tx + (-12.861f * s), ty + (1.104f * s));
    glVertex2f(tx + (-14.238f * s), ty + (1.104f * s));
    glEnd();

    // Window frame outline
    glColor3ub(80, 60, 40);
    glBegin(GL_LINE_LOOP);
    glVertex2f(tx + (-14.238f * s), ty + (3.090f * s));
    glVertex2f(tx + (-12.861f * s), ty + (3.086f * s));
    glVertex2f(tx + (-12.861f * s), ty + (1.104f * s));
    glVertex2f(tx + (-14.238f * s), ty + (1.104f * s));
    glEnd();
}

// ------------------------ Door ------------------------

void door() {

    glColor3ub(157, 128, 121);
    drawSemiCircleFromPoints(
        -3.852f, -11.282f,   // Z8
        2.811f, -11.282f,   // A9
        1);

    glBegin(GL_POLYGON);
    glVertex2f(-3.852f, -11.282f);   // A
    glVertex2f(2.811f, -11.282f);   // B
    glVertex2f(2.811f, -14.743f); // B9
    glVertex2f(-3.852f, -14.743f); // C9
    glEnd();
    glColor3ub(210, 180, 160);
    drawSemiCircleFromPoints(
        -3.365f, -11.327f,   // V8
        2.347f, -11.327f,   // W8
        1
    );
    glBegin(GL_POLYGON);
    glVertex2f(-3.365f, -11.327f);   // A
    glVertex2f(2.347f, -11.327f);   // B
    glVertex2f(2.347f, -14.743f); // E9
    glVertex2f(-3.365f, -14.743f); // D9

    glEnd();

    glColor3ub(180, 150, 120);
    glBegin(GL_POLYGON);
    glVertex2f(-2.741f, -10.971f); // L9
    glVertex2f(1.690f, -10.977f); // M9
    glVertex2f(1.691f, -14.767f); // N9
    glVertex2f(-2.740f, -14.775f); // O9
    glEnd();
    //glColor3ub(180, 150, 120);
    glColor3ub(157, 128, 121);
    glBegin(GL_POLYGON);
    glVertex2f(-0.289f, -14.743f); // H9
    glVertex2f(-0.736f, -14.748f); // I9
    glVertex2f(-0.736f, -11.117f); // F9
    glVertex2f(-0.289f, -11.082f); // G9
    glEnd();

    drawCircle(-0.511f, -12.212f, 0.365f);

    glColor3ub(90, 60, 60);
    glBegin(GL_POLYGON);
    glVertex2f(-39.921f, -14.762f); // C11
    glVertex2f(30.267f, -14.762f);  // E11
    glVertex2f(30.267f, -15.668f);  // D11
    // glVertex2f(20.267f, -14.762f);  // E11
    glVertex2f(-39.921f, -15.668f); // F11
    glEnd();


}

void tree1() {
    glPushMatrix();
    glTranslatef(-20.5f, -3.5f, 0.0f);       // pivot = trunk base
    glRotatef(rainOn ? treeSway : 0.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(20.5f, 3.5f, 0.0f);

    glColor3ub(44, 51, 20);
    drawCircle(-19.0f, -5.0f, 5.013f);
    drawCircle(-22.804f, -5.950f, 3.311f);

    glColor3ub(72, 80, 48);
    drawCircle(-21.347f, -3.749f, 3.789f);
    drawCircle(-22.804f, -5.474f, 3.103f);
    drawCircle(-19.473f, -4.094f, 3.603f);

    glColor3ub(74, 63, 49);
    glBegin(GL_POLYGON);
    glVertex2f(-19.529f, -15.415f);
    glVertex2f(-19.254f, -7.238f);
    glVertex2f(-19.132f, -7.234f);
    glVertex2f(-18.609f, -15.400f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-19.101f, -10.289f);
    glVertex2f(-17.577f, -7.491f);
    glVertex2f(-18.986f, -10.784f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-19.302f, -10.569f);
    glVertex2f(-21.088f, -6.697f);
    glVertex2f(-21.030f,  -6.589f);
    glVertex2f(-19.216f,  -9.773f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-20.470f, -7.956f);
    glVertex2f(-21.450f, -7.237f);
    glVertex2f(-21.350f, -7.130f);
    glVertex2f(-20.682f, -7.501f);
    glEnd();

    glPopMatrix();
}

void tree2() {
    glPushMatrix();
    glTranslatef(19.5f, -4.0f, 0.0f);        // pivot = trunk base
    glRotatef(rainOn ? -treeSway : 0.0f, 0.0f, 0.0f, 1.0f);  // opposite sway
    glTranslatef(-19.5f, 4.0f, 0.0f);

    glColor3ub(44, 51, 20);
    drawCircle(20.249f, -3.995f, 4.191f);
    drawCircle(19.851f, -2.887f, 2.921f);
    glColor3ub(72, 80, 48);
    drawCircle(19.635f, -3.330f, 3.809f);
    drawCircle(15.176f, -4.387f, 2.020f);
    glColor3ub(44, 51, 20);
    drawCircle(19.851f, -2.887f, 2.921f);
    glColor3ub(72, 80, 48);
    drawCircle(17.025f, -1.549f, 2.198f);
    drawCircle(19.428f, -0.422f, 1.333f);
    drawCircle(19.501f, -2.362f, 2.665f);

    glColor3ub(74, 63, 49);
    glBegin(GL_POLYGON);
    glVertex2f(19.224f, -15.405f);
    glVertex2f(19.577f, -5.882f);
    glVertex2f(19.745f, -5.870f);
    glVertex2f(20.375f,  -15.348f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(19.739f, -9.621f);
    glVertex2f(21.703f, -6.075f);
    glVertex2f(19.900f, -10.288f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(19.554f,  -10.017f);
    glVertex2f(17.404f, -5.272f);
    glVertex2f(17.464f, -5.086f);
    glVertex2f(19.608f, -8.876f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(18.129f, -6.747f);
    glVertex2f(16.960f, -5.896f);
    glVertex2f(17.082f, -5.770f);
    glVertex2f(17.900f, -6.224f);
    glEnd();

    glPopMatrix();
}




void tree2_translated() {
    glPushMatrix();
    glTranslatef(15.2f, -8.2f, 0.0f);        // pivot = trunk base
    glRotatef(rainOn ? treeSway * 0.8f : 0.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-15.2f, 8.2f, 0.0f);

    glColor3ub(44, 51, 20);
    drawCircle(15.20f, -7.80f, 2.80f);
    drawCircle(14.50f, -8.50f, 2.50f);
    drawCircle(15.80f, -8.30f, 2.60f);

    glColor3ub(72, 80, 48);
    drawCircle(15.10f, -8.00f, 2.40f);
    drawCircle(14.20f, -8.80f, 2.20f);
    drawCircle(15.90f, -7.90f, 2.30f);
    drawCircle(15.50f, -8.70f, 2.10f);

    glColor3ub(44, 51, 20);
    drawCircle(15.20f, -8.20f, 1.90f);

    glColor3ub(72, 80, 48);
    drawCircle(14.70f, -8.50f, 1.80f);
    drawCircle(15.60f, -8.40f, 1.70f);
    drawCircle(15.20f, -7.70f, 1.60f);

    glColor3ub(74, 63, 49);
    glBegin(GL_POLYGON);
    glVertex2f(14.95f, -15.35f);
    glVertex2f(15.10f, -8.90f);
    glVertex2f(15.25f, -8.88f);
    glVertex2f(15.45f, -15.30f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(15.15f, -11.80f);
    glVertex2f(16.50f, -8.50f);
    glVertex2f(15.30f, -12.50f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(15.05f, -11.50f);
    glVertex2f(13.80f, -7.80f);
    glVertex2f(13.85f, -7.60f);
    glVertex2f(15.10f, -10.50f);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(14.50f, -8.50f);
    glVertex2f(13.50f, -7.90f);
    glVertex2f(13.60f, -7.70f);
    glVertex2f(14.60f, -8.20f);
    glEnd();

    glPopMatrix();
}

void airbln() {  //AirBalloon
    glPushMatrix();
    // move left to right + up-down float + scale from center
    glTranslatef(balloonX, balloonOffset, 0.0f);
    glTranslatef(19.821f, 20.0f, 0.0f);
    glScalef(balloonScale, balloonScale, 1.0f);
    glTranslatef(-19.821f, -20.0f, 0.0f);

    glColor3ub(208, 96, 115);
    drawCircle(19.821f, 20.024f, 4.964f);
    glBegin(GL_POLYGON);
    glVertex2f(15.842f, 17.058f); // J4
    glVertex2f(18.354f, 13.434f); // K4
    glVertex2f(21.304f, 13.434f); // L4
    glVertex2f(24.102f, 17.513f);  // M4
    glEnd();

    glColor3ub(194, 178, 167);
    glBegin(GL_POLYGON);
    glVertex2f(18.354f, 13.434f); // K4
    glVertex2f(21.304f, 13.434f); // L4
    glVertex2f(20.958f, 12.825f); // N4
    glVertex2f(18.694f, 12.825f); // O4
    glEnd();


    glColor3ub(131, 121, 134);
    for (int i = 0; i < 5; i++) {
        float shift = i * 0.530f;
        glBegin(GL_POLYGON);
        glVertex2f(18.694f + shift, 12.825f); // O4
        glVertex2f(18.840f + shift, 12.825f); // P4
        glVertex2f(18.840f + shift, 11.231f); // Q4
        glVertex2f(18.694f + shift, 11.228f); // R4
        glEnd();

    }

    glColor3ub(194, 178, 167);
    glBegin(GL_POLYGON);
    glVertex2f(18.694f, 11.228f); // R4
    glVertex2f(20.971f, 11.228f); // S4
    glVertex2f(20.971f, 9.453f);              // T4
    glVertex2f(18.694f, 9.453f);  // U4

    glEnd();


    glPopMatrix();

}


void road() {   //Road
    glColor3ub(250, 250, 249);

    glBegin(GL_POLYGON);
    glVertex2f(-30.0f, -15.7f);   // f
    glVertex2f(30.0f, -15.7f);   // h
    glVertex2f(30.0f, -16.45f);  // D
    glVertex2f(-30.0f, -16.45f);  // C
    glEnd();
    glColor3ub(157, 145, 145);
    glBegin(GL_POLYGON);
    glVertex2f(-30.0f, -16.45f);  // C
    glVertex2f(30.0f, -16.45f);  // D
    glVertex2f(30.0f, -20.0f);   // B
    glVertex2f(-30.0f, -20.0f);   // A
    glEnd();
}


void drawBench(float dx, float dy, float scale) {   //Bench

    glColor3f(0.72f, 0.52f, 0.26f); // brown
    glBegin(GL_POLYGON);
    glVertex2f((-1.1f * scale) + dx, (0.45f * scale) + dy);
    glVertex2f((1.1f * scale) + dx, (0.45f * scale) + dy);
    glVertex2f((1.1f * scale) + dx, (0.55f * scale) + dy);
    glVertex2f((-1.1f * scale) + dx, (0.55f * scale) + dy);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f((-1.1f * scale) + dx, (0.30f * scale) + dy);
    glVertex2f((1.1f * scale) + dx, (0.30f * scale) + dy);
    glVertex2f((1.1f * scale) + dx, (0.40f * scale) + dy);
    glVertex2f((-1.1f * scale) + dx, (0.40f * scale) + dy);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f((-1.1f * scale) + dx, (0.15f * scale) + dy);
    glVertex2f((1.1f * scale) + dx, (0.15f * scale) + dy);
    glVertex2f((1.1f * scale) + dx, (0.25f * scale) + dy);
    glVertex2f((-1.1f * scale) + dx, (0.25f * scale) + dy);
    glEnd();


    glColor3f(0.82f, 0.62f, 0.32f); // lighter brown
    glBegin(GL_POLYGON);
    glVertex2f((-1.1f * scale) + dx, (-0.05f * scale) + dy);
    glVertex2f((1.1f * scale) + dx, (-0.05f * scale) + dy);
    glVertex2f((1.1f * scale) + dx, (0.10f * scale) + dy);
    glVertex2f((-1.1f * scale) + dx, (0.10f * scale) + dy);
    glEnd();

    glColor3f(0.1f, 0.1f, 0.1f);


    glBegin(GL_POLYGON);
    glVertex2f((-0.9f * scale) + dx, (-0.05f * scale) + dy);
    glVertex2f((-0.8f * scale) + dx, (-0.05f * scale) + dy);
    glVertex2f((-0.75f * scale) + dx, (-0.5f * scale) + dy);
    glVertex2f((-0.95f * scale) + dx, (-0.5f * scale) + dy);
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f((0.8f * scale) + dx, (-0.05f * scale) + dy);
    glVertex2f((0.9f * scale) + dx, (-0.05f * scale) + dy);
    glVertex2f((0.95f * scale) + dx, (-0.5f * scale) + dy);
    glVertex2f((0.75f * scale) + dx, (-0.5f * scale) + dy);
    glEnd();
}



void NdrawLampPost(float x, float y, float scale)  //night //Lamp Post
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    glColor3ub(60, 60, 60);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.100f, 0.368f);   // C
    glVertex2f(0.002f, 0.429f);    // D
    glVertex2f(0.104f, 0.368f);    // E
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.003f, 0.369f);    // F
    glVertex2f(0.008f, 0.369f);       // G
    glVertex2f(0.007f, 0.043f);     // H
    glVertex2f(-0.002f, 0.043f);   // I
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(-0.014f, 0.042f);     // J
    glVertex2f(0.018f, 0.043f);    // K
    glVertex2f(0.039f, -0.557f);   // M
    glVertex2f(-0.035f, -0.557f);  // L
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.058f, -0.557f);  // N
    glVertex2f(0.061f, -0.557f);   // O
    glVertex2f(0.061f, -0.565f);   // P
    glVertex2f(-0.057f, -0.566f);  // Q

    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.081f, 0.368f);   // R
    glVertex2f(-0.072f, 0.369f);   // S
    glVertex2f(-0.053f, 0.266f);    // T
    glVertex2f(-0.06f, 0.26f);                         // U
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(0.076f, 0.368f);    // V
    glVertex2f(0.086f, 0.368f);     // W
    glVertex2f(0.065f, 0.264f);    // Z
    glVertex2f(0.056f, 0.269f);      // A1
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.06f, 0.26f);                         // U
    glVertex2f(-0.053f, 0.266f);    // T
    glVertex2f(0.056f, 0.269f);      // A1
    glVertex2f(0.065f, 0.264f);    // Z
    glVertex2f(0.054f, 0.240f);    // B1
    glVertex2f(-0.049f, 0.238f);   // C1

    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_POLYGON);
    glVertex2f(-0.072f, 0.369f);   // S
    glVertex2f(-0.003f, 0.369f);    // F
    glVertex2f(-0.004f, 0.267f);  // D1
    glVertex2f(-0.053f, 0.266f);    // T
    glColor3f(1.0f, 1.0f, 1.0f);
    glVertex2f(-0.072f, 0.369f);   // S
    glVertex2f(-0.003f, 0.369f);    // F
    glVertex2f(-0.004f, 0.267f);  // D1
    glVertex2f(-0.053f, 0.266f);    // T
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(0.008f, 0.369f);       // G
    glVertex2f(0.076f, 0.368f);    // V
    glVertex2f(0.056f, 0.269f);      // A1
    glVertex2f(0.008f, 0.266f);   // E1
    glColor3f(1.0f, 1.0f, 1.0f);

    glVertex2f(0.008f, 0.369f);       // G
    glVertex2f(0.076f, 0.368f);    // V
    glVertex2f(0.056f, 0.269f);      // A1
    glVertex2f(0.008f, 0.266f);   // E1
    glEnd();

    glPopMatrix();
}



void drawLampPost(float x, float y, float scale)  //day//Lamp Post
{
    glPushMatrix();
    glTranslatef(x, y, 0.0f);
    glScalef(scale, scale, 1.0f);
    glColor3ub(60, 60, 60);
    glBegin(GL_TRIANGLES);
    glVertex2f(-0.100f, 0.368f);   // C
    glVertex2f(0.002f, 0.429f);    // D
    glVertex2f(0.104f, 0.368f);    // E
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.003f, 0.369f);    // F
    glVertex2f(0.008f, 0.369f);       // G
    glVertex2f(0.007f, 0.043f);     // H
    glVertex2f(-0.002f, 0.043f);   // I
    glEnd();
    glBegin(GL_QUADS);
    glVertex2f(-0.014f, 0.042f);     // J
    glVertex2f(0.018f, 0.043f);    // K
    glVertex2f(0.039f, -0.557f);   // M
    glVertex2f(-0.035f, -0.557f);  // L
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.058f, -0.557f);  // N
    glVertex2f(0.061f, -0.557f);   // O
    glVertex2f(0.061f, -0.565f);   // P
    glVertex2f(-0.057f, -0.566f);  // Q

    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.081f, 0.368f);   // R
    glVertex2f(-0.072f, 0.369f);   // S
    glVertex2f(-0.053f, 0.266f);    // T
    glVertex2f(-0.06f, 0.26f);                         // U
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(0.076f, 0.368f);    // V
    glVertex2f(0.086f, 0.368f);     // W
    glVertex2f(0.065f, 0.264f);    // Z
    glVertex2f(0.056f, 0.269f);      // A1
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-0.06f, 0.26f);                         // U
    glVertex2f(-0.053f, 0.266f);    // T
    glVertex2f(0.056f, 0.269f);      // A1
    glVertex2f(0.065f, 0.264f);    // Z
    glVertex2f(0.054f, 0.240f);    // B1
    glVertex2f(-0.049f, 0.238f);   // C1

    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_POLYGON);
    glVertex2f(-0.072f, 0.369f);   // S
    glVertex2f(-0.003f, 0.369f);    // F
    glVertex2f(-0.004f, 0.267f);  // D1
    glVertex2f(-0.053f, 0.266f);    // T
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex2f(-0.072f, 0.369f);   // S
    glVertex2f(-0.003f, 0.369f);    // F
    glVertex2f(-0.004f, 0.267f);  // D1
    glVertex2f(-0.053f, 0.266f);    // T
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(0.008f, 0.369f);       // G
    glVertex2f(0.076f, 0.368f);    // V
    glVertex2f(0.056f, 0.269f);      // A1
    glVertex2f(0.008f, 0.266f);   // E1
    glColor3f(1.0f, 1.0f, 0.0f);

    glVertex2f(0.008f, 0.369f);       // G
    glVertex2f(0.076f, 0.368f);    // V
    glVertex2f(0.056f, 0.269f);      // A1
    glVertex2f(0.008f, 0.266f);   // E1
    glEnd();

    glPopMatrix();
}


void bus()  //Bus
{
    glPushMatrix();
    glTranslatef(Y, 0.0f, 0.0f);
    glColor3ub(220, 220, 80);
    glBegin(GL_POLYGON);
    glVertex2f(-13.317f, -14.966f); // F1
    glVertex2f(-13.325f, -17.878f); // G1
    glVertex2f(-14.868f, -17.962f); // P9
    glVertex2f(-23.045f, -17.877f); // Q9
    glVertex2f(-23.735f, -17.775f); // R9
    glVertex2f(-23.709f, -15.167f); // S9
    glVertex2f(-21.757f, -13.141f); // T9
    glVertex2f(-15.019f, -13.127f); // U9
    glEnd();



    drawSemiCircleFromPoints(-16.889f, -13.791f,
        -13.802f, -16.479f, 1);

    drawSemiCircleFromPoints(-23.542f, -16.149f,
        -19.568f, -14.402f, 1);
    glColor3ub(0, 0, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-13.250f, -16.953f); // H10
    glVertex2f(-13.250f, -18.002f); // I10
    glVertex2f(-23.737f, -18.002f); // J10
    glVertex2f(-23.737f, -16.953f); // J10
    glEnd();
    // Circle p (D1, E1)
    glColor3ub(35, 39, 42);
    drawCircle(-21.828f, -18.119f, 1.047f);

    // Circle q (D1, F1)
    glColor3ub(255, 255, 255);
    drawCircle(-21.828f, -18.119f, 0.662f);

    // Circle k2 (D1, G1)
    glColor3ub(177, 173, 167);
    drawCircle(-21.828f, -18.119f, 0.384f);

    // Circle P2 (B9, C9)
    glColor3ub(35, 39, 42);
    drawCircle(-16.184f, -17.868f, 1.038f);

    // Circle 42 (B9, D9)
    glColor3ub(255, 255, 255);
    drawCircle(-16.184f, -17.868f, 0.658f);

    // Circle 12 (B9, E9)
    glColor3ub(177, 173, 167);
    drawCircle(-16.184f, -17.868f, 0.376f);

    glColor3ub(0, 0, 0);
    glBegin(GL_POLYGON);
    glVertex2f(-22.674f, -14.039f); // F4
    glVertex2f(-20.932f, -14.050f); // G9
    glVertex2f(-20.930f, -15.179f); // I9
    glVertex2f(-22.674f, -15.190f); // H9
    glEnd();


    glBegin(GL_POLYGON);
    glVertex2f(-20.418f, -13.980f); // J9
    glVertex2f(-17.603f, -13.980f); // K9
    glVertex2f(-17.603f, -15.191f); // L9
    glVertex2f(-20.418f, -15.191f); // N9
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(-16.964f, -13.985f); // M9
    glVertex2f(-14.263f, -13.985f); // O9
    glVertex2f(-14.263f, -15.216f); // P9
    glVertex2f(-16.964f, -15.216f); // Q9
    glEnd();



    glLineWidth(1.0f);
    glColor3ub(0, 0, 0);

    glBegin(GL_LINES);

    glVertex2f(-22.931f, -13.690f); // R9
    glVertex2f(-13.901f, -13.690f); // S9


    glVertex2f(-20.680f, -14.050f); // T
    glVertex2f(-20.680f, -17.925f);   //


    glVertex2f(-17.424f, -13.944f); // V
    glVertex2f(-17.424f, -17.921f); // W


    glVertex2f(-23.720f, -15.492f); // Z
    glVertex2f(-13.286f, -15.492f); // A10


    glVertex2f(-13.802f, -17.030f); // B10
    glVertex2f(-13.802f, -13.740f); // C10


    glVertex2f(-18.211f, -15.941f); // D10
    glVertex2f(-17.697f, -15.941f); // E10

    glVertex2f(-17.063f, -15.988f); // F10
    glVertex2f(-16.545f, -15.988f); // G10

    glEnd();
    glPopMatrix();

}


void truck() {  //Truck
    glPushMatrix();
    glTranslatef(X, 0.0f, 0.0f);

    glColor3ub(15, 60, 60);

    glBegin(GL_POLYGON);
    glVertex2f(19.351f, -14.873f);    // R21
    glVertex2f(19.351f, -13.188f);    // S21
    glVertex2f(17.308f, -13.132f);    // T21
    glVertex2f(16.681f, -13.764f);    // U21
    glVertex2f(16.221f, -14.517f);    // V21
    glVertex2f(13.251f, -15.140f);    // W21
    glVertex2f(12.732f, -15.672f);      // B3
    glVertex2f(12.930f, -16.825f);    // Z21=
    glVertex2f(24.395f, -16.825f);         // Q21
    glVertex2f(24.395f, -14.873f);         // P21
    glEnd();
    glColor3f(0.6f, 0.8f, 0.9f);   // Light sky-blue glass
    glBegin(GL_POLYGON);
    glVertex2f(18.753f, -14.551f);         // E22
    glVertex2f(18.753f, -13.441f);                // F22
    glVertex2f(17.430f, -13.441f);          // G22
    glVertex2f(16.862f, -14.551f);  // H22
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f); // black lines
    glLineWidth(1.0f);           // optional: line thickness
    glBegin(GL_LINE_LOOP);
    glVertex2f(16.709f, -16.451f);   // I22
    glVertex2f(19.060f, -16.451f);   // J22
    glVertex2f(19.060f, -14.707f);  // K22
    glVertex2f(16.709f, -14.707f);   // L22
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2f(16.709f, -14.707f);   // L22
    glVertex2f(16.709f, -14.432f);   // F2
    glVertex2f(17.362, -13.299);   // L2
    glVertex2f(19.060, -13.299);   // G5
    glVertex2f(19.060f, -14.707f);  // K22
    glEnd();
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(15.718f, -16.825f);   // B22
    glVertex2f(20.840f, -16.825f);   // C22
    glEnd();
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(18.2f, -15.3f);    // I13
    glVertex2f(18.765f, -15.3f);  // J13
    glEnd();

    drawSemiCircleFromPoints(13.339f, -16.825f, 15.718f, -16.825f, 1);

    drawSemiCircleFromPoints(20.840f, -16.825f, 23.189f, -16.825f, 1);

    glColor3f(0.2f, 0.2f, 0.2f);
    drawCircle(22.017f, -16.847f, 0.965f);
    drawCircle(14.555f, -16.691f, 0.969f);
    glColor3f(1.0f, 1.0f, 1.0f);
    drawCircle(14.555f, -16.691f, 0.591f);  // e2
    drawCircle(22.017f, -16.847f, 0.607f);  // f2


    glPopMatrix();
}
void timer(int value) {
    if (currentScenario == 1) {
        isDay = !isDay;
        glutPostRedisplay();
        glutTimerFunc(5000, timer, 0);
    }
}
//======........................end of Scenario1 ....................============


//.....................start Scenario 2............................

//...... Scenario 2 variables...
float V = 35.0f;
float W = -35.0f;
float suvSpeed = -1.0f;
float BusS268Speed = 1.0f;  // Normal smooth speed
float planeX = 15.0f;
float planeSpeed = -.250f;
float planeScale = 1.0f;
float planeScaleDir = 1.0f;
float cloudSX = -25.0f;    // Starting position (left side)
float cloudSSpeed = 0.05f; // Slower cloud speed for Scenario 2
bool trafficLightGreen = true;      // Light stays green until both vehicles arrive
bool trafficLightTimerRunning = false;
bool trafficLightReadyForNextTrigger = true;
bool suvStoppedAtLight = false;
bool busStoppedAtLight = false;


// Function declarations Scenario 2
void planeS268();
void cloudS268();
void sun(float a, float b, float c);
void moon(float a, float b, float c);
void sky268();
void Factory268();
void wareHouse();
void road268();
void palace268();
void doggo();
void suv268();
void treeS1268(float baseX, float baseY);
void drawAllTrees();
void lampPost(float baseX, float baseY);
void lampPostNight(float baseX, float baseY);
void drawAllLampPosts();
void drawNightAllLampPosts();
void bus268();
void drawTrafficLightS2();
void turnTrafficLightGreenS2(int value);
void tryTriggerTrafficLightS2();


//.....Animation function Scenario 2.....


///.................................................................................................
void turnTrafficLightGreenS2(int value) {
    trafficLightGreen = true;
    trafficLightTimerRunning = false;
    suvStoppedAtLight = false;
    busStoppedAtLight = false;
    glutPostRedisplay();
}

void tryTriggerTrafficLightS2() {
    float suvFrontX = V + 12.732f;  // SUV moves right -> left
    float busFrontX = W - 11.643f;  // Bus moves left -> right

    // After a green release, wait until both vehicles leave the signal area
    // before allowing the next red cycle.
    if (trafficLightGreen && !trafficLightReadyForNextTrigger &&
        suvFrontX < -2.0f && busFrontX > 2.0f) {
        trafficLightReadyForNextTrigger = true;
    }

    bool suvAtLight = (suvFrontX <= 4.0f && suvFrontX >= -2.0f);
    bool busAtLight = (busFrontX >= -4.0f && busFrontX <= 2.0f);

    // Red light starts only when BOTH vehicles are near the traffic light.
    if (trafficLightGreen && !trafficLightTimerRunning &&
        trafficLightReadyForNextTrigger &&
        suvAtLight && busAtLight) {
        trafficLightGreen = false;
        trafficLightTimerRunning = true;
        trafficLightReadyForNextTrigger = false;

        // Stop both vehicles cleanly at the two stop lines.
        V = 2.0f - 12.732f;
        W = -2.0f + 11.643f;
        suvStoppedAtLight = true;
        busStoppedAtLight = true;

        glutTimerFunc(4000, turnTrafficLightGreenS2, 0);
    }
}

void animationSUV(int value) {
    if (currentScenario == 2 && !vehiclePaused) {
        tryTriggerTrafficLightS2();

        if (!trafficLightGreen && suvStoppedAtLight) {
            V = 2.0f - 12.732f;
        } else {
            V += suvSpeed;
        }

        if (V < -60.0f) V = 35.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(40, animationSUV, 0);
}
///...............................................................................................
void animationPlane(int value) { //
    if (currentScenario == 2) {
        planeX += planeSpeed;   // move truck
        if (planeX < -60.0f) {
            planeX = 35.0f;     // reset to right side
        }

        // gentle grow-shrink animation for the airplane
        planeScale += 0.003f * planeScaleDir;
        if (planeScale >= 1.15f) planeScaleDir = -1.0f;
        if (planeScale <= 0.85f) planeScaleDir =  1.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(40, animationPlane, 0);  // 16 ms
}
///..................................................................................................
void animationBus(int value) {
    if (currentScenario == 2 && !vehiclePaused) {
        tryTriggerTrafficLightS2();

        if (!trafficLightGreen && busStoppedAtLight) {
            W = -2.0f + 11.643f;
        } else {
            W += BusS268Speed;
        }

        if (W > 60.0f) W = -35.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(40, animationBus, 0);
}
///Cloud.................................................................................................

void animationCloudS268(int value) {
    if (currentScenario == 2) {
        cloudSX += cloudSSpeed;      // Move right
        if (cloudSX > 55.0f) {      // If goes off screen, reset to left
            cloudSX = -55.0f;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(10, animationCloudS268, 0);  // Call again after 40ms
}

void timers(int value) {
    if (currentScenario == 2)
    {
        isDay = !isDay;
        glutPostRedisplay();
        glutTimerFunc(5000, timers, 0);
    }
}
void drawSnowOnRoofsScenario2() {
    //render snow Scenario 2 building
}


void drawSnowOnTreesScenario2() {
    if (!winterMode) return;
    glColor3ub(255, 255, 255);

    struct P { float x, y, r; } pts[] = {
        { -28.0f, -10.1f, 0.9f },
        { -14.05f, -10.1f, 0.8f },
        { -11.17f, -9.23f, 0.8f },
        { 11.83f, -9.23f, 0.8f },
        { 14.18f, -10.1f, 0.8f },
        { 26.45f, -10.1f, 0.9f }
    };
    int ptsCount = sizeof(pts) / sizeof(pts[0]);
    for (int i = 0; i < ptsCount; ++i) {
        drawCircle(pts[i].x, pts[i].y, pts[i].r);
    }
}
void drawSnowOnLampPostsScenario2() {
    if (!winterMode) return;
    glColor3ub(255, 255, 255);
    // Lamp heads: use baseX and baseY from Scenario 2 definitions
    struct L { float bx, by; } lamps[] = {
        { -24.310f, -15.875f },
        { -7.624f, -15.689f },
        { 7.126f, -15.760f },
        { 22.444f, -15.760f }
    };
    int lampsCount = sizeof(lamps) / sizeof(lamps[0]);
    for (int i = 0; i < lampsCount; ++i) {
        float hx = lamps[i].bx;
        float hy = lamps[i].by + 10.04f; // lamp head center approx
        drawCircle(hx, hy, 0.8f);
    }
}


void planeS268() //Plane
{
    glPushMatrix();
    glTranslatef(planeX, 0.0f, 0.0f);
    glTranslatef(21.0f, 14.5f, 0.0f);
    glScalef(planeScale, planeScale, 1.0f);
    glTranslatef(-21.0f, -14.5f, 0.0f);

    glColor3f(1.0f, 0.780f, 0.173f);//plane body color
    glBegin(GL_POLYGON);
    glVertex2f(17.511f, 14.986f);   // P12
    glVertex2f(23.613f, 14.986f);   // O12
    glVertex2f(25.308f, 15.052f); // Q12
    glVertex2f(25.319f, 14.116f);  // R12
    glVertex2f(23.534f, 13.385f);  // S12
    glVertex2f(21.0f, 13.0f);                     // T12
    glVertex2f(17.977f, 12.715f); // U12
    glVertex2f(17.044f, 12.732f);  // V12
    glVertex2f(16.759f, 12.767f); // W12
    glVertex2f(16.432f, 12.838f); // Z12
    glVertex2f(15.894f, 13.026f); // A13
    glEnd();

    drawSemiCircleFromPoints(15.813f, 14.070f, 16.585f, 13.203f, 1);//p1
    drawSemiCircleFromPoints(16.341f, 14.200f, 18.645f, 13.320f, 1);//p5
    drawSemiCircleFromPoints(25.308f, 15.052f, 25.319f, 14.116f, 1);//s

    glBegin(GL_POLYGON);
    glVertex2f(18.120f, 14.990f);  // F13
    glVertex2f(17.934f, 15.719f); // G14
    glVertex2f(18.528f, 15.713f); // H14
    glVertex2f(19.203f, 15.024f); // I14
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(19.681f, 13.261f); // J14
    glVertex2f(21.130f, 13.754f); // O14
    glVertex2f(25.340f, 13.033f); // P14
    glVertex2f(24.309f, 12.477f);  // Q14
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(23.613f, 14.986f);   // O12
    glVertex2f(24.933f, 16.632f);   // k16
    glVertex2f(25.680f, 16.806f);   // k16
    glVertex2f(25.308f, 15.052f); // Q12

    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(25.340f, 13.033f); // P14
    glVertex2f(25.571f, 13.059f); // E16
    glVertex2f(25.460f, 12.889f); // D16
    glVertex2f(24.591f, 12.524f); // C16
    glVertex2f(24.309f, 12.477f);  // Q14
    glVertex2f(24.507f, 12.663f); // B16

    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(25.308f, 15.052f);  // Q12
    glVertex2f(25.319f, 14.116f);   // R12
    glVertex2f(26.541f, 14.655f);  // D17
    glVertex2f(26.827f, 14.986f);   // C17
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(16.585f, 13.203f);  // C13
    glVertex2f(18.645f, 13.320f); // E13
    glVertex2f(17.049f, 14.102f);  // A17
    glVertex2f(16.516f, 14.290f);  // B17
    glVertex2f(16.0f, 13.5f);  // j16
    glEnd();


    glColor3f(0.753f, 0.369f, 0.239f);
    drawSemiCircleFromPoints(15.894f, 13.026f, 15.813f, 14.070f, 1);//t
    glColor3f(0.0f, 0.0f, 0.0f);//black
    drawCircle(21.368f, 12.373f, 0.645f); //q5
    drawCircle(23.329f, 12.353f, 0.687f); //r5
    drawCircle(20.166f, 14.385f, 0.281f);//e6
    drawCircle(20.771f, 14.396f, 0.257f);//f6
    drawCircle(21.371f, 14.396f, 0.245f);//g6
    drawCircle(21.961f, 14.402f, 0.236f);//h6

    glColor3f(0.008f, 0.518f, 0.780f);// enginee
    drawCircle(21.368f, 12.373f, 0.492f);
    drawCircle(23.329f, 12.353f, 0.485f);

    glColor3f(0.2f, 0.737f, 0.784f);//light blue
    drawCircle(21.368f, 12.373f, 0.330f);
    drawCircle(23.329f, 12.353f, 0.226f);
    // k6:
    drawCircle(21.961f, 14.402f, 0.174f);
    // P6:
    drawCircle(21.371f, 14.396f, 0.172f);
    // 96:
    drawCircle(20.771f, 14.396f, 0.172f);
    // r6:
    drawCircle(20.166f, 14.385f, 0.198f);

    glBegin(GL_POLYGON);
    glVertex2f(16.994f, 14.728f); // V16
    glVertex2f(17.771f, 14.614f);   // W16
    glVertex2f(17.804f, 14.063f); // Z16
    glVertex2f(17.049f, 14.102f);  // A17
    glVertex2f(16.516f, 14.290f);  // B17
    glEnd();

    glPopMatrix();

}


void cloudS268() //Cloud
{
    glPushMatrix();
    glTranslatef(cloudSX, 0.0f, 0.0f);

    glColor3ub(255, 255, 255);

    //SobjCloud1  start
    drawCircle(-10.204f, 19.572f, 1.607f);
    drawSemiCircleFromPoints(-20.405f, 19.122f, -19.288f, 19.271f, 1);
    drawSemiCircleFromPoints(-19.676f, 19.436f, -18.590f, 20.137f, 1);
    drawSemiCircleFromPoints(-19.164f, 20.048f, -17.302f, 21.048f, 1);
    drawSemiCircleFromPoints(-17.954f, 21.302f, -13.0f, 20.0f, 1);
    drawSemiCircleFromPoints(-13.159f, 20.929f, -11.453f, 20.122f, 1);
    drawSemiCircleFromPoints(-11.670f, 20.421f, -9.429f, 18.651f, 1);
    drawSemiCircleFromPoints(-9.513f, 19.436f, -11.065f, 18.346f, 0);
    drawSemiCircleFromPoints(-10.801f, 18.779f, -15.735f, 20.735f, 0);
    drawSemiCircleFromPoints(-14.509f, 17.838f, -17.0f, 20.0f, 0);
    drawSemiCircleFromPoints(-16.712f, 18.390f, -18.466f, 19.376f, 0);
    drawSemiCircleFromPoints(-18.326f, 18.495f, -20.064f, 19.301f, 0);
    drawSemiCircleFromPoints(-19.303f, 18.346f, -21.150f, 19.212f, 0);
    drawSemiCircleFromPoints(-21.122f, 18.770f, -20.211f, 19.363f, 1);

    glBegin(GL_POLYGON); //First cloud polygon
    glVertex2f(-21.2f, 18.8f);                     // I25
    glVertex2f(-20.447f, 19.452f); // J25
    glVertex2f(-20.306f, 19.330f); // K25
    glVertex2f(-19.8f, 19.6f);                     // L25
    glVertex2f(-19.045f, 20.333f); // M25
    glVertex2f(-17.732f, 21.258f); // N25
    glVertex2f(-13.014f, 20.935f); // O25
    glVertex2f(-11.164f, 20.519f); // P25
    glVertex2f(-9.357f, 19.875f);  // Q25
    glVertex2f(-9.637f, 18.642f);  // R25
    glVertex2f(-11.200f, 18.517f); // S25
    glVertex2f(-15.700f, 18.268f); // T25
    glVertex2f(-17.600f, 18.316f); // U25
    glVertex2f(-18.695f, 18.392f); // V25
    glVertex2f(-19.638f, 18.219f); // W25
    glVertex2f(-20.732f, 18.081f); // Z25
    glEnd();

    //SobjCloud2 ...... 2nd cloud start
    drawSemiCircleFromPoints(-9.513f, 19.436f, -11.065f, 18.346f, 0);
    drawSemiCircleFromPoints(14.182f, 6.413f, 18.366f, 6.505f, 1);
    drawSemiCircleFromPoints(16.992f, 8.226f, 20.409f, 9.271f, 1);
    drawSemiCircleFromPoints(19.867f, 8.995f, 21.847f, 6.843f, 1);
    drawSemiCircleFromPoints(21.304f, 7.243f, 23.316f, 5.644f, 1);
    drawSemiCircleFromPoints(23.124f, 6.259f, 26.350f, 4.784f, 1);
    drawSemiCircleFromPoints(26.0f, 6.0f, 26.0f, 4.0f, 1);
    drawSemiCircleFromPoints(26.341f, 4.428f, 20.833f, 6.974f, 0);
    drawSemiCircleFromPoints(22.316f, 5.251f, 17.302f, 6.654f, 0);
    drawSemiCircleFromPoints(18.036f, 5.572f, 13.815f, 5.662f, 0);
    drawSemiCircleFromPoints(13.815f, 5.662f, 11.891f, 6.466f, 0);
    drawSemiCircleFromPoints(12.352f, 6.256f, 11.265f, 7.279f, 0);
    drawSemiCircleFromPoints(11.125f, 7.069f, 14.341f, 6.326f, 1);

    glBegin(GL_POLYGON); //2nd Cloud polygon
    glVertex2f(11.5f, 7.5f);               // H25
    glVertex2f(14.844f, 6.935f); // A26
    glVertex2f(17.352f, 8.841f); // B26
    glVertex2f(19.674f, 9.740f);   // C26
    glVertex2f(20.419f, 8.890f); // D26
    glVertex2f(21.769f, 7.943f); // E26
    glVertex2f(22.375f, 7.020f);  // F26
    glVertex2f(23.927f, 6.570f); // G26
    glVertex2f(26.035f, 6.400f);   // H26
    glVertex2f(26.275f, 5.744f); // I26
    glVertex2f(25.694f, 3.850f); // J26
    glVertex2f(22.097f, 3.728f); // K26
    glVertex2f(21.113f, 4.591f);   // L26
    glVertex2f(19.220f, 4.275f);// M26
    glVertex2f(16.784f, 4.749f);// N26
    glVertex2f(14.260f, 5.016f);// O26
    glVertex2f(13.566f, 5.963f);// P26
    glVertex2f(12.178f, 5.902f);// Q26
    glVertex2f(11.522f, 6.655f); // R26
    glEnd();
    drawCircle(24.910f, 5.332f, 2.149f);
    drawCircle(12.700f, 6.764f, 1.568f);
    drawCircle(21.128f, 8.089f, 1.146f);
    //2nd cloud end


   //SobjCloud3 small cloud start
    drawSemiCircleFromPoints(5.908f, 21.515f, 8.116f, 21.949f, 1); // Z26-A27
    drawSemiCircleFromPoints(7.824f, 21.985f, 8.576f, 21.786f, 1); // B27-C27
    drawSemiCircleFromPoints(5.776f, 20.918f, 4.753f, 21.334f, 0); // D27-E27
    drawSemiCircleFromPoints(4.884f, 21.072f, 6.124f, 21.243f, 1); // F27-G27
    drawSemiCircleFromPoints(8.5f, 22.0f, 9.327f, 21.063f, 2); // H27-127
    drawSemiCircleFromPoints(9.368f, 21.333f, 8.5f, 21.0f, 0); // J27-K27
    drawSemiCircleFromPoints(8.830f, 21.259f, 6.780f, 21.867f, 0); // L27-M27
    drawSemiCircleFromPoints(7.284f, 20.946f, 5.618f, 21.588f, 0); // N27-O27
    drawCircle(5.178f, 21.046f, sqrt(0.374f)); // P27
    drawCircle(9.234f, 21.497f, sqrt(0.289f)); // Q27

    glBegin(GL_POLYGON);
    glVertex2f(5.372f, 20.774f); // S27
    glVertex2f(5.324f, 21.373f);  // T27
    glVertex2f(7.133f, 22.150f); // U27
    glVertex2f(9.202f, 21.769f); // V27
    glVertex2f(8.208f, 20.805f); // W27
    glVertex2f(6.552f, 20.821f); // Z27
    glEnd();
    //Small cloud end

    glPopMatrix();

}


void sun(float a, float b, float c)//Sun
{
    glColor3ub(255, 255, 0);
    drawCircle(15.798f, 19.783f, 3.965f);

}

void moon(float a, float b, float c)//Moon
{
    glColor3f(0.85f, 0.85f, 0.80f);//moon color
    drawCircle(15.798f, 19.783f, 3.965f);

    glColor3f(0.0f, 0.0f, 0.2f);
    drawCircle(17.321f, 19.792f, 3.493f);

}


void sky268() { //Sky
    glColor3ub(219, 241, 249); // Sky blue color

    glBegin(GL_POLYGON);
    glVertex2f(-30.0f, -20.0f);    // A
    glVertex2f(30.0f, -20.0f);    // B
    glVertex2f(30.0f, 26.774f); // D
    glVertex2f(-30.0f, 26.774f); // C
    glEnd();
}
void Factory268() { //Factory

    //glColor3f(0.945f, 0.478f, 0.380f);//factory pink color
    glColor3f(0.110f, 0.141f, 0.231f);//Deep blue factory color
    glBegin(GL_POLYGON);
    glVertex2f(-30.0f, -15.758f);              // F
    glVertex2f(-30.0f, -4.235f);            // W23
    glVertex2f(-25.532f, -4.235f);         // U23
    glVertex2f(-25.532f, -15.758f);          // T23
    glEnd();
    glColor3f(0.945f, 0.478f, 0.380f);//factory pink color
    glBegin(GL_POLYGON);
    glVertex2f(-14.583f, -4.235f);        // R23
    glVertex2f(-14.583f, -15.758f);         // S23
    glVertex2f(-25.532f, -15.758f);          // T23
    glVertex2f(-25.532f, -4.235f);         // U23
    glVertex2f(-20.058f, -0.01f);          // V23
    glEnd();

    glColor3f(0.133f, 0.176f, 0.388f);//lite blue color
    glBegin(GL_POLYGON);
    glVertex2f(-25.532f, -4.235f);         // U23
    glVertex2f(-20.058f, -0.01f);          // V23
    glVertex2f(-24.526f, -0.014f);         // Z23
    glVertex2f(-30.0f, -3.401f);          // L13
    glVertex2f(-30.0f, -4.235f);            // W23
    glEnd();
    glColor3f(0.945f, 0.478f, 0.380f);//factory pink color
    glBegin(GL_POLYGON);
    glVertex2f(-25.501f, -1.779f);   // K29
    glVertex2f(-25.501f, 1.436f);   // L29
    glVertex2f(-24.357f, 1.436f);   // M29
    glVertex2f(-24.357f, -1.778f);  // N29
    glEnd();
    glColor3f(0.133f, 0.176f, 0.388f);//lite blue color
    glBegin(GL_POLYGON);
    glVertex2f(-24.357f, 1.436f);   // M29
    glVertex2f(-24.357f, -1.778f);  // N29
    glVertex2f(-23.627f, -0.850f);  // O29
    glVertex2f(-23.627f, 0.975f);   // P29
    glEnd();



    // factory top  windows
    for (int i = 0; i < 3; i++) {
        float dx = i * 3.0f;
        if (isDay) glColor3f(0.133f, 0.176f, 0.388f);
        else       glColor3f(1.0f, 0.78f, 0.28f);
        glBegin(GL_POLYGON);
        glVertex2f(-23.785f + dx, -6.066f); // S6
        glVertex2f(-21.777f + dx, -6.066f); // T6
        glVertex2f(-21.777f + dx, -8.124f);
        glVertex2f(-23.785f + dx, -8.124f);
        glEnd();
        for (int i = 0; i < 3; i++)
        {
            float dx = i * 3.0f;
            drawSemiCircleFromPoints(-23.785f + dx, -6.066f, -21.777f + dx, -6.066f, 1);
        }
    }
    // factory down big single windows
    if (isDay) glColor3f(0.133f, 0.176f, 0.388f);
    else       glColor3f(1.0f, 0.78f, 0.28f);
    glBegin(GL_POLYGON);
    glVertex2f(-23.785f, -13.041f);   // W28
    glVertex2f(-23.785f, -15.264f);   // Z28
    glVertex2f(-15.401f, -15.264f);   // A29
    glVertex2f(-15.401f, -13.041f);   // B29
    glEnd();
    //factory downwindows
    for (int i = 0; i < 3; i++)
    {
        float dx = i * 3.0f;
        if (isDay) glColor3f(0.133f, 0.176f, 0.388f);
        else       glColor3f(1.0f, 0.78f, 0.28f);
        glBegin(GL_POLYGON);
        glVertex2f(-23.785f + dx, -10.555f); // S6
        glVertex2f(-21.777f + dx, -10.555f); // T6
        glVertex2f(-21.777f + dx, -12.213f);
        glVertex2f(-23.785f + dx, -12.213f);
        glEnd();
        for (int i = 0; i < 3; i++) {
            float dx = i * 3.0f;
            drawSemiCircleFromPoints(-23.785f + dx, -10.555f, -21.777f + dx, -10.555f, 1);
        }

    }

}
void wareHouse() { //WareHouse

    glColor3f(0.184f, 0.275f, 0.161f);//ware house left color
    glBegin(GL_POLYGON);
    glVertex2f(17.021f, -2.265f);   // S22
    glVertex2f(23.184f, 4.212f);    // W22
    glVertex2f(23.184f, -15.672f);  // V22
    glVertex2f(17.021f, -15.672f);  // R22
    glEnd();
    if (isDay) glColor3f(1.0f, 1.0f, 1.0f);
    else       glColor3f(1.0f, 0.82f, 0.35f);
    glBegin(GL_POLYGON);
    glVertex2f(21.609f, -3.847f);   // A23
    glVertex2f(21.609f, -6.826f);   // B23
    glVertex2f(18.126f, -6.826f);   // C23
    glVertex2f(18.126f, -3.847f);   // D23
    glEnd();
    glColor3f(0.184f, 0.275f, 0.161f);//ware house left color
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(19.868f, -3.847f);   // I23
    glVertex2f(19.868f, -6.826f);          // J23
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(21.609f, -5.337f);          // K23
    glVertex2f(18.126f, -5.337f);          // L23
    glEnd();

    glColor3f(0.161f, 0.224f, 0.122f); //ware house right color
    glBegin(GL_POLYGON);
    glVertex2f(23.184f, 4.212f);    // W22
    glVertex2f(23.184f, -15.672f);  // V22
    glVertex2f(29.627f, -15.672f);  // Q22
    glVertex2f(29.627f, -2.265f);   // T22
    glEnd();
    if (isDay) glColor3f(1.0f, 1.0f, 1.0f);
    else       glColor3f(1.0f, 0.82f, 0.35f);
    glBegin(GL_POLYGON);
    glVertex2f(24.759f, -3.847f);   // E23
    glVertex2f(28.242f, -3.847f);   // F23
    glVertex2f(28.242f, -6.826f);   // G23
    glVertex2f(24.759f, -6.826f);   // H23
    glEnd();
    glColor3f(0.161f, 0.224f, 0.122f); //ware house right color
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(28.242f, -5.337f);          // P23
    glVertex2f(24.759f, -5.337f);          // Q23
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(26.629f, -3.847f);  // M23
    glVertex2f(26.629f, -6.826f);  // O23
    glEnd();

}
void road268() { //Road
    glColor3ub(250, 250, 249); // Light road color

    glBegin(GL_POLYGON);
    glVertex2f(-30.0f, -15.7f);   // f
    glVertex2f(30.0f, -15.7f);   // h
    glVertex2f(30.0f, -16.45f);  // D
    glVertex2f(-30.0f, -16.45f);  // C
    glEnd();
    glColor3ub(157, 145, 145);
    glBegin(GL_POLYGON);
    glVertex2f(-30.0f, -16.45f);  // C
    glVertex2f(30.0f, -16.45f);  // D
    glVertex2f(30.0f, -20.0f);   // B
    glVertex2f(-30.0f, -20.0f);   // A
    glEnd();
}


void palace268() {//Palace - Modern mid-rise building (Scenario 2)
    // Main body - muted beige
    glColor3ub(190, 178, 165);
    glBegin(GL_POLYGON);
    glVertex2f(-10.0f, 7.4f);   // Top left
    glVertex2f(10.0f, 7.4f);    // Top right
    glVertex2f(10.0f, -15.7f);  // Bottom right (ground level)
    glVertex2f(-10.0f, -15.7f); // Bottom left
    glEnd();

    // Roof cap - gray-beige
    glColor3ub(170, 158, 145);
    glBegin(GL_POLYGON);
    glVertex2f(-10.2f, 8.2f);  // Top left overhang
    glVertex2f(10.2f, 8.2f);   // Top right overhang
    glVertex2f(10.2f, 7.4f);   // Bottom right
    glVertex2f(-10.2f, 7.4f);  // Bottom left
    glEnd();

    // Base plinth - light brown
    glColor3ub(170, 155, 140);
    glBegin(GL_POLYGON);
    glVertex2f(-10.3f, -13.0f); // Top left
    glVertex2f(10.3f, -13.0f);  // Top right
    glVertex2f(10.3f, -15.7f);  // Bottom right
    glVertex2f(-10.3f, -15.7f); // Bottom left
    glEnd();

    // Vertical side accents
    glColor3ub(180, 168, 155);
    // Left accent
    glBegin(GL_POLYGON);
    glVertex2f(-10.0f, 7.4f);
    glVertex2f(-9.5f, 7.4f);
    glVertex2f(-9.5f, -13.0f);
    glVertex2f(-10.0f, -13.0f);
    glEnd();
    // Right accent
    glBegin(GL_POLYGON);
    glVertex2f(9.5f, 7.4f);
    glVertex2f(10.0f, 7.4f);
    glVertex2f(10.0f, -13.0f);
    glVertex2f(9.5f, -13.0f);
    glEnd();

    // Horizontal facade band (mid-level)
    glColor3ub(175, 162, 150);
    glBegin(GL_POLYGON);
    glVertex2f(-10.0f, -3.0f);
    glVertex2f(10.0f, -3.0f);
    glVertex2f(10.0f, -3.5f);
    glVertex2f(-10.0f, -3.5f);
    glEnd();

    // === ENTRANCE ===
    // Door - brown
    glColor3ub(120, 100, 85);
    glBegin(GL_POLYGON);
    glVertex2f(-1.5f, -13.0f);  // Top left
    glVertex2f(1.5f, -13.0f);   // Top right
    glVertex2f(1.5f, -15.7f);   // Bottom right
    glVertex2f(-1.5f, -15.7f);  // Bottom left
    glEnd();

    // Door frame - darker
    glColor3ub(95, 80, 68);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-1.5f, -13.0f);
    glVertex2f(1.5f, -13.0f);
    glVertex2f(1.5f, -15.7f);
    glVertex2f(-1.5f, -15.7f);
    glEnd();

    // Canopy above door
    glColor3ub(160, 145, 130);
    glBegin(GL_POLYGON);
    glVertex2f(-2.0f, -12.5f);
    glVertex2f(2.0f, -12.5f);
    glVertex2f(2.0f, -13.0f);
    glVertex2f(-2.0f, -13.0f);
    glEnd();

    // Door handle - brass color
    glColor3ub(190, 175, 125);
    glBegin(GL_POLYGON);
    glVertex2f(0.8f, -14.0f);
    glVertex2f(1.2f, -14.0f);
    glVertex2f(1.2f, -14.5f);
    glVertex2f(0.8f, -14.5f);
    glEnd();

    // === WINDOWS (4 columns x 3 rows = 12 windows) ===
    float windowWidth = 1.8f;
    float windowHeight = 2.2f;
    float startX = -7.5f;
    float columnSpacing = 5.0f;
    float startY = 5.0f;
    float rowSpacing = 4.5f;

    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 4; col++) {
            float x = startX + col * columnSpacing;
            float y = startY - row * rowSpacing;

            // Window glass - light color
            if (isDay) glColor3ub(230, 235, 240);
            else       glColor3ub(255, 215, 120);   // warm lit windows at night
            glBegin(GL_POLYGON);
            glVertex2f(x, y);
            glVertex2f(x + windowWidth, y);
            glVertex2f(x + windowWidth, y - windowHeight);
            glVertex2f(x, y - windowHeight);
            glEnd();

            // Window frame - darker
            glColor3ub(160, 150, 140);
            glLineWidth(1.5f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(x, y);
            glVertex2f(x + windowWidth, y);
            glVertex2f(x + windowWidth, y - windowHeight);
            glVertex2f(x, y - windowHeight);
            glEnd();

            // Window cross-bars
            glBegin(GL_LINES);
            // Horizontal bar
            glVertex2f(x, y - windowHeight / 2.0f);
            glVertex2f(x + windowWidth, y - windowHeight / 2.0f);
            // Vertical bar
            glVertex2f(x + windowWidth / 2.0f, y);
            glVertex2f(x + windowWidth / 2.0f, y - windowHeight);
            glEnd();
        }
    }
}

void doggo() {
    glPushMatrix();
    glTranslatef(-2.0f, 0.0f, 0.0f);  // move dog slightly left

    glColor3f(0.0f, 0.0f, 0.0f);//black dog
    glBegin(GL_POLYGON);
    glVertex2f(4.118f, -13.724f); // I17
    glVertex2f(4.236f, -13.740f); // J17
    glVertex2f(4.279f, -13.732f); // N17
    glVertex2f(4.318f, -13.696f); // O17
    glVertex2f(4.4f, -13.7f);                         // P17
    glVertex2f(4.480f, -13.727f); // Q17
    glVertex2f(4.559f, -13.757f); // R17
    glVertex2f(4.594f, -13.783f);  // S17
    glVertex2f(4.630f, -13.819f); // T17
    glVertex2f(4.646f, -13.853f); // U17
    glVertex2f(4.658f, -13.959f); // V17
    glVertex2f(4.687f, -14.019f);     // W17
    glVertex2f(4.7f, -14.1f);                         // Z17
    glVertex2f(4.723f, -14.289f);   // A18
    glVertex2f(4.8f, -14.4f);                         // B18
    glVertex2f(5.110f, -14.830f); // C18
    glVertex2f(5.221f, -14.938f); // D18
    glVertex2f(5.290f, -15.013f); // E18
    glVertex2f(5.380f, -15.145f); // F18
    glVertex2f(5.471f, -15.318f); // G18
    glVertex2f(5.515f, -15.369f);  // P18
    glVertex2f(5.561f, -15.550f);  // Q18
    glVertex2f(5.587f, -15.663f);   // R18
    glVertex2f(5.6f, -15.8f);                         // S18
    glVertex2f(5.561f, -15.839f);  // T18
    glVertex2f(5.366f, -15.471f); // Q29
    glVertex2f(4.854f, -14.838f); // R29
    glVertex2f(4.418f, -14.359f); // J21
    glVertex2f(4.397f, -14.031f); // K21
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(5.366f, -15.471f); // Q29
    glVertex2f(5.561f, -15.550f);  // Q18
    glVertex2f(5.587f, -15.663f);   // R18
    glVertex2f(5.6f, -15.8f);                         // S18
    glVertex2f(5.561f, -15.839f);  // T18
    glVertex2f(5.038f, -15.769f); // T19

    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(5.561f, -15.839f);  // T18
    glVertex2f(5.574f, -15.883f); // L21
    glVertex2f(5.568f, -15.943f); // M21
    glVertex2f(5.556f, -15.987f); // N21
    glVertex2f(5.531f, -16.039f); // O21
    glVertex2f(5.503f, -16.074f); // U21
    glVertex2f(5.474f, -16.101f); // H22
    glVertex2f(5.441f, -16.120f); // G26
    glVertex2f(5.420f, -16.134f); // E28
    glVertex2f(5.382f, -16.143f); // F28
    glVertex2f(5.350f, -16.144f); // M19
    glVertex2f(4.590f, -16.140f); // N19
    glVertex2f(4.625f, -16.051f); // P19
    glVertex2f(4.675f, -16.008f);  // Q19
    glVertex2f(5.1f, -16.0f);                         // R19

    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(5.1f, -16.0f);                         // R19
    glVertex2f(5.052f, -15.939f); // S19
    glVertex2f(4.871f, -15.632f); // W19
    glVertex2f(4.757f, -15.544f); // Z19
    glVertex2f(4.677f, -15.490f); // A20
    glVertex2f(4.550f, -15.431f); // B20
    glVertex2f(4.314f, -15.192f); // C20
    glVertex2f(4.216f, -14.818f); // G20
    glVertex2f(4.241f, -14.600f); // I20
    glVertex2f(4.854f, -14.838f); // R29
    glVertex2f(5.366f, -15.471f); // Q29
    glEnd();//
    glBegin(GL_POLYGON);
    glVertex2f(4.241f, -14.600f); // I20
    glVertex2f(4.287f, -14.274f); // K20
    glVertex2f(4.723f, -14.289f);   // A18
    glVertex2f(4.848f, -14.722f);   // A18
    glEnd();//

    glBegin(GL_POLYGON);
    glVertex2f(4.397f, -14.031f); // K21
    glVertex2f(4.182f, -14.038f);   // N20
    glVertex2f(4.110f, -13.997f); // O20
    glVertex2f(4.062f, -13.955f); // P20
    glVertex2f(4.019f, -13.914f); // Q20
    glVertex2f(3.997f, -13.893f);  // R20
    glVertex2f(3.984f, -13.875f); // S20
    glVertex2f(3.976f, -13.856f); // H17
    glEnd();//
    // front legs
    glBegin(GL_POLYGON);
    glVertex2f(4.550f, -15.431f); // B20
    glVertex2f(4.323f, -16.132f);  // T20
    glVertex2f(4.106f, -16.127f);  // U20
    glVertex2f(4.122f, -16.096f); // V20 (2nd)
    glVertex2f(4.034f, -16.093f); // W20
    glVertex2f(4.0f, -16.1f);                         // Z20
    glVertex2f(3.997f, -16.067f); // A21
    glVertex2f(4.071f, -16.001f);  // B21
    glVertex2f(4.162f, -15.993f); // C21
    glVertex2f(4.239f, -15.818f); // D21
    glVertex2f(4.268f, -15.614f); // E21
    glVertex2f(4.299f, -15.358f); // F21
    glVertex2f(4.314f, -15.192f); // C20
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(4.110f, -13.997f); // O20
    glVertex2f(4.062f, -13.955f); // P20
    glVertex2f(4.019f, -13.914f); // Q20
    glVertex2f(3.997f, -13.893f);  // R20
    glVertex2f(3.984f, -13.875f); // S20                      // G17
    glVertex2f(3.976f, -13.856f); // H17
    glVertex2f(3.963f, -13.680f); //G17
    glVertex2f(4.118f, -13.724f); // I17
    glVertex2f(4.236f, -13.740f); // J17
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(4.118f, -13.724f); // I17
    glVertex2f(4.236f, -13.740f); // J17
    glVertex2f(4.397f, -14.031f); //k21
    glVertex2f(4.182f, -14.038f); // N20 (2nd)

    glEnd();

    // S6
    drawSemiCircleFromPoints(5.561f, -15.839f, 5.114f, -15.981f, 1);
    // t6
    drawSemiCircleFromPoints(5.052f, -15.939f, 5.038f, -15.769f, 1);
    // C7
    drawSemiCircleFromPoints(4.988f, -15.839f, 5.004f, -15.516f, 1);
    // d7
    drawSemiCircleFromPoints(4.314f, -15.192f, 4.848f, -14.722f, 1);
    // e7
    drawSemiCircleFromPoints(4.304f, -15.076f, 4.529f, -14.643f, 1);
    // f7
    drawSemiCircleFromPoints(4.216f, -14.818f, 4.663f, -14.648f, 1);
    // g7
    glColor3f(0.82f, 0.62f, 0.45f);
    drawSemiCircleFromPoints(3.974f, -14.003f, 4.116f, -14.469f, 1);
    //eyes
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glBegin(GL_POLYGON);
    glVertex2f(4.292f, -13.777f); // S29
    glVertex2f(4.336f, -13.800f);  // T29
    glVertex2f(4.368f, -13.778f); // U29
    glVertex2f(4.375f, -13.746f); // V29
    glVertex2f(4.360f, -13.720f); // W29
    glVertex2f(4.340f, -13.708f);   // Z29
    glEnd();

    glColor3f(0.0f, 0.0f, 0.0f);//black dog
    glBegin(GL_LINES);
    glVertex2f(5.6f, -15.8f);                         // S18
    glVertex2f(5.737f, -15.837f);  // A30
    glEnd();
    glBegin(GL_LINES);

    glVertex2f(5.737f, -15.837f);  // A30
    glVertex2f(5.8f, -15.9f);                         // B30
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(5.8f, -15.9f);                         // B30
    glVertex2f(5.841f, -16.013f); // C30
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(5.841f, -16.013f); // C30
    glVertex2f(5.902f, -16.147f); // D30
    glEnd();
    glBegin(GL_LINES);
    glVertex2f(5.902f, -16.147f); // D30
    glVertex2f(6.1f, -16.2f);
    glEnd();                       // E30
    glBegin(GL_LINES);
    glVertex2f(6.1f, -16.2f);                         // E30
    glVertex2f(6.246f, -16.176f);  // F30
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f); // white belt
    glBegin(GL_POLYGON);
    glVertex2f(4.287f, -14.274f); // K20
    glVertex2f(4.274f, -14.406f); // J20
    glVertex2f(4.7f, -14.1f);                         // Z17
    glVertex2f(4.723f, -14.289f);   // A18
    glEnd();

    glPopMatrix();
}
void suv268() { //Suv
    glColor3f(0.502f, 0.0f, 0.0f);//maroon truck
    glPushMatrix();
    glTranslatef(V, 0.0f, 0.0f);

    glBegin(GL_POLYGON);
    glVertex2f(19.351f, -14.873f);    // R21
    glVertex2f(19.351f, -13.188f);    // S21
    glVertex2f(17.308f, -13.132f);    // T21
    glVertex2f(16.681f, -13.764f);    // U21
    glVertex2f(16.221f, -14.517f);    // V21
    glVertex2f(13.251f, -15.140f);    // W21
    glVertex2f(12.732f, -15.672f);      // B3
    glVertex2f(12.930f, -16.825f);    // Z21=
    glVertex2f(24.395f, -16.825f);         // Q21
    glVertex2f(24.395f, -14.873f);         // P21
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(19.351f, -14.873f);    // R21
    glVertex2f(19.351f, -13.188f);    // S21
    glVertex2f(24.010f, -13.188f);  // L11
    glVertex2f(24.395f, -14.873f);         // P21
    glEnd();
    glColor3f(0.6f, 0.8f, 0.9f);   // Light sky-blue glass
    glBegin(GL_POLYGON);
    glVertex2f(18.753f, -14.551f);         // E22
    glVertex2f(18.753f, -13.441f);                // F22
    glVertex2f(17.430f, -13.441f);          // G22
    glVertex2f(16.862f, -14.551f);  // L22
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(19.464f, -13.441f);         // E22
    glVertex2f(23.717f, -13.441f);                // F22
    glVertex2f(24.072f, -14.551f);          // G22
    glVertex2f(19.464f, -14.551f);  // L22
    glEnd();
    glColor3f(0.0f, 0.0f, 0.0f); // black lines
    glLineWidth(1.0f);           // optional: line thickness
    glBegin(GL_LINE_LOOP);
    glVertex2f(16.709f, -16.451f);   // I22
    glVertex2f(19.060f, -16.451f);   // J22
    glVertex2f(19.060f, -14.707f);  // K22
    glVertex2f(16.709f, -14.707f);   // L22
    glEnd();
    glBegin(GL_LINE_LOOP);
    glVertex2f(16.709f, -14.707f);   // L22
    glVertex2f(16.709f, -14.432f);   // F2
    glVertex2f(17.362, -13.299);   // L2
    glVertex2f(19.060, -13.299);   // G5
    glVertex2f(19.060f, -14.707f);  // K22
    glEnd();
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(15.718f, -16.825f);   // B22
    glVertex2f(20.840f, -16.825f);   // C22
    glEnd();
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex2f(18.2f, -15.3f);    // I13
    glVertex2f(18.765f, -15.3f);  // J13
    glEnd();
    //semicircle for front wheel
    drawSemiCircleFromPoints(13.339f, -16.825f, 15.718f, -16.825f, 1);
    //for rare wheel
    drawSemiCircleFromPoints(20.840f, -16.825f, 23.189f, -16.825f, 1);

    // Rare wheel tyre (O22, P22)
    glColor3f(0.2f, 0.2f, 0.2f);  // 20% black (dark grey)
    drawCircle(22.017f, -16.847f, 0.965f);
    // front wheel tyre
    drawCircle(14.555f, -16.691f, 0.969f);
    // front wheel rim
    glColor3f(1.0f, 1.0f, 1.0f);  // Pure white
    drawCircle(14.555f, -16.691f, 0.591f);  // e2
    //rare while rim
    drawCircle(22.017f, -16.847f, 0.607f);  // f2


    glPopMatrix();
}


void treeS1268(float baseX, float baseY) { // tree code
    glPushMatrix();
    glTranslatef(baseX, baseY, 0.0f);
    glRotatef(rainOn ? treeSway * 0.7f : 0.0f, 0.0f, 0.0f, 1.0f);
    glTranslatef(-baseX, -baseY, 0.0f);

    glTranslatef(baseX + 11.180f, baseY + 15.066f, 0.0f);

    glColor3f(0.690f, 0.871f, 0.498f); // tree color (pata)

    glBegin(GL_TRIANGLES);
    glVertex2f(-11.172f, -9.233f);   //U18
    glVertex2f(-12.135f, -10.813f);  //V18
    glVertex2f(-10.326f, -10.813f); //W18
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-12.029f, -11.146f);
    glVertex2f(-11.163f, -10.122f);
    glVertex2f(-10.369f, -11.146f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-12.181f, -11.630f);
    glVertex2f(-11.196f, -10.675f);
    glVertex2f(-10.191f, -11.630f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-12.333f, -12.094f);
    glVertex2f(-11.196f, -10.675f);
    glVertex2f(-10.092f, -12.094f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-12.742f, -12.801f);
    glVertex2f(-11.145f, -11.212f);
    glVertex2f(-9.710f, -12.801f);
    glEnd();

    glBegin(GL_TRIANGLES);
    glVertex2f(-9.306f, -13.649f);
    glVertex2f(-13.109f, -13.649f);
    glVertex2f(-11.180f, -12.421f);
    glEnd();

    // Trunk color
    glColor3f(0.396f, 0.263f, 0.129f);
    glLineWidth(3.0f);

    glBegin(GL_LINES);
    glVertex2f(-11.180f, -15.066f); //I18
    glVertex2f(-11.180f, -11.467f); //J18
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-11.180f, -12.421f);
    glVertex2f(-11.852f, -11.952f);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-11.180f, -13.019f);
    glVertex2f(-10.516f, -12.599f);
    glEnd();

    glPopMatrix();
}
void drawAllTrees() {
    //Tree1  at R17
    treeS1268(-28.0f, -16.0f);
    // Tree2 at O18
    treeS1268(-14.055f, -16.0f);
    // Tree3 at I18 (original)
    treeS1268(-11.180f, -15.066f);
    // Tree4 at M19
    treeS1268(11.824f, -15.066f);
    // Tree5 at T19
    treeS1268(14.177f, -16.0f);

    //Tree6R17
    treeS1268(26.441f, -16.0f);
}
void lampPost(float baseX, float baseY) { // DAY Lamp post code
    glPushMatrix();
    // Translate so that new L10 = (baseX, baseY)
    glTranslatef(baseX + 24.310f, baseY + 15.875f, 0.0f);
    // White part (lamp light)
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_POLYGON);
    glVertex2f(-24.763f, -7.552f);     // R10
    glVertex2f(-24.763f, -6.6f);         // S10
    glVertex2f(-23.854f, -6.6f);         // T10
    glVertex2f(-23.854f, -7.552f);     // U10
    glVertex2f(-24.145f, -7.877f);    // P10
    glVertex2f(-24.459f, -7.877f);     // O10
    glEnd();
    // Lamp head (circle bulb)
    glColor3f(0.0f, 0.0f, 0.0f);
    drawCircle(-24.310f, -5.836f, 0.181f);
    // Lamp cap
    glBegin(GL_TRIANGLES);
    glVertex2f(-24.647f, -6.380f);     // V10
    glVertex2f(-23.916f, -6.380f);    // C11
    glVertex2f(-24.310f, -6.101f);    // D11
    glEnd();
    // Pole
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(-24.310f, -15.875f);    // L10
    glVertex2f(-24.310f, -8.169f);     // M10
    glEnd();

    // Extra details
    glBegin(GL_LINES);
    glVertex2f(-24.310f, -6.101f);    // D11
    glVertex2f(-24.310f, -7.969f);   // H11
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.872f, -15.663f);   // U7
    glVertex2f(-23.721f, -15.663f);     // V7
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-25.148f, -15.804f);  // S7
    glVertex2f(-23.533f, -15.804f);   // T7
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.034f, -15.549f);     // B8
    glVertex2f(-24.590f, -15.549f);   // W7
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.835f, -14.074f);    // C8
    glVertex2f(-23.780f, -14.074f);    // D8
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.720f, -13.813f);   // E8
    glVertex2f(-23.891f, -13.813f);    // K10
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.594f, -13.573f);     // Z7
    glVertex2f(-24.034f, -13.573f);      // A8
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.310f, -7.969f);   // H11
    glVertex2f(-24.763f, -7.552f);     // R10
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-23.854f, -7.552f);     // U10
    glVertex2f(-24.310f, -7.969f);   // H11
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.763f, -7.552f);     // R10
    glVertex2f(-24.763f, -6.6f);         // S10
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-23.854f, -6.6f);         // T10
    glVertex2f(-23.854f, -7.552f);     // U10
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.310f, -6.101f);    // D11
    glVertex2f(-24.310f, -5.947f);     // E11
    glEnd();

    // Base polygons
    glBegin(GL_POLYGON);
    glVertex2f(-24.594f, -15.568f);    // W7
    glVertex2f(-24.594f, -13.573f);     // Z7
    glVertex2f(-24.034f, -13.573f);      // A8
    glVertex2f(-24.034f, -15.549f);     // B8
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-24.459f, -8.150f);    // N10
    glVertex2f(-24.459f, -7.877f);     // O10
    glVertex2f(-24.145f, -7.877f);    // P10
    glVertex2f(-24.145f, -8.150f);   // Q10
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-24.763f, -6.6f);         // S10
    glVertex2f(-23.854f, -6.6f);         // T10
    glVertex2f(-23.916f, -6.380f);    // C11
    glVertex2f(-24.647f, -6.380f);     // V10
    glEnd();

    glPopMatrix();
}
void lampPostNight(float baseX, float baseY) { //  Night lamp post code
    glPushMatrix();
    // Translate so that new L10 = (baseX, baseY)
    // Original base L10 was (-24.310f, -15.875f)
    glTranslatef(baseX + 24.310f, baseY + 15.875f, 0.0f);
    // yellow part (lamp light)
    glColor3f(1.0f, 1.0f, 0.0f);
    glBegin(GL_POLYGON);
    glVertex2f(-24.763f, -7.552f);     // R10
    glVertex2f(-24.763f, -6.6f);         // S10
    glVertex2f(-23.854f, -6.6f);         // T10
    glVertex2f(-23.854f, -7.552f);     // U10
    glVertex2f(-24.145f, -7.877f);    // P10
    glVertex2f(-24.459f, -7.877f);     // O10
    glEnd();

    // Lamp head (circle bulb)
    glColor3f(0.0f, 0.0f, 0.0f);
    drawCircle(-24.310f, -5.836f, 0.181f);

    // Lamp cap
    glBegin(GL_TRIANGLES);
    glVertex2f(-24.647f, -6.380f);     // V10
    glVertex2f(-23.916f, -6.380f);    // C11
    glVertex2f(-24.310f, -6.101f);    // D11
    glEnd();

    // Pole
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);
    glVertex2f(-24.310f, -15.875f);    // L10
    glVertex2f(-24.310f, -8.169f);     // M10
    glEnd();

    // Extra details
    glBegin(GL_LINES);
    glVertex2f(-24.310f, -6.101f);    // D11
    glVertex2f(-24.310f, -7.969f);   // H11
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.872f, -15.663f);   // U7
    glVertex2f(-23.721f, -15.663f);     // V7
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-25.148f, -15.804f);  // S7
    glVertex2f(-23.533f, -15.804f);   // T7
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.034f, -15.549f);     // B8
    glVertex2f(-24.590f, -15.549f);   // W7
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.835f, -14.074f);    // C8
    glVertex2f(-23.780f, -14.074f);    // D8
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.720f, -13.813f);   // E8
    glVertex2f(-23.891f, -13.813f);    // K10
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.594f, -13.573f);     // Z7
    glVertex2f(-24.034f, -13.573f);      // A8
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.310f, -7.969f);   // H11
    glVertex2f(-24.763f, -7.552f);     // R10
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-23.854f, -7.552f);     // U10
    glVertex2f(-24.310f, -7.969f);   // H11
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.763f, -7.552f);     // R10
    glVertex2f(-24.763f, -6.6f);         // S10
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-23.854f, -6.6f);         // T10
    glVertex2f(-23.854f, -7.552f);     // U10
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(-24.310f, -6.101f);    // D11
    glVertex2f(-24.310f, -5.947f);     // E11
    glEnd();

    // Base polygons
    glBegin(GL_POLYGON);
    glVertex2f(-24.594f, -15.568f);    // W7
    glVertex2f(-24.594f, -13.573f);     // Z7
    glVertex2f(-24.034f, -13.573f);      // A8
    glVertex2f(-24.034f, -15.549f);     // B8
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-24.459f, -8.150f);    // N10
    glVertex2f(-24.459f, -7.877f);     // O10
    glVertex2f(-24.145f, -7.877f);    // P10
    glVertex2f(-24.145f, -8.150f);   // Q10
    glEnd();

    glBegin(GL_POLYGON);
    glVertex2f(-24.763f, -6.6f);         // S10
    glVertex2f(-23.854f, -6.6f);         // T10
    glVertex2f(-23.916f, -6.380f);    // C11
    glVertex2f(-24.647f, -6.380f);     // V10
    glEnd();

    glPopMatrix();
}
void drawAllLampPosts() {
    //Lamp1
    lampPost(-24.310f, -15.875f);
    //Lamp2
    lampPost(-7.624f, -15.689f);
    //Lamp3
    lampPost(7.126f, -15.760f);
    //Lamp4
    lampPost(22.444f, -15.760f);
}
void drawNightAllLampPosts() {
    //Lamp1
    lampPostNight(-24.310f, -15.875f);
    //Lamp2
    lampPostNight(-7.624f, -15.689f);
    //Lamp3
    lampPostNight(7.126f, -15.760f);
    //Lamp4
    lampPostNight(22.444f, -15.760f);
}

void bus268() { //Bus
    glPushMatrix();
    glTranslatef(W, 0.0f, 0.0f);
    glColor3f(0.133f, 0.545f, 0.133f);   // Forest Green

    glBegin(GL_POLYGON);
    glVertex2f(-25.919f, -13.254f);       // Q9
    glVertex2f(-25.919f, -17.919f);   // O9
    glVertex2f(-11.643f, -17.919f);   // P9
    glVertex2f(-11.643f, -16.0f);         // T9
    glVertex2f(-12.013f, -13.254f);// R9
    glEnd();

    glColor3f(0.6f, 0.8f, 0.9f);   // Light sky-blue glass
    glBegin(GL_POLYGON);
    glVertex2f(-25.332f, -13.592f);      // C10
    glVertex2f(-25.332f, -15.567f);      // D10
    glVertex2f(-11.744f, -15.567f); // E10
    glVertex2f(-12.001f, -13.592f); // F10

    glEnd();
    glColor3f(0.6f, 0.6f, 0.6f);

    glLineWidth(4.0f);     // optional: line thickness
    glBegin(GL_LINE_LOOP);
    glVertex2f(-21.695f, -13.195f);     // G10
    glVertex2f(-13.588f, -13.195f);   // H10
    glEnd();

    for (int i = 0; i < 4; i++) {
        float dx = i * 2.5f;
        glColor3f(0.3f, 0.3f, 0.3f);// ashe color
        glLineWidth(1.0f);           // optional: line thickness
        glBegin(GL_LINE_LOOP);
        glVertex2f(-22.783f + dx, -13.592f);   // I10
        glVertex2f(-22.783f + dx, -15.567f);   // J10
        glEnd();
    }
    // Rare wheel tyre (, )
    glColor3f(0.2f, 0.2f, 0.2f);  // 20% black (dark grey)
    drawCircle(-22.618f, -17.974f, 0.879f);
    // front wheel tyre
    drawCircle(-13.613f, -17.974f, 0.879f);
    // front wheel rim
    glColor3f(1.0f, 1.0f, 1.0f);  // Pure white
    // Circle h1 (center U9, through B10)
    drawCircle(-13.613f, -17.974f, 0.506f);
    //rare while rim
    // Circle g1 (center V9, through A10)
    drawCircle(-22.618f, -17.974f, 0.506f);

    glPopMatrix();
}


//.................................end Scenario 2..............


void drawTrafficLightS2() {
    // stop line on road
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex2f(-2.2f, -16.45f);
    glVertex2f( 2.2f, -16.45f);
    glVertex2f( 2.2f, -16.8f);
    glVertex2f(-2.2f, -16.8f);
    glEnd();

    // pole
    glColor3f(0.08f, 0.08f, 0.08f);
    glBegin(GL_QUADS);
    glVertex2f(-0.25f, -15.7f);
    glVertex2f( 0.25f, -15.7f);
    glVertex2f( 0.25f, -8.5f);
    glVertex2f(-0.25f, -8.5f);
    glEnd();

    // signal box
    glColor3f(0.12f, 0.12f, 0.12f);
    glBegin(GL_QUADS);
    glVertex2f(-1.1f, -8.5f);
    glVertex2f( 1.1f, -8.5f);
    glVertex2f( 1.1f, -3.7f);
    glVertex2f(-1.1f, -3.7f);
    glEnd();

    // red light
    if (trafficLightGreen) glColor3f(0.25f, 0.0f, 0.0f);
    else                   glColor3f(1.0f, 0.0f, 0.0f);
    drawCircle(0.0f, -5.2f, 0.65f);

    // green light
    if (trafficLightGreen) glColor3f(0.0f, 1.0f, 0.0f);
    else                   glColor3f(0.0f, 0.25f, 0.0f);
    drawCircle(0.0f, -7.0f, 0.65f);
}

///.................................................Display function...............//scenario1...........................
void drawSmallCar1(float x, float y) {  // Red car
    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    // Body
    glColor3ub(200, 30, 30);
    glBegin(GL_POLYGON);
    glVertex2f(-3.0f, -17.0f);
    glVertex2f(-3.0f, -15.5f);
    glVertex2f(-1.5f, -14.0f);
    glVertex2f(1.5f, -14.0f);
    glVertex2f(3.0f, -15.5f);
    glVertex2f(3.0f, -17.0f);
    glEnd();

    // Roof
    glColor3ub(170, 20, 20);
    glBegin(GL_POLYGON);
    glVertex2f(-1.5f, -15.5f);
    glVertex2f(-1.0f, -14.0f);
    glVertex2f(1.0f, -14.0f);
    glVertex2f(1.5f, -15.5f);
    glEnd();

    // Windows
    glColor3ub(180, 220, 255);
    glBegin(GL_POLYGON);
    glVertex2f(-1.2f, -15.4f);
    glVertex2f(-0.8f, -14.3f);
    glVertex2f(0.8f, -14.3f);
    glVertex2f(1.2f, -15.4f);
    glEnd();

    // Wheels
    glColor3ub(30, 30, 30);
    drawCircle(-1.8f, -17.2f, 0.7f);
    drawCircle(1.8f, -17.2f, 0.7f);

    // Wheel rims
    glColor3ub(200, 200, 200);
    drawCircle(-1.8f, -17.2f, 0.35f);
    drawCircle(1.8f, -17.2f, 0.35f);

    glPopMatrix();
}

void drawSmallCar2(float x, float y) {  // Yellow car
    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    // Body
    glColor3ub(230, 200, 30);
    glBegin(GL_POLYGON);
    glVertex2f(-3.0f, -17.0f);
    glVertex2f(-3.0f, -15.5f);
    glVertex2f(-1.5f, -14.0f);
    glVertex2f(1.5f, -14.0f);
    glVertex2f(3.0f, -15.5f);
    glVertex2f(3.0f, -17.0f);
    glEnd();

    // Roof
    glColor3ub(200, 170, 20);
    glBegin(GL_POLYGON);
    glVertex2f(-1.5f, -15.5f);
    glVertex2f(-1.0f, -14.0f);
    glVertex2f(1.0f, -14.0f);
    glVertex2f(1.5f, -15.5f);
    glEnd();

    // Windows
    glColor3ub(180, 220, 255);
    glBegin(GL_POLYGON);
    glVertex2f(-1.2f, -15.4f);
    glVertex2f(-0.8f, -14.3f);
    glVertex2f(0.8f, -14.3f);
    glVertex2f(1.2f, -15.4f);
    glEnd();

    // Wheels
    glColor3ub(30, 30, 30);
    drawCircle(-1.8f, -17.2f, 0.7f);
    drawCircle(1.8f, -17.2f, 0.7f);

    // Wheel rims
    glColor3ub(200, 200, 200);
    drawCircle(-1.8f, -17.2f, 0.35f);
    drawCircle(1.8f, -17.2f, 0.35f);

    glPopMatrix();
}

void drawSmallCar3(float x, float y) {  // Blue car
    glPushMatrix();
    glTranslatef(x, y, 0.0f);

    // Body
    glColor3ub(30, 80, 200);
    glBegin(GL_POLYGON);
    glVertex2f(-3.0f, -17.0f);
    glVertex2f(-3.0f, -15.5f);
    glVertex2f(-1.5f, -14.0f);
    glVertex2f(1.5f, -14.0f);
    glVertex2f(3.0f, -15.5f);
    glVertex2f(3.0f, -17.0f);
    glEnd();

    // Roof
    glColor3ub(20, 60, 170);
    glBegin(GL_POLYGON);
    glVertex2f(-1.5f, -15.5f);
    glVertex2f(-1.0f, -14.0f);
    glVertex2f(1.0f, -14.0f);
    glVertex2f(1.5f, -15.5f);
    glEnd();

    // Windows
    glColor3ub(180, 220, 255);
    glBegin(GL_POLYGON);
    glVertex2f(-1.2f, -15.4f);
    glVertex2f(-0.8f, -14.3f);
    glVertex2f(0.8f, -14.3f);
    glVertex2f(1.2f, -15.4f);
    glEnd();

    // Wheels
    glColor3ub(30, 30, 30);
    drawCircle(-1.8f, -17.2f, 0.7f);
    drawCircle(1.8f, -17.2f, 0.7f);

    // Wheel rims
    glColor3ub(200, 200, 200);
    drawCircle(-1.8f, -17.2f, 0.35f);
    drawCircle(1.8f, -17.2f, 0.35f);

    glPopMatrix();
}
void updateCar1S1(int value) {
    if (currentScenario == 1 && !vehiclePaused) {
        car1X += car1Speed;
        if (car1X < -65.0f) car1X = 35.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(40, updateCar1S1, 0);
}

void updateCar2S1(int value) {
    if (currentScenario == 1 && !vehiclePaused) {
        car2X += car2Speed;
        if (car2X > 65.0f) car2X = -35.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(40, updateCar2S1, 0);
}

void updateCar3S1(int value) {
    if (currentScenario == 1 && !vehiclePaused) {
        car3X += car3Speed;
        if (car3X < -65.0f) car3X = 35.0f;
    }
    glutPostRedisplay();
    glutTimerFunc(40, updateCar3S1, 0);
}
// -- NEW BUILDING 1 - Scenario 1 (left gap, x: -19.5 to -12.5) --
void drawBuildingShade(float x, float y, float w, float h) {
    // right side shadow
    glColor4f(0.0f, 0.0f, 0.0f, 0.25f);
    glBegin(GL_POLYGON);
    glVertex2f(x + w,        y);
    glVertex2f(x + w + 1.5f, y - 1.5f);
    glVertex2f(x + w + 1.5f, y - h - 1.5f);
    glVertex2f(x + w,        y - h);
    glEnd();

    // bottom shadow
    glBegin(GL_POLYGON);
    glVertex2f(x,            y - h);
    glVertex2f(x + w,        y - h);
    glVertex2f(x + w + 1.5f, y - h - 1.5f);
    glVertex2f(x,            y - h - 1.5f);
    glEnd();
}
void newBuilding1S1() {
    // Main body
    glColor3ub(148, 132, 110);
    glBegin(GL_POLYGON);
    glVertex2f(-19.5f,  6.0f);
    glVertex2f(-12.5f,  6.0f);
    glVertex2f(-12.5f, -15.7f);
    glVertex2f(-19.5f, -15.7f);
    glEnd();

    // Rooftop band
    glColor3ub(120, 106, 88);
    glBegin(GL_POLYGON);
    glVertex2f(-19.8f, 6.0f);
    glVertex2f(-12.2f, 6.0f);
    glVertex2f(-12.2f, 6.5f);
    glVertex2f(-19.8f, 6.5f);
    glEnd();

    // Small tower
    glColor3ub(160, 144, 120);
    glBegin(GL_POLYGON);
    glVertex2f(-18.8f, 6.5f);
    glVertex2f(-16.8f, 6.5f);
    glVertex2f(-16.8f, 9.0f);
    glVertex2f(-18.8f, 9.0f);
    glEnd();

    // Tower roof band
    glColor3ub(100, 88, 72);
    glBegin(GL_POLYGON);
    glVertex2f(-19.0f, 9.0f);
    glVertex2f(-16.6f, 9.0f);
    glVertex2f(-16.6f, 9.4f);
    glVertex2f(-19.0f, 9.4f);
    glEnd();

    // Facade bands
    glColor3ub(135, 120, 100);
    glBegin(GL_POLYGON);
    glVertex2f(-19.5f, 1.5f);
    glVertex2f(-12.5f, 1.5f);
    glVertex2f(-12.5f, 1.3f);
    glVertex2f(-19.5f, 1.3f);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(-19.5f, -3.5f);
    glVertex2f(-12.5f, -3.5f);
    glVertex2f(-12.5f, -3.7f);
    glVertex2f(-19.5f, -3.7f);
    glEnd();

    // Door
    glColor3ub(80, 60, 40);
    glBegin(GL_POLYGON);
    glVertex2f(-16.8f,  -8.5f);
    glVertex2f(-15.2f,  -8.5f);
    glVertex2f(-15.2f, -15.7f);
    glVertex2f(-16.8f, -15.7f);
    glEnd();

    // Door handle
    glColor3ub(180, 160, 100);
    glBegin(GL_POLYGON);
    glVertex2f(-15.6f, -11.0f);
    glVertex2f(-15.4f, -11.0f);
    glVertex2f(-15.4f, -11.6f);
    glVertex2f(-15.6f, -11.6f);
    glEnd();

    // Windows (2 columns x 5 rows)
    float wx[2] = {-18.8f, -14.8f};
    float wy[5] = {4.5f, 2.5f, 0.0f, -2.0f, -5.5f};
    float ww = 1.4f, wh = 1.8f;
    for (int col = 0; col < 2; col++) {
        for (int row = 0; row < 5; row++) {
            glColor3ub(185, 220, 240);
            glBegin(GL_POLYGON);
            glVertex2f(wx[col],      wy[row]);
            glVertex2f(wx[col]+ww,   wy[row]);
            glVertex2f(wx[col]+ww,   wy[row]-wh);
            glVertex2f(wx[col],      wy[row]-wh);
            glEnd();
            glColor3ub(100, 85, 65);
            glLineWidth(1.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(wx[col],      wy[row]);
            glVertex2f(wx[col]+ww,   wy[row]);
            glVertex2f(wx[col]+ww,   wy[row]-wh);
            glVertex2f(wx[col],      wy[row]-wh);
            glEnd();
        }
    }

    // Tower window
    glColor3ub(185, 220, 240);
    glBegin(GL_POLYGON);
    glVertex2f(-18.5f, 8.5f);
    glVertex2f(-17.5f, 8.5f);
    glVertex2f(-17.5f, 7.2f);
    glVertex2f(-18.5f, 7.2f);
    glEnd();
    glColor3ub(100, 85, 65);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-18.5f, 8.5f);
    glVertex2f(-17.5f, 8.5f);
    glVertex2f(-17.5f, 7.2f);
    glVertex2f(-18.5f, 7.2f);
    glEnd();
}

// -- NEW BUILDING 2 - Scenario 1 (right gap, x: 12.5 to 19.5) --
void newBuilding2S1() {
    // Main body
    glColor3ub(168, 148, 110);
    glBegin(GL_POLYGON);
    glVertex2f(12.5f,  5.5f);
    glVertex2f(19.5f,  5.5f);
    glVertex2f(19.5f, -15.7f);
    glVertex2f(12.5f, -15.7f);
    glEnd();

    // Rooftop band
    glColor3ub(130, 115, 85);
    glBegin(GL_POLYGON);
    glVertex2f(12.2f, 5.5f);
    glVertex2f(19.8f, 5.5f);
    glVertex2f(19.8f, 6.0f);
    glVertex2f(12.2f, 6.0f);
    glEnd();

    // Small tower
    glColor3ub(155, 138, 105);
    glBegin(GL_POLYGON);
    glVertex2f(16.5f, 6.0f);
    glVertex2f(19.0f, 6.0f);
    glVertex2f(19.0f, 8.8f);
    glVertex2f(16.5f, 8.8f);
    glEnd();

    // Tower roof band
    glColor3ub(105, 93, 70);
    glBegin(GL_POLYGON);
    glVertex2f(16.3f, 8.8f);
    glVertex2f(19.2f, 8.8f);
    glVertex2f(19.2f, 9.2f);
    glVertex2f(16.3f, 9.2f);
    glEnd();

    // Facade bands
    glColor3ub(145, 128, 96);
    glBegin(GL_POLYGON);
    glVertex2f(12.5f, 1.0f);
    glVertex2f(19.5f, 1.0f);
    glVertex2f(19.5f, 0.8f);
    glVertex2f(12.5f, 0.8f);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex2f(12.5f, -4.0f);
    glVertex2f(19.5f, -4.0f);
    glVertex2f(19.5f, -4.2f);
    glVertex2f(12.5f, -4.2f);
    glEnd();

    // Door
    glColor3ub(80, 60, 40);
    glBegin(GL_POLYGON);
    glVertex2f(14.8f,  -8.5f);
    glVertex2f(16.4f,  -8.5f);
    glVertex2f(16.4f, -15.7f);
    glVertex2f(14.8f, -15.7f);
    glEnd();

    // Door handle
    glColor3ub(180, 160, 100);
    glBegin(GL_POLYGON);
    glVertex2f(16.0f, -11.0f);
    glVertex2f(16.2f, -11.0f);
    glVertex2f(16.2f, -11.6f);
    glVertex2f(16.0f, -11.6f);
    glEnd();

    // Windows (2 columns x 5 rows)
    float wx[2] = {13.2f, 17.2f};
    float wy[5] = {4.2f, 2.2f, -0.2f, -2.2f, -5.5f};
    float ww = 1.4f, wh = 1.8f;
    for (int col = 0; col < 2; col++) {
        for (int row = 0; row < 5; row++) {
            glColor3ub(185, 220, 240);
            glBegin(GL_POLYGON);
            glVertex2f(wx[col],    wy[row]);
            glVertex2f(wx[col]+ww, wy[row]);
            glVertex2f(wx[col]+ww, wy[row]-wh);
            glVertex2f(wx[col],    wy[row]-wh);
            glEnd();
            glColor3ub(105, 92, 68);
            glLineWidth(1.0f);
            glBegin(GL_LINE_LOOP);
            glVertex2f(wx[col],    wy[row]);
            glVertex2f(wx[col]+ww, wy[row]);
            glVertex2f(wx[col]+ww, wy[row]-wh);
            glVertex2f(wx[col],    wy[row]-wh);
            glEnd();
        }
    }

    // Tower window
    glColor3ub(185, 220, 240);
    glBegin(GL_POLYGON);
    glVertex2f(17.0f, 8.3f);
    glVertex2f(18.6f, 8.3f);
    glVertex2f(18.6f, 7.0f);
    glVertex2f(17.0f, 7.0f);
    glEnd();
    glColor3ub(105, 92, 68);
    glBegin(GL_LINE_LOOP);
    glVertex2f(17.0f, 8.3f);
    glVertex2f(18.6f, 8.3f);
    glVertex2f(18.6f, 7.0f);
    glVertex2f(17.0f, 7.0f);
    glEnd();
}

// -- NEW BUILDING 1 - Scenario 2 (left gap, x: -14.2 to -10.5) --
void newBuilding1S2() {
    // Main body
    glColor3f(0.18f, 0.22f, 0.30f);
    glBegin(GL_POLYGON);
    glVertex2f(-14.2f,  5.0f);
    glVertex2f(-10.5f,  5.0f);
    glVertex2f(-10.5f, -15.758f);
    glVertex2f(-14.2f, -15.758f);
    glEnd();

    // Rooftop
    glColor3f(0.13f, 0.17f, 0.24f);
    glBegin(GL_POLYGON);
    glVertex2f(-14.4f, 5.0f);
    glVertex2f(-10.3f, 5.0f);
    glVertex2f(-10.3f, 5.5f);
    glVertex2f(-14.4f, 5.5f);
    glEnd();

    // Facade band
    glColor3f(0.14f, 0.18f, 0.25f);
    glBegin(GL_POLYGON);
    glVertex2f(-14.2f, -1.5f);
    glVertex2f(-10.5f, -1.5f);
    glVertex2f(-10.5f, -1.7f);
    glVertex2f(-14.2f, -1.7f);
    glEnd();

    // Windows (2 columns x 5 rows)
    float wx[2] = {-13.8f, -12.2f};
    float wy[5] = {4.2f, 2.0f, -0.5f, -3.0f, -7.0f};
    for (int col = 0; col < 2; col++) {
        for (int row = 0; row < 5; row++) {
            if (isDay) glColor3f(0.13f, 0.18f, 0.39f);
            else       glColor3f(1.0f, 0.82f, 0.35f);   // glowing windows at night
            glBegin(GL_POLYGON);
            glVertex2f(wx[col],      wy[row]);
            glVertex2f(wx[col]+1.0f, wy[row]);
            glVertex2f(wx[col]+1.0f, wy[row]-1.6f);
            glVertex2f(wx[col],      wy[row]-1.6f);
            glEnd();
        }
    }

    // Door with arch
    glColor3f(0.10f, 0.12f, 0.18f);
    glBegin(GL_POLYGON);
    glVertex2f(-13.0f, -10.5f);
    glVertex2f(-11.8f, -10.5f);
    glVertex2f(-11.8f, -15.758f);
    glVertex2f(-13.0f, -15.758f);
    glEnd();
    drawSemiCircleFromPoints(-13.0f, -10.5f, -11.8f, -10.5f, 1);
}

// -- NEW BUILDING 2 - Scenario 2 (right gap, x: 10.5 to 16.8) --
void newBuilding2S2() {
    // Main body
    glColor3f(0.20f, 0.30f, 0.18f);
    glBegin(GL_POLYGON);
    glVertex2f(10.5f,  6.5f);
    glVertex2f(16.8f,  6.5f);
    glVertex2f(16.8f, -15.672f);
    glVertex2f(10.5f, -15.672f);
    glEnd();

    // Rooftop band
    glColor3f(0.16f, 0.24f, 0.14f);
    glBegin(GL_POLYGON);
    glVertex2f(10.3f, 6.5f);
    glVertex2f(17.0f, 6.5f);
    glVertex2f(17.0f, 7.0f);
    glVertex2f(10.3f, 7.0f);
    glEnd();

    // Facade band
    glColor3f(0.17f, 0.27f, 0.15f);
    glBegin(GL_POLYGON);
    glVertex2f(10.5f, -0.5f);
    glVertex2f(16.8f, -0.5f);
    glVertex2f(16.8f, -0.7f);
    glVertex2f(10.5f, -0.7f);
    glEnd();

    // Windows (2 columns x 4 rows)
    float wx[2] = {11.0f, 14.2f};
    float wy[4] = {5.8f, 3.0f, 0.5f, -2.5f};
    for (int col = 0; col < 2; col++) {
        for (int row = 0; row < 4; row++) {
            if (isDay) glColor3f(1.0f, 1.0f, 1.0f);
            else       glColor3f(1.0f, 0.82f, 0.35f);   // glowing windows at night
            glBegin(GL_POLYGON);
            glVertex2f(wx[col],      wy[row]);
            glVertex2f(wx[col]+1.8f, wy[row]);
            glVertex2f(wx[col]+1.8f, wy[row]-2.0f);
            glVertex2f(wx[col],      wy[row]-2.0f);
            glEnd();
            glColor3f(0.16f, 0.25f, 0.14f);
            glLineWidth(1.5f);
            glBegin(GL_LINES);
            glVertex2f(wx[col],      wy[row]-1.0f);
            glVertex2f(wx[col]+1.8f, wy[row]-1.0f);
            glVertex2f(wx[col]+0.9f, wy[row]);
            glVertex2f(wx[col]+0.9f, wy[row]-2.0f);
            glEnd();
            glLineWidth(1.0f);
        }
    }

    // Large lower window
    if (isDay) glColor3f(1.0f, 1.0f, 1.0f);
    else       glColor3f(1.0f, 0.82f, 0.35f);   // glowing lower window at night
    glBegin(GL_POLYGON);
    glVertex2f(11.0f, -5.5f);
    glVertex2f(16.3f, -5.5f);
    glVertex2f(16.3f, -8.5f);
    glVertex2f(11.0f, -8.5f);
    glEnd();
    glColor3f(0.16f, 0.25f, 0.14f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(13.65f, -5.5f);
    glVertex2f(13.65f, -8.5f);
    glVertex2f(11.0f,  -7.0f);
    glVertex2f(16.3f,  -7.0f);
    glEnd();
    glLineWidth(1.0f);
}

void drawScenario1(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    //Sky different color for winter
    if (winterMode) {
        if (isDay) glClearColor(0.8f, 0.85f, 0.9f, 1.0f);  // Light grayish-blue for winter day
        else       glClearColor(0.05f, 0.05f, 0.15f, 1.0f);  // Darker blue for winter night
    }
    else {
        if (isDay) glClearColor(0.859f, 0.945f, 0.976f, 1.0f); // light blue
        else       glClearColor(0.0f, 0.0f, 0.2f, 1.0f); // dark navy
    }

    glClear(GL_COLOR_BUFFER_BIT);

    // Sun or Moon
    if (isDay) drawSun(6.0f, 20.0f, 3.0f);   //Sunmoon transformation
    else {
        drawMoon(6.0f, 20.0f, 3.0f);
        drawStars();
    }


    //Sky();
    //drawSun(0.0f,0.0f,0.0f);
    drawCloud();
    road();
    house2();
    drawBuildingShade(-29.3f, 8.6f, 9.3f, 24.3f);   // shade for house2

    newBuilding1S1();
    drawBuildingShade(-19.5f, 6.5f, 7.0f, 22.2f);   // shade for newBuilding1S1

    palace();
    drawBuildingShade(-12.0f, 11.0f, 24.0f, 26.7f); // shade for palace

    newBuilding2S1();
    drawBuildingShade(12.5f, 6.0f, 7.0f, 21.7f);    // shade for newBuilding2S1

    house1();
    drawBuildingShade(20.3f, 8.2f, 9.7f, 23.9f);    // shade for house1






    tree1();
    tree2();
    tree2_translated();

    // Draw snow on trees if winter
    drawSnowOnTrees();

    airbln();
    drawBench(-15.0f, -14.20f, 3.0f);
    drawBench(23.0f, -14.20f, 3.0f);

    // Draws the snow on ground if winter
    drawSnowOnGround();
    drawSnowOnBenchesAndLampsScenario1();


    if (!isDay) {
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);

        glBegin(GL_QUADS);
        glVertex2f(-30.0f, -20.0f);
        glVertex2f(30.0f, -20.0f);
        glVertex2f(30.0f, 26.774f);
        glVertex2f(-30.0f, 26.774f);
        glEnd();
    }

    if (isDay) {   //Lamppost transformation

        NdrawLampPost(-18.0f, -11.90f, 6.7f);
        NdrawLampPost(-28.0f, -11.90f, 6.7f);
        NdrawLampPost(-8.0f, -11.90f, 6.7f);
        NdrawLampPost(8.0f, -11.90f, 6.7f);
        NdrawLampPost(18.0f, -11.90f, 6.7f);
        NdrawLampPost(28.0f, -11.90f, 6.7f);
    }

    else {

        drawLampPost(-18.0f, -11.90f, 6.7f);
        drawLampPost(-28.0f, -11.90f, 6.7f);
        drawLampPost(-8.0f, -11.90f, 6.7f);
        drawLampPost(8.0f, -11.90f, 6.7f);
        drawLampPost(18.0f, -11.90f, 6.7f);
        drawLampPost(28.0f, -11.90f, 6.7f);
    }
    if (isDay) drawSun(6.0f, 20.0f, 3.0f);
    else {
        drawMoon(6.0f, 20.0f, 3.0f);

    }

    // Draw snow on roofs if winter mode
    drawSnowOnRoofs();
    // inside drawScenario1(), after road();
    drawSmallCar1(car1X, 0.0f);
    drawSmallCar2(car2X, 0.0f);
    drawSmallCar3(car3X, 0.0f);

    truck();
    bus();
    drawRain();
    drawLightning();
    drawSnow();

    glFlush();
}


///.................................................Display function...............//scenario2...........................
void drawScenario2(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    ///Stranzition01 day-night....................................................................................................
    if (isDay) glClearColor(0.859f, 0.945f, 0.976f, 1.0f); // light blue
    else       glClearColor(0.0f, 0.0f, 0.2f, 1.0f); // dark navy

    glClear(GL_COLOR_BUFFER_BIT);

    ///Stranzition02  Sun-Moon.....................................................................................................
    if (isDay) sun(6.0f, 20.0f, 3.0f);
    else {
        moon(6.0f, 20.0f, 3.0f);

    }
    ///Stranzition03 Lamp post ...................................................................................................
    if (isDay)
        drawAllLampPosts();
    else {
        drawNightAllLampPosts();

    }
    planeS268();
    cloudS268();
    Factory268();
    drawBuildingShade(-30.0f, -4.2f, 15.4f, 11.5f); // shade for factory

    newBuilding1S2();
    drawBuildingShade(-14.2f, 5.5f, 3.7f, 21.3f);   // shade for newBuilding1S2

    road268();
    palace268();
    drawBuildingShade(-10.0f, 8.2f, 20.0f, 23.9f);  // shade for palace268

    newBuilding2S2();
    drawBuildingShade(10.5f, 7.0f, 6.3f, 22.7f);    // shade for newBuilding2S2

    wareHouse();
    drawBuildingShade(17.0f, 4.2f, 12.6f, 19.9f);   // shade for warehouse

    // Draw after buildings so the traffic light stays visible in front
    drawTrafficLightS2();




    if (isDay) {
        doggo();   // dog visible only during daytime
    }
    drawAllTrees();
    if (isDay) {

        drawAllLampPosts();
    }
    else {

        drawNightAllLampPosts();

    }
    suv268();
    bus268();

    // Draw snow overlays for winter BEFORE vehicles for proper layering
    if (winterMode) {
        // Ground snow sits on road surface
        drawSnowOnGround();
        // Roof and tree accumulations on buildings/trees
        drawSnowOnRoofsScenario2();
        drawSnowOnTreesScenario2();
        // Lamp post caps
        drawSnowOnLampPostsScenario2();
    }

    // Vehicles drawn after snow to appear above ground snow
    suv268();
    bus268();

    // Draw rain particles
    drawRain();
    drawLightning();

    // Draw snow particles
    drawSnow();

    glFlush();

}



///......................................main Display function

void display() {
    if (currentScenario == 1) {
        drawScenario1();
    }

    else if (currentScenario == 2) {
        drawScenario2();
    }
    glFlush();

}
//...........................................

// Input handling

//.....Scenario 1......
void truckKeys(int key, int x, int y) {
    if (currentScenario == 1) {
        switch (key) {
        case GLUT_KEY_UP:
            truckSpeed -= 1.0f;
            break;
        case GLUT_KEY_DOWN:
            truckSpeed += 0.1f;
            break;
        case GLUT_KEY_RIGHT:
            truckSpeed = -2.0f;
            break;
        case GLUT_KEY_LEFT:
            truckSpeed = 0.0f;
            break;

        }
        glutPostRedisplay();
    }
}

void Keys(unsigned char key, int x, int y) {
    // -- Global controls (all scenarios) --
    if (key == 'v' || key == 'V') {
        vehiclePaused = !vehiclePaused;    // pause / play all vehicles
        glutPostRedisplay();
    }
    // Rain controls - works in ALL scenarios
    if (key == 'R') {
        rainOn = true;  // Rain ON
        glutPostRedisplay();
        return;
    }
    if (key == 'S') {
        rainOn = false;  // Rain OFF
        glutPostRedisplay();
        return;
    }
    // Snow controls - works in ALL scenarios
    if (key == 'W') {
        snowOn = true;  // Snow ON
        glutPostRedisplay();
        return;
    }
    if (key == 'E') {
        snowOn = false;  // Snow OFF
        winterMode = false;  // Remove snow layers from ground/trees/lamp posts too
        glutPostRedisplay();
        return;
    }
    // Truck / SUV speed
    if (key == 't' && currentScenario == 1) {
        truckSpeed -= 0.5f;                // truck faster
        glutPostRedisplay();
    }
    if (key == 'y' && currentScenario == 1) {
        truckSpeed += 0.5f;                // truck slower
        glutPostRedisplay();
    }
    // Bus speed Scenario 1
    if (key == 'u' && currentScenario == 1) {
        BusSpeed += 0.5f;                  // bus faster
        glutPostRedisplay();
    }
    if (key == 'i' && currentScenario == 1) {
        BusSpeed -= 0.5f;                  // bus slower
        glutPostRedisplay();
    }
    // Small cars Scenario 1
    if (key == 'z' && currentScenario == 1) {
        car1Speed -= 0.2f;                 // red car faster
        car2Speed += 0.2f;                 // yellow car faster
        car3Speed -= 0.2f;                 // blue car faster
        glutPostRedisplay();
    }
    if (key == 'x' && currentScenario == 1) {
        car1Speed += 0.2f;                 // red car slower
        car2Speed -= 0.2f;                 // yellow car slower
        car3Speed += 0.2f;                 // blue car slower
        glutPostRedisplay();
    }
    // Bus speed Scenario 2
    if (key == 'u' && currentScenario == 2) {
        BusS268Speed += 0.5f;              // bus faster
        glutPostRedisplay();
    }
    if (key == 'i' && currentScenario == 2) {
        BusS268Speed -= 0.5f;              // bus slower
        if (BusS268Speed < 0.0f) BusS268Speed = 0.0f; // keep bus moving right only
        glutPostRedisplay();
    }

    ///........Scenario 1...........
    if (currentScenario == 1) {
        switch (key) {
        case 'a': case 'A':
            BusSpeed += 1.0f;
            break;
        case 'b': case 'B':
            BusSpeed -= 0.1f;
            break;
        case 'c':  case 'C':
            BusSpeed = 2.0f;
            break;
        case 'd': case 'D':
            BusSpeed = 0.0f;
            break;
        case 'r': case 'R':
            rainOn = true;
            break;
        case 's': case 'S':
            rainOn = false;
            break;
        case 'w': case 'W':
            snowOn = true;
            break;
        case 'e': case 'E':
            snowOn = false;
            break;
            // ADD BEFORE THE CLOSING } OF switch(key) IN SCENARIO 1 (around line 743)
case 'f': case 'F':
    car1Speed = 0.0f;    // stop red car
    break;
case 'g': case 'G':
    car1Speed = -0.8f;   // start red car
    break;
case 'h': case 'H':
    car2Speed = 0.0f;    // stop yellow car
    break;
case 'j': case 'J':
    car2Speed = 0.8f;    // start yellow car
    break;
case 'k': case 'K':
    car3Speed = 0.0f;    // stop blue car
    break;
case 'n': case 'N':
    car3Speed = -1.2f;   // start blue car
    break;
        case 'x': case 'X':
            winterMode = !winterMode;  // Toggle winter season
            if (winterMode) {
                snowOn = true;  // Auto-enable snowfall in winter
            }
            else {
                snowOn = false;  // Disable snow when leaving winter
            }
            break;
            case '+': case '=':
            balloonXSpeed += 0.02f;   // speed up left-right
            break;
        case '-': case '_':
            balloonXSpeed -= 0.02f;   // slow down left-right
            if (balloonXSpeed < 0.0f) balloonXSpeed = 0.0f;
            break;
        case '0':
            balloonX      = -30.0f;   // reset position
            balloonXSpeed =  0.05f;   // reset speed
            balloonScale  =  1.0f;    // reset size
            balloonScaleDir = 1.0f;
            break;




        case 'Y':
              isDay = true;  break; // Day
         case 'Z':
             isDay = false; break; // Night
        }
        glutPostRedisplay();
    }


    ///........Scenario 2...........

    else if (currentScenario == 2) {
        switch (key) {
        case 'X': case 'x':
            winterMode = !winterMode;  // Toggle winter season
            if (winterMode) {
                snowOn = true;  // Auto-enable snowfall in winter
            }
            else {
                snowOn = false;  // Disable snow when leaving winter
            }
            break;
         case 'Y':
              isDay = true;  break; // Day
         case 'Z':
             isDay = false; break; // Night
        }
        glutPostRedisplay();
    }

    // Global keys for switching scenarios
    if (key == '1') {
        currentScenario = 1;
        glutPostRedisplay();
    }
    else if (key == '2') {
        currentScenario = 2;
        glutPostRedisplay();
    }


}


void handledisplayMouse(int button, int state, int x, int y) {
}

/// Special keys.............Scenario2.................................................................................
void suvKeys(int key, int x, int y) { // Special keys // special keys (Arrow keys) for SUV
    switch (key) {
    case GLUT_KEY_UP:
        suvSpeed -= 1.0f;
        break;
    case GLUT_KEY_DOWN:
        suvSpeed += 0.8f;
        if (suvSpeed > 0.0f) suvSpeed = 0.0f;
        break;
    case GLUT_KEY_RIGHT:
        suvSpeed = -2.0f;
        break;
    case GLUT_KEY_LEFT:
        suvSpeed = 0.0f;
        break;
    }
    glutPostRedisplay();
}


void specialKeys(int key, int x, int y) {
    if (currentScenario == 1) {  //....Scenario1
        truckKeys(key, x, y);
    }
    else if (currentScenario == 2) {   //...Scenario2
        suvKeys(key, x, y);
        glutPostRedisplay();
    }
}

//..................Initialization....................

void initGL(void) {
    for (int i = 0; i < 200; i++) {
        rainX[i] = (rand() % 60) - 30;
        rainY[i] = (rand() % 50);
    }
    for (int i = 0; i < 150; i++) {
        snowX[i] = (rand() % 60) - 30;
        snowY[i] = (rand() % 50);
    }
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-30.0, 30.0, -20.0, 26.774);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(1026.495 / 2, 800.333 / 2);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Merged Scenarios");

    initGL();
    glutDisplayFunc(display);
    ///.........Scenario 1.....
        // Set up timers
    glutTimerFunc(3000, timer, 0);
    glutTimerFunc(40, updateTruck, 0);
    glutTimerFunc(40, updateBus, 0);
    glutTimerFunc(2, updateCloud, 0);
    glutTimerFunc(30, updateBalloon, 0);
    glutTimerFunc(30, updateRain, 0);
    glutTimerFunc(40, updateSnow, 0);
    glutTimerFunc(40, updateCar1S1, 0);
    glutTimerFunc(40, updateCar2S1, 0);
    glutTimerFunc(40, updateCar3S1, 0);  // Snow animation timer
    // glutTimerFunc(16, updateScenario2, 0);


      ///.......Scenario 2.......
    glutTimerFunc(1500, timers, 0); //day night tranzition
    glutTimerFunc(40, animationSUV, 0);
    glutTimerFunc(40, animationPlane, 0);
    glutTimerFunc(40, animationBus, 0);
    glutTimerFunc(2, animationCloudS268, 0);

    // EXISTING lines ~862-866
    glutTimerFunc(40, updateTruck, 0);
    glutTimerFunc(40, updateBus, 0);
    glutTimerFunc(2,  updateCloud, 0);
    glutTimerFunc(30, updateBalloon, 0);

// ADD THESE 3 LINES right after (around line 867)
    glutTimerFunc(40, updateCar1S1, 0);   // line ~867
    glutTimerFunc(40, updateCar2S1, 0);   // line ~868
    glutTimerFunc(40, updateCar3S1, 0);   // line ~869


    /// Set up input handlers
    glutKeyboardFunc(Keys);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(handledisplayMouse);

    glutMainLoop();
    return 0;





  }
