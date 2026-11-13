# The Unfinished Cut — Campaña compacta 1.1

> **v1.1:** revisión completa sin abrir Unreal. Corrige bloqueos que impedían terminar la v1.0 (ver `REVISION_v1.1.md`).

Juego corto completo para Unreal Engine 5, construido en código para que pueda abrirse y jugarse sin montar mapas ni Blueprints manualmente.

## Implementado en código

- Ivo en primera persona con linterna.
- Nilo en tercera persona con placeholder.
- Cambio de personaje con `Tab`.
- Orden seguir/esperar con `F`.
- Interacción con `E`.
- Campaña generada en tiempo de ejecución con cuatro capítulos: cine original, años 80, futuro y Archivo de los Ecos.
- Cadena completa de puzles cooperativos que obliga a alternar Ivo y Nilo.
- Tres viajes entre películas con checkpoints automáticos.
- HUD con personaje activo, objetivo y prompt contextual.
- Cuatro centinelas (uno por capítulo) con cono de visión visible, barra de detección y rebobinado al último checkpoint.
- Prólogo «CORTE 001 — FINAL REGISTRADO» en la portada.
- Tres fragmentos del Ancla opcionales.
- Tres finales canónicos: A (Corte original, Ivo sale), B (Corte invertido, Nilo sale) y C (Corte inacabado, salen ambos; requiere Ancla 3/3).
- Pantalla de inicio, pausa, créditos y guardado automático de campaña.
- Dos assets comunitarios CC0 descargados: gato base y proyector retro.
- Herramienta para crear copias optimizadas de FBX sin modificar los originales.

## Controles

| Control | Acción |
|---|---|
| WASD | Moverse |
| Ratón | Mirar |
| E (mando: R2 / A) | Interactuar |
| Tab (mando: L1) | Cambiar entre Ivo y Nilo |
| F o Q (mando: Triángulo / Y) | Ordenar seguir/esperar al compañero |
| P (mando: Options) | Pausa / continuar |
| Backspace (mando: Share) | Borrar el guardado y comenzar una partida nueva |

El mando también se mueve con los sticks.

## Apertura en Windows

1. Instalar Unreal Engine 5.1 o superior y Visual Studio 2022 con **Game development with C++**.
2. Hacer clic derecho en `TheUnfinishedCut.uproject` y elegir **Generate Visual Studio project files**.
3. Abrir el `.sln`, compilar `Development Editor | Win64` y abrir el `.uproject`.
4. Si Unreal pregunta por la versión del motor, elegir la instalada.
5. Presionar Play y luego `Enter`: la campaña se construye automáticamente incluso usando el mapa vacío de inicio.
6. Después de comprobar que funciona, ejecutar `Tools/import_external_assets.py` desde **Tools → Execute Python Script**: importa las copias optimizadas del gato y el proyector, crea sus materiales PBR y deja los nombres que espera el código. La escala se ajusta sola.

Si el mapa de inicio de Unreal no permite Play en tu versión concreta, ejecuta `Tools/create_prototype_map.py`, abre `L0_Campaign` y vuelve a presionar Play.

## Alcance real

Esta es una campaña compacta de jam con principio, progresión, final y guardado. No es todavía una producción comercial con animaciones finales, cinemáticas renderizadas o arte propio en cada objeto. La entrega es código fuente preparado, pero todavía debe compilarse y probarse dentro de Unreal: el entorno donde fue generado no incluye Unreal Editor, por lo que cualquier incompatibilidad específica de tu versión se corrige con el primer log de compilación.

Los modelos descargados están en `ExternalAssets/Meshy`. Los originales son pesados; importa en Unreal las versiones de `Optimized`, no los FBX originales. El gato descargado es un siamés estático (sin rig ni animaciones) y no coincide con el diseño de Nilo (gato esmoquin con collar rojo): sirve como referencia de escala.

Consulta `CAMPAIGN_WALKTHROUGH.md` si necesitas verificar todo el recorrido o salir de un bloqueo.

## Clonar en otra PC

1. Instala Unreal Engine 5 (5.1 o superior) y Visual Studio 2022 con "Game development with C++".
2. `git clone https://github.com/adrijonas16/TheUnfinishedCut.git`
3. Doble clic en `TheUnfinishedCut.uproject` → acepta **Yes** cuando pida compilar los módulos.
4. La primera vez, el editor importa solo todos los modelos (ExternalAssets → /Game/TUC). Espera a que termine y pulsa Play.

Si luego guardas assets de Unreal (`.uasset`/`.umap`) en el repo, instala Git LFS (`git lfs install`) antes de hacer commit.
