local texture = Texture()
texture:fill_color(Color(1, 1, 1, 1))
local model = Model()
model:load("BrainStem.glb")
local draw3d = Draw3D()
draw3d.material:append(texture)
draw3d.position = Vec3(0, 0, 0)
draw3d.model = model

local vertex_shader = Shader()
vertex_shader:compile_and_load_vertex_shader("skinning.slang", 1)
fragment_shader = Shader()
fragment_shader:compile_and_load_fragment_shader("skinning.slang", 0)

local pipeline = GraphicsPipeline3D()
pipeline:set_vertex_shader(vertex_shader)
pipeline:set_fragment_shader(fragment_shader)
pipeline:set_animation(true)
pipeline:build()


pos = Vec3(0, -3, 1)
scene.camera():lookat(pos, Vec3(0, 0, 1), Vec3(0, 0, 1))

model:play(0)
function update()
    model:update(scene.delta_time())
end

function draw()
    Graphics.bind_pipeline3d(pipeline)
    Graphics.set_uniform_data(1, model:bone_uniform_data())
    -- Draw texture
    draw3d:draw()
end
