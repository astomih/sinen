local model = sn.Model()
model:load("DamagedHelmet.glb")
local material = model:get_material()


local transform = sn.Transform()
transform.position = sn.Vec3(0, 0, 0)
transform.rotation = sn.Vec3(90, 0, 0)
transform.scale = sn.Vec3(1, 1, 1)

local light_transform = sn.Transform()
light_transform.position = sn.Vec3(0)
light_transform.rotation = sn.Vec3(90, 0, 0)
light_transform.scale = sn.Vec3(1)


local pos = sn.Vec3(0.7, 0.7, 2.1)
local at = sn.Vec3(0)
local up = sn.Vec3(0, 1, 0)
sn.Graphics.get_camera():lookat(pos, at, up)

local vertex_shader = sn.Shader()
vertex_shader:compile_load_vertex_shader("shader_custom.slang")
local fragment_shader = sn.Shader()
fragment_shader:compile_load_fragment_shader("shader_custom.slang")
local pipeline3d = sn.GraphicsPipeline3D()
pipeline3d:set_vertex_shader(vertex_shader)
pipeline3d:set_fragment_shader(fragment_shader)
pipeline3d:set_enable_tangent(true)
pipeline3d:build()

local light_pos = sn.Vec3(2, 0, 0)
local light_intensity = 2.5

local uniform_data = sn.UniformData()
uniform_data:add(pos.x)
uniform_data:add(pos.y)
uniform_data:add(pos.z)
uniform_data:add(light_pos.x)
uniform_data:add(light_pos.y)
uniform_data:add(light_pos.z)
uniform_data:add(light_intensity)


function update()
    if sn.Keyboard.is_pressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    transform.rotation.z = transform.rotation.z + sn.Time.delta() * 10
    if sn.Keyboard.is_down(sn.Keyboard.LEFT) then
        light_pos.x = light_pos.x - sn.Time.delta() * 5
    end
    if sn.Keyboard.is_down(sn.Keyboard.RIGHT) then
        light_pos.x = light_pos.x + sn.Time.delta() * 5
    end
    if sn.Keyboard.is_down(sn.Keyboard.UP) then
        if sn.Keyboard.is_down(sn.Keyboard.LSHIFT) then
            light_pos.z = light_pos.z + sn.Time.delta() * 5
        else
            light_pos.y = light_pos.y + sn.Time.delta() * 5
        end
    end
    if sn.Keyboard.is_down(sn.Keyboard.DOWN) then
        if sn.Keyboard.is_down(sn.Keyboard.LSHIFT) then
            light_pos.z = light_pos.z - sn.Time.delta() * 5
        else
            light_pos.y = light_pos.y - sn.Time.delta() * 5
        end
    end
    uniform_data:change(light_pos.x, 3)
    uniform_data:change(light_pos.y, 4)
    uniform_data:change(light_pos.z, 5)
    light_transform.position = light_pos
end

function draw()
    sn.Graphics.bind_pipeline3d(pipeline3d)
    sn.Graphics.set_uniform_data(1, uniform_data)
    sn.Graphics.draw_model(model, transform, material)
    sn.Graphics.draw_model(model, light_transform, material)
end
