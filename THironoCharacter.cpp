#define M_PI 3.141592653589793
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
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

void getVariantTint(int variantIndex, float& tintR, float& tintG, float& tintB)
{
    switch (variantIndex % 2)
    {
        case 1:
            tintR = 0.82f; tintG = 0.93f; tintB = 1.15f;
            break;
        default:
            tintR = 1.0f; tintG = 1.0f; tintB = 1.0f;
            break;
    }
}

} // namespace

ProjectCharacter::ProjectCharacter()
    : ProjectCharacter("")
{
}

ProjectCharacter::ProjectCharacter(const std::string& prefix)
    : loaded(false)
    , baseCenterX(0.0f)
    , baseMinY(0.0f)
    , baseCenterZ(0.0f)
    , scaleFactor(1.0f)
    , positionX(0.0f)
    , positionZ(0.0f)
    , facingAngleDegrees(180.0f)
    , walkCycle(0.0f)
    , movementBlend(0.0f)
    , movingForward(false)
    , movingBackward(false)
    , movingLeft(false)
    , movingRight(false)
    , sprintEnabled(false)
    , activeVariant(0)
    , assetPrefix(prefix)
{
    body.loaded = false;
    leftHand.loaded = false;
    rightHand.loaded = false;
    leftFoot.loaded = false;
    rightFoot.loaded = false;
}

void ProjectCharacter::init()
{
    if (!loaded)
    {
        const std::string prefix = assetPrefix.empty() ? std::string("character") : assetPrefix;
        const bool loadedBody = loadFromFile(prefix + "_other.txt", body);
        const bool loadedLeftHand = loadFromFile(prefix + "_left_hand.txt", leftHand);
        const bool loadedRightHand = loadFromFile(prefix + "_right_hand.txt", rightHand);
        const bool loadedLeftFoot = loadFromFile(prefix + "_left_foot.txt", leftFoot);
        const bool loadedRightFoot = loadFromFile(prefix + "_right_foot.txt", rightFoot);

        loaded = loadedBody && loadedLeftHand && loadedRightHand && loadedLeftFoot && loadedRightFoot;
        if (loaded)
        {
            computeBounds();
            resetPosition();
        }
    }
}

void ProjectCharacter::setAssetPrefix(const std::string& prefix)
{
    assetPrefix = prefix;
    loaded = false;
    body.loaded = false;
    leftHand.loaded = false;
    rightHand.loaded = false;
    leftFoot.loaded = false;
    rightFoot.loaded = false;
}

bool ProjectCharacter::loadFromFile(const std::string& modelPath, ModelMesh& mesh)
{
    std::vector<std::string> candidates;
    candidates.push_back(modelPath);
    candidates.push_back(std::string("..\\") + modelPath);
    candidates.push_back(std::string("..\\..\\") + modelPath);
    candidates.push_back(std::string("bin\\Debug\\") + modelPath);

    std::ifstream input;
    for (std::size_t i = 0; i < candidates.size(); ++i)
    {
        input.open(candidates[i].c_str());
        if (input)
        {
            mesh.loadedPath = candidates[i];
            break;
        }
        input.clear();
    }

    if (!input)
    {
        std::cout << "[ProjectCharacter] Unable to find model file: " << modelPath << std::endl;
        return false;
    }

    mesh.vertices.clear();
    mesh.indices.clear();

    std::string firstLine;
    std::string secondLine;
    if (!std::getline(input, firstLine) || !std::getline(input, secondLine))
    {
        std::cout << "[ProjectCharacter] Model file is too short: "
                  << mesh.loadedPath << std::endl;
        return false;
    }

    std::istringstream vertexCountStream(firstLine);
    std::istringstream indexCountStream(secondLine);
    std::size_t numericVertexCount = 0;
    std::size_t numericIndexCount = 0;
    char trailingCheck = '\0';

    if ((vertexCountStream >> numericVertexCount) &&
        !(vertexCountStream >> trailingCheck) &&
        (indexCountStream >> numericIndexCount) &&
        !(indexCountStream >> trailingCheck))
    {
        mesh.vertices.reserve(numericVertexCount);
        mesh.indices.reserve(numericIndexCount * 3);

        for (std::size_t i = 0; i < numericVertexCount; ++i)
        {
            std::string line;
            if (!std::getline(input, line))
            {
                break;
            }

            std::istringstream vertexLine(line);
            Vertex vertex;
            vertex.nx = 0.0f;
            vertex.ny = 1.0f;
            vertex.nz = 0.0f;
            vertex.a = 1.0f;
            vertex.u = 0.0f;
            vertex.v = 0.0f;

            if (!(vertexLine >> vertex.x >> vertex.y >> vertex.z
                             >> vertex.r >> vertex.g >> vertex.b))
            {
                continue;
            }

            mesh.vertices.push_back(vertex);
        }

        for (std::size_t i = 0; i < numericIndexCount; ++i)
        {
            std::string line;
            if (!std::getline(input, line))
            {
                break;
            }

            std::istringstream indexLine(line);
            unsigned int primitiveSize = 0;
            unsigned int a = 0;
            unsigned int b = 0;
            unsigned int c = 0;
            if (!(indexLine >> primitiveSize >> a >> b >> c))
            {
                continue;
            }

            mesh.indices.push_back(a);
            mesh.indices.push_back(b);
            mesh.indices.push_back(c);
        }
    }
    else
    {
        input.close();
        input.open(mesh.loadedPath.c_str());

        std::string token;
        std::size_t expectedVertexCount = 0;
        std::size_t expectedIndexCount = 0;

        while (input >> token)
        {
            if (token == "vertex_count")
            {
                input >> expectedVertexCount;
                mesh.vertices.reserve(expectedVertexCount);
            }
            else if (token == "index_count")
            {
                input >> expectedIndexCount;
                mesh.indices.reserve(expectedIndexCount);
            }
            else if (token == "part_count")
            {
                std::size_t ignoredPartCount = 0;
                input >> ignoredPartCount;
            }
            else if (token == "vertices")
            {
                continue;
            }
            else if (token == "v")
            {
                Vertex vertex;
                input >> vertex.x >> vertex.y >> vertex.z
                      >> vertex.nx >> vertex.ny >> vertex.nz
                      >> vertex.r >> vertex.g >> vertex.b >> vertex.a
                      >> vertex.u >> vertex.v;
                mesh.vertices.push_back(vertex);
            }
            else if (token == "indices")
            {
                continue;
            }
            else if (token == "i")
            {
                unsigned int a = 0;
                unsigned int b = 0;
                unsigned int c = 0;
                input >> a >> b >> c;
                mesh.indices.push_back(a);
                mesh.indices.push_back(b);
                mesh.indices.push_back(c);
            }
        }
    }

    if (mesh.vertices.empty() || mesh.indices.empty())
    {
        mesh.vertices.clear();
        mesh.indices.clear();
        std::cout << "[ProjectCharacter] Model file found but contains no drawable geometry: "
                  << mesh.loadedPath << std::endl;
        return false;
    }

    mesh.loaded = true;
    std::cout << "[ProjectCharacter] Loaded " << mesh.vertices.size()
              << " vertices and " << mesh.indices.size() / 3
              << " triangles from " << mesh.loadedPath << std::endl;
    return true;
}

void ProjectCharacter::computeBounds()
{
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    const ModelMesh* meshes[] = { &body, &leftHand, &rightHand, &leftFoot, &rightFoot };
    for (std::size_t meshIndex = 0; meshIndex < 5; ++meshIndex)
    {
        const ModelMesh& mesh = *meshes[meshIndex];
        for (std::size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            const Vertex& vertex = mesh.vertices[i];
            if (vertex.x < minX) minX = vertex.x;
            if (vertex.y < minY) minY = vertex.y;
            if (vertex.z < minZ) minZ = vertex.z;
            if (vertex.x > maxX) maxX = vertex.x;
            if (vertex.y > maxY) maxY = vertex.y;
            if (vertex.z > maxZ) maxZ = vertex.z;
        }
    }

    baseCenterX = (minX + maxX) * 0.5f;
    baseMinY = minY;
    baseCenterZ = (minZ + maxZ) * 0.5f;

    const float sizeX = maxX - minX;
    const float sizeY = maxY - minY;
    const float sizeZ = maxZ - minZ;
    float maxSize = sizeX;
    if (sizeY > maxSize) maxSize = sizeY;
    if (sizeZ > maxSize) maxSize = sizeZ;

    if (maxSize < 0.0001f)
    {
        scaleFactor = 1.0f;
        return;
    }

    scaleFactor = 9.0f / maxSize;
}

void ProjectCharacter::setVariant(int variantIndex)
{
    activeVariant = variantIndex;
}

void ProjectCharacter::resetPosition()
{
    positionX = 0.0f;
    positionZ = 0.0f;
    facingAngleDegrees = 180.0f;
    walkCycle = 0.0f;
    movementBlend = 0.0f;
    movingForward = false;
    movingBackward = false;
    movingLeft = false;
    movingRight = false;
}

void ProjectCharacter::setMovingForward(bool active) { movingForward = active; }
void ProjectCharacter::setMovingBackward(bool active) { movingBackward = active; }
void ProjectCharacter::setMovingLeft(bool active) { movingLeft = active; }
void ProjectCharacter::setMovingRight(bool active) { movingRight = active; }
void ProjectCharacter::setSprintEnabled(bool enabled) { sprintEnabled = enabled; }

void ProjectCharacter::setPosition(float x, float z, float facingAngle)
{
    positionX = clampValue(x, -25.0f, 25.0f);
    positionZ = clampValue(z, -25.0f, 25.0f);
    facingAngleDegrees = facingAngle;
}

void ProjectCharacter::moveByInput(float inputX, float inputZ, float distanceScale)
{
    const float inputLength = std::sqrt(inputX * inputX + inputZ * inputZ);
    if (inputLength <= 0.001f)
    {
        return;
    }

    inputX /= inputLength;
    inputZ /= inputLength;

    const float stepDistance = 0.9f * distanceScale;
    positionX += inputX * stepDistance;
    positionZ += inputZ * stepDistance;
    positionX = clampValue(positionX, -25.0f, 25.0f);
    positionZ = clampValue(positionZ, -25.0f, 25.0f);
    facingAngleDegrees = std::atan2(inputX, inputZ) * 180.0f / static_cast<float>(M_PI);
    walkCycle += 0.75f;
    movementBlend = 1.0f;
}

void ProjectCharacter::update(float deltaSeconds)
{
    if (!loaded)
    {
        return;
    }

    float moveX = 0.0f;
    float moveZ = 0.0f;
    if (movingLeft)  moveX -= 1.0f;
    if (movingRight) moveX += 1.0f;
    if (movingForward) moveZ -= 1.0f;
    if (movingBackward) moveZ += 1.0f;

    const float inputLength = std::sqrt(moveX * moveX + moveZ * moveZ);
    const bool isMoving = inputLength > 0.001f;
    if (isMoving)
    {
        moveX /= inputLength;
        moveZ /= inputLength;
        const float moveSpeed = sprintEnabled ? 16.0f : 7.0f;
        positionX += moveX * moveSpeed * deltaSeconds;
        positionZ += moveZ * moveSpeed * deltaSeconds;
        positionX = clampValue(positionX, -25.0f, 25.0f);
        positionZ = clampValue(positionZ, -25.0f, 25.0f);
        facingAngleDegrees = std::atan2(moveX, moveZ) * 180.0f / static_cast<float>(M_PI);
        walkCycle += deltaSeconds * 11.0f;
        movementBlend = std::min(1.0f, movementBlend + deltaSeconds * 4.0f);
    }
    else
    {
        movementBlend = std::max(0.0f, movementBlend - deltaSeconds * 4.5f);
    }
}

void ProjectCharacter::drawMesh(const ModelMesh& mesh, float tintR, float tintG, float tintB) const
{
    glBegin(GL_TRIANGLES);
        for (std::size_t i = 0; i + 2 < mesh.indices.size(); i += 3)
        {
            if (mesh.indices[i] >= mesh.vertices.size() ||
                mesh.indices[i + 1] >= mesh.vertices.size() ||
                mesh.indices[i + 2] >= mesh.vertices.size())
            {
                continue;
            }

            const Vertex& a = mesh.vertices[mesh.indices[i]];
            const Vertex& b = mesh.vertices[mesh.indices[i + 1]];
            const Vertex& c = mesh.vertices[mesh.indices[i + 2]];

            const float abx = b.x - a.x;
            const float aby = b.y - a.y;
            const float abz = b.z - a.z;
            const float acx = c.x - a.x;
            const float acy = c.y - a.y;
            const float acz = c.z - a.z;
            float nx = aby * acz - abz * acy;
            float ny = abz * acx - abx * acz;
            float nz = abx * acy - aby * acx;
            const float normalLength = std::sqrt(nx * nx + ny * ny + nz * nz);
            if (normalLength > 0.0001f)
            {
                nx /= normalLength;
                ny /= normalLength;
                nz /= normalLength;
            }
            else
            {
                nx = 0.0f;
                ny = 1.0f;
                nz = 0.0f;
            }

            glColor4f(std::min(1.0f, a.r * tintR), std::min(1.0f, a.g * tintG), std::min(1.0f, a.b * tintB), 1.0f);
            glNormal3f(nx, ny, nz);
            glVertex3f(a.x, a.y, a.z);

            glColor4f(std::min(1.0f, b.r * tintR), std::min(1.0f, b.g * tintG), std::min(1.0f, b.b * tintB), 1.0f);
            glNormal3f(nx, ny, nz);
            glVertex3f(b.x, b.y, b.z);

            glColor4f(std::min(1.0f, c.r * tintR), std::min(1.0f, c.g * tintG), std::min(1.0f, c.b * tintB), 1.0f);
            glNormal3f(nx, ny, nz);
            glVertex3f(c.x, c.y, c.z);
        }
    glEnd();
}

void ProjectCharacter::drawInternal(float baseX, float baseY, float baseZ, float worldScale,
                                    float facingAngle, bool selectedOutline) const
{
    if (!loaded)
    {
        return;
    }

    GLboolean cullWasOn = GL_FALSE;
    GLboolean lightingWasOn = GL_FALSE;
    glGetBooleanv(GL_CULL_FACE, &cullWasOn);
    glGetBooleanv(GL_LIGHTING, &lightingWasOn);
    glDisable(GL_CULL_FACE);

    const float armSwing = std::sin(walkCycle) * 28.0f * movementBlend;
    const float legSwing = std::sin(walkCycle) * 14.0f * movementBlend;
    float tintR = 1.0f;
    float tintG = 1.0f;
    float tintB = 1.0f;
    getVariantTint(activeVariant, tintR, tintG, tintB);

    glPushMatrix();
        glTranslatef(baseX, baseY, baseZ);
        glScalef(worldScale, worldScale, worldScale);
        glRotatef(facingAngle, 0.0f, 1.0f, 0.0f);
        glTranslatef(-baseCenterX, -baseMinY + 0.02f, -baseCenterZ);

        if (selectedOutline)
        {
            glDisable(GL_LIGHTING);
            glColor3f(1.0f, 0.95f, 0.25f);
            glutWireCube(1.9f);
            if (lightingWasOn == GL_TRUE)
            {
                glEnable(GL_LIGHTING);
            }
        }

        drawMesh(body, tintR, tintG, tintB);

        glPushMatrix();
            glRotatef(armSwing, 1.0f, 0.0f, 0.0f);
            drawMesh(leftHand, tintR, tintG, tintB);
        glPopMatrix();

        glPushMatrix();
            glRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
            drawMesh(rightHand, tintR, tintG, tintB);
        glPopMatrix();

        glPushMatrix();
            glRotatef(-legSwing, 1.0f, 0.0f, 0.0f);
            drawMesh(leftFoot, tintR, tintG, tintB);
        glPopMatrix();

        glPushMatrix();
            glRotatef(legSwing, 1.0f, 0.0f, 0.0f);
            drawMesh(rightFoot, tintR, tintG, tintB);
        glPopMatrix();
    glPopMatrix();

    if (cullWasOn == GL_TRUE)
    {
        glEnable(GL_CULL_FACE);
    }
}

void ProjectCharacter::draw() const
{
    drawInternal(positionX, -4.0f, positionZ, scaleFactor, facingAngleDegrees, false);
}

void ProjectCharacter::drawPreview(float x, float y, float z, float facingAngle,
                                   float previewScale, bool selected) const
{
    drawInternal(x, y, z, scaleFactor * previewScale, facingAngle, selected);
}

const char* ProjectCharacter::getCharacterName() const
{
    if (assetPrefix == "character2")
    {
        return "Hirono";
    }
    return "Twinkle-Twinkle";
}

float ProjectCharacter::getPositionX() const
{
    return positionX;
}

float ProjectCharacter::getPositionZ() const
{
    return positionZ;
}

float ProjectCharacter::getFacingAngle() const
{
    return facingAngleDegrees;
}
