"""Mini librería de modelado low-poly para el kit TUC (Blender bpy + bmesh).

Convenciones:
- Unidades en metros (Unreal importa 1 m = 100 cm).
- Frente del asset = +X (el eje X no se invierte al pasar Blender -> FBX -> Unreal).
- Pivote en la base, centrado en X/Y (salvo que se indique).
"""
import math
import bpy
import bmesh
from mathutils import Matrix, Vector, Euler

# name: (color, roughness, metallic, emissive, texture, tile_m)
MATERIALS = {
    # superficies texturizadas
    "M_Asphalt":     ((1, 1, 1), 0.85, 0.0, 0.0, "T_Asphalt", 2.0),
    "M_Sidewalk":    ((1, 1, 1), 0.8, 0.0, 0.0, "T_Sidewalk", 2.0),
    "M_Carpet":      ((1, 1, 1), 0.95, 0.0, 0.0, "T_Carpet", 2.0),
    "M_Checker":     ((1, 1, 1), 0.35, 0.0, 0.0, "T_Checker", 1.0),
    "M_MetalPanel":  ((1, 1, 1), 0.45, 0.6, 0.0, "T_MetalPanel", 2.0),
    "M_StoneTile":   ((1, 1, 1), 0.7, 0.0, 0.0, "T_StoneTile", 2.0),
    "M_Brick":       ((1, 1, 1), 0.9, 0.0, 0.0, "T_Brick", 2.0),
    "M_CinemaPanel": ((1, 1, 1), 0.9, 0.0, 0.0, "T_CinemaPanel", 2.0),
    "M_RetroGrid":   ((1, 1, 1), 0.6, 0.0, 0.6, "T_RetroGrid", 2.0),
    "M_StoneBlock":  ((1, 1, 1), 0.8, 0.0, 0.0, "T_StoneBlock", 2.0),
    "M_Windows":     ((1, 1, 1), 0.3, 0.0, 1.6, "T_Windows", 4.0),
    "M_ArcadeScreen":((1, 1, 1), 0.2, 0.0, 3.0, "T_ArcadeScreen", 0.0),
    "M_FilmStrip":   ((1, 1, 1), 0.3, 0.0, 2.0, "T_FilmStrip", 0.0),
    "M_Poster0":     ((1, 1, 1), 0.4, 0.0, 0.6, "T_Poster0", 0.0),
    "M_Poster1":     ((1, 1, 1), 0.4, 0.0, 0.6, "T_Poster1", 0.0),
    "M_Poster2":     ((1, 1, 1), 0.4, 0.0, 0.6, "T_Poster2", 0.0),
    # colores lisos
    "M_Black":       ((0.015, 0.015, 0.018), 0.6, 0.0, 0.0, None, 0),
    "M_DarkMetal":   ((0.06, 0.065, 0.08), 0.4, 0.8, 0.0, None, 0),
    "M_Steel":       ((0.45, 0.47, 0.52), 0.35, 1.0, 0.0, None, 0),
    "M_Gold":        ((0.85, 0.6, 0.18), 0.3, 1.0, 0.0, None, 0),
    "M_RedVelvet":   ((0.32, 0.015, 0.04), 0.95, 0.0, 0.0, None, 0),
    "M_Wood":        ((0.25, 0.12, 0.06), 0.7, 0.0, 0.0, None, 0),
    "M_Concrete":    ((0.16, 0.16, 0.18), 0.9, 0.0, 0.0, None, 0),
    "M_FacadeRed":   ((0.22, 0.03, 0.05), 0.8, 0.0, 0.0, None, 0),
    "M_Glass":       ((0.12, 0.18, 0.22), 0.05, 0.0, 0.05, None, 0),
    "M_ScreenWhite": ((0.75, 0.75, 0.8), 0.6, 0.0, 0.25, None, 0),
    "M_White":       ((0.8, 0.8, 0.82), 0.5, 0.0, 0.0, None, 0),
    "M_Orange":      ((1.0, 0.35, 0.03), 0.6, 0.0, 0.0, None, 0),
    "M_VanRed":      ((0.42, 0.04, 0.04), 0.4, 0.3, 0.0, None, 0),
    "M_Rubber":      ((0.02, 0.02, 0.02), 0.9, 0.0, 0.0, None, 0),
    "M_Purple":      ((0.12, 0.03, 0.2), 0.5, 0.0, 0.0, None, 0),
    "M_Teal":        ((0.03, 0.12, 0.16), 0.5, 0.3, 0.0, None, 0),
    # neones y luces
    "M_NeonRed":     ((1.0, 0.05, 0.12), 0.3, 0.0, 8.0, None, 0),
    "M_NeonCyan":    ((0.05, 0.8, 1.0), 0.3, 0.0, 8.0, None, 0),
    "M_NeonMagenta": ((1.0, 0.08, 0.75), 0.3, 0.0, 8.0, None, 0),
    "M_NeonViolet":  ((0.55, 0.15, 1.0), 0.3, 0.0, 8.0, None, 0),
    "M_NeonGreen":   ((0.2, 1.0, 0.3), 0.3, 0.0, 8.0, None, 0),
    "M_NeonAmber":   ((1.0, 0.65, 0.15), 0.3, 0.0, 6.0, None, 0),
    "M_LampWarm":    ((1.0, 0.8, 0.5), 0.3, 0.0, 10.0, None, 0),
    # acento que el C++ tiñe según la pieza (parámetro Tint)
    "M_AccentGlow":  ((1.0, 1.0, 1.0), 0.3, 0.0, 6.0, None, 0),
    # personajes
    "M_Skin":        ((0.72, 0.5, 0.38), 0.7, 0.0, 0.0, None, 0),
    "M_Hair":        ((0.16, 0.09, 0.05), 0.8, 0.0, 0.0, None, 0),
    "M_Jacket":      ((0.30, 0.17, 0.08), 0.75, 0.0, 0.0, None, 0),
    "M_Shirt":       ((0.14, 0.24, 0.42), 0.8, 0.0, 0.0, None, 0),
    "M_Jeans":       ((0.08, 0.11, 0.2), 0.85, 0.0, 0.0, None, 0),
    "M_CatBlack":    ((0.02, 0.02, 0.025), 0.75, 0.0, 0.0, None, 0),
    "M_CatWhite":    ((0.88, 0.88, 0.86), 0.75, 0.0, 0.0, None, 0),
    "M_CatPink":     ((0.85, 0.42, 0.48), 0.6, 0.0, 0.0, None, 0),
    "M_CatEye":      ((1.0, 0.62, 0.08), 0.15, 0.0, 0.5, None, 0),
    "M_Collar":      ((0.75, 0.03, 0.03), 0.5, 0.0, 0.0, None, 0),
}


def material(name):
    mat = bpy.data.materials.get(name)
    if mat is None:
        color, rough, metal, emis, tex, _ = MATERIALS[name]
        mat = bpy.data.materials.new(name)
        mat.use_nodes = True
        bsdf = mat.node_tree.nodes.get("Principled BSDF")
        bsdf.inputs["Base Color"].default_value = (*color, 1)
        bsdf.inputs["Roughness"].default_value = rough
        bsdf.inputs["Metallic"].default_value = metal
        if emis > 0:
            bsdf.inputs["Emission Color"].default_value = (*color, 1)
            bsdf.inputs["Emission Strength"].default_value = emis
    return mat


class Kit:
    """Acumula geometría de un asset en un bmesh con materiales por cara."""

    def __init__(self, name):
        self.name = name
        self.bm = bmesh.new()
        self.mats = []
        self.uv = self.bm.loops.layers.uv.new("UVMap")

    def _mat_index(self, mat):
        if mat not in self.mats:
            self.mats.append(mat)
        return self.mats.index(mat)

    def _finish(self, verts, mat, matrix):
        geom_faces = list({f for v in verts for f in v.link_faces})
        idx = self._mat_index(mat)
        for f in geom_faces:
            f.material_index = idx
        bmesh.ops.transform(self.bm, matrix=matrix, verts=verts)
        return geom_faces

    @staticmethod
    def _matrix(loc, rot, scale):
        m = Matrix.Translation(Vector(loc)) @ Euler([math.radians(a) for a in rot]).to_matrix().to_4x4()
        s = Matrix.Diagonal((*scale, 1.0))
        return m @ s

    def box(self, center, size, mat, rot=(0, 0, 0)):
        """Caja centrada en center con tamaño size (m)."""
        res = bmesh.ops.create_cube(self.bm, size=1.0)
        return self._finish(res["verts"], mat, self._matrix(center, rot, size))

    def box_from(self, mn, mx, mat):
        c = [(a + b) / 2 for a, b in zip(mn, mx)]
        s = [abs(b - a) for a, b in zip(mn, mx)]
        return self.box(c, s, mat)

    def cyl(self, center, radius, height, mat, segs=12, rot=(0, 0, 0), radius2=None):
        """Cilindro (o cono si radius2) centrado en center, eje Z local."""
        r2 = radius if radius2 is None else radius2
        res = bmesh.ops.create_cone(self.bm, cap_ends=True, cap_tris=False, segments=segs,
                                    radius1=1.0, radius2=r2 / radius if radius else 0.0, depth=1.0)
        return self._finish(res["verts"], mat, self._matrix(center, rot, (radius, radius, height)))

    def cone(self, center, radius, height, mat, segs=8, rot=(0, 0, 0)):
        res = bmesh.ops.create_cone(self.bm, cap_ends=True, cap_tris=False, segments=segs,
                                    radius1=1.0, radius2=0.0, depth=1.0)
        return self._finish(res["verts"], mat, self._matrix(center, rot, (radius, radius, height)))

    def sphere(self, center, radii, mat, segs=10, rings=7, rot=(0, 0, 0)):
        res = bmesh.ops.create_uvsphere(self.bm, u_segments=segs, v_segments=rings, radius=1.0)
        return self._finish(res["verts"], mat, self._matrix(center, rot, radii))

    def ico(self, center, radii, mat, subdiv=1, rot=(0, 0, 0)):
        res = bmesh.ops.create_icosphere(self.bm, subdivisions=subdiv, radius=1.0)
        return self._finish(res["verts"], mat, self._matrix(center, rot, radii))

    def torus(self, center, major, minor, mat, segs=14, ring=6, rot=(0, 0, 0)):
        verts = []
        rings = []
        for i in range(segs):
            a = 2 * math.pi * i / segs
            ring_verts = []
            for j in range(ring):
                b = 2 * math.pi * j / ring
                p = Vector(((major + minor * math.cos(b)) * math.cos(a), (major + minor * math.cos(b)) * math.sin(a), minor * math.sin(b)))
                ring_verts.append(self.bm.verts.new(p))
            rings.append(ring_verts)
            verts += ring_verts
        for i in range(segs):
            for j in range(ring):
                a, b = rings[i][j], rings[(i + 1) % segs][j]
                c, d = rings[(i + 1) % segs][(j + 1) % ring], rings[i][(j + 1) % ring]
                self.bm.faces.new((a, b, c, d))
        return self._finish(verts, mat, self._matrix(center, rot, (1, 1, 1)))

    def build(self, smooth=False):
        bm = self.bm
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        # UV: proyección por cara según el eje dominante, en metros / tamaño de tile.
        for f in bm.faces:
            mname = self.mats[f.material_index]
            tile = MATERIALS[mname][5]
            n = f.normal
            ax = max(range(3), key=lambda i: abs(n[i]))
            if tile > 0:
                for loop in f.loops:
                    co = loop.vert.co
                    if ax == 2:
                        u, v = co.x, co.y
                    elif ax == 0:
                        u, v = co.y, co.z
                    else:
                        u, v = co.x, co.z
                    loop[self.uv].uv = (u / tile, v / tile)
            else:
                # textura completa por cara (pantallas, pósters)
                cos = [l.vert.co for l in f.loops]
                if ax == 2:
                    pts = [(c.x, c.y) for c in cos]
                elif ax == 0:
                    pts = [(c.y, c.z) for c in cos]
                else:
                    pts = [(c.x, c.z) for c in cos]
                us = [p[0] for p in pts]; vs = [p[1] for p in pts]
                du = (max(us) - min(us)) or 1; dv = (max(vs) - min(vs)) or 1
                for loop, p in zip(f.loops, pts):
                    u = (p[0] - min(us)) / du
                    if ax == 0 and n.x < 0:
                        u = 1 - u
                    if ax == 1 and n.y > 0:
                        u = 1 - u
                    loop[self.uv].uv = (u, (p[1] - min(vs)) / dv)
        me = bpy.data.meshes.new(self.name)
        bm.to_mesh(me)
        bm.free()
        for m in self.mats:
            me.materials.append(material(m))
        for p in me.polygons:
            p.use_smooth = smooth
        obj = bpy.data.objects.new(self.name, me)
        bpy.context.scene.collection.objects.link(obj)
        return obj


def export(obj, path):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.ops.export_scene.fbx(filepath=path, use_selection=True, object_types={"MESH"}, apply_unit_scale=True,
                             bake_space_transform=False, path_mode="STRIP", embed_textures=False,
                             add_leaf_bones=False, mesh_smooth_type="FACE")
