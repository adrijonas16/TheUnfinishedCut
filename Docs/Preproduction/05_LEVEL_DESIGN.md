# 05 — Diseño de niveles (LDD)

## Principios espaciales
- Graybox antes de art. Una sala debe entenderse en blanco y gris.
- Cada obstáculo comunica el rol necesario: tamaños de ductos, volúmenes de cajas, altura de interruptores.
- Rutas seguras de reagrupación del compañero en todos los checkpoints.
- Una interacción activa en cada momento; evitar puzzles que requieran simultaneidad real de un solo jugador.
- Misma zona puede tener dos variantes de universo con anclas de proyección en correspondencia espacial.

## L1 — La primera fractura
**Ruta:** Casa → Calle → Refugio cine → Retro80 arcade → Azotea → Proyector retorno.  
**Macroobjetivo:** aprender alternancia y película para desbloquear siguiente pista.  
**Puzzle A:** mover caja+ducto+cerrojo.  
**Puzzle B:** carro+seguro+tira bloqueadora+energía+pasarela.  
**Item crítico:** batería garantizada.  
**Amenazas:** patrulla lenta; se puede evitar por cobertura y rutas secundarias.  
**Checkpoint:** tras puerta calle, al entrar en Retro80, antes de azotea, tras pasarela.  
**Riesgo de softlock:** dejar gato tras puerta, perder batería o bloquear caja; mitigación: puntos de reapertura, inventario crítico persistente, reset del puzzle.

## L2 — Ecos del futuro [PROPUESTA]
**Ruta:** Calle de llegada → Instalación → Sala de energía → Galería de películas → Laboratorio opcional → Terminal Archivo.  
**Macroobjetivo:** descubrir finales paralelos y mecanismo para tercera salida.  
**Puzle central:** Ivo fija energía; Nilo recorre cableado accesible, desbloquea consola remota; regresar para abrir sala.  
**Item crítico:** `archive_access_reel` garantizado.  
**Recompensa opcional significativa:** planos del Ancla y 2 componentes. Debe tener pista ambiental y puerta reabrible antes de salir.  
**Amenazas:** dos patrullas con alternativas de ruta.  
**Checkpoint:** al entrar, tras sala de energía, tras galería, antes de terminal.

## L3 — El último fotograma [PROPUESTA]
**Ruta:** Vestíbulo Archivo → Núcleos dobles → Corredor de convergencia → Sala del prólogo → salida final.  
**Macroobjetivo:** detener fractura y resolver destino.  
**Puzle principal:** usar película en 2 puntos de proyección, unir estados de núcleos persistentes.  
**Amenazas:** defensa de guardianes/huida por rutas, no multitudes simuladas.  
**Final A:** Nilo mantiene acceso; Ivo sale.  
**Final B:** Ivo mantiene acceso; Nilo sale.  
**Final C:** Ancla preparada mantiene acceso sin dejar a ninguno atrás; completado mediante pasos secuenciales controlados por jugador.  
**Checkpoint:** antes del final; recargar permite intentar otra ruta con condiciones ya ganadas, sin repetir toda la campaña.

## Fichas de puerta / puzzle (plantilla)
- ID / nivel / objetivo narrativo / prerequisitos / personajes / acciones / feedback / fail+recovery / checkpoint / telemetría QA / prueba de softlock.

## Entregables de diseño por nivel
1. Diagrama de zonas y flujo; 2. graybox a escala; 3. lista de actores interactivos; 4. tabla de estados de puzzles; 5. lista de triggers/cinemáticas; 6. recorrido QA con capturas; 7. playtest sin guía.
