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

    vfxWorld.tickTime();

    if (isSkillPlaying) {
        skillTimer -= deltaSeconds;
        if (skillTimer <= 0.0f) {
            isSkillPlaying = false;
        }
    }

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
            if (currentScene == SCENE_HOME)
            {
                showHistory = !showHistory;
                return true;
            }
            if (currentScene == SCENE_BATTLE)
            {
                if (!battlePaused && !battleEnded) applySkillDamage(true, 2);
                return true;
            }
            return false;


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
            if (currentScene == SCENE_BATTLE)
            {
                if (!battlePaused && !battleEnded) {
                    applySkillDamage(false, 2);
                    isSkillPlaying = true;
                    activeSkillIsPlayer1 = false;
                    activeSkillIndex = 2;

                    skillTimer = 2.5f;
                }
                return true;
            }
            return false;

        case 'f':
        case 'F':
            if (currentScene == SCENE_BATTLE)
            {
                if (!battlePaused && !battleEnded) {
                    applySkillDamage(false, 1);
                    isSkillPlaying = true;
                    activeSkillIsPlayer1 = false;
                    activeSkillIndex = 1;
                    skillTimer = 2.0f;
                }
                return true;
            }
            return false;
        case '>':
        case '.':
            if (currentScene == SCENE_BATTLE)
            {
                if (!battlePaused && !battleEnded) {
                    applySkillDamage(true, 1);
                    isSkillPlaying = true;
                    activeSkillIsPlayer1 = true;
                    activeSkillIndex = 1;
                    skillTimer = 2.0f;
                }
                return true;
            }
            return false;


        case '?':
        case '/':
            if (currentScene == SCENE_BATTLE)
            {
                if (!battlePaused && !battleEnded) {
                    applySkillDamage(true, 2);
                    isSkillPlaying = true;
                    activeSkillIsPlayer1 = true;
                    activeSkillIndex = 2;
                    skillTimer = 2.5f;
                }
                return true;
            }
            return false;


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

void MyVirtualWorld::applySkillDamage(bool attackerIsPlayer1, int skillIndex)
{
    const ProjectCharacter& attacker = attackerIsPlayer1 ? player1Character : player2Character;
    const ProjectCharacter& defender = attackerIsPlayer1 ? player2Character : player1Character;
    int& defenderHealth = attackerIsPlayer1 ? player2Health : player1Health;

    if (skillSystem.tryApplySkillDamage(attackerIsPlayer1, skillIndex, attacker, defender, defenderHealth))
    {
        resolveBattleIfNeeded();
    }
}

void MyVirtualWorld::drawBattleScene()
{
    arena.draw();
    player1Character.draw();
    player2Character.draw();

    if (isSkillPlaying) {
        glPushMatrix();
        // 你可能需要根据施法者的位置，把特效平移到角色身上
        // float x = activeSkillIsPlayer1 ? player1Character.getPositionX() : player2Character.getPositionX();
        // float z = activeSkillIsPlayer1 ? player1Character.getPositionZ() : player2Character.getPositionZ();
        // glTranslatef(x, 0.0f, z);

        vfxWorld.draw(activeSkillIsPlayer1, activeSkillIndex);
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
    drawBitmapText(38.0f, height - 114.0f, GLUT_BITMAP_HELVETICA_12, "Skill 1: G   Skill 2: H");
    drawBitmapTextRight(width - 38.0f, height - 114.0f, GLUT_BITMAP_HELVETICA_12, "Skill 1: >   Skill 2: ?");

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
