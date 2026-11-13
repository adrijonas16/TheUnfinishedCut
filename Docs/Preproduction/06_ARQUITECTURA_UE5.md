# 06 — Documento técnico de arquitectura (UE5, propuesta)
**Motor base:** Unreal Engine 5 según documento original. [POR DEFINIR] build exacta, Blueprint-only vs C++ híbrido, hardware mínimo.

## Estructura de alto nivel
`PlayerController` posee al Pawn activo (`IvoCharacter` o `NiloCharacter`); el Pawn inactivo queda bajo su `AIController`. `CharacterSwitchSubsystem` gestiona transferencia, ordenes y cámara. `CompanionBrain` gestiona seguir/esperar/interactuar/reagrupar.

`InteractionComponent` detecta `BPI_Interactable`; `PuzzleStateComponent` resuelve flags sin depender de animaciones; `FilmTravelSubsystem` verifica anchors/destinos y llama a transición; `QuestStateSubsystem` guarda eventos; `SaveGame` serializa datos críticos.

## Diagrama lógico (texto)
```
LocalPlayer / EnhancedInput
  -> BP_PlayerController
      -> BP_SwitchCoordinator -> BP_IvoCharacter / BP_NiloCharacter
      -> BP_InteractionComponent -> BPI_Interactable -> PuzzleActors
      -> BP_FilmTravelManager -> BP_ProjectionAnchor / WorldState
      -> BP_CompanionAIController -> BehaviorTree/StateTree (a evaluar)
GameInstanceSubsystems -> WorldProgress / Inventory / Save
QuestDirector -> SequenceTriggers -> Cinematics / Endings
```

## Sistemas sugeridos
| Sistema | Responsabilidad | Observación |
|---|---|---|
| Input | Enhanced Input: `IMC_Common`, `IMC_Ivo`, `IMC_Nilo` | cambios contextuales según Pawn |
| Switch | `BP_CharacterSwitchCoordinator` | una sola fuente de verdad para `active_character` |
| AI | `BP_CompanionController`, BT/Blackboard o StateTree | comenzar con FSM pequeña; no IA generativa |
| Interacción | `BPI_Interactable`, `AC_Interactor` | prompt/validación/execute desacoplados |
| Puzzle | `BP_PuzzleDirector` por sala | flags y acciones idempotentes |
| Películas | `BP_FilmTravelManager`, `DA_Film`, `BP_ProjectionAnchor` | destino, spawn, reglas y loading |
| Inventario | `BP_InventoryComponent`, `DA_Item` | separar crítico/opcional/narrativo |
| Progreso | `BP_QuestDirector`, `DA_QuestNode` | desbloquea escenas/finales |
| Save | `SG_GameProgress` + versionado | estado persistente lógico, no referencias a actores |
| UI | `WBP_HUD_Ivo`, `WBP_HUD_Nilo`, `WBP_InteractionPrompt` | cámaras distintas, HUD legible |

## Interfaces clave (pseudocontrato)
- `IInteractable.CanInteract(actor, context) -> {allowed, reason}`
- `IInteractable.Interact(actor, context) -> InteractionResult`
- `PuzzleDirector.ApplyAction(puzzle_id, action_id, character_id) -> PuzzleState`
- `SwitchCoordinator.RequestSwitch(target) -> {success, reason}`
- `FilmTravel.RequestTravel(film_id, anchor_id) -> {success, reason}`
- `Progress.SetFlag(flag, value)`, `Progress.Has(flag)`, `Progress.GetEndingOptions()`

## Seguridad del estado / transacciones
1. Nunca modificar SaveGame directamente desde los interactuables.
2. Las acciones de puzzle son idempotentes: dos interacciones no duplican reward.
3. La transición de película guarda estado solo al confirmar destino seguro; en fallo restaura última escena.
4. Posiciones se normalizan por `SafeSpawnID` al cargar; no confiar en posición de mundo previo.
5. El segundo personaje reaparece en `CompanionSafeSpawn` si navegación no es viable.
6. `QuestDirector` emite eventos, pero `SaveGame` escribe un snapshot coherente con checksum/versión.

## Árbol de assets sugerido
```
Content/TUC/
  Characters/Ivo | Characters/Nilo
  AI/Companion | AI/Enemies
  Systems/Switch | Systems/Interaction | Systems/Films | Systems/Puzzles
  Systems/Inventory | Systems/Quests | Systems/Save
  Data/Items | Data/Films | Data/Levels | Data/Quests
  Maps/L0_Prologue | Maps/L1_Origin | Maps/L1_Retro80
  Maps/L2_Future | Maps/L3_Archive
  UI | Audio | Materials | Meshes | Cinematics | DevTests
```

## Rendimiento y producción
- Arte low-poly; iluminación horneada o híbrida simple según pruebas, sin efectos caros por defecto.
- Reutilizar módulos de edificios, luces y props; limitar materiales únicos y transparencias.
- Objetivo provisional PC: **60 fps preferidos, 30 fps mínimos** en hardware objetivo pendiente de definir.
- Perfilado temprano en build empaquetada; medir cuello de botella GPU/CPU y cargas.
- Control de versiones: Git + Git LFS para assets binarios, ramas cortas, naming y convenciones.

## Referencia técnica
Enhanced Input permite mapping contexts por estado; Unreal recomienda blockout antes de arte final; Behavior Trees/Blackboard o StateTree son opciones documentadas. La selección exacta se hará tras validar VS0 y fijar motor. Ver `14_FUENTES.md`.
