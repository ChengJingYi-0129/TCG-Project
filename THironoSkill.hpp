#ifndef YP_THIRONO_SKILL_HPP
#define YP_THIRONO_SKILL_HPP

#include <string>

namespace THirono {

struct SkillDefinition {
    std::string id;
    std::string displayName;
    std::string animationFile;
    int damage;
    float range;
};

class ProjectCharacter;

class SkillSystem {
public:
    SkillSystem();

    SkillDefinition getCharacterSkill(int characterIndex, int skillIndex) const;
    SkillDefinition getPlayer1Skill(int skillIndex) const;
    SkillDefinition getPlayer2Skill(int skillIndex) const;
    bool tryApplySkillDamage(bool attackerIsPlayer1,
                             int skillIndex,
                             const ProjectCharacter& attacker,
                             const ProjectCharacter& defender,
                             int& defenderHealth) const;
};

}

#endif
