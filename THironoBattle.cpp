#define M_PI 3.141592653589793
#include <GL/glut.h>
#include <windows.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include "THirono.hpp"
#include "skill.hpp"

using namespace THirono;

namespace {

float clampValue(float value, float minimumValue, float maximumValue)
{
    if (value < minimumValue) return minimumValue;
    if (value > maximumValue) return maximumValue;
    return value;
}

void drawBitmapText(float x, float y, void* font, const std::string& text)
{
    glRasterPos2f(x, y);
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        glutBitmapCharacter(font, text[i]);
    }
}

int getBitmapTextWidth(void* font, const std::string& text)
{
    int width = 0;
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        width += glutBitmapWidth(font, text[i]);
    }
    return width;
}

void drawBitmapTextRight(float rightX, float y, void* font, const std::string& text)
{
    drawBitmapText(rightX - static_cast<float>(getBitmapTextWidth(font, text)), y, font, text);
}

void drawBitmapTextCentered(float centerX, float y, void* font, const std::string& text)
{
    drawBitmapText(centerX - static_cast<float>(getBitmapTextWidth(font, text)) * 0.5f, y, font, text);
}

void beginScreenOverlay(int windowWidth, int windowHeight)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, windowWidth, 0.0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void endScreenOverlay()
{
    glPopAttrib();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawOverlayPanel(float left, float bottom, float right, float top,
                      float red, float green, float blue, float alpha)
{
    glColor4f(red, green, blue, alpha);
    glBegin(GL_QUADS);
        glVertex2f(left, bottom);
        glVertex2f(right, bottom);
        glVertex2f(right, top);
        glVertex2f(left, top);
    glEnd();
}

void drawHudBar(float left, float bottom, float width, float height, float fillRatio,
                float fillR, float fillG, float fillB)
{
    drawOverlayPanel(left, bottom, left + width, bottom + height, 0.14f, 0.16f, 0.22f, 0.95f);
    const float clampedRatio = clampValue(fillRatio, 0.0f, 1.0f);
    drawOverlayPanel(left + 4.0f, bottom + 4.0f,
                     left + 4.0f + (width - 8.0f) * clampedRatio, bottom + height - 4.0f,
                     fillR, fillG, fillB, 0.95f);
}

} // namespace

void MyVirtualWorld::tickTime()
{
    const int currentTimeMs = glutGet(GLUT_ELAPSED_TIME);
    float deltaSeconds = static_cast<float>(currentTimeMs - lastTickTimeMs) * 0.001f;
    if (deltaSeconds < 0.0f)
    {
        deltaSeconds = 0.0f;
    }
    if (deltaSeconds > 0.05f)
    {
        deltaSeconds = 0.05f;
    }
    lastTickTimeMs = currentTimeMs;

    vfxWorldP1.tickTime();
    vfxWorldP2.tickTime();

    if (p1Skill1Active) {
        p1Skill1Timer -= deltaSeconds;
        if (p1Skill1Timer <= 0.0f) p1Skill1Active = false;

        float angleRad = p1Skill1Angle * 3.14159265f / 180.0f;
        p1Skill1PosX += sinf(angleRad) * 11.0f * deltaSeconds;
        p1Skill1PosZ += cosf(angleRad) * 11.0f * deltaSeconds;

        p1Skill1TickTimer -= deltaSeconds;
        if (p1Skill1TickTimer <= 0.0f) {
            float dx = player2Character.getPositionX() - p1Skill1PosX;
            float dz = player2Character.getPositionZ() - p1Skill1PosZ;
            if (sqrt(dx*dx + dz*dz) <= 3.0f) {
                player2Health -= 4;
                p1Skill1TickTimer = 0.4f;
                resolveBattleIfNeeded();
            }
        }
    }

    if (p1Skill2Active) {
        p1Skill2Timer -= deltaSeconds;
        if (p1Skill2Timer <= 0.0f) p1Skill2Active = false;

        if (!p1Skill2HasHit) {
            float dx = player2Character.getPositionX() - p1Skill2PosX;
            float dz = player2Character.getPositionZ() - p1Skill2PosZ;
            float rad = p1Skill2Angle * 3.14159265f / 180.0f;
            float localX = dx * cosf(rad) - dz * sinf(rad);
            float localZ = dx * sinf(rad) + dz * cosf(rad);

            if (localX >= -4.0f && localX <= 4.0f && localZ >= -2.0f && localZ <= 18.0f) {
                player2Health -= 20;
                p1Skill2HasHit = true;
                resolveBattleIfNeeded();
            }
        }
    }

    if (p2Skill1Active) {
        p2Skill1Timer -= deltaSeconds;
        if (p2Skill1Timer <= 0.0f) p2Skill1Active = false;

        p2Skill1TickTimer -= deltaSeconds;
        if (p2Skill1TickTimer <= 0.0f) {
            float dx = player1Character.getPositionX() - p2Skill1PosX;
            float dz = player1Character.getPositionZ() - p2Skill1PosZ;
            float rad = p2Skill1Angle * 3.14159265f / 180.0f;
            float localX = dx * cosf(rad) - dz * sinf(rad);
            float localZ = dx * sinf(rad) + dz * cosf(rad);


            if (localX >= -4.0f && localX <= 4.0f && localZ >= -2.0f && localZ <= 18.0f) {
                player1Health -= 4;
                p2Skill1TickTimer = 0.4f;
                resolveBattleIfNeeded();
            }
        }
    }

    if (p2Skill2Active) {
        p2Skill2Timer -= deltaSeconds;
        if (p2Skill2Timer <= 0.0f) p2Skill2Active = false;

        float angleRad = p2Skill2Angle * 3.14159265f / 180.0f;
        p2Skill2PosX += sinf(angleRad) * 9.0f * deltaSeconds;
        p2Skill2PosZ += cosf(angleRad) * 9.0f * deltaSeconds;

        if (!p2Skill2HasHit) {
            float dx = player1Character.getPositionX() - p2Skill2PosX;
            float dz = player1Character.getPositionZ() - p2Skill2PosZ;
            if (sqrt(dx*dx + dz*dz) <= 4.0f) {
                player1Health -= 20;
                p2Skill2HasHit = true;
                resolveBattleIfNeeded();
            }
        }
    }

    if (p1Skill1CD > 0.0f) p1Skill1CD -= deltaSeconds;
    if (p1Skill2CD > 0.0f) p1Skill2CD -= deltaSeconds;
    if (p2Skill1CD > 0.0f) p2Skill1CD -= deltaSeconds;
    if (p2Skill2CD > 0.0f) p2Skill2CD -= deltaSeconds;


    if (currentScene == SCENE_BATTLE && !battlePaused && !battleEnded)
    {
        matchTimeRemainingMs -= static_cast<int>(deltaSeconds * 1000.0f);
        if (matchTimeRemainingMs < 0)
        {
            matchTimeRemainingMs = 0;
        }
        player1Character.setMovingForward((GetAsyncKeyState('W') & 0x8000) != 0);
        player1Character.setMovingBackward((GetAsyncKeyState('S') & 0x8000) != 0);
        player1Character.setMovingLeft((GetAsyncKeyState('A') & 0x8000) != 0);
        player1Character.setMovingRight((GetAsyncKeyState('D') & 0x8000) != 0);
        player2Character.setMovingForward((GetAsyncKeyState(VK_UP) & 0x8000) != 0);
        player2Character.setMovingBackward((GetAsyncKeyState(VK_DOWN) & 0x8000) != 0);
        player2Character.setMovingLeft((GetAsyncKeyState(VK_LEFT) & 0x8000) != 0);
        player2Character.setMovingRight((GetAsyncKeyState(VK_RIGHT) & 0x8000) != 0);
        player1Character.update(deltaSeconds);
        player2Character.update(deltaSeconds);
        resolveBattleIfNeeded();
    }
}

bool MyVirtualWorld::handleKeyDown(unsigned char key)
{
    switch (key)
    {
        case 13:
        case ' ':
            if (currentScene == SCENE_HOME)
            {
                selectingPlayer = 1;
                showHistory = false;
                currentScene = SCENE_SELECT;
                return true;
            }
            if (currentScene == SCENE_SELECT)
            {
                if (selectingPlayer == 1)
                {
                    player1Character = (player1SelectedCharacterIndex == 0) ? gCharacter1Preview : gCharacter2Preview;
                    player1Character.resetPosition();
                    selectingPlayer = 2;
                }
                else
                {
                    player2Character = (player2SelectedCharacterIndex == 0) ? gCharacter1Preview : gCharacter2Preview;
                    player1Character.resetPosition();
                    player2Character.resetPosition();
                    player1Character.moveByInput(-10.0f, 0.0f, 1.0f);
                    player2Character.moveByInput(10.0f, 0.0f, 1.0f);
                    player1Health = 100;
                    player2Health = 100;
                    matchTimeRemainingMs = 90000;
                    battlePaused = false;
                    battleEnded = false;
                    battleResultLogged = false;
                    battleResultText = "";
                    currentScene = SCENE_BATTLE;
                }
                return true;
            }
            return false;

        case 'b':
        case 'B':
            if (currentScene == SCENE_BATTLE)
            {
                if (battleEnded)
                {
                    battleEnded = false;
                    battlePaused = false;
                    battleResultText = "";
                    currentScene = SCENE_HOME;
                    return true;
                }
                player1Character.resetPosition();
                player2Character.resetPosition();
                selectingPlayer = 2;
                battlePaused = false;
                battleEnded = false;
                battleResultLogged = false;
                battleResultText = "";
                currentScene = SCENE_SELECT;
                return true;
            }
            if (currentScene == SCENE_SELECT)
            {
                if (selectingPlayer == 2)
                {
                    selectingPlayer = 1;
                }
                else
                {
                    currentScene = SCENE_HOME;
                }
                return true;
            }
            return false;

        case 'h':
        case 'H':
            if (currentScene == SCENE_HOME) { showHistory = !showHistory; return true; }
            if (currentScene == SCENE_BATTLE && !battlePaused && !battleEnded && p1Skill2CD <= 0.0f) {
                p1Skill2Active = true; p1Skill2Timer = 2.5f; p1Skill2CD = 7.0f;
                p1Skill2HasHit = false;
                vfxWorldP1.treeScale = 0.0f; vfxWorldP1.fallAngle = 0.0f; vfxWorldP1.fallSpeed = 0.0f;
                float angleRad = player1Character.getFacingAngle() * 3.14159265f / 180.0f;
                p1Skill2PosX = player1Character.getPositionX() + sinf(angleRad) * 3.0f;
                p1Skill2PosZ = player1Character.getPositionZ() + cosf(angleRad) * 3.0f;
                p1Skill2Angle = player1Character.getFacingAngle();
            } return true;


        case 'a':
        case 'A':
            if (currentScene == SCENE_SELECT)
            {
                if (selectingPlayer == 1)
                    player1SelectedCharacterIndex = (player1SelectedCharacterIndex + 1) % 2;
                return true;
            }
            if (currentScene == SCENE_BATTLE)
            {
                if (battlePaused || battleEnded) return true;
                player1Character.setMovingLeft(true);
                player1Character.moveByInput(-1.0f, 0.0f, 1.0f);
                return true;
            }
            return false;

        case 'd':
        case 'D':
            if (currentScene == SCENE_SELECT)
            {
                if (selectingPlayer == 1)
                    player1SelectedCharacterIndex = (player1SelectedCharacterIndex + 1) % 2;
                return true;
            }
            if (currentScene == SCENE_BATTLE)
            {
                if (battlePaused || battleEnded) return true;
                player1Character.setMovingRight(true);
                player1Character.moveByInput(1.0f, 0.0f, 1.0f);
                return true;
            }
            return false;

        case 'w':
        case 'W':
            if (currentScene == SCENE_BATTLE)
            {
                if (battlePaused || battleEnded) return true;
                player1Character.setMovingForward(true);
                player1Character.moveByInput(0.0f, -1.0f, 1.0f);
                return true;
            }
            return false;

        case 's':
        case 'S':
            if (currentScene == SCENE_BATTLE)
            {
                if (battlePaused || battleEnded) return true;
                player1Character.setMovingBackward(true);
                player1Character.moveByInput(0.0f, 1.0f, 1.0f);
                return true;
            }
            return false;

        case 'p':
        case 'P':
            if (currentScene == SCENE_BATTLE)
            {
                battlePaused = true;
                return true;
            }
            return false;

        case 'c':
        case 'C':
            if (currentScene == SCENE_BATTLE)
            {
                if (battleEnded) return true;
                battlePaused = false;
                return true;
            }
            return false;

        case 'g':
        case 'G':
            if (currentScene == SCENE_BATTLE && !battlePaused && !battleEnded && p1Skill1CD <= 0.0f) {
                p1Skill1Active = true; p1Skill1Timer = 2.0f; p1Skill1CD = 3.5f;
                p1Skill1TickTimer = 0.0f;
                float angleRad = player1Character.getFacingAngle() * 3.14159265f / 180.0f;
                p1Skill1PosX = player1Character.getPositionX() + sinf(angleRad) * 3.0f;
                p1Skill1PosZ = player1Character.getPositionZ() + cosf(angleRad) * 3.0f;
                p1Skill1Angle = player1Character.getFacingAngle();
            } return true;

        case '>':
        case '.':
           if (currentScene == SCENE_BATTLE && !battlePaused && !battleEnded && p2Skill1CD <= 0.0f) {
                p2Skill1Active = true; p2Skill1Timer = 2.0f; p2Skill1CD = 3.5f;
                p2Skill1TickTimer = 0.0f;
                vfxWorldP2.animationStartTime = glutGet(GLUT_ELAPSED_TIME);
                float angleRad = player2Character.getFacingAngle() * 3.14159265f / 180.0f;
                p2Skill1PosX = player2Character.getPositionX() + sinf(angleRad) * 3.0f;
                p2Skill1PosZ = player2Character.getPositionZ() + cosf(angleRad) * 3.0f;
                p2Skill1Angle = player2Character.getFacingAngle();
            } return true;

        case '?':
        case '/':
            if (currentScene == SCENE_BATTLE && !battlePaused && !battleEnded && p2Skill2CD <= 0.0f) {
                p2Skill2Active = true; p2Skill2Timer = 2.5f; p2Skill2CD = 7.0f;
                p2Skill2HasHit = false;
                float angleRad = player2Character.getFacingAngle() * 3.14159265f / 180.0f;
                p2Skill2PosX = player2Character.getPositionX() + sinf(angleRad) * 3.0f;
                p2Skill2PosZ = player2Character.getPositionZ() + cosf(angleRad) * 3.0f;
                p2Skill2Angle = player2Character.getFacingAngle();
            } return true;

        default:
            return false;
    }
}

void MyVirtualWorld::handleKeyUp(unsigned char key)
{
    switch (key)
    {
        case 'a':
        case 'A':
            player1Character.setMovingLeft(false);
            break;
        case 'd':
        case 'D':
            player1Character.setMovingRight(false);
            break;
        case 'w':
        case 'W':
            player1Character.setMovingForward(false);
            break;
        case 's':
        case 'S':
            player1Character.setMovingBackward(false);
            break;
    }
}

bool MyVirtualWorld::handleSpecialKey(int key)
{
    if (currentScene == SCENE_BATTLE)
    {
        if (battlePaused || battleEnded)
        {
            return true;
        }
        if (key == GLUT_KEY_UP)
        {
            player2Character.moveByInput(0.0f, -1.0f, 1.0f);
            return true;
        }
        if (key == GLUT_KEY_DOWN)
        {
            player2Character.moveByInput(0.0f, 1.0f, 1.0f);
            return true;
        }
        if (key == GLUT_KEY_LEFT)
        {
            player2Character.moveByInput(-1.0f, 0.0f, 1.0f);
            return true;
        }
        if (key == GLUT_KEY_RIGHT)
        {
            player2Character.moveByInput(1.0f, 0.0f, 1.0f);
            return true;
        }
    }

    if (currentScene == SCENE_SELECT)
    {
        if (selectingPlayer == 2 && key == GLUT_KEY_LEFT)
        {
            player2SelectedCharacterIndex = (player2SelectedCharacterIndex + 1) % 2;
            return true;
        }
        if (selectingPlayer == 2 && key == GLUT_KEY_RIGHT)
        {
            player2SelectedCharacterIndex = (player2SelectedCharacterIndex + 1) % 2;
            return true;
        }
    }
    return false;
}

void MyVirtualWorld::resolveBattleIfNeeded()
{
    if (battleEnded)
    {
        return;
    }

    if (player1Health <= 0 || player2Health <= 0 || matchTimeRemainingMs <= 0)
    {
        battleEnded = true;
        battlePaused = false;

        if (player1Health <= 0 && player2Health <= 0)
        {
            battleResultText = "Draw";
        }
        else if (player1Health <= 0)
        {
            battleResultText = "Player 2 Wins";
        }
        else if (player2Health <= 0)
        {
            battleResultText = "Player 1 Wins";
        }
        else if (player1Health > player2Health)
        {
            battleResultText = "Player 1 Wins";
        }
        else if (player2Health > player1Health)
        {
            battleResultText = "Player 2 Wins";
        }
        else
        {
            battleResultText = "Draw";
        }

        appendBattleHistory();
    }
}



void MyVirtualWorld::drawBattleScene()
{
    arena.draw();
    player1Character.draw();
    player2Character.draw();

    if (p1Skill1Active)
    {
        glPushMatrix(); glTranslatef(p1Skill1PosX, -4.0f, p1Skill1PosZ); glRotatef(p1Skill1Angle, 0.0f, 1.0f, 0.0f);
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT); glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.0f, 0.0f, 0.22f); glBegin(GL_TRIANGLE_FAN); glVertex3f(0.0f, 0.02f, 0.0f); for (int i=0; i<=360; i+=15) glVertex3f(sinf(i*3.14f/180.0f)*3.0f, 0.02f, cosf(i*3.14f/180.0f)*3.0f); glEnd(); glPopAttrib();
        vfxWorldP1.draw(true, 1);
        glPopMatrix();
    }

    if (p1Skill2Active)
    {
        glPushMatrix(); glTranslatef(p1Skill2PosX, -4.0f, p1Skill2PosZ); glRotatef(p1Skill2Angle, 0.0f, 1.0f, 0.0f);
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT); glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.0f, 0.0f, 0.25f); glBegin(GL_QUADS); glVertex3f(-4.0f, 0.02f, -2.0f); glVertex3f(4.0f, 0.02f, -2.0f); glVertex3f(4.0f, 0.02f, 18.0f); glVertex3f(-4.0f, 0.02f, 18.0f); glEnd();
        glLineWidth(2.5f); glColor4f(1.0f, 0.0f, 0.0f, 0.75f); glBegin(GL_LINE_LOOP); glVertex3f(-4.0f, 0.02f, -2.0f); glVertex3f(4.0f, 0.02f, -2.0f); glVertex3f(4.0f, 0.02f, 18.0f); glVertex3f(-4.0f, 0.02f, 18.0f); glEnd(); glPopAttrib();
        vfxWorldP1.draw(true, 2);

        glPopMatrix();
    }

    if (p2Skill1Active)
    {
        glPushMatrix(); glTranslatef(p2Skill1PosX, -4.0f, p2Skill1PosZ); glRotatef(p2Skill1Angle, 0.0f, 1.0f, 0.0f);
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT); glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.0f, 0.0f, 0.25f); glBegin(GL_QUADS); glVertex3f(-4.0f, 0.02f, -2.0f); glVertex3f(4.0f, 0.02f, -2.0f); glVertex3f(4.0f, 0.02f, 18.0f); glVertex3f(-4.0f, 0.02f, 18.0f); glEnd();
        glLineWidth(2.5f); glColor4f(1.0f, 0.0f, 0.0f, 0.75f); glBegin(GL_LINE_LOOP); glVertex3f(-4.0f, 0.02f, -2.0f); glVertex3f(4.0f, 0.02f, -2.0f); glVertex3f(4.0f, 0.02f, 18.0f); glVertex3f(-4.0f, 0.02f, 18.0f); glEnd(); glPopAttrib();
        vfxWorldP2.draw(false, 1);
        glPopMatrix();
    }

    if (p2Skill2Active)
    {
        glPushMatrix(); glTranslatef(p2Skill2PosX, -4.0f, p2Skill2PosZ); glRotatef(p2Skill2Angle, 0.0f, 1.0f, 0.0f);
        float radius = 4.0f;
        glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT); glDisable(GL_LIGHTING); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.0f, 0.0f, 0.25f); glBegin(GL_TRIANGLE_FAN); glVertex3f(0.0f, 0.02f, 0.0f); for (int i=0; i<=360; i+=12) glVertex3f(sinf(i*3.14159f/180.0f)*radius, 0.02f, cosf(i*3.14159f/180.0f)*radius); glEnd();
        glLineWidth(2.5f); glColor4f(1.0f, 0.0f, 0.0f, 0.75f); glBegin(GL_LINE_LOOP); for (int i=0; i<=360; i+=12) glVertex3f(sinf(i*3.14159f/180.0f)*radius, 0.02f, cosf(i*3.14159f/180.0f)*radius); glEnd(); glPopAttrib();
        vfxWorldP2.draw(false, 2);
        glPopMatrix();
    }


    const int width = glutGet(GLUT_WINDOW_WIDTH);
    const int height = glutGet(GLUT_WINDOW_HEIGHT);
    beginScreenOverlay(width, height);
    drawOverlayPanel(20.0f, height - 120.0f, 340.0f, height - 20.0f, 0.04f, 0.06f, 0.12f, 0.82f);
    drawOverlayPanel(width - 340.0f, height - 120.0f, width - 20.0f, height - 20.0f, 0.04f, 0.06f, 0.12f, 0.82f);

    const int totalSeconds = matchTimeRemainingMs / 1000;
    const int minutes = totalSeconds / 60;
    const int seconds = totalSeconds % 60;
    std::ostringstream timerStream;
    timerStream << std::setfill('0') << std::setw(2) << minutes
                << ":" << std::setfill('0') << std::setw(2) << seconds;
    const std::string timerText = timerStream.str();

    glColor3f(1.0f, 0.95f, 0.55f);
    drawBitmapText(38.0f, height - 46.0f, GLUT_BITMAP_HELVETICA_18, "PLAYER 1");
    drawBitmapTextRight(width - 38.0f, height - 46.0f, GLUT_BITMAP_HELVETICA_18, "PLAYER 2");
    drawBitmapText(width * 0.5f - 30.0f, height - 46.0f, GLUT_BITMAP_HELVETICA_18, timerText);

    glColor3f(0.88f, 0.93f, 1.0f);
    drawBitmapText(38.0f, height - 68.0f, GLUT_BITMAP_HELVETICA_18, player1Character.getCharacterName());
    drawBitmapTextRight(width - 38.0f, height - 68.0f, GLUT_BITMAP_HELVETICA_18, player2Character.getCharacterName());

    drawHudBar(38.0f, height - 98.0f, 250.0f, 20.0f, player1Health / 100.0f, 0.85f, 0.20f, 0.24f);
    drawHudBar(width - 288.0f, height - 98.0f, 250.0f, 20.0f, player2Health / 100.0f, 0.85f, 0.20f, 0.24f);

    glColor3f(0.70f, 0.82f, 1.0f);
    std::ostringstream p1HUD;
    p1HUD << "G: Apple ";
    if (p1Skill1CD > 0.0f) {
        p1HUD << "[" << (int)p1Skill1CD + 1 << "s]";
    } else {
        p1HUD << "[Ready]";
    }

    p1HUD << "  H: Tree ";
    if (p1Skill2CD > 0.0f) {
        p1HUD << "[" << (int)p1Skill2CD + 1 << "s]";
    } else {
        p1HUD << "[Ready]";
    }

    glColor3f(0.70f, 0.82f, 1.0f);
    drawBitmapText(38.0f, height - 114.0f, GLUT_BITMAP_HELVETICA_12, p1HUD.str());

    std::ostringstream p2HUD;
    p2HUD << ">: Vine ";
    if (p2Skill1CD > 0.0f) {
        p2HUD << "[" << (int)p2Skill1CD + 1 << "s]";
    } else {
        p2HUD << "[Ready]";
    }

    p2HUD << "  ?: Tornado ";
    if (p2Skill2CD > 0.0f) {
        p2HUD << "[" << (int)p2Skill2CD + 1 << "s]";
    } else {
        p2HUD << "[Ready]";
    }

    drawBitmapTextRight(width - 38.0f, height - 114.0f, GLUT_BITMAP_HELVETICA_12, p2HUD.str());

    glColor3f(1.0f, 0.95f, 0.55f);
    drawBitmapText(width * 0.5f - 220.0f, height - 22.0f, GLUT_BITMAP_HELVETICA_18, "P: Pause    C: Continue    B: Back");
    if (battlePaused)
    {
        drawOverlayPanel(width * 0.5f - 220.0f, height * 0.5f - 70.0f,
                         width * 0.5f + 220.0f, height * 0.5f + 70.0f,
                         0.05f, 0.08f, 0.16f, 0.94f);
        drawOverlayPanel(width * 0.5f - 200.0f, height * 0.5f + 18.0f,
                         width * 0.5f + 200.0f, height * 0.5f + 54.0f,
                         0.96f, 0.82f, 0.18f, 0.96f);
        glColor3f(0.10f, 0.10f, 0.12f);
        drawBitmapTextCentered(width * 0.5f, height * 0.5f + 28.0f, GLUT_BITMAP_TIMES_ROMAN_24, "PAUSED");
        glColor3f(0.94f, 0.97f, 1.0f);
        drawBitmapTextCentered(width * 0.5f, height * 0.5f - 8.0f, GLUT_BITMAP_HELVETICA_18, "Press C to continue");
        drawBitmapTextCentered(width * 0.5f, height * 0.5f - 34.0f, GLUT_BITMAP_HELVETICA_18, "Press B to leave battle");
    }
    if (battleEnded)
    {
        drawOverlayPanel(width * 0.5f - 280.0f, height * 0.5f - 110.0f,
                         width * 0.5f + 280.0f, height * 0.5f + 110.0f,
                         0.08f, 0.10f, 0.18f, 0.94f);
        drawOverlayPanel(width * 0.5f - 260.0f, height * 0.5f + 30.0f,
                         width * 0.5f + 260.0f, height * 0.5f + 80.0f,
                         0.90f, 0.24f, 0.30f, 0.96f);
        glColor3f(1.0f, 0.98f, 0.92f);
        drawBitmapTextCentered(width * 0.5f, height * 0.5f + 48.0f, GLUT_BITMAP_TIMES_ROMAN_24, "MATCH OVER");
        drawBitmapTextCentered(width * 0.5f, height * 0.5f - 2.0f, GLUT_BITMAP_HELVETICA_18, battleResultText);
        drawBitmapTextCentered(width * 0.5f, height * 0.5f - 34.0f, GLUT_BITMAP_HELVETICA_18, "Result saved to battle_history.txt");
        drawBitmapTextCentered(width * 0.5f, height * 0.5f - 64.0f, GLUT_BITMAP_HELVETICA_18, "Press B to return to Home");
    }
    endScreenOverlay();
}
