# 2026 Epic AI MegaJam — declaración de assets y herramientas

Tema: **DIRECTOR'S CUT**. Juego: **The Unfinished Cut**.

## Hecho antes del 12 de noviembre de 2026 (hay que declararlo)
- Documentos de preproducción (biblia narrativa, GDD, guion, level design, contratos de estados): `Docs/Preproduction`.
- Modelos Meshy generados antes de la jam: gato siamés (30 jun 2026), proyector retro (24 may 2026) y personaje animado usado como Ivo (29 oct 2026).
- Prototipo v1.0 del código (subido como `The_Unfinished_Cut_UE5_Complete_v1_0.zip`).

## Hecho durante la jam (13 nov 2026)
- Revisión y reescritura del código C++ (v1.1–v1.2), HUD, guardado, patrullas, finales.
- Kit de 46 modelos low-poly y 17 texturas propias generadas por script (`Tools/Kit`, `ExternalAssets/TUC_Kit`).
- Integración de Ivo animado (esqueleto UE5 Mannequin) y scripts de importación.

## Herramientas
- Unreal Engine 5, Visual Studio.
- Claude (Anthropic): agente de código, assets procedurales y revisión.
- Meshy: modelos 3D, rig y animaciones (cuenta propia, sin licencias de terceros).
- Blender (módulo `bpy`): exportación y renders de verificación.

## Assets de terceros (comunidad Meshy, licencia CC0, descargados el 13 nov 2026 sin gastar créditos)
| Uso en el juego | Modelo | Autor en Meshy |
|---|---|---|
| Nilo (animado) | Ashen Tuxedo Cat (walking) | luzianlin33 |
| Butacas del cine | Cinema Seat | eric.stemen644 |
| Máquinas arcade | arcade cabinet | meishancui |
| Centinela | enraged-core-eye-phase2 | emircakmakx |
| Carrito de palomitas | popcorn machine | golfbart |
| Farolas | street lamp | OG_Games |
| Rockola | Retro jukebox neon | Tansy |
| Terminal futurista | Maintenance Terminal | axx.s |
| Rollo de película | film cassette | payenchetnoe |
| Consola de energía | Rusty open electrical panel | crivethieves |
| Hidrante | Fire Hydrant | codiejacobanderson |
| Estanterías del Archivo | tall wooden shelf | mulvaney66 |
| Barreras | construction barrier | EslemZeki |
| Altavoces | Speaker | gengarswapshop |

Además: contenido básico del motor (`/Engine/BasicShapes`) y componentes de cielo/niebla del motor.

## Entrega (recordatorio)
- Build empaquetado (Development, Windows) en `.zip` llamado `TeamName_GameName`, menos de 1 GB.
- Video de juego de 30–60 s (mostrar jugabilidad en los primeros 5 s).
- Fecha límite: 15 nov 2026, 8:59 PM EST (7:59 PM Bogotá).
