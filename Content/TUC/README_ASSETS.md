# Assets de producción

Esta carpeta recibirá los `.uasset` generados al importar los FBX/GLB de Meshy.

Estructura prevista:

- `Characters/Ivo`
- `Characters/Nilo`
- `Environment/Cinema`
- `Environment/Street`
- `Props/Projector`
- `Props/PowerConsole`
- `Materials`
- `Maps`

El prototipo 0.1 no depende de estos assets: genera un blockout con formas básicas para validar la jugabilidad primero.

## Importación (v1.1)

`Tools/import_external_assets.py` crea:

- `Characters/Nilo/SM_Nilo_Base_Optimized` + `M_Nilo_Base_PBR` + `Textures/`
- `Props/Projector/SM_RetroProjector_Optimized` + `M_RetroProjector_PBR` + `Textures/`

El código busca exactamente esas dos rutas de malla. Si no existen, usa placeholders.
