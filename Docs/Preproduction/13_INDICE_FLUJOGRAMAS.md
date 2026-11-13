# 13 — Catálogo de flujogramas
Todos los diagramas están en `02_Flujogramas/` con **fuente `.mmd`** (Mermaid editable) y **vista `.svg` / `.png`** (Graphviz, misma lógica).

| Diagrama | Objetivo | Pregunta principal |
|---|---|---|
| `01_flujo_juego` | camino de pantalla inicial a epílogos | ¿Qué juega y desbloquea el usuario? |
| `02_ramas_narrativas` | condiciones de tres finales | ¿Cómo llega a cada final? |
| `03_cambio_personajes_ia` | control y compañero | ¿Qué pasa al pulsar cambiar? |
| `04_pelicula_universos` | activación, carga y fallback | ¿Cuándo es seguro cambiar realidad? |
| `05_flujo_nivel1` | ruta y dependencias de L1 | ¿Qué acciones desbloquean la salida? |
| `06_deps_desarrollo` | orden de construcción | ¿Qué programa primero un desarrollador/IA? |

## Reglas de mantenimiento
- Mermaid es fuente fácil de editar y se usa en repositorios/documentación.
- SVG/PNG son renders visuales generados con Graphviz, NO código ejecutable.
- Los nombres de flags deben coincidir con `10_CONTRATOS_ESTADOS.md` y `03_Datos_IA/narrative_graph.json`.
- Ningún diagrama reemplaza los casos de prueba ni criterios de aceptación.

## Vista interactiva
Abrir `04_Vistas/Mapa_de_Flujos.html` en navegador; permite seleccionar un diagrama y aumentar tamaño. No requiere Internet.
