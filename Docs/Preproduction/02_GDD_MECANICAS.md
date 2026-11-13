# 02 — GDD: sistemas y reglas de gameplay

## Core loop
**Observar peligro → explorar con personaje idóneo → encontrar pista/objeto → coordinar una acción con IA → activar mecanismo/película → superar obstáculo/amenaza → checkpoint → nueva revelación.**

## Control y cámara
- [CONFIRMADO] Ivo: **primera persona**, cámara a altura de ojos, interacción directa, manos/linterna opcionales.
- [CONFIRMADO] Nilo: **tercera persona**, cámara de seguimiento baja y visible; permite leer espacios estrechos y saltos.
- Cambio al instante donde sea seguro: mantiene posición y estado del compañero, interpola cámara o usa fundido muy breve; cambio bloqueado durante transición, cinemática o acciones atómicas.
- [PROPUESTA] PC: WASD mover, ratón mirar, espacio salto, E interactuar, Tab cambiar, Q orden compañero, F usar película contextual, Esc pausa. Reasignables antes de beta.
- Sin movimiento automático que rompa un puzle. Detectar colisiones y salida de cámaras para impedir ver a través de paredes.

## Reparto de habilidades
| Capacidad | Ivo | Nilo | Ejemplo |
|---|---|---|---|
| Caminar / saltar | estándar | ágil | ruta con desniveles |
| Mover caja pesada | sí | no | descubrir conducto |
| Accionar palanca grande | sí | no | mantener puerta |
| Entrar en ducto | no | sí | abrir cerrojo remoto |
| Trepar repisas estrechas | limitado | sí | cabina superior |
| Mover objeto pequeño | sí | sí | recuperar batería |
| Interactuar con consola | según alcance | pequeños controles | circuito dividido |
| Película en proyector | sí | apoya explorando | activar ruta paralela |

## IA acompañante
- Estados: `FOLLOW`, `WAIT`, `INTERACT`, `REGROUP`, `SAFE_IDLE`.
- El personaje inactivo responde solo a una orden explícita o rutinas seguras (seguir, apartarse de peligro, teletransporte de recuperación si se atasca fuera de cámara). No resuelve puzzle de manera autónoma.
- Si se cambia de personaje, el control humano se reasigna; el anterior recibe IA conservando orden y estado de animación.
- Interacciones largas se convierten en estados sostenidos (`lever_held=true`) para evitar la necesidad de entrada simultánea.
- [PROPUESTA] botón de llamada/contextual y feedback auditivo/visual de orden recibida.

## Películas / universos paralelos
- `FilmID` identifica el universo. Activación en `ProjectionAnchor` compatible. El jugador no puede cambiar en cualquier lugar inicialmente.
- Destinos: `ORIGIN`, `RETRO80`, `FUTURE`, `ARCHIVE`. Archivo actúa como hub/final, no necesariamente película recogible.
- Regla: **viajar entre universos, no viajar en el tiempo**. El mismo sector espacial puede tener geometría de otro universo.
- Al cruzar: validar ancla, cerrar interacción actual, almacenar estado local, hacer transición, cargar mapa/datalayer, ubicar a ambos en puntos seguros emparejados, restaurar cámara y control.
- Si ancla/destino falta o compañero no está en rango, mostrar bloqueo **legible** y no consumir película.
- Solo estados marcados como `CrossWorldPersistent` persisten entre mundos; las modificaciones normales pertenecen a cada universo.

## Inventario / loot
- Inventario compacto de **ranuras limitadas** [PROPUESTA], sin combinación compleja de objetos en v1.
- `KEY_ITEM`: siempre garantizado en ruta crítica (película, batería, llave, tijeras ficcionales).
- `OPTIONAL`: distractores, pistas, recursos de apoyo; nunca obligatorios para finalizar.
- `NARRATIVE`: registros/cintas que habilitan pistas o condiciones del final C.
- Cajas: `Closed→Opened→LootGranted`; apertura idempotente (no duplica).
- Objetos consumidos se registran para permitir volver al checkpoint sin pérdida injusta.

## Enemigos / tensiones
- Enemigo tipo 1: centinela que patrulla y detecta al protagonista.
- Enemigo tipo 2: guardián que custodia un mecanismo en un área puntual.
- Enemigo tipo 3 (final): presión ambiental de oleadas simplificadas/recorridos por fases, sin combate masivo.
- Recompensar desvío, distracción, sigilo y uso del entorno además de confrontación ficticia.
- Al fallar: regreso a checkpoint con estados seguros; evitar repetir cinemáticas obligatorias.

## Puzle reusable
Entrada: `puzzle_id`, roles requeridos, interacciones, flags y hint.  
Salida: `SOLVED` persistente, ruta abierta, feedback visual/sonoro.  
Estados: `LOCKED`, `AVAILABLE`, `PARTIAL`, `SOLVED`.

### Ejemplo PUZ-L1-01
1. Ivo mueve la caja (`crate_moved`).
2. Nilo entra al ducto y habilita cerrojo (`latch_open`).
3. Se abre puerta (`door_open = crate_moved && latch_open`).
4. Checkpoint tras apertura. Si cambias a Ivo antes de cerrar, el cerrojo no se revierte.

## Checkpoints y recuperación
Guardar: nivel, universo, personaje controlado, posiciones seguras de ambos, items, flags por universo, puzzles, estado de finales, enemigos reseteables. Nunca guardar un destino de transición incompleto.
- Falla → checkpoint; no borra progreso narrativo desbloqueado ni items críticos.
- Cargar con versiones de datos y valor por defecto para flags ausentes.

## Finales
- `final_A`: usar salida estándar con control que mantiene Nilo.
- `final_B`: usar salida invertida con control que mantiene Ivo.
- `final_C`: requiere `anchor_knowledge=true`, `anchor_parts>=2` y `anchor_activated=true`, resolviendo puzzle de coordinación. Los umbrales son [PROPUESTA] y ajustables.
- Los finales no dependen de selección secreta aleatoria; mostrar consecuencias y selección explícita jugable.
