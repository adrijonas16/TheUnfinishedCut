# 01 — Visión, experiencia objetivo y alcance

## Elevator pitch
**The Unfinished Cut** es una aventura 3D individual de puzles, exploración y tensión ambiental. El jugador alterna entre un humano en primera persona y su gato en tercera persona. Mediante películas salta a universos paralelos para sortear una invasión dimensional y cambiar un final anunciado.

## Fantasía del jugador
- «Soy dos personajes con capacidades distintas y necesito a ambos para sobrevivir».
- «Una película transforma las posibilidades de un mismo desafío».
- «Vi un final y mis acciones pueden construir otro».

## Pilares no negociables
1. **Dos protagonistas útiles**: cada misión incorpora acciones significativas para humano y gato.
2. **Película = decisión jugable**: cada cambio altera rutas/amenazas/soluciones, no solo apariencia.
3. **Puzle antes que espectáculo**: el contenido se diseña en graybox y después se decora.
4. **Resultado preparado**: tres finales coherentes, condicionados por información y acciones previas.
5. **Low-poly deliberado**: estética sólida, producción viable, buen contraste visual.

## Experiencia objetivo [PROPUESTA]
- Juego completo: 50–70 minutos en primera partida; prólogo + 3 niveles + epílogo.
- Vertical slice de validación: 8–12 minutos, un área, un puzzle para ambos, una película y un checkpoint.
- Audiencia: público general aficionado a aventuras de misterio; intensidad moderada, sin detalles gráficos violentos.
- Plataforma objetivo inicial: PC Windows, offline/sin cuentas ni red. Controles: teclado/ratón primero.

## In scope (juego completo)
- Alternancia de personaje, cámara dependiente del personaje, compañero IA básico fiable.
- Dos habilidades diferenciadas por protagonista, extensibles con interfaces de interacción.
- Puzles de objetos/mecanismos con estados persistentes y hints opcionales.
- Películas con destinos fijos y puntos de proyección; universos distinguibles.
- Loot mínimo: herramientas garantizadas, recompensas opcionales; inventario sencillo.
- Enemigos con patrulla, detección, persecución breve y estado de derrota/evitación; sin IA compleja.
- Tres niveles; guardado y checkpoints; HUD claro y accesibilidad básica.
- Ramas A/B/C vinculadas a flags de progreso y selección explícita en la resolución.

## Fuera de alcance de v1
Multijugador, mundo abierto, generación procedural, crafting extenso, economía, vehículos, parkour avanzado, docenas de enemigos, combates complejos, cinemáticas largas, viajes temporales, portales libres en cualquier punto, sistemas en línea.

## Diseño por vertical slices
- **VS0 — Mecánica pura**: blockout, cambiar personaje, obedecer/esperar, empujar caja, conducto de gato.
- **VS1 — Realidad paralela**: escena ochentera, cambio por película, un puzzle cuyo resultado depende del universo.
- **VS2 — Loop representativo**: checkpoint, item garantizado, amenaza simple, puerta de salida, feedback.
- **Alpha**: tres niveles de principio a fin con finales simplificados.
- **Beta**: pulido visual/sonoro, QA, accesibilidad, empaquetado.

## Criterio de éxito
Sin instrucciones del desarrollador, alguien puede completar VS1: controlar a Ivo, cambiar a Nilo, ordenar esperar/interactuar, resolver un obstáculo, activar una película, comprender que está en otro universo y llegar al checkpoint sin perder progresión.

## Dependencias críticas
Decidir motor/versión, plataforma y soporte mando; confirmar el diseño exacto del antagonista; confirmar tono de combates, nivel de dificultad, duración y forma de adquisición de películas.
