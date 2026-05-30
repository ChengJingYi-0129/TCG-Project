#include <GL/glut.h>
#include <string>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "skill.hpp"

using namespace skill;
using namespace std;


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


void MyVirtualWorld::resetSkill()
{
    vfxAngle = tornadoAngle = roseAngle = 0.0f;
    warningAlpha = fallAngle = fallSpeed = treeScale = 0.0f;

    timeold = glutGet(GLUT_ELAPSED_TIME);
    animationStartTime = glutGet(GLUT_ELAPSED_TIME);
}



void MyVirtualWorld::init()
{
   myVine.load("data/vine.txt", 1.0);
   myVineLeaf.load("data/vine_leaf.txt", 1.0);
   tornadoloader.load("data/tornado_2.txt", 3.0);
   roseloader.load("data/tornado_rose.txt", 3.0);
   tornadoVineLoader.load("data/tornado_vine.txt", 3.0);
   appleLoader.load("data/apple.txt", 1.0);
   appleStarLoader.load("data/apple_star.txt", 1.0);
   treeLogLoader.load("data/tree_log.txt", 1.0);
   treeLeafLoader.load("data/tree_leaf.txt", 1.0);
   treeAppleLoader.load("data/tree_apple.txt", 1.0);

   vfxAngle = tornadoAngle = roseAngle = 0.0f;
   warningAlpha = fallAngle = fallSpeed = treeScale = 0.0f;
   timeold = glutGet(GLUT_ELAPSED_TIME);
   resetSkill();
}



void MyVirtualWorld::draw(bool attackerIsPlayer1, int skillIndex)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    float currentTime = (glutGet(GLUT_ELAPSED_TIME) - animationStartTime) / 1000.0f;

    if (!attackerIsPlayer1)
    {
       if (skillIndex != 2) // P2 vine_spike
        {
            float startX = -3.6f;
            float endX = 3.6f;
            float startZ = -1.5f;
            float endZ = 15.5f;


            int vineCountX = 15;
            int vineCountZ = 25;

            glDisable(GL_CULL_FACE);

            for(int i = 0; i < vineCountX; i++) {
                for(int j = 0; j < vineCountZ; j++) {
                    glPushMatrix();

                    float posX = startX + (endX - startX) * ((float)i / (vineCountX - 1));
                    float posZ = startZ + (endZ - startZ) * ((float)j / (vineCountZ - 1));

                    float seed = (float)i * 17.3f + (float)j * 29.1f;

                    posX += sin(seed * 4.3f) * 0.25f;
                    posZ += cos(seed * 3.7f) * 0.5f;

                    // 3. 基础缩放 (生成大小不一的随机感)
                    float baseScale = 1.0f + fabs(sin(seed * 5.1f)) * 1.5f;

                    float t = currentTime * 5.0f;
                    float phase = (float)i * 3.14159f + (float)j * 0.3f + sin(seed) * 1.5f;
                    float whipAngle = sin(t + phase) * 35.0f;
                    float sideSway = cos(t * 0.8f + phase) * 4.0f;
                    float dynamicStretch = 1.0f + sin(t + phase + 1.57f) * 0.15f;

                    glTranslatef(posX, 0.0f, posZ);

                    glRotatef(whipAngle, 1.0f, 0.0f, 0.0f);
                    glRotatef(sideSway, 0.0f, 0.0f, 1.0f);

                    // --- 默认模型变换 ---
                    glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
                    glTranslatef(0.0f, 0.5f, 0.0f);
                    // -------------------------

                    // ==========================================
                    // 🌟 【核心修改：整体模型巨大化】 🌟
                    // 觉得不够大就改成 4.0f、5.0f，觉得太大就改成 2.0f
                    float modelSizeMultiplier = 3.0f;

                    // 应用缩放（基础大小 * 甩动拉伸 * 巨大化倍数）
                    glScalef(baseScale * modelSizeMultiplier,
                             dynamicStretch * baseScale * modelSizeMultiplier,
                             baseScale * modelSizeMultiplier);
                    // ==========================================

                    // 双模型上色渲染
                    // 1. 画藤蔓主体（极暗的深棕/黑色）
                    glColor3f(0.09f, 0.07f, 0.06f);
                    myVine.draw();

                    // 2. 画藤蔓的叶子（深邃的暗绿色）
                    glColor3f(0.15f, 0.35f, 0.15f);
                    myVineLeaf.draw();

                    glPopMatrix();
                }
            }

            // 画完恢复背面剔除状态
            glEnable(GL_CULL_FACE);
        }
        else // P2 vines_tornado
        {
            glPushMatrix();
                glScalef(2.0f, 2.0f, 2.0f);
                glRotatef(tornadoAngle, 0.0f, 1.0f, 0.0f);

                glColor3f(0.7f, 0.7f, 0.7f); tornadoloader.draw();
                glColor3f(0.1f, 0.5f, 0.1f); tornadoVineLoader.draw();
                glColor3f(0.9f, 0.2f, 0.2f); roseloader.draw();
            glPopMatrix();
        }
    }
    else
    {
        if (skillIndex != 2) // P1 apple_star
        {
            glPushMatrix();
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
        else // P1 apple_tree
        {
            if (treeScale < 1.0f) {
                treeScale += 0.8f;
                if (treeScale > 1.0f) treeScale = 1.0f;
                fallAngle = 0.0f;
            }
            else {
                if (fallAngle < 90.0f) {
                    fallSpeed += 1.0f;
                    fallAngle += fallSpeed;
                } else {
                    fallAngle = 90.0f;
                }
            }

            glPushMatrix();

                glRotatef(fallAngle, 1.0f, 0.0f, 0.0f);
                glScalef(treeScale, treeScale, treeScale);
                glScalef(0.3f, 0.3f, 0.3f);

                glPushMatrix();
                    glColor3f(0.45f, 0.25f, 0.08f);
                    treeLogLoader.draw();
                glPopMatrix();

                glPushMatrix();
                    glPushMatrix();
                        glScalef(1.0f, 1.0f, 1.0f);
                        glDisable(GL_CULL_FACE);
                        glColor3f(0.15f, 0.6f, 0.15f);
                        treeLeafLoader.draw();
                        glEnable(GL_CULL_FACE);
                    glPopMatrix();

                    glPushMatrix();
                        glScalef(2.5f, 2.5f, 2.5f);
                        glTranslatef(0.5f, 0.5f, 0.0f);
                        glDisable(GL_CULL_FACE);
                        glColor3f(1.0f, 0.1f, 0.2f);
                        treeAppleLoader.draw();
                        glEnable(GL_CULL_FACE);
                    glPopMatrix();
                glPopMatrix();
            glPopMatrix();
        }
    }
    glPopAttrib();
}
