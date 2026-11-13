# 11 — Dirección visual, UI, cámaras y sonido

## Arte aprobado: estilo low-poly oscuro
Referencia aprobada: figuras poligonales simples, materiales de pocos detalles, luces rojo/azul/violeta, atmósfera nocturna. **NO** hiperrealismo, texturas PBR complejas o densidad excesiva.

### Kit modular de producción
- Edificios por bloques, fachadas repetibles, puertas, postes, ventanas, contenedores, cajas, ductos, marquesinas, proyectores.
- Siluetas legibles a distancia. Color de interacción suficiente contra fondo oscuro.
- Detalle concentrado en piezas de puzzle y puntos narrativos, no en cada muro.
- Niebla moderada solo donde no dificulte orientarse; postprocesado y transparencias limitados.

### Paleta funcional [PROPUESTA]
| Función | Color | Uso |
|---|---|---|
| Base mundo | azul pizarra `#172638` | entorno y niebla |
| Peligro/ruptura | rojo `#E34A55` | fracturas, alarma |
| Interacción | ámbar `#E6B65B` | elementos utilizables |
| Futuro | cian `#43B8D5` | circuitos, tecnología |
| Retro80 | magenta `#C858C9` | marcas arcade |
| UI neutra | marfil `#E6EDF3` | textos |

## Perfil cámaras
**Ivo FPS:** punto de vista a altura creíble; manos solo cuando aporten legibilidad, crosshair pequeño; interactúa en retícula con límite de distancia.  
**Nilo TPS:** cámara baja detrás del gato, margen para anticipar saltos, corrección automática de obstáculos.  
**Switch:** transición ≤0.6 s [PROPUESTA]; no cambiar durante loading/cinemática/uso de objeto crítico.

## UI HUD mínima
- Esquina: personaje activo y retrato pequeño del compañero.
- Centro: marcador discreto solo en Ivo; prompt `E Interactuar` contextual.
- Parte superior: objetivo corto y estado de película.
- Inventario: ranuras y número de recursos, no menús extensos.
- Señal distinta para «controlando» vs «IA siguiendo/esperando».

## Audio
- Ambiente ORIGIN: viento, electricidad, proyector lejano.
- RETRO80: sintetizadores sutiles, sonidos de recreativa; no copiar música comercial.
- FUTURE: drones electrónicos ambientales.
- ARCHIVE: reel-to-reel, ruido de proyección y motivos anteriores transformados.
- Nilo: maullidos breves y señales propias; Ivo frases cortas.

## Accesibilidad y rendimiento
- Subtítulos, pistas visuales + auditivas, no depender de color, opción reducir parpadeos.
- Motion blur y camera shake apagables; FOV y sensibilidad configurables.
- Estilo oscuro **no significa** ocultar rutas necesarias por falta de contraste.
- Validar con build empaquetada: materiales, sombras, niebla, postprocesado y luces bajo presupuesto.
