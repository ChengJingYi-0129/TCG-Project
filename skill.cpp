#include <GL/glut.h>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "skill.hpp"

using namespace skill;
using namespace std;

// 1. 计算法线（模型加载所需）
void computeSurfaceNormal(GLfloat vertex1[], GLfloat vertex2[], GLfloat vertex3[], GLfloat normal[])
{
   GLfloat v1[3], v2[3];
   v1[0] = vertex3[0] - vertex2[0]; v1[1] = vertex3[1] - vertex2[1]; v1[2] = vertex3[2] - vertex2[2];
   v2[0] = vertex1[0] - vertex2[0]; v2[1] = vertex1[1] - vertex2[1]; v2[2] = vertex1[2] - vertex2[2];

   normal[0] = v1[1]*v2[2] - v1[2]*v2[1];
   normal[1] = v1[2]*v2[0] - v1[0]*v2[2];
   normal[2] = v1[0]*v2[1] - v1[1]*v2[0];

   GLfloat magnitude = sqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
   if (magnitude > 0) { normal[0] /= magnitude; normal[1] /= magnitude; normal[2] /= magnitude; }
}

// 2. 加载模型
void MyModelLoader::load(string filename, float scale)
{
   int numberOfVertices, numberOfFaces;
   ifstream fin(filename.c_str());
   if (fin.good()) {
      fin >> numberOfVertices >> numberOfFaces;
      int varraysize = numberOfVertices * 3;
      vertices.reserve(varraysize); faces.reserve(numberOfFaces * 4);
      float coord;
      for (int i=0; i<varraysize && fin.good(); i++) { fin >> coord; vertices.push_back(coord * scale); }
      int vcount, index;
      for (int i=0; i<numberOfFaces && fin.good(); ++i) {
         fin >> vcount; faces.push_back(vcount);
         for (int j=0; j<vcount && fin.good(); ++j) { fin >> index; faces.push_back(index); }
      }
      fin.close();
   }

   vector<int>::iterator itr;
   GLfloat normal[3];
   displayListId = glGenLists(1);
   glNewList(displayListId, GL_COMPILE);
      int vcount;
      for (itr = faces.begin(); itr!=faces.end();) {
         vcount = *itr; itr++;
         if (vcount >= 3) {
             int i1 = (*itr)*3, i2 = (*(itr+1))*3, i3 = (*(itr+2))*3;
             computeSurfaceNormal(&vertices[i1], &vertices[i2], &vertices[i3], normal);
             glBegin(GL_POLYGON);
                glNormal3fv(normal);
                for (int k=0; k<vcount; k++) { glVertex3fv(&vertices[(*itr)*3]); itr++; }
             glEnd();
         } else { itr += vcount; }
      }
   glEndList();
}

void MyModelLoader::draw() { glCallList(displayListId); }

// 3. 特效初始化
void MyVirtualWorld::init()
{
   myVine.load("data/vine.txt", 1.0);
   myVineSpikes.load("data/vine_spikes.txt", 1.0);
   myVineRoses.load("data/vine_roses.txt", 1.0);
   tornadoloader.load("data/tornado_2.txt", 3.0);
   roseloader.load("data/tornado_rose.txt", 3.0);
   tornadoVineLoader.load("data/tornado_vine.txt", 3.0);
   appleLoader.load("data/apple.txt", 1.0);
   appleStarLoader.load("data/apple_star.txt", 1.0);
   treeLogLoader.load("data/tree_log.txt", 1.0);
   treeLeafLoader.load("data/tree_leaf.txt", 1.0);
   treeAppleLoader.load("data/tree_apple.txt", 1.0);
   treeAppleStarLoader.load("data/tree_apple_star.txt", 1.0);

   vfxAngle = tornadoAngle = roseAngle = 0.0f;
   warningAlpha = fallAngle = fallSpeed = treeScale = 0.0f;
   timeold = glutGet(GLUT_ELAPSED_TIME);
}

// 4. 特效分支绘制
void MyVirtualWorld::draw(bool attackerIsPlayer1, int skillIndex)
{
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    float currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    if (attackerIsPlayer1)
    {
        if (skillIndex != 2) // P1 基础：散射藤蔓
        {
            for (int j = 0; j < 5; j++) {
                glPushMatrix();
                    glRotatef(-22.5f + (j * 11.25f), 0.0f, 1.0f, 0.0f);
                    for (int i = 0; i < 5; i++) {
                        glPushMatrix();
                            float animPhase = max(0.0f, float(sin(currentTime * 5.0f - i * 0.4f)));
                            float thrustDist = animPhase * 4.0f;
                            glTranslatef(0.0f, thrustDist * 0.7f, i * 1.5f + thrustDist * 0.7f);
                            glRotatef(-140.0f, 1.0f, 0.0f, 0.0f);
                            glColor3f(0.1f, 0.5f, 0.1f); myVine.draw();
                            glColor3f(0.6f, 0.4f, 0.2f); myVineSpikes.draw();
                            glColor3f(0.9f, 0.1f, 0.3f); myVineRoses.draw();
                        glPopMatrix();
                    }
                glPopMatrix();
            }
        }
        else // P1 大招：龙卷风合体
        {
            glPushMatrix();
                glRotatef(tornadoAngle, 0.0f, 1.0f, 0.0f);
                glColor3f(0.6f, 0.8f, 0.9f); tornadoloader.draw();
                glColor3f(0.1f, 0.5f, 0.1f); tornadoVineLoader.draw();
                glColor3f(0.9f, 0.2f, 0.2f); roseloader.draw();
            glPopMatrix();
        }
        treeScale = fallAngle = fallSpeed = 0.0f; // 重置 P2 状态机
    }
    else
    {
        if (skillIndex != 2) // P2 基础：流星雨
        {
            glPushMatrix();
                glTranslatef(0.0f, 0.0f, 7.0f);
                glDisable(GL_LIGHTING); glLineWidth(3.0f); glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
                glBegin(GL_LINE_LOOP);
                for (int i = 0; i < 360; i += 10) glVertex3f(cosf(i * 3.14159f / 180.0f) * 3.0f, 0.01f, sinf(i * 3.14159f / 180.0f) * 3.0f);
                glEnd(); glEnable(GL_LIGHTING);

                glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                for (int p = 0; p < 15; p++) {
                    float targetX = cosf(fmodf(p * 73.17f, 6.28f)) * fmodf(p * 4.33f, 3.0f);
                    float targetZ = sinf(fmodf(p * 73.17f, 6.28f)) * fmodf(p * 4.33f, 3.0f);
                    float startY = 18.0f + fmodf(p * 3.5f, 6.0f);
                    float randomSpeed = 18.0f + fmodf(p * 4.7f, 10.0f);

                    for (int t = 0; t < 3; t++) {
                        float progress = fmod(currentTime * randomSpeed + (p * 1.5f), 28.0f) - (t * 0.05f * randomSpeed);
                        if (progress < 0.0f) continue;
                        float factor = min(1.0f, progress / startY);
                        float currentY = max(0.0f, startY - progress);
                        if (currentY <= 0.0f && t > 0) continue;

                        glPushMatrix();
                        glTranslatef(targetX + sinf(p * 12.3f) * 4.0f * (1.0f - factor), currentY, targetZ + 3.0f + (targetZ - targetZ - 3.0f) * factor);
                        glRotatef(currentTime * 350.0f + (p * 30.0f), 1.0f, 0.5f, 0.2f);

                        float trailScale = 0.28f * (1.0f - t * 0.3f);
                        float colorFade = 1.0f - (t * 0.35f);
                        if (currentY == 0.0f && factor >= 1.0f) {
                            float groundTime = (progress - startY) * 0.2f;
                            if (groundTime < 1.0f) { trailScale *= (1.0f + groundTime * 4.0f); colorFade *= (1.0f - groundTime); }
                            else colorFade = 0.0f;
                        }

                        if (colorFade > 0.0f) {
                            glDepthMask(GL_TRUE); glPushMatrix(); glScalef(2.0f*trailScale, 2.0f*trailScale, 2.0f*trailScale); glColor4f(1.0f, 0.1f, 0.2f, colorFade); appleLoader.draw(); glPopMatrix();
                            glDepthMask(GL_FALSE); glPushMatrix(); glScalef(1.83f*trailScale, 1.83f*trailScale, 1.83f*trailScale); glDisable(GL_CULL_FACE); glColor4f(colorFade, 0.9f*colorFade, 0.1f*colorFade, colorFade); appleStarLoader.draw(); glEnable(GL_CULL_FACE); glPopMatrix();
                        }
                        glPopMatrix();
                    }
                }
                glDepthMask(GL_TRUE); glDisable(GL_BLEND);
            glPopMatrix();
        }
        else // P2 大招：苹果树砸下
        {
            if (treeScale < 1.0f) { treeScale += 0.04f; if (treeScale > 1.0f) treeScale = 1.0f; fallAngle = 0.0f; warningAlpha = treeScale * 0.9f; }
            else { if (fallAngle < 90.0f) { fallSpeed += 0.04f; fallAngle += fallSpeed; warningAlpha = 0.9f * (1.0f - (fallAngle / 90.0f)); } else { fallAngle = 90.0f; warningAlpha = 0.0f; } }

            if (warningAlpha > 0.0f) {
                glPushMatrix(); glTranslatef(0.0f, 0.02f, 0.0f); glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); glDisable(GL_LIGHTING);
                glColor4f(1.0f, 0.0f, 0.0f, warningAlpha); glBegin(GL_QUADS); glVertex3f(-4.0f, 0.0f, 2.0f); glVertex3f(4.0f, 0.0f, 2.0f); glVertex3f(4.0f, 0.0f, -18.0f); glVertex3f(-4.0f, 0.0f, -18.0f); glEnd();
                glLineWidth(4.0f); glColor4f(1.0f, 0.2f, 0.2f, warningAlpha + 0.1f); glBegin(GL_LINE_LOOP); glVertex3f(-4.0f, 0.0f, 2.0f); glVertex3f(4.0f, 0.0f, 2.0f); glVertex3f(4.0f, 0.0f, -18.0f); glVertex3f(-4.0f, 0.0f, -18.0f); glEnd();
                glEnable(GL_LIGHTING); glDisable(GL_BLEND); glPopMatrix();
            }

            glPushMatrix();
                glRotatef(-fallAngle, 1.0f, 0.0f, 0.0f); glScalef(treeScale, treeScale, treeScale); glRotatef(90.0f, 1.0f, 0.0f, 0.0f); glScalef(2.0f, 2.0f, 2.0f);
                glColor3f(0.45f, 0.25f, 0.08f); treeLogLoader.draw();
                glDisable(GL_CULL_FACE); glColor3f(0.15f, 0.6f, 0.15f); treeLeafLoader.draw(); glColor3f(1.0f, 0.1f, 0.2f); treeAppleLoader.draw(); glColor3f(1.0f, 0.9f, 0.1f); treeAppleStarLoader.draw(); glEnable(GL_CULL_FACE);
            glPopMatrix();
        }
    }
    glDisable(GL_COLOR_MATERIAL);
}
