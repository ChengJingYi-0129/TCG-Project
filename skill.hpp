
#ifndef YP_CGLAB01_HPP
#define YP_CGLAB01_HPP

#include <GL/glut.h>
#include <string>
#include <vector>

using namespace std;

namespace skill {

// 模型加载器
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

// 特效世界主类
class MyVirtualWorld
{
public:
    // 特效模型库
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
    MyModelLoader treeAppleLoader;
    MyModelLoader treeAppleStarLoader;

    // 动画控制变量
    float vfxAngle;
    float tornadoAngle;
    float roseAngle;
    float warningAlpha;
    float fallAngle;
    float fallSpeed;
    float treeScale;
    long int timeold, timenew, elapseTime;

    void init();
    // 核心渲染接口，接收技能释放者与技能序号
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
