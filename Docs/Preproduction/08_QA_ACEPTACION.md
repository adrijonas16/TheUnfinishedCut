# 08 — Plan de QA, aceptación y accesibilidad

## Estrategia
Pruebas en tres capas: lógica de estado (automatizable), recorridos integrados (PIE y build), playtest humano sin guía. Priorizar **softlocks y pérdida de progreso** sobre pulido cosmético.

## Matriz de pruebas prioritarias
| ID | Escenario | Resultado esperado | Criticidad |
|---|---|---|---|
| QA-01 | cambiar Ivo→Nilo→Ivo en zona libre 20 veces | controles/cámara/posesión correctos | Bloqueante |
| QA-02 | cambiar mientras Ivo mantiene mecanismo | IA mantiene acción válida, puzzle no se revierte | Bloqueante |
| QA-03 | intentar cambiar durante cinematica o transición | acción rechazada con feedback; no crash | Alta |
| QA-04 | Nilo cruza ducto estrecho | cámara no se mete en paredes | Alta |
| QA-05 | ordenar esperar/seguir y cambiar | compañero cumple o informa imposibilidad | Alta |
| QA-06 | IA se atasca en ruta o puerta | se reagrupa en punto seguro | Bloqueante |
| QA-07 | abrir caja dos veces | recompensa se da una sola vez | Alta |
| QA-08 | perder/consumir item de ruta crítica | no hay bloqueo permanente | Bloqueante |
| QA-09 | viajar sin ancla válida | no cambia universo ni consume película | Alta |
| QA-10 | alternar universo y recargar save | estado local y persistente correctos | Bloqueante |
| QA-11 | fallar puzzle con palanca | reset o retry sin reiniciar campaña | Bloqueante |
| QA-12 | cargar checkpoint con gato separado | ambos en posiciones válidas | Bloqueante |
| QA-13 | llegar al final sin Ancla | A/B disponibles, C no | Bloqueante |
| QA-14 | resolver conocimiento + 2 componentes y activar Ancla | C se habilita claramente | Bloqueante |
| QA-15 | ver un epílogo y cargar checkpoint | flags no mezclan endings | Alta |
| QA-16 | terminar en build empaquetada | sin dependencia del editor | Bloqueante |
| QA-17 | pausa/restauración en transición de película | no se duplica carga ni corrupción de estado | Alta |
| QA-18 | indicadores de interacción sin usar color | texto/iconos comprensibles | Media |
| QA-19 | subtítulos, volumen y sensibilidad cámara | configuraciones persistentes | Alta |
| QA-20 | FPS/uso memoria en PC objetivo | dentro de objetivo validado | Alta |

## Riesgos y mitigaciones
- **Softlock por IA**: checkpoints con companion safe spawn y orden de reagrupar.
- **Cámara FPS/TPS**: evitar clipping, giro brusco y teleports visibles.
- **Save entre universos**: guardar snapshot de estado por mundo, sin referencias a actores instanciados.
- **Branching roto**: pruebas A, B, C y C bloqueado con estados generados.
- **Pistas insuficientes**: 1 pista ambiental y 1 hint opcional por puzzle difícil.
- **Loot incorrecto**: item crítico garantizado; rutas opcionales nunca imprescindibles.

## Accesibilidad mínima
- Subtítulos para voz e información de pistas; instrucciones redundantes con icono + texto, nunca solo color.
- Sensibilidad cámara y FOV configurables cuando sea viable; desactivar head bob / motion blur.
- Opción de reducir parpadeos/flash en transiciones cinematográficas.
- Reasignación de controles y prompts correctos al dispositivo seleccionado [objetivo beta].
- Pistas progresivas y reintento cercano.

## Evidencia QA por caso
`test_id`, build/version, mapa, perfil de guardado, pasos, resultado esperado/obtenido, video o captura, severidad, estado, commit/cambio asociado.

## Gate vertical slice
QA-01, 02, 04, 05, 06, 07, 09, 10, 11, 12 y 16 pasan; playtest puede completar sin explicaciones orales. Las métricas de tiempo son objetivo tentativo, no compromiso.
