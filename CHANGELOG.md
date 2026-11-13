# Changelog

## 1.4.0 — modelos realistas de la comunidad Meshy (CC0)

- Nilo animado: gato esmoquin con caminata (camina al moverse, pose quieta al parar).
- 13 reemplazos realistas del kit: butacas, arcades, centinela (ojo), carrito de palomitas, farolas, rockola, terminal, rollo, consola de energía, hidrante, estanterías, barreras y altavoces. Reducidos de polígonos, escalados a la medida de cada pieza y con texturas a 2K.
- Las colisiones se recalculan con las medidas reales de cada modelo (`Tools/Kit/normalize_meshy.py` + `layout.py`).
- Sin gastar créditos de Meshy.

## 1.3.0 — Ivo realista animado (Meshy)

- Ivo usa un personaje realista de tu cuenta de Meshy con esqueleto UE5 Mannequin y animaciones de caminar y correr (descargado, sin gastar créditos).
- La animación cambia sola entre quieto, caminar y correr según la velocidad.
- El script de importación crea `SK_Ivo`, `A_Ivo_Walk`, `A_Ivo_Run` y su material PBR.
- `JAM_DECLARACION.md` con lo hecho antes y durante la jam.

## 1.2.0 — kit de assets propio (sin cubos)

- 46 modelos low-poly con 17 texturas propias (ExternalAssets/TUC_Kit): suelos y paredes por set, calle con edificios, farolas y furgoneta, fachada y marquesina del cine, butacas, mostrador, pantalla con telón, arcades, tarima, torres sci-fi, estanterías de película, puerta del prólogo, todas las piezas interactivas, Ivo, Nilo (gato esmoquin con collar y campanilla) y el centinela.
- El decorado sale de una sola tabla (Tools/Kit/layout.py → TUCLayoutData.h): el render de verificación y el juego usan los mismos datos.
- Colisión invisible con la forma de cada objeto; si el kit falta, se ve el blockout y el juego sigue funcionando.
- Importación automática al abrir el editor (Content/Python/init_unreal.py) con material maestro e instancias.
- Marcador del objetivo en pantalla: quién debe usarlo (IVO/NILO), distancia y flecha en el borde si está fuera de cámara.
- Balanceo al caminar para Ivo y Nilo y flotación del centinela (no hay rig todavía).
- Piezas de pared pegadas a la pared (consola, conducto, terminal, ducto).

## 1.1.0 — revisión sin abrir Unreal

Corrige bloqueos de la 1.0 (detalle en `REVISION_v1.1.md`):
- Interacción reescrita: en la 1.0 ninguna pieza de Ivo era alcanzable y la campaña no avanzaba.
- El guardado se aplica en el orden correcto al continuar.
- Checkpoints fuera de las patrullas; centinelas con cono de visión, barra de detección, gracia tras rebobinar y uno por capítulo.
- Paredes de cierre en todos los sets y rebobinado si alguien cae.
- Fragmentos del Ancla guardados por identificador (no se pueden duplicar).
- Compatibilidad UE 5.1+: `BuildSettingsVersion.Latest`, input clásico explícito, fuentes UTF-8 con BOM.
- Reinicio sin recargar el nivel; soporte de mando.

Historia alineada con la biblia narrativa:
- Prólogo CORTE 001; finales A/B/C canónicos; C requiere el Ancla 3/3; aparece El Montador.

Assets:
- FBX optimizados re-exportados sin rutas de textura absolutas y con pivote en la base.
- Script de importación con materiales PBR y nombres garantizados (también con Interchange).
- Escala automática del gato y el proyector por bounds; placeholders de Ivo y Nilo.

Escena:
- Cielo de crepúsculo con SkyAtmosphere, sol bajo y SkyLight en tiempo real (sin assets externos).

HUD:
- Textos alineados a cualquier resolución, tarjeta de capítulo, mensajes de historia, motivo cuando una pieza no responde, epílogo por final.

## 1.0.0 — campaña compacta completa

- Cuatro capítulos: cine original, años 80, futuro y Archivo de los Ecos.
- Nueve hitos principales con cooperación Ivo/Nilo.
- Tres recuerdos opcionales persistentes.
- Tres finales y variante de final verdadero con 3/3 recuerdos.
- Viajes dimensionales y checkpoints automáticos.
- Cuatro patrullas con detección y rebobinado.
- Portada, pausa, créditos y nueva partida.
- El mundo completo se genera sin montaje manual de mapa.
- Sin generación de modelos ni consumo de créditos.

## 0.3.0 — vertical slice funcional en código

- Ivo FPS y Nilo TPS con cambio de control.
- Compañero con seguir/esperar.
- HUD, objetivos y prompts de interacción.
- Calle, fachada y cine generados automáticamente.
- Puzle de consola, conducto, proyector y salida.
- Transformación dimensional visual.
- Patrulla con detección y rebobinado.
- Checkpoints y guardado automático.
- Reinicio completo con Backspace.
- Assets Meshy CC0 originales y optimizados, sin consumo de créditos.
- Scripts de creación de mapa e importación de assets.

## 0.2.0 — assets

- Gato base y proyector retro descargados.
- Copias optimizadas para tiempo real.

## 0.1.0 — núcleo

- Proyecto C++ y blockout inicial.

## 1.5 — interfaz y superficies (13 nov 2026)
- Menú principal real (Continuar / Nueva partida / Cómo jugar / Salir) con ratón, teclado y mando.
- Pantalla "Cómo jugar", prólogo con botón Empezar y menú de pausa (seguir, cómo jugar, reiniciar, menú, salir).
- Texto nítido y grande (fuentes Slate escaladas a la resolución) en todo el HUD.
- Tarjeta de objetivo con DÓNDE está, QUIÉN lo hace y aviso "pulsa TAB" si estás con el personaje equivocado.
- Marcador de objetivo más grande con flecha "GIRA" cuando queda fuera de pantalla; tutorial al empezar.
- Pisos y paredes con texturas PBR reales (ambientCG CC0: color, normal, rugosidad, AO).
- Nilo: textura extraída del FBX de Meshy y material propio; búsqueda robusta de la animación de caminar.
- Importación con el importador FBX clásico también en UE 5.5+ (Interchange ignoraba escala/esqueleto/animación).
- La importación se repite sola al abrir el editor si cambió su versión (tras un `git pull`).
