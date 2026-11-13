# Revisión v1.1 — auditoría sin abrir Unreal

Fecha: 13 de noviembre de 2026. Revisión hecha leyendo código, configuración, scripts y assets, sin ejecutar Unreal Editor. Todo lo que dependa del motor (compilación real, iluminación, escala visual) se valida en la primera apertura con la lista del final.

## 1. Bloqueos encontrados en la v1.0 (y corregidos)

| # | Problema | Efecto en la v1.0 | Corrección v1.1 |
|---|---|---|---|
| 1 | La interacción trazaba una línea **horizontal** desde la cápsula (Ivo a ~142 cm, Nilo a ~59 cm). Todas las piezas medían menos: consola hasta 55 cm, conducto hasta 6 cm, rollo hasta 52 cm. | **Ninguna pieza de Ivo era alcanzable**: la campaña se quedaba en el paso 0. Nilo tampoco alcanzaba conducto, rollo ni ducto. | Selección por proximidad + dirección de la mirada, con línea de visión. Ya no depende de la altura ni de apuntar al píxel. |
| 2 | El guardado se leía en `BeginPlay` del Director, que corre **después** del `BeginPlay` del GameMode. | Al continuar una partida: personajes en la calle, mundo paralelo apagado, objetivo de otro capítulo. | Inicialización explícita y ordenada: estado → mundo → personajes → patrullas → checkpoint. |
| 3 | Checkpoints dentro del radio de las patrullas (pasos 2, 4, 6 y 9; el 9 justo encima de la ruta del centinela). Detección por radio de 370 cm sin cono de visión. | Bucle de detección → rebobinado → detección cada 3 s. | Checkpoints fuera de rango, cono de visión de 50° visible en el suelo, barra de detección progresiva, 2,5 s de gracia tras rebobinar, una patrulla por capítulo y desactivada fuera de él. |
| 4 | Salas de años 80, futuro y Archivo sin paredes en los extremos; calle sin bordes. | Caída infinita al vacío; el personaje se destruye y la partida queda rota. | Paredes de cierre en todos los sets y rebobinado automático si alguien cae. |
| 5 | Los recuerdos se guardaban solo como contador. | Tras recargar, el mismo recuerdo se podía recoger otra vez: 3/3 con uno solo. | Máscara por fragmento en el guardado; los ya recogidos no reaparecen. Compatible con guardados v1.0. |
| 6 | `BuildSettingsVersion.V5` | No compila en UE 5.1–5.3. | `BuildSettingsVersion.Latest`. |
| 7 | Entrada con `BindAxis/BindAction` pero sin fijar las clases de input. | En UE 5.1+ el proyecto queda con Enhanced Input por defecto: riesgo de que no responda el teclado. | `DefaultInput.ini` fija `PlayerInput`/`InputComponent` clásicos. |
| 8 | Textos con tildes en archivos UTF-8 sin BOM. | MSVC con configuración regional en español los lee como Windows-1252: «CAPÃTULO». | Todos los `.h/.cpp` con BOM UTF-8. |
| 9 | Reinicio con `OpenLevel(GetWorld()->GetName())`. | En PIE y con `/Engine/Maps/Entry` puede abrir un mapa equivocado o fallar. | Reinicio en el mismo mundo (sin recargar el nivel). |
| 10 | El mando Options cerraba la portada y pausaba en la misma pulsación. | — (no existía soporte de mando) | Mando añadido con protección de esa pulsación. |

## 2. Assets

| Problema | Corrección |
|---|---|
| Los FBX `Optimized` referenciaban texturas con **rutas absolutas de otra máquina** (`/workspace/scratch/.../Image_0.jpg`). Al importar, el material salía gris. | FBX re-exportados sin referencias a imágenes. `import_external_assets.py` importa los PNG, crea un material PBR (base color, normal, roughness, metallic) y lo asigna. |
| Escala: el gato mide 1,9 m de largo en el FBX y el código lo ponía a escala 1 → **gato de casi 2 m** atravesando la cápsula. El proyector a escala 1 medía ~1,9 m. | El C++ ajusta escala y apoyo por los bounds de la malla (gato 70 cm, proyector 110 cm), venga el FBX en metros o centímetros. |
| `LoadObject` del gato en el **constructor** (se ejecuta al cargar el módulo). | Carga en `BeginPlay`, solo si el paquete existe: sin errores en el log antes de importar. |
| Con Interchange (UE 5.5+) el asset podía quedar con otro nombre y el C++ no lo encontraba. | El script busca la malla importada y la renombra al nombre esperado. |
| Ivo no tenía cuerpo: al controlar a Nilo, Ivo era invisible. | Placeholder low-poly (invisible en su propia primera persona). Nilo placeholder ahora parece un gato esmoquin con collar rojo. |

**Cielo:** las salas no tienen techo y en la v1.0 arriba solo había negro. Ahora el mundo genera un cielo de crepúsculo con componentes del motor (SkyAtmosphere + sol bajo + SkyLight en tiempo real). No necesita assets descargados.

Vistas previas de los FBX limpios con su textura: `Docs/Previews/`.

**Diferencia con el arte del proyecto:** el gato descargado es un **siamés de ojos azules**; Nilo en el mockup y en el plan de assets es un **gato esmoquin blanco y negro con collar rojo y ojos ámbar**. Sirve como referencia de escala, no como Nilo final. Tampoco tiene rig ni animaciones.

## 3. Historia: alineación con la biblia narrativa

| Canon (03_BIBLIA / 04_GUION / 10_CONTRATOS) | v1.0 | v1.1 |
|---|---|---|
| Prólogo P0 «CORTE 001 — FINAL REGISTRADO», Ivo sale solo con la campanilla | No existía | Portada con el prólogo; flag `preview_seen` guardado |
| Final A: Ivo sale, Nilo queda | «Ivo conserva la película, pero pierde la salida» (contradice el canon) | «Corte original», se activa con Nilo sosteniendo la puerta |
| Final B: Nilo sale, Ivo queda | «Nilo rompe el montaje» (ambiguo) | «Corte invertido», se activa con Ivo |
| Final C: solo con el Ancla preparada, nunca por azar | Siempre disponible; los recuerdos solo cambiaban el texto | Requiere Ancla 3/3; el prompt dice cuántos fragmentos faltan |
| El Montador: «El corte registrado garantiza estabilidad» | No aparecía | Aparece al abrir el Archivo |
| Registros alternativos (L2-S03), «Vamos juntos, Nilo», «Esto no es una película normal» | No aparecían | Integrados en los mensajes de avance |
| Universos paralelos, no viajes en el tiempo | Textos tipo «CORTE 02: FUTURO» | Se mantiene la estética, sin lenguaje de viaje temporal |

## 4. Lo que no cambió (decisiones abiertas)

- La campaña sigue siendo **compacta** (4 sets en un solo mapa generado en código), no los tres niveles completos del roadmap.
- Sin animaciones, audio, inventario ni loot (el mockup los muestra; el código aún no).
- Los nombres Ivo, Nilo, Montador y Ancla siguen siendo [PROPUESTA] en la documentación.

## 5. Lista para la primera apertura

1. Cerrar el otro proyecto de Unreal o abrir una segunda instancia del editor.
2. Clic derecho en `TheUnfinishedCut.uproject` → **Generate Visual Studio project files** → abrir el `.sln` → compilar **Development Editor | Win64**.
   - Si falla, el primer error del *Output* de Visual Studio es el que importa. Errores típicos según la versión: firma de `LoadObject`, `APointLight::PointLightComponent` o `AHUD::GetTextSize`. Son una línea cada uno.
3. Abrir el proyecto → Play → `Enter`. Comprobar:
   - [ ] La portada muestra el prólogo con tildes correctas.
   - [ ] WASD/ratón funcionan; `Tab` cambia de cámara; `E` sobre la consola celeste muestra `[E] Restaurar energía`.
   - [ ] El centinela del cine proyecta un cono de luz y la barra «TE ESTÁN VIENDO» se llena.
   - [ ] Los letreros «ETERNAL WAVE», «ARCADE» y «CORTE 001» se leen. Si salen invisibles o espejados, girar 180° en `SpawnSign` (yaw 0).
   - [ ] Arriba se ve un cielo de crepúsculo (no negro). Si sale demasiado claro u oscuro, cambiar la intensidad del sol en `TUCBlockoutBuilder.cpp` (2.5).
   - [ ] La escena no está ni negra ni quemada. Si lo está, ajustar exposición (`DefaultEngine.ini`) o las intensidades en `TUCBlockoutBuilder.cpp`.
4. `Tools → Execute Python Script → Tools/import_external_assets.py`. Volver a Play:
   - [ ] Nilo es el gato importado, de ~70 cm, apoyado en el suelo y mirando hacia delante. Si mira de lado, cambiar `ImportedCatYawOffset` (−90 → 90 o 0).
   - [ ] Los tres proyectores usan el modelo con textura.
   - [ ] Si la iluminación del gato se ve invertida, poner `FLIP_NORMAL_GREEN = False` en el script y repetir.
5. Opcional: `Tools/create_prototype_map.py` para tener un mapa propio en lugar de `/Engine/Maps/Entry`.

## 6. Inventario de assets (comprobado)

| Asset | Origen | Estado |
|---|---|---|
| Cubo, cilindro, esfera, cono, `BasicShapeMaterial` | Incluidos en todo UE5 (`/Engine/BasicShapes`) | Disponibles siempre |
| Cielo, sol, niebla, luz ambiental | Componentes del motor | Disponibles siempre |
| `SM_Nilo_Base_Optimized` + 4 texturas | `ExternalAssets/Meshy/Optimized/Nilo_Base` | Se crean con el script de importación |
| `SM_RetroProjector_Optimized` + 4 texturas | `ExternalAssets/Meshy/Optimized/Retro_Projector` | Se crean con el script de importación |

Todas las rutas del código existen o las crea el script. Si falta un asset importado, el juego usa el placeholder y sigue funcionando.

Lo que **no** tiene todavía: texturas de suelo y paredes (son colores planos de blockout), animaciones, audio y modelos de Ivo, centinelas y decorado.
