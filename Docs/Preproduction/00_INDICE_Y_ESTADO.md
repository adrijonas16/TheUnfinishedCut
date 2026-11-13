# THE UNFINISHED CUT — Kit maestro de preproducción v1.0
**Fecha de corte:** 12 de noviembre de 2026  
**Público:** equipo creativo, programadores, QA y agentes de IA  
**Estado:** preproducción, NO juego implementado

## Cómo utilizar el kit
1. Leer `01_VISION_ALCANCE.md` y `02_GDD_MECANICAS.md` antes de programar.
2. Leer `03_BIBLIA_NARRATIVA.md`, `04_GUION_TIMELINE.md` y `05_LEVEL_DESIGN.md` antes de crear mapas.
3. Usar `06_ARQUITECTURA_UE5.md` y `10_CONTRATOS_ESTADOS.md` para separar responsabilidades.
4. Priorizar `07_ROADMAP_BACKLOG.md`; validar con `08_QA_ACEPTACION.md`.
5. Leer `09_PROTOCOLO_IA.md` antes de delegar a Codex; consultar los flujogramas en `02_Flujogramas/`.
6. Registrar cualquier decisión nueva en `12_DECISIONES_RIESGOS.md`.

## Etiquetas de certeza
- **[CONFIRMADO]**: decisión comunicada por el creador.
- **[PROPUESTA]**: diseño sugerido por el equipo y pendiente de validación.
- **[POR DEFINIR]**: falta una decisión necesaria antes de cerrar producción.

## Decisiones confirmadas
- PC, aventura 3D, un jugador, alternar entre humano y gato; el no controlado usa IA de acompañamiento.
- Ivo (nombre provisional): cámara **primera persona**. Nilo (nombre provisional): cámara **tercera persona**.
- Universos **paralelos** conectados mediante películas; no son viajes a pasado/futuro del mismo universo.
- Invasión inicial de adversarios procedentes de otras dimensiones.
- Mundo original, universo de estética ochentera, universo futurista y Archivo de los Ecos (Archivo, nombre propuesto).
- Puzles, exploración, obstáculos, loot, encuentros con enemigos y múltiples soluciones.
- Prólogo que anticipa la separación final; tres desenlaces: Ivo solo, Nilo solo o ambos salen.
- Estética **3D low-poly oscura**, planos simples, niebla moderada y acentos rojo/cian/violeta.

## Propuestas estructurales pendientes
- Título `The Unfinished Cut`; nombres Ivo/Nilo; antagonista `El Montador`; `Proyector Matriz`, `Ancla de Continuidad`.
- Tres niveles principales, un prólogo y epílogo; estimación de experiencia completa de 50–70 minutos.
- Unreal Engine 5 como motor de referencia del documento original. La **versión exacta** no está confirmada.
- Windows como primera plataforma, teclado/ratón y controlador como posible soporte adicional.

## Entregables incluidos
| Documento | Uso |
|---|---|
| 01_VISION_ALCANCE | Visión, público, límites, prioridades, definición de éxito |
| 02_GDD_MECANICAS | Reglas jugables, personaje, cámara, IA, películas, loot, enemigos |
| 03_BIBLIA_NARRATIVA | Canon, personajes, mundo, tema, finales |
| 04_GUION_TIMELINE | Hitos, escenas y condiciones narrativas |
| 05_LEVEL_DESIGN | Diseño espacial y objetivos por nivel |
| 06_ARQUITECTURA_UE5 | Módulos, flujos de datos, responsabilidades |
| 07_ROADMAP_BACKLOG | Producción por hitos y tareas trazables |
| 08_QA_ACEPTACION | Matriz de pruebas, accesibilidad y riesgos |
| 09_PROTOCOLO_IA | Instrucciones de colaboración con IA |
| 10_CONTRATOS_ESTADOS | Estados, eventos y reglas de persistencia |
| 11_ARTE_AUDIO_UI | Guía de estilo, cámaras, sonido, HUD |
| 12_DECISIONES_RIESGOS | Registro de decisiones y preguntas pendientes |
| 13_INDICE_FLUJOGRAMAS | Índice y lectura de diagramas |
| 14_FUENTES | Fuentes externas y alcance de referencias |

## Carpeta para IA
`03_Datos_IA/` incluye manifest, progreso narrativo JSON y backlog CSV. Los datos son **especificación**, no assets funcionales de Unreal.
