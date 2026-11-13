# 12 — Registro de decisiones y riesgos

## Historial de decisiones
| ID | Decisión | Estado | Fuente/contexto |
|---|---|---|---|
| D-001 | Humano y gato, roles complementarios | CONFIRMADO | conversación |
| D-002 | Películas = universos paralelos | CONFIRMADO | conversación |
| D-003 | Invasión de enemigos dimensionales | CONFIRMADO | conversación |
| D-004 | Prólogo anuncia final con gato ausente | CONFIRMADO | conversación |
| D-005 | Tres niveles, tres desenlaces | CONFIRMADO | conversación |
| D-006 | Un jugador cambia entre personajes + IA | CONFIRMADO | conversación |
| D-007 | Ivo FPS, Nilo TPS | CONFIRMADO | conversación |
| D-008 | 3D low-poly oscuro, neones y contraste | CONFIRMADO | conversación e imágenes |
| D-009 | Unreal Engine 5, plataforma Windows | PROPUESTA basada en GDD anterior | documento original |
| D-010 | El Montador como antagonista | PROPUESTA | biblia v1 |
| D-011 | Ancla y umbral de 2 componentes final C | PROPUESTA | este kit |
| D-012 | Juego 50–70 min | PROPUESTA | alcance preliminar |

## Pendientes antes de implementación final
1. Versión UE5 y equipo/hardware objetivo.
2. Confirmar nombres Ivo, Nilo y The Unfinished Cut.
3. Confirmar antagonista/origen exacto de fractura (accidente, inteligencia o invasor).
4. Definir tono de encuentros con amenazas: evitar/sigilo/confrontación estilizada.
5. Confirmar inventario (ranuras/cantidad), reglas de pérdida y checkpoint.
6. Confirmar controles, dispositivo inicial y soporte de gamepad.
7. Confirmar si los tres finales deben poder reintentarse desde el checkpoint final.
8. Confirmar duración objetivo real con playtests.

## Riesgos por prioridad
| Riesgo | Impacto | Prob. | Mitigación |
|---|---|---|---|
| R-01 IA se atasca en portales/puertas | crítico | alta | safe spawn, regroup, test de navegación |
| R-02 combinaciones de flags producen softlocks | crítico | alta | contrato de estados + tests de progresión |
| R-03 dos perspectivas aumentan trabajo de cámara | alto | media | prototipo de cambio antes de producción |
| R-04 4 variantes de mundo costosas | alto | alta | kits modulares, 1 zona representativa primero |
| R-05 final C demasiado oculto | alto | media | pistas graduales + playtesting |
| R-06 efectos neón/niebla cuestan FPS | medio | media | perfilado, simplificar luces y sombras |
| R-07 contenido de 3 niveles excede capacidad | crítico | alta | vertical slice y scope freeze |

## Política de cambios
Un cambio al canon requiere actualizar simultáneamente: Biblia, GDD, Timeline, flujogramas, narrative_graph JSON, QA y backlog afectado. Guardar versión y motivo.
