#define M_PI 3.141592653589793
#include <GL/glut.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "THirono.hpp"
#include "skill.hpp"

using namespace THirono;

namespace {

void drawBitmapText(float x, float y, void* font, const std::string& text)
{
    glRasterPos2f(x, y);
    for (std::size_t i = 0; i < text.size(); ++i)
    {
        glutBitmapCharacter(font, text[i]);
    }
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

} // namespace

ProjectCharacter THirono::gCharacter1Preview("character1");
ProjectCharacter THirono::gCharacter2Preview("character2");
bool THirono::gPreviewCharactersInitialized = false;

MyVirtualWorld::MyVirtualWorld()
    : player1Character("character1")
    , player2Character("character2")
    , currentScene(SCENE_HOME)
    , player1SelectedCharacterIndex(0)
    , player2SelectedCharacterIndex(1)
    , selectingPlayer(1)
    , player1Health(100)
    , player2Health(100)
    , matchTimeRemainingMs(90000)
    , battlePaused(false)
    , battleEnded(false)
    , battleResultLogged(false)
    , showHistory(false)
    , battleResultText("")
    , lastTickTimeMs(0)
{
}

void MyVirtualWorld::init()
{
    arena.init();
    vfxWorldP1.init();
    vfxWorldP2.init();
    loadBattleHistory();
    if (!gPreviewCharactersInitialized)
    {
        gCharacter1Preview.init();
        gCharacter2Preview.init();
        gPreviewCharactersInitialized = true;
    }
    player1Character = gCharacter1Preview;
    player2Character = gCharacter2Preview;
    player1Character.resetPosition();
    player2Character.resetPosition();
    player1Character.moveByInput(-10.0f, 0.0f, 1.0f);
    player2Character.moveByInput(10.0f, 0.0f, 1.0f);
    lastTickTimeMs = glutGet(GLUT_ELAPSED_TIME);
}

void MyVirtualWorld::draw()
{
    switch (currentScene)
    {
        case SCENE_SELECT:
            drawCharacterSelectScene();
            break;
        case SCENE_BATTLE:
            drawBattleScene();
            break;
        case SCENE_HOME:
        default:
            drawHomeScene();
            break;
    }
}

void MyVirtualWorld::appendBattleHistory()
{
    if (battleResultLogged)
    {
        return;
    }

    std::ostringstream entry;
    entry << player1Character.getCharacterName() << " vs "
          << player2Character.getCharacterName() << " - "
          << battleResultText << " (P1 HP: " << player1Health
          << ", P2 HP: " << player2Health << ")";

    battleHistoryEntries.push_back(entry.str());

    std::ofstream output("battle_history.txt", std::ios::app);
    if (output)
    {
        output << entry.str() << "\n";
    }

    battleResultLogged = true;
}

void MyVirtualWorld::loadBattleHistory()
{
    if (!battleHistoryEntries.empty())
    {
        return;
    }

    std::ifstream input("battle_history.txt");
    std::string line;
    while (std::getline(input, line))
    {
        if (!line.empty())
        {
            battleHistoryEntries.push_back(line);
        }
    }
}

void MyVirtualWorld::drawHomeScene()
{
    arena.draw();

    glPushMatrix();
        glTranslatef(0.0f, -0.8f, -8.0f);
        glRotatef(18.0f, 0.0f, 1.0f, 0.0f);
        player1Character.drawPreview(0.0f, -3.2f, 0.0f, 0.0f, 1.0f, false);
    glPopMatrix();

    const int width = glutGet(GLUT_WINDOW_WIDTH);
    const int height = glutGet(GLUT_WINDOW_HEIGHT);
    beginScreenOverlay(width, height);
    drawOverlayPanel(40.0f, 50.0f, static_cast<float>(width - 40), static_cast<float>(height - 40),
                     0.02f, 0.03f, 0.08f, 0.72f);

    glColor3f(1.0f, 0.92f, 0.45f);
    drawBitmapText(70.0f, height - 90.0f, GLUT_BITMAP_TIMES_ROMAN_24, "STAR BLOOM ARENA");
    glColor3f(0.90f, 0.94f, 1.0f);
    drawBitmapText(70.0f, height - 145.0f, GLUT_BITMAP_HELVETICA_18, "How to Play");
    drawBitmapText(70.0f, height - 180.0f, GLUT_BITMAP_HELVETICA_18, "1. Press ENTER to go to the character selection page.");
    drawBitmapText(70.0f, height - 210.0f, GLUT_BITMAP_HELVETICA_18, "2. Player 1 uses A/D to choose, then Player 2 uses LEFT/RIGHT.");
    drawBitmapText(70.0f, height - 240.0f, GLUT_BITMAP_HELVETICA_18, "3. Press ENTER to lock Player 1, then ENTER again to lock Player 2.");
    drawBitmapText(70.0f, height - 270.0f, GLUT_BITMAP_HELVETICA_18, "4. In battle: P1 moves with WASD, P2 moves with Arrow Keys.");
    drawBitmapText(70.0f, height - 300.0f, GLUT_BITMAP_HELVETICA_18, "5. Skills: P1 = G/H, P2 = >/? . Time up compares HP.");
    drawBitmapText(70.0f, height - 330.0f, GLUT_BITMAP_HELVETICA_18, "6. Press H to open History.");
    drawBitmapText(70.0f, height - 360.0f, GLUT_BITMAP_HELVETICA_18, "ESC to quit anytime.");
    glColor3f(1.0f, 0.95f, 0.60f);
    drawBitmapText(70.0f, 90.0f, GLUT_BITMAP_HELVETICA_18, "Press ENTER to start");

    if (showHistory)
    {
        drawOverlayPanel(width - 620.0f, 70.0f, width - 40.0f, height - 80.0f, 0.06f, 0.08f, 0.14f, 0.92f);
        glColor3f(1.0f, 0.95f, 0.55f);
        drawBitmapText(width - 590.0f, height - 115.0f, GLUT_BITMAP_TIMES_ROMAN_24, "History");
        glColor3f(0.88f, 0.93f, 1.0f);

        if (battleHistoryEntries.empty())
        {
            drawBitmapText(width - 590.0f, height - 155.0f, GLUT_BITMAP_HELVETICA_18, "No battle records yet.");
        }
        else
        {
            int startIndex = static_cast<int>(battleHistoryEntries.size()) - 8;
            if (startIndex < 0) startIndex = 0;
            float y = static_cast<float>(height - 155);
            for (std::size_t i = static_cast<std::size_t>(startIndex); i < battleHistoryEntries.size(); ++i)
            {
                drawBitmapText(width - 590.0f, y, GLUT_BITMAP_HELVETICA_12, battleHistoryEntries[i]);
                y -= 26.0f;
            }
        }
    }
    endScreenOverlay();
}

void MyVirtualWorld::drawCharacterSelectScene()
{
    arena.draw();

    const bool leftSelected = (selectingPlayer == 1)
        ? (player1SelectedCharacterIndex == 0)
        : (player2SelectedCharacterIndex == 0);
    const bool rightSelected = (selectingPlayer == 1)
        ? (player1SelectedCharacterIndex == 1)
        : (player2SelectedCharacterIndex == 1);

    gCharacter1Preview.drawPreview(-11.0f, -2.4f, 11.0f, 0.0f, 0.82f, leftSelected);
    gCharacter2Preview.drawPreview(11.0f, -2.4f, 11.0f, 0.0f, 0.82f, rightSelected);

    const int width = glutGet(GLUT_WINDOW_WIDTH);
    const int height = glutGet(GLUT_WINDOW_HEIGHT);
    beginScreenOverlay(width, height);
    drawOverlayPanel(55.0f, static_cast<float>(height - 150), static_cast<float>(width - 55), static_cast<float>(height - 55),
                     0.04f, 0.05f, 0.11f, 0.75f);
    glColor3f(1.0f, 0.95f, 0.45f);
    if (selectingPlayer == 1)
        drawBitmapText(85.0f, height - 95.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Choose Character for Player 1");
    else
        drawBitmapText(85.0f, height - 95.0f, GLUT_BITMAP_TIMES_ROMAN_24, "Choose Character for Player 2");
    glColor3f(0.88f, 0.93f, 1.0f);
    if (selectingPlayer == 1)
        drawBitmapText(85.0f, height - 125.0f, GLUT_BITMAP_HELVETICA_18, "Player 1: A/D to switch, ENTER to confirm.");
    else
        drawBitmapText(85.0f, height - 125.0f, GLUT_BITMAP_HELVETICA_18, "Player 2: LEFT/RIGHT to switch, ENTER to confirm.");

    const int activeIndex = selectingPlayer == 1 ? player1SelectedCharacterIndex : player2SelectedCharacterIndex;
    drawOverlayPanel(180.0f + activeIndex * 380.0f, 45.0f,
                     420.0f + activeIndex * 380.0f, 95.0f,
                     0.96f, 0.78f, 0.18f, 0.85f);
    glColor3f(0.07f, 0.06f, 0.04f);
    drawBitmapText(220.0f, 63.0f, GLUT_BITMAP_HELVETICA_18, "Twinkle-Twinkle");
    drawBitmapText(640.0f, 63.0f, GLUT_BITMAP_HELVETICA_18, "Hirono");
    endScreenOverlay();
}
