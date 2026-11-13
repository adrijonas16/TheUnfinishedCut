# 07 — Roadmap de producción y backlog
**Estimaciones:** tamaños relativos S/M/L, NO calendario contractual. El calendario depende de experiencia/equipo, alcance y assets disponibles.

## Hitos con puerta de calidad (gates)
| Hito | Resultado demostrable | Gate obligatorio |
|---|---|---|
| P0 Preproducción | documentos y diagramas consistentes | decisiones críticas marcadas, flujo reproducible |
| P1 Mecánica de personajes | Ivo FPS, Nilo TPS, cambiar, seguir/esperar | 20 cambios sin pérdida de control |
| P2 Puzle cooperativo | caja, conducto, palanca, puerta | sin softlocks; guardar/cargar mantiene puzzle |
| P3 Película / universo | salto Origin ↔ Retro80 en ancla | posiciones seguras, items persistentes |
| P4 Vertical slice | 8–12 min: intro + puzle + loot + película + salida | tester sin guía puede finalizar |
| P5 Alpha | L1–L3 conectados, endings A/B/C | campaña completable, ramas correctas |
| P6 Beta | arte low-poly, audio, accesibilidad, QA | rendimiento y regresión sin bloqueantes |
| P7 Build | ejecutable limpio, créditos, notas | prueba en PC sin editor |

## Prioridades
**P0** imprescindible para prototipo; **P1** primera versión completa; **P2** pulido o post-v1.

## Backlog por épicas (IDs corresponden a backlog.csv)
### E01 — Control y cámaras
- TUC-001 Crear proyecto UE5, estructuras carpetas, repositorio y mapa graybox (P0/S).
- TUC-002 Control FPS Ivo con interacción de objeto (P0/M).
- TUC-003 Control TPS Nilo con locomoción y entrada en ducto (P0/M).
- TUC-004 Cambiar personaje conservando estado y reasignando IA/cámara (P0/L).
- TUC-005 Añadir instrucciones a compañero, señales y recuperación de navegación (P0/L).

### E02 — Interacción y puzle
- TUC-006 Interfaz genérica de interacción y prompt contextual (P0/M).
- TUC-007 Puzle mover caja → ducto → cerrojo → puerta (P0/M).
- TUC-008 Estados de mecanismo persistentes y checkpoint (P0/M).
- TUC-009 Puzle pasarela nivel 1 y señales de roles (P1/L).

### E03 — Películas
- TUC-010 Modelo de película/destino/anclas de proyección (P0/M).
- TUC-011 Cambio seguro Origin/Retro80, fades, spawns (P0/L).
- TUC-012 Persistencia por universo y retorno estable (P1/M).

### E04 — Progreso y narrativa
- TUC-013 QuestDirector, flags y triggers (P0/M).
- TUC-014 Prólogo breve y reconocimientos visuales (P1/M).
- TUC-015 Nivel 2: registros alternativos y pista Ancla (P1/L).
- TUC-016 Nivel 3: 3 endings con condiciones verificables (P1/L).

### E05 — Inventario, amenazas, arte y UX
- TUC-017 Inventario limitado y cajas sin duplicación de loot (P1/M).
- TUC-018 Amenaza de patrulla, detección, escape/reset (P1/M).
- TUC-019 Arte low-poly y luces estilizadas por universo (P1/L).
- TUC-020 UI accesible: indicadores de cámara, objetivos, instrucciones (P1/M).
- TUC-021 Audio de proyecciones, mundo y personajes (P1/M).

### E06 — Calidad y empaquetado
- TUC-022 Pruebas funcionales automatizables y checklist manual (P0/M).
- TUC-023 Pruebas de regresión de SaveGame, swaps y endings (P1/M).
- TUC-024 Profiling y optimización de mapa crítico (P1/M).
- TUC-025 Build ejecutable y prueba de instalación (P1/M).

## Dependencias secuenciales importantes
- TUC-004 depende de TUC-002, TUC-003.
- TUC-007 depende de TUC-004, TUC-006 y TUC-005.
- TUC-011 depende de TUC-010 y TUC-008.
- TUC-016 depende de TUC-013, TUC-015 y del flujo de películas.
- TUC-019 no debe iniciarse a escala hasta que P4 sea validado.

## Definición de terminado (DoD)
- Requisito trazado a ID, screenshots o video de evidencia, test ejecutado, regresiones sin bloqueantes.
- Sin placeholders de mecánica crítica sin declarar.
- Actor y datos separados; ninguna acción esencial depende de un único frame o timing preciso.
- Build empaquetada probada, no solo Play-In-Editor.

## Cadencia sugerida (sprints de alcance, no fechas)
1. Núcleo FPS/TPS + IA.
2. Interacciones + puzzle y checkpoint.
3. Película y doble universo.
4. Vertical slice y playtest.
5. L2/L3 con branching y pruebas de endings.
6. Dirección visual/sonora + optimización y QA.
