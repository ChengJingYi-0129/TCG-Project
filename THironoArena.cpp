#define M_PI 3.141592653589793
#include <GL/glut.h>
#include <windows.h>
#include <gdiplus.h>
#include <cmath>
#include <ctime>
#include <vector>
#include "THirono.hpp"
#include "skill.hpp"

using namespace THirono;
using namespace Gdiplus;

namespace {

float randomRange(float minimumValue, float maximumValue)
{
    const float unit = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
    return minimumValue + (maximumValue - minimumValue) * unit;
}

class GdiPlusSession {
public:
    GdiPlusSession()
        : token(0)
    {
        GdiplusStartupInput startupInput;
        GdiplusStartup(&token, &startupInput, NULL);
    }

    ~GdiPlusSession()
    {
        if (token != 0)
        {
            GdiplusShutdown(token);
        }
    }

private:
    ULONG_PTR token;
};

} // namespace

ArenaPlatform::ArenaPlatform()
    : arenaTextureId(0)
    , arenaTextureReady(false)
{
}

void ArenaPlatform::init()
{
    static bool seeded = false;
    if (!seeded)
    {
        srand(static_cast<unsigned int>(time(NULL)));
        seeded = true;
    }

    if (!arenaTextureReady)
    {
        arenaTextureReady = loadArenaTexture("699pic_1ikoqx_xy.jpg");
    }

    if (meteors.empty())
    {
        const float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;
        meteors.resize(5);
        for (size_t i = 0; i < meteors.size(); ++i)
        {
            spawnMeteor(meteors[i], currentTime - randomRange(0.0f, 3.5f));
        }
    }
}

void ArenaPlatform::draw()
{
    drawStars();
    drawMeteors();
    drawArena();
}

void ArenaPlatform::drawArena()
{
    const GLfloat topRadius = 32.2f;
    const GLfloat bodyRadius = 42.9f;
    const GLfloat bodyRadiusY = 30.4f;
    const GLfloat topCutY = 19.7f;
    const GLint slices = 64;
    const GLint stacks = 24;

    GLboolean lightingWasOn = GL_FALSE;
    GLboolean cullWasOn = GL_FALSE;
    glGetBooleanv(GL_LIGHTING, &lightingWasOn);
    glGetBooleanv(GL_CULL_FACE, &cullWasOn);

    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);

    GLUquadric* quadric = gluNewQuadric();
    if (quadric == NULL)
    {
        if (cullWasOn == GL_TRUE)
        {
            glEnable(GL_CULL_FACE);
        }
        if (lightingWasOn == GL_TRUE)
        {
            glEnable(GL_LIGHTING);
        }
        return;
    }

    gluQuadricNormals(quadric, GLU_SMOOTH);
    gluQuadricTexture(quadric, GL_TRUE);

    glPushMatrix();
        glTranslatef(0.0f, -4.0f, 0.0f);

        if (arenaTextureReady)
        {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, arenaTextureId);
        }

        glColor3f(1.0f, 1.0f, 1.0f);
        drawTexturedDisk(topRadius, slices, true);

        glColor3f(1.0f, 1.0f, 1.0f);
        for (int stack = 0; stack < stacks; ++stack)
        {
            const float t0 = static_cast<float>(stack) / static_cast<float>(stacks);
            const float t1 = static_cast<float>(stack + 1) / static_cast<float>(stacks);
            const float yRel0 = topCutY - (topCutY + bodyRadiusY) * t0;
            const float yRel1 = topCutY - (topCutY + bodyRadiusY) * t1;
            const float radialFactor0 = std::max(0.0f, 1.0f - (yRel0 * yRel0) / (bodyRadiusY * bodyRadiusY));
            const float radialFactor1 = std::max(0.0f, 1.0f - (yRel1 * yRel1) / (bodyRadiusY * bodyRadiusY));
            const float r0 = bodyRadius * std::sqrt(radialFactor0);
            const float r1 = bodyRadius * std::sqrt(radialFactor1);
            const float y0 = yRel0 - topCutY;
            const float y1 = yRel1 - topCutY;

            glBegin(GL_QUAD_STRIP);
                for (int slice = 0; slice <= slices; ++slice)
                {
                    const float angle = static_cast<float>(slice) / static_cast<float>(slices) * static_cast<float>(2.0 * M_PI);
                    const float cosA = std::cos(angle);
                    const float sinA = std::sin(angle);
                    const float u = static_cast<float>(slice) / static_cast<float>(slices);
                    const float v0 = 0.15f + t0 * 0.85f;
                    const float v1 = 0.15f + t1 * 0.85f;

                    glTexCoord2f(u, v0);
                    glVertex3f(cosA * r0, y0, sinA * r0);
                    glTexCoord2f(u, v1);
                    glVertex3f(cosA * r1, y1, sinA * r1);
                }
            glEnd();
        }

        if (arenaTextureReady)
        {
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_TEXTURE_2D);
        }
    glPopMatrix();

    gluDeleteQuadric(quadric);

    if (cullWasOn == GL_TRUE)
    {
        glEnable(GL_CULL_FACE);
    }
    if (lightingWasOn == GL_TRUE)
    {
        glEnable(GL_LIGHTING);
    }
}

void ArenaPlatform::drawTexturedDisk(float radius, int slices, bool topFace) const
{
    const float step = static_cast<float>(2.0 * M_PI / slices);

    glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0.0f, topFace ? 1.0f : -1.0f, 0.0f);
        glTexCoord2f(0.5f, 0.5f);
        glVertex3f(0.0f, 0.0f, 0.0f);

        for (int i = 0; i <= slices; ++i)
        {
            const float angle = i * step;
            const float x = cos(angle) * radius;
            const float z = sin(angle) * radius;
            const float u = 0.5f + cos(angle) * 0.5f;
            const float v = 0.5f + sin(angle) * 0.5f;

            glTexCoord2f(u, v);
            if (topFace)
            {
                glVertex3f(x, 0.0f, z);
            }
            else
            {
                glVertex3f(x, 0.0f, -z);
            }
        }
    glEnd();
}

void ArenaPlatform::drawStars()
{
    glPointSize(1.0);
    glColor3f(1.0f, 1.0f, 1.0f);
    static std::vector<float> stars;

    if (stars.empty())
    {
        unsigned int starSeed = 1u;
        stars.resize(1000 * 3);
        for (size_t i = 0; i < stars.size(); i += 3)
        {
            starSeed = starSeed * 1664525u + 1013904223u;
            stars[i + 0] = (starSeed % 10000) / 10000.0f * 200.0f - 100.0f;
            starSeed = starSeed * 1664525u + 1013904223u;
            stars[i + 1] = (starSeed % 10000) / 10000.0f * 200.0f - 100.0f;
            starSeed = starSeed * 1664525u + 1013904223u;
            stars[i + 2] = (starSeed % 10000) / 10000.0f * 200.0f - 100.0f;
        }
    }

    glBegin(GL_POINTS);
        for (size_t i = 0; i < stars.size(); i += 3)
        {
            glVertex3f(stars[i + 0], stars[i + 1], stars[i + 2]);
        }
    glEnd();
}

void ArenaPlatform::drawMeteors()
{
    if (meteors.empty())
    {
        init();
    }

    const float currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

    GLboolean lightingWasOn = GL_FALSE;
    GLboolean cullWasOn = GL_FALSE;
    GLboolean blendWasOn = GL_FALSE;
    glGetBooleanv(GL_LIGHTING, &lightingWasOn);
    glGetBooleanv(GL_CULL_FACE, &cullWasOn);
    glGetBooleanv(GL_BLEND, &blendWasOn);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (size_t i = 0; i < meteors.size(); ++i)
    {
        MeteorState& meteor = meteors[i];
        float age = currentTime - meteor.spawnTime;
        if (age > meteor.lifeTime)
        {
            spawnMeteor(meteor, currentTime);
            age = 0.0f;
        }

        const float x = meteor.startX + meteor.dirX * meteor.speed * age;
        const float y = meteor.startY + meteor.dirY * meteor.speed * age;
        const float z = meteor.startZ + meteor.dirZ * meteor.speed * age;

        glLineWidth(meteor.size * 5.0f);
        glBegin(GL_LINE_STRIP);
            for (int segment = 6; segment >= 0; --segment)
            {
                const float t = static_cast<float>(segment) / 6.0f;
                const float tailDistance = meteor.length * t;
                const float alpha = 0.08f + 0.55f * (1.0f - t);
                glColor4f(1.0f, 0.75f + 0.2f * (1.0f - t), 0.25f, alpha);
                glVertex3f(x - meteor.dirX * tailDistance,
                           y - meteor.dirY * tailDistance,
                           z - meteor.dirZ * tailDistance);
            }
        glEnd();

        glPushMatrix();
            glTranslatef(x, y, z);
            glColor4f(meteor.headR, meteor.headG, meteor.headB, 0.95f);
            glutSolidSphere(meteor.size, 14, 14);
        glPopMatrix();
    }

    if (blendWasOn == GL_FALSE)
    {
        glDisable(GL_BLEND);
    }
    if (cullWasOn == GL_TRUE)
    {
        glEnable(GL_CULL_FACE);
    }
    if (lightingWasOn == GL_TRUE)
    {
        glEnable(GL_LIGHTING);
    }
}

void ArenaPlatform::spawnMeteor(MeteorState& meteor, float currentTime)
{
    const float edgeChoice = randomRange(0.0f, 4.0f);

    if (edgeChoice < 1.0f)
    {
        meteor.startX = randomRange(-45.0f, -28.0f);
        meteor.startZ = randomRange(-38.0f, 38.0f);
    }
    else if (edgeChoice < 2.0f)
    {
        meteor.startX = randomRange(28.0f, 45.0f);
        meteor.startZ = randomRange(-38.0f, 38.0f);
    }
    else if (edgeChoice < 3.0f)
    {
        meteor.startX = randomRange(-40.0f, 40.0f);
        meteor.startZ = randomRange(-45.0f, -28.0f);
    }
    else
    {
        meteor.startX = randomRange(-40.0f, 40.0f);
        meteor.startZ = randomRange(28.0f, 45.0f);
    }

    meteor.startY = randomRange(16.0f, 34.0f);

    const float targetX = randomRange(-8.0f, 8.0f);
    const float targetY = randomRange(1.0f, 8.0f);
    const float targetZ = randomRange(-8.0f, 8.0f);

    meteor.dirX = targetX - meteor.startX;
    meteor.dirY = targetY - meteor.startY;
    meteor.dirZ = targetZ - meteor.startZ;

    const float directionLength = sqrt(meteor.dirX * meteor.dirX +
                                       meteor.dirY * meteor.dirY +
                                       meteor.dirZ * meteor.dirZ);

    if (directionLength > 0.0001f)
    {
        meteor.dirX /= directionLength;
        meteor.dirY /= directionLength;
        meteor.dirZ /= directionLength;
    }

    meteor.speed = randomRange(11.0f, 16.0f);
    meteor.length = randomRange(1.6f, 3.2f);
    meteor.size = randomRange(0.08f, 0.16f);
    meteor.lifeTime = randomRange(1.8f, 3.6f);
    meteor.spawnTime = currentTime;
    meteor.headR = 1.0f;
    meteor.headG = randomRange(0.72f, 0.9f);
    meteor.headB = randomRange(0.28f, 0.5f);
}

bool ArenaPlatform::loadArenaTexture(const std::string& texturePath)
{
    static GdiPlusSession gdiPlusSession;

    std::wstring widePath(texturePath.begin(), texturePath.end());
    Bitmap bitmap(widePath.c_str());
    if (bitmap.GetLastStatus() != Ok)
    {
        return false;
    }

    BitmapData bitmapData;
    Rect lockRect(0, 0, bitmap.GetWidth(), bitmap.GetHeight());
    if (bitmap.LockBits(&lockRect, ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) != Ok)
    {
        return false;
    }

    const int width = bitmap.GetWidth();
    const int height = bitmap.GetHeight();
    std::vector<unsigned char> pixels(width * height * 4);

    for (int y = 0; y < height; ++y)
    {
        const unsigned char* srcRow =
            static_cast<const unsigned char*>(bitmapData.Scan0) + y * bitmapData.Stride;

        for (int x = 0; x < width; ++x)
        {
            const int srcIndex = x * 4;
            const int dstIndex = ((height - 1 - y) * width + x) * 4;

            pixels[dstIndex + 0] = srcRow[srcIndex + 2];
            pixels[dstIndex + 1] = srcRow[srcIndex + 1];
            pixels[dstIndex + 2] = srcRow[srcIndex + 0];
            pixels[dstIndex + 3] = srcRow[srcIndex + 3];
        }
    }

    bitmap.UnlockBits(&bitmapData);

    if (arenaTextureId == 0)
    {
        glGenTextures(1, &arenaTextureId);
    }

    glBindTexture(GL_TEXTURE_2D, arenaTextureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

void ArenaPlatform::drawSingleRose()
{
}

void ArenaPlatform::drawRoseBarrier()
{
}
