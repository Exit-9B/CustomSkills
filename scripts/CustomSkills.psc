Scriptname CustomSkills Hidden

; Get the current Custom Skills API version.
int Function GetAPIVersion() global native

; Open the custom skill menu for the given skill.
Function OpenCustomSkillMenu(string asSkillId) global native

; Show the skill increase message on the HUD for the given skill and level.
Function ShowSkillIncreaseMessage(string asSkillId, int aiSkillLevel) global native
