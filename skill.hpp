
#ifndef YP_CGLAB01_HPP
#define YP_CGLAB01_HPP

#include <GL/glut.h>
#include <string>
#include <vector>

using namespace std;

namespace skill {

class MyModelLoader
{
public:
    MyModelLoader() {}
    ~MyModelLoader() {}
    void load(string filename, float scale = 1.0);
    void draw();
private:
    vector<GLfloat> vertices;
    vector<int> faces;
    GLuint displayListId;
};

class MyVirtualWorld
{
public:

    MyModelLoader myVine;
    MyModelLoader myVineSpikes;
    MyModelLoader myVineRoses;
    MyModelLoader tornadoloader;
    MyModelLoader roseloader;
    MyModelLoader tornadoVineLoader;
    MyModelLoader appleLoader;
    MyModelLoader appleStarLoader;
    MyModelLoader treeLogLoader;
    MyModelLoader treeLeafLoader;
    MyModelLoader treeAppleLoader;float rad = p1Skill2Angle * 3.14159265f / 180.0f;
    MyModelLoader treeAppleStarLoader;

    float vfxAngle;
    float tornadoAngle;
    float roseAngle;
    float warningAlpha;
    float fallAngle;
    float fallSpeed;
    float treeScale;
    long int timeold, timenew, elapseTime;

    long int animationStartTime;
    void init();
    void resetSkill();
    void draw(bool attackerIsPlayer1, int skillIndex);

    void tickTime()
    {
       timenew    = glutGet(GLUT_ELAPSED_TIME);
       elapseTime = timenew - timeold;
       timeold    = timenew;

       vfxAngle += elapseTime * 0.1f;
       if (vfxAngle > 360.0f) vfxAngle -= 360.0f;

       tornadoAngle += elapseTime * 0.5f;
       if (tornadoAngle > 360.0f) tornadoAngle -= 360.0f;

       roseAngle += elapseTime * 0.02f;
       if (roseAngle > 360.0f) roseAngle -= 360.0f;
    }
};

}; // namespace CGLab01

#endif // YP_CGLAB01_HPP
