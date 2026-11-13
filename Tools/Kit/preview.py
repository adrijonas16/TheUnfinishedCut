import sys, os, json, math, bpy
from mathutils import Vector
out_png, root, names = sys.argv[-3], sys.argv[-2], sys.argv[-1].split(',')
man=json.load(open(os.path.join(root,'kit_manifest.json')))
bpy.ops.wm.read_factory_settings(use_empty=True)
x=0.0; objs=[]
for n in names:
    bpy.ops.import_scene.fbx(filepath=os.path.join(root,'Meshes',n+'.fbx'))
    o=[o for o in bpy.context.selected_objects if o.type=='MESH'][0]
    w=o.dimensions.y
    o.location=(0, x + w/2, 0); x += w + 0.4; objs.append(o)
for m in bpy.data.materials:
    base=m.name.split('.')[0]; spec=man['materials'].get(base)
    if not spec: continue
    m.use_nodes=True; nt=m.node_tree; b=nt.nodes.get('Principled BSDF')
    b.inputs['Base Color'].default_value=(*spec['color'],1); b.inputs['Roughness'].default_value=spec['roughness']; b.inputs['Metallic'].default_value=spec['metallic']
    if spec['texture']:
        t=nt.nodes.new('ShaderNodeTexImage'); t.image=bpy.data.images.load(os.path.join(root,'Textures',spec['texture']+'.png'))
        nt.links.new(t.outputs['Color'], b.inputs['Base Color'])
        if spec['emissive']>0: nt.links.new(t.outputs['Color'], b.inputs['Emission Color'])
    elif spec['emissive']>0: b.inputs['Emission Color'].default_value=(*spec['color'],1)
    b.inputs['Emission Strength'].default_value=min(spec['emissive'],3)
sc=bpy.context.scene; sc.render.engine='CYCLES'; sc.cycles.samples=24; sc.render.resolution_x=1400; sc.render.resolution_y=600
w=bpy.data.worlds.new('w'); w.use_nodes=True; w.node_tree.nodes['Background'].inputs[0].default_value=(0.35,0.37,0.42,1); sc.world=w
cx=x/2; H=max(o.dimensions.z for o in objs)
cam=bpy.data.cameras.new('c'); cam.type='ORTHO'; cam.ortho_scale=max(x, H*2.4)*1.02
co=bpy.data.objects.new('c',cam); sc.collection.objects.link(co); sc.camera=co
co.location=(9, cx-7, H*0.55+3.2); d=Vector((0,cx-0.2,H*0.45))-co.location; co.rotation_euler=d.to_track_quat('-Z','Y').to_euler()
l=bpy.data.lights.new('s','SUN'); l.energy=3.5; lo=bpy.data.objects.new('s',l); sc.collection.objects.link(lo); lo.rotation_euler=(0.7,0.2,1.2)
sc.render.filepath=out_png; bpy.ops.render.render(write_still=True); print('RENDERED',out_png)
