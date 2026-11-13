# 09 — Protocolo para programador o agente IA (Codex/otros)

## Misión
Implementar **vertical slices comprobables** de The Unfinished Cut en UE5, sin inventar reglas de juego contradictorias ni agregar sistemas fuera del alcance aprobado.

## Fuentes de verdad, en orden
1. `00_INDICE_Y_ESTADO.md`: confirmar qué está aprobado.
2. `01_VISION_ALCANCE.md` + `02_GDD_MECANICAS.md`: requisitos funcionales.
3. `10_CONTRATOS_ESTADOS.md` + `03_Datos_IA/*.json`: estados, IDs y ramificaciones.
4. `05_LEVEL_DESIGN.md` + flujogramas: aceptación espacial/recorrido.
5. `06_ARQUITECTURA_UE5.md`: módulos y contratos propuestos.
6. `08_QA_ACEPTACION.md`: cómo demostrar que funciona.

## Protocolo de cambio
- Antes de tocar código, identificar **ID de backlog**, dependencias, mapa/Blueprints afectados, criterio de aceptación y plan de prueba.
- Si falta una decisión importante (`[POR DEFINIR]`), proponer opción mínima y registrar incertidumbre. No elevarla a canon sin aprobación.
- Trabajar en **una tarea por iteración**, compilar, probar en PIE, guardar evidencia, reportar archivos modificados.
- Nunca reescribir Biblia Narrativa y GDD para justificar una implementación distinta sin solicitar aprobación.
- No cambiar masivamente estructura de assets/inputs sin mapa de migración.
- Reutilizar interfaces/data assets; evitar lógica duplicada entre niveles.

## Prompt de arranque sugerido
> Actúa como desarrollador de Unreal Engine 5 especializado en aventuras single-player con compañero IA. Estudia `00_INDICE_Y_ESTADO.md`, `01_VISION_ALCANCE.md`, `02_GDD_MECANICAS.md`, `06_ARQUITECTURA_UE5.md`, `10_CONTRATOS_ESTADOS.md`, los flujogramas y el backlog. NO implementes aún. Devuélveme: (1) decisiones por confirmar, (2) auditoría del proyecto existente, (3) plan P0 en tareas pequeñas con dependencias, (4) pruebas de aceptación, (5) riesgos. Después espera aprobación para cambios. Mantén Ivo FPS, Nilo TPS, viajes a universos paralelos, ambientación low-poly oscura, modo individual y finales A/B/C.

## Prompt para cada tarea
> Implementa exclusivamente `[TASK_ID]` del backlog. Primero explica archivos a crear/modificar, interfaces y condiciones de aceptación. Tras aprobación implementa, ejecuta las verificaciones disponibles e informa resultados y limitaciones. No cambies el canon ni agregues features P2.

## Formato de reporte de avance
- ID: / Estado: planeado|en progreso|implementado|validado|bloqueado
- Alcance / Archivos modificados / Decisiones tomadas
- Test por ID / Evidencias / Riesgos / Próximo paso

## Criterios para revisar código generado
La IA no debe confundir «compañero IA» con multijugador, universos paralelos con viajes temporales ni «cámara por personaje» con cámara opcional sin reglas. Asegurar reversión de transición, guardado, recuperación de acompañante y gating del final C.

## Convenciones de trabajo recomendadas
IDs `TUC-###` en commits; assets `BP_`, `DA_`, `WBP_`, `BPI_`, `SG_`; estados/eventos exactamente como en JSON; pruebas automatizables en carpeta `DevTests`. Prohibido afirmar «probado» sin aportar ejecución real.
