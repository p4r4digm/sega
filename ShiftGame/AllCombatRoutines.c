#include "CombatRoutines.h"

void buildAllCombatRoutines(CombatRoutineLibrary *self){
   combatRoutineLibraryAdd(self, stringIntern("melee"), buildMeleeAttackRoutine());
   combatRoutineLibraryAdd(self, stringIntern("bow"), buildBowAttackRoutine());
   combatRoutineLibraryAdd(self, stringIntern("projectile"), buildProjectileAttackRoutine());
   combatRoutineLibraryAdd(self, stringIntern("swap"), buildSwapAttackRoutine());
   combatRoutineLibraryAdd(self, stringIntern("swap-other"), buildSwapOtherAttackRoutine());
   combatRoutineLibraryAdd(self, stringIntern("auto"), buildAutoAttackRoutine());
   combatRoutineLibraryAdd(self, stringIntern("stun"), buildStunAttackRoutine());
   
}