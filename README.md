
# Learn UE5 multiplayer game development
Thanks to Stephen Ulibarri's MultiplayerCourseBlasterGame

### Engine Version
- master: UE5.2
- UE5.6

### Modification
- UI: MultiplayerSession plugins、player HUD
- New GameMap: Captur the Zone
  - player holding the flag: can't fire but import speed、heal rate etc; can capture zone
  - Captured zone: teleport owner team player; heal the player holding the sword (own team)
  - a knockout blow
- can throw the weapon
- Data based driven banner widget: legendary、pentakill etc
- public chat and team chat when team and capture the zone game mode
- Some bug fix

### TODO(maybe not)
- minimap and display available teleport when ready to teleport
- zone progress will be black when standing behind the zone (always face to player or more efficient solution)
- skill/AI?
