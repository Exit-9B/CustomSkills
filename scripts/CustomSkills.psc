Scriptname CustomSkills Native Hidden

; Get the current Custom Skills API version.
; Current version: 2
int Function GetAPIVersion() global native

; Open the custom skill menu for the given skill or group (config file).
Function OpenCustomSkillMenu(string asSkillId) global native

; Displays the training menu for the given skill.
Function ShowTrainingMenu(string asSkillId, int aiMaxLevel, Actor akTrainer) global native

; Displays the skill increase message on the HUD for the given skill and level.
Function ShowSkillIncreaseMessage(string asSkillId, int aiSkillLevel) global native
