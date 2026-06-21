#include <cmath>
#include "THirono.hpp"
#include "THironoSkill.hpp"
#include "skill.hpp"


using namespace THirono;

SkillSystem::SkillSystem()
{
}

SkillDefinition SkillSystem::getCharacterSkill(int characterIndex, int skillIndex) const
{
    SkillDefinition skill;
    const bool useCharacter1 = (characterIndex == 0);

    if (skillIndex == 2)
    {
        if (useCharacter1)
        {
            skill.displayName = "Tree";
            skill.damage = 30;
            skill.range = 18.0f;
        }
        else
        {
            skill.displayName = "Tornado";
            skill.damage = 30;
            skill.range = 18.0f;
        }
    }
    else
    {
        if (useCharacter1)
        {
            skill.displayName = "Apple";
            skill.damage = 25;
            skill.range = 12.0f;
        }
        else
        {
            skill.displayName = "Vine";
            skill.damage = 16;
            skill.range = 12.0f;
        }
    }

    return skill;
}

SkillDefinition SkillSystem::getPlayer1Skill(int skillIndex) const
{
    return getCharacterSkill(0, skillIndex);
}

SkillDefinition SkillSystem::getPlayer2Skill(int skillIndex) const
{
    return getCharacterSkill(1, skillIndex);
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
