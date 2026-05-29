#ifndef YP_THIRONO_HPP
#define YP_THIRONO_HPP

#include "CGLabmain.hpp"
#include "THironoSkill.hpp"
#include "skill.hpp"
#include <string>
#include <vector>
#include <cmath>


namespace THirono {

enum GameScene {
    SCENE_HOME = 0,
    SCENE_SELECT = 1,
    SCENE_BATTLE = 2
};

class ArenaPlatform {
public:
    ArenaPlatform();
    void draw();
    void drawArena();
    void drawStars();
    void drawMeteors();
    void drawRoseBarrier();
    void drawSingleRose();
    void init();

private:
    struct MeteorState {
        float startX;
        float startY;
        float startZ;
        float dirX;
        float dirY;
        float dirZ;
        float speed;
        float length;
        float size;
        float lifeTime;
        float spawnTime;
        float headR;
        float headG;
        float headB;
    };

    GLuint arenaTextureId;
    bool arenaTextureReady;
    std::vector<MeteorState> meteors;

    void drawTexturedDisk(float radius, int slices, bool topFace) const;
    void spawnMeteor(MeteorState& meteor, float currentTime);
    bool loadArenaTexture(const std::string& texturePath);
};

class ProjectCharacter {
public:
    ProjectCharacter();
    explicit ProjectCharacter(const std::string& assetPrefix);
    void init();
    void draw() const;
    void drawPreview(float x, float y, float z, float facingAngle, float previewScale, bool selected) const;
    void setVariant(int variantIndex);
    void setAssetPrefix(const std::string& assetPrefix);
    void resetPosition();
    void setMovingForward(bool active);
    void setMovingBackward(bool active);
    void setMovingLeft(bool active);
    void setMovingRight(bool active);
    void moveByInput(float inputX, float inputZ, float distanceScale);
    void update(float deltaSeconds);
    const char* getCharacterName() const;
    float getPositionX() const;
    float getPositionZ() const;
    float getFacingAngle() const;

private:
    struct Vertex {
        float x;
        float y;
        float z;
        float nx;
        float ny;
        float nz;
        float r;
        float g;
        float b;
        float a;
        float u;
        float v;
    };

    struct ModelMesh {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::string loadedPath;
        bool loaded;
    };

    ModelMesh body;
    ModelMesh leftHand;
    ModelMesh rightHand;
    ModelMesh leftFoot;
    ModelMesh rightFoot;

    bool loaded;
    float baseCenterX;
    float baseMinY;
    float baseCenterZ;
    float scaleFactor;
    float positionX;
    float positionZ;
    float facingAngleDegrees;
    float walkCycle;
    float movementBlend;
    bool movingForward;
    bool movingBackward;
    bool movingLeft;
    bool movingRight;
    int activeVariant;
    std::string assetPrefix;

    bool loadFromFile(const std::string& modelPath, ModelMesh& mesh);
    void computeBounds();
    void drawInternal(float baseX, float baseY, float baseZ, float worldScale, float facingAngle, bool selectedOutline) const;
    void drawMesh(const ModelMesh& mesh, float tintR, float tintG, float tintB) const;
};

class MyVirtualWorld {
public:
    ArenaPlatform arena;
    SkillSystem skillSystem;
    ProjectCharacter player1Character;
    ProjectCharacter player2Character;
    GameScene currentScene;
    int player1SelectedCharacterIndex;
    int player2SelectedCharacterIndex;
    int selectingPlayer;
    int player1Health;
    int player2Health;
    int matchTimeRemainingMs;
    bool battlePaused;
    bool battleEnded;
    bool battleResultLogged;
    bool showHistory;
    std::string battleResultText;
    std::vector<std::string> battleHistoryEntries;
    int lastTickTimeMs;


    skill::MyVirtualWorld vfxWorldP1;
    skill::MyVirtualWorld vfxWorldP2;

    bool p1Skill1Active = false, p1Skill2Active = false;
    float p1Skill1Timer = 0.0f, p1Skill2Timer = 0.0f;
    float p1Skill1PosX = 0.0f, p1Skill2PosX = 0.0f;
    float p1Skill1PosZ = 0.0f, p1Skill2PosZ = 0.0f;
    float p1Skill1Angle = 0.0f, p1Skill2Angle = 0.0f;
    float p1Skill1CD = 0.0f, p1Skill2CD = 0.0f;
    float p1Skill1TickTimer = 0.0f;
    bool p1Skill2HasHit = false;

    bool p2Skill1Active = false, p2Skill2Active = false;
    float p2Skill1Timer = 0.0f, p2Skill2Timer = 0.0f;
    float p2Skill1PosX = 0.0f, p2Skill2PosX = 0.0f;
    float p2Skill1PosZ = 0.0f, p2Skill2PosZ = 0.0f;
    float p2Skill1Angle = 0.0f, p2Skill2Angle = 0.0f;
    float p2Skill1CD = 0.0f, p2Skill2CD = 0.0f;
    float p2Skill1TickTimer = 0.0f;
    bool p2Skill2HasHit = false;

    MyVirtualWorld();
    void draw();
    void tickTime();
    void init();
    bool handleKeyDown(unsigned char key);
    void handleKeyUp(unsigned char key);
    bool handleSpecialKey(int key);

private:
    void drawHomeScene();
    void drawCharacterSelectScene();
    void drawBattleScene();
    void resolveBattleIfNeeded();
    void applySkillDamage(bool attackerIsPlayer1, int skillIndex);
    void appendBattleHistory();
    void loadBattleHistory();
};

extern ProjectCharacter gCharacter1Preview;
extern ProjectCharacter gCharacter2Preview;
extern bool gPreviewCharactersInitialized;

};

#endif
