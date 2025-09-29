local texture = sn.Texture()
texture:fill(sn.Color(1, 1, 1, 1))
local model = sn.Model()
model:load("BrainStem.glb")
local draw3d = sn.Draw3D(texture)
draw3d.position = sn.Vec3(0, 0, 0)
draw3d.model = model

local vertex_shader = sn.Shader()
vertex_shader:compile_load_vertex_shader("skinning.slang", 1)
local fragment_shader = sn.Shader()
fragment_shader:compile_load_fragment_shader("skinning.slang", 0)

local pipeline = sn.GraphicsPipeline3D()
pipeline:set_vertex_shader(vertex_shader)
pipeline:set_fragment_shader(fragment_shader)
pipeline:set_animation(true)
pipeline:build()


sn.Graphics.get_camera():lookat(sn.Vec3(0, -3, 1), sn.Vec3(0, 0, 1), sn.Vec3(0, 0, 1))

model:play(0)
function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    model:update(sn.Time.deltatime())
end

function draw()
    sn.Graphics.bind_pipeline3d(pipeline)
    sn.Graphics.set_uniform_data(1, model:get_bone_uniform_data())
    -- Draw texture
    sn.Graphics.draw3d(draw3d)
end
