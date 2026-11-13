# 10 — Contratos de estado, eventos y guardado

## Enumeraciones
```
Character = IVO | NILO
CameraMode = FPS | TPS
CompanionState = FOLLOW | WAIT | INTERACT | REGROUP | SAFE_IDLE
WorldID = ORIGIN | RETRO80 | FUTURE | ARCHIVE
PuzzleState = LOCKED | AVAILABLE | PARTIAL | SOLVED
FilmState = LOCKED | AVAILABLE | TRAVELLING | ARRIVED | FAILED
Ending = NONE | A | B | C
```

## Invariantes
- Exactamente **un personaje controlado** a la vez.
- `active_character=IVO ↔ active_camera=FPS`; `NILO ↔ TPS`.
- Nunca más de una transición de película simultánea.
- `PuzzleState.SOLVED` no retorna a parcial salvo reset explícito de nuevo intento y puzzle marcado como reseteable.
- `critical_items` no se pierden de forma que bloqueen la ruta necesaria.
- Ending `C` requiere condiciones verdaderas y se ejecuta mediante interacción específica.

## Eventos recomendados
`EVT_CHARACTER_SWITCHED`, `EVT_COMPANION_ORDERED`, `EVT_INTERACTED`, `EVT_PUZZLE_SOLVED`, `EVT_FILM_UNLOCKED`, `EVT_FILM_TRAVEL_STARTED`, `EVT_FILM_TRAVEL_DONE`, `EVT_ITEM_GRANTED`, `EVT_CHECKPOINT_SAVED`, `EVT_STORY_FLAG_SET`, `EVT_ENDING_STARTED`.

## Flags de historia
| Flag | Origen | Uso |
|---|---|---|
| `preview_seen` | P0 | evita repetir intro al cargar |
| `tutorial_swap_done` | L1-S01 | habilita tutorial superado |
| `invasion_seen` | L1-S02 | cambia ambientación origen |
| `puzzle_l1a_solved` | L1-S02 | puerta cine disponible |
| `film_retro_unlocked` | L1-S03 | habilita Retro80 |
| `battery_found` | L1-S04 | proyector retorno |
| `puzzle_l1b_solved` | L1-S05 | pasarela abierta |
| `archive_hint_found` | L1-S06 | desbloquea objetivo L2 |
| `future_entered` | L2-S01 | L2 activo |
| `puzzle_l2a_solved` | L2-S02 | galería abierta |
| `alternate_cuts_seen` | L2-S03 | revelación |
| `anchor_knowledge` | L2-S04 | requisito C |
| `anchor_parts` | L2-S04 | entero 0..2, requisito C [PROPUESTA] |
| `archive_unlocked` | L2-S05 | acceso L3 |
| `archive_entered` | L3-S01 | L3 activo |
| `puzzle_l3a_solved` | L3-S02 | sala final accesible |
| `exit_reached` | L3-S04 | finales A/B accesibles |
| `anchor_activated` | L3-S04 | requisito C, se activa en puzzle final |
| `ending_selected` | E-A/B/C | A, B o C |
| `game_completed` | Epílogo | mostrar créditos |

## Condiciones de finales (propuesta de aceptación)
```
can_A = exit_reached
can_B = exit_reached
can_attempt_C = exit_reached AND anchor_knowledge AND anchor_parts >= 2
can_C = can_attempt_C AND anchor_activated
```
**Importante:** `can_attempt_C` permite iniciar puzzle del Ancla; `can_C` permite salida conjunta solo tras resolverlo. Nunca marcar C terminado solo por tener componentes.

## Snapshot guardado (conceptual)
```
{
  "save_version": 1,
  "level_id": "L1",
  "world_id": "RETRO80",
  "active_character": "NILO",
  "companion_state": "WAIT",
  "safe_spawn_ids": {"IVO": "SP_L1_ROOF_A", "NILO": "SP_L1_ROOF_B"},
  "inventory": ["ITEM_FILM_RETRO", "ITEM_BATTERY"],
  "story_flags": {"film_retro_unlocked": true, "battery_found": true},
  "puzzle_states": {"PUZ-L1-01": "SOLVED"},
  "world_flags": {"RETRO80": {"roof_latch_open": true}},
  "ending_selected": "NONE"
}
```

## Eventos de dominio vs efectos
Un evento de interacción cambia primero el estado lógico persistente y después ejecuta sonido/VFX/animación. Los efectos no deben determinar verdad del puzzle, para evitar fallos de sincronización de cinemáticas.
