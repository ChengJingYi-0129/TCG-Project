#include <cmath>
#include "THirono.hpp"
#include "THironoSkill.hpp"
#include "skill.hpp"


using namespace THirono;

SkillSystem::SkillSystem()
{
}

SkillDefinition SkillSystem::getPlayer1Skill(int skillIndex) const
{
    SkillDefinition skill;
    if (skillIndex == 2)
    {
        skill.id = "p1_skill_h";
        skill.displayName = "Skill H";
        skill.animationFile = "skills/player1/skill_h.txt";
        skill.damage = 20;
        skill.range = 18.0f;
    }
    else
    {
        skill.id = "p1_skill_g";
        skill.displayName = "Skill G";
        skill.animationFile = "skills/player1/skill_g.txt";
        skill.damage = 12;
        skill.range = 12.0f;
    }
    return skill;
}

SkillDefinition SkillSystem::getPlayer2Skill(int skillIndex) const
{
    SkillDefinition skill;
    if (skillIndex == 2)
    {
        skill.id = "p2_skill_question";
        skill.displayName = "Skill ?";
        skill.animationFile = "skills/player2/skill_qm.txt";
        skill.damage = 20;
        skill.range = 18.0f;
    }
    else
    {
        skill.id = "p2_skill_gt";
        skill.displayName = "Skill >";
        skill.animationFile = "skills/player2/skill_gt.txt";
        skill.damage = 12;
        skill.range = 12.0f;
    }
    return skill;
}

bool SkillSystem::tryApplySkillDamage(bool attackerIsPlayer1,
                                      int skillIndex,
                                      const ProjectCharacter& attacker,
                                      const ProjectCharacter& defender,
                                      int& defenderHealth) const
{
    const SkillDefinition skill = attackerIsPlayer1
        ? getPlayer1Skill(skillIndex)
        : getPlayer2Skill(skillIndex);

    const float dx = attacker.getPositionX() - defender.getPositionX();
    const float dz = attacker.getPositionZ() - defender.getPositionZ();
    const float distance = std::sqrt(dx * dx + dz * dz);

    if (distance > skill.range)
    {
        return false;
    }

    defenderHealth -= skill.damage;
    if (defenderHealth < 0)
    {
        defenderHealth = 0;
    }
    return true;
}
