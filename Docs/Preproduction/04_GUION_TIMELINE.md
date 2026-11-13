# 04 — Guion por escenas y línea de tiempo
**Atención:** el prólogo y L1 son borradores narrativos ya discutidos; L2 y L3 son escaletas nuevas [PROPUESTA], NO diálogos definitivos.

## Orden de experiencia del jugador
| Escena | Nivel | Acción principal | Evento/pista | Duración guía |
|---|---|---|---|---|
| P0 | Prólogo | Cinemática: Ivo sale solo con la campanilla | `preview_seen` | 1 min |
| L1-S01 | Casa | Tutorial Ivo mueve caja; Nilo recupera objeto | `tutorial_swap_done` | 3 min |
| L1-S02 | Calle | Aparece fractura; puerta con ducto | `invasion_seen`,`puzzle_l1a_solved` | 3 min |
| L1-S03 | Cine | Encontrar película y activar proyector | `film_retro_unlocked` | 3 min |
| L1-S04 | Retro80 | Explorar y abrir primera caja | `battery_found` | 4 min |
| L1-S05 | Azotea | Puzle cooperativo de pasarela | `puzzle_l1b_solved` | 4 min |
| L1-S06 | Proyector | Retorno y pista Archivo | `archive_hint_found` | 2 min |
| L2-S01 | Futuro | Ingreso a instalación, lectura del entorno | `future_entered` | 3–4 min |
| L2-S02 | Futuro | Puzle de sistema con control humano/gato | `puzzle_l2a_solved` | 5–6 min |
| L2-S03 | Laboratorio | Hallar registros de finales distintos | `alternate_cuts_seen` | 3–4 min |
| L2-S04 | Laboratorio opcional | Recuperar plano/partes del Ancla | `anchor_knowledge`, `anchor_parts` | 5–7 min |
| L2-S05 | Terminal | Mensaje del Montador y acceso Archivo | `archive_unlocked` | 2 min |
| L3-S01 | Archivo | Superposición de escenarios y reto de ambas cámaras | `archive_entered` | 4 min |
| L3-S02 | Archivo | Reactivar 2 núcleos en realidades diferentes | `puzzle_l3a_solved` | 5–7 min |
| L3-S03 | Corredor | Evitar/contener amenazas y abrir salida | `exit_reached` | 4–6 min |
| L3-S04 | Sala final | Reconocer prólogo; escoger solución jugable | `ending_selected` | 3–5 min |
| E-A/B/C | Epílogo | Consecuencia específica | `game_completed` | 1–2 min |

## Guion detallado del prólogo
**P0 — Sala del Archivo, momento indeterminado.** Un proyector suena; Ivo atraviesa una puerta, mira hacia atrás, espera a Nilo, sostiene la campanilla. Cartela: `CORTE 001 — FINAL REGISTRADO`. Rebobinado. Corte a casa. No mostrar daño explícito ni explicar la ausencia del gato.

## Guion detallado Nivel 1
**L1-S01 — Casa, atardecer.** Ivo ordena; Nilo juega con caja. Tutorial en contexto. Ivo recoloca campanilla: «¿Otra vez escondiendo mis cosas?»; gato maúlla. Anomalía luminosa afuera.

**L1-S02 — Calle.** Fractura permite paso a intrusos. Ivo mueve caja, Nilo entra por ducto, abre cerrojo; jugadores alcanzan refugio. Línea de Ivo: «¿Qué está pasando?».

**L1-S03 — Cine.** Proyector en cabina inaccesible para Ivo, accesible para Nilo. Film muestra ciudad de neón. Amenaza se aproxima, se abre portal, cambio de universo. «Esto no es una película normal...».

**L1-S04 — Retro80.** Arcade antiguo en mundo paralelo. En almacén: tijeras/batería garantizadas, distractor opcional. Nilo abre ruta lateral, ambos alcanzan azotea. «Este lugar parece de otra época...» (el entorno, no el pasado del mismo universo).

**L1-S05 — Pasarela.** Ivo desplaza carro; Nilo libera seguro; Ivo retira atadura del mecanismo; Nilo activa interruptor; Ivo despliega pasarela. Evitar amenaza es válido. Si falla, repetir desde azotea sin perder item clave.

**L1-S06 — Proyector de salida.** Batería activa retorno; fragmento revela `ARCHIVO DE LOS ECOS — ACCESO 02`; Ivo dice «Vamos juntos, Nilo». Fin de nivel.

## Escaleta propuesta Nivel 2
- S01 **Entrada al mundo futurista**: arquitectura más geométrica; guardianes automatizados.
- S02 **Puzle de doble rol**: Ivo estabiliza energía local; Nilo cruza conducto y habilita puerta; diseño permite cambiar sin sincronización perfecta.
- S03 **Galería de películas**: reproducir 2 registros alternativos del mismo corredor del prólogo. Inicia revelación: no es una predicción única.
- S04 **Sala opcional del Ancla**: recuperar conocimiento y dos componentes [umbral propuesto]; pistas suficientes desde la ruta principal para que el usuario no quede desinformado.
- S05 **Advertencia del Montador**: «El corte registrado garantiza estabilidad»; se desbloquea Archivo.

## Escaleta propuesta Nivel 3
- S01 **Convergencia**: pasillos conectan zonas de estilos anteriores; estaciones de proyección separadas.
- S02 **Núcleos duales**: resolver mecanismo con Ivo/Nilo y una película, activando dos banderas persistentes.
- S03 **Presión de amenaza**: escena de persecución/obstáculo con alternativas de entorno, no exigir control simultáneo.
- S04 **La sala conocida**: misma puerta y campanilla del prólogo. El Montador propone salida estándar.
- S05 **Ramas**: A o B accesibles por rutas claras; C solo si el Ancla fue preparada y se resuelve minipuzle final.

## Cronología interna vs orden del relato
**Orden interno [PROPUESTA]:** Archivo mantiene conexiones → se da fractura → adversarios cruzan → invade ORIGIN → protagonistas encuentran películas → alcanzan Archivo → se determina un desenlace.

**Orden mostrado:** primero una película del desenlace A de un universo posible, luego el inicio de la aventura y las revelaciones. No es una línea temporal que viaje al pasado: es un registro anticipado al jugador.

## Eventos y condiciones
Eventos canónicos y flags en `10_CONTRATOS_ESTADOS.md` y `03_Datos_IA/narrative_graph.json`. Si cambian estas claves, actualizar diagramas y QA.
