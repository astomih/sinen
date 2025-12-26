local model = sn.Model.new()
local material = model:getMaterial()
local transform = sn.Transform.new()
local light_transform = sn.Transform.new()
local vertex_shader = sn.Shader.new()
local fragment_shader = sn.Shader.new()
local pipeline3d = sn.GraphicsPipeline.new()
local light_pos = sn.Vec3.new(2, 0, 0)
local light_intensity = 2.5
local uniform_data = sn.UniformData.new()

function Setup()
    model:load("DamagedHelmet.glb")

    transform.position = sn.Vec3.new(0, 0, 0)
    transform.rotation = sn.Vec3.new(90, 0, 0)
    transform.scale = sn.Vec3.new(1, 1, 1)

    light_transform.position = sn.Vec3.new(0)
    light_transform.rotation = sn.Vec3.new(90, 0, 0)
    light_transform.scale = sn.Vec3.new(1)


    local pos = sn.Vec3.new(0.7, 0.7, 2.1)
    local at = sn.Vec3.new(0)
    local up = sn.Vec3.new(0, 1, 0)
    sn.Graphics.getCamera():lookat(pos, at, up)

    vertex_shader:compileLoadVertexShader("shader_custom.slang")
    fragment_shader:compileLoadFragmentShader("shader_custom.slang")
    pipeline3d:setVertexShader(vertex_shader)
    pipeline3d:setFragmentShader(fragment_shader)
    pipeline3d:setEnableTangent(true)
    pipeline3d:setEnableDepthTest(true)
    pipeline3d:build()
    sn.Graphics.bindPipeline(pipeline3d)

    uniform_data:add(pos.x)
    uniform_data:add(pos.y)
    uniform_data:add(pos.z)
    uniform_data:add(light_pos.x)
    uniform_data:add(light_pos.y)
    uniform_data:add(light_pos.z)
    uniform_data:add(light_intensity)
end

function Update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        sn.Script.load("main", ".")
    end
    transform.rotation.z = transform.rotation.z + sn.Time.delta() * 10
    if sn.Keyboard.isDown(sn.Keyboard.LEFT) then
        light_pos.x = light_pos.x - sn.Time.delta() * 5
    end
    if sn.Keyboard.isDown(sn.Keyboard.RIGHT) then
        light_pos.x = light_pos.x + sn.Time.delta() * 5
    end
    if sn.Keyboard.isDown(sn.Keyboard.UP) then
        if sn.Keyboard.isDown(sn.Keyboard.LSHIFT) then
            light_pos.z = light_pos.z + sn.Time.delta() * 5
        else
            light_pos.y = light_pos.y + sn.Time.delta() * 5
        end
    end
    if sn.Keyboard.isDown(sn.Keyboard.DOWN) then
        if sn.Keyboard.isDown(sn.Keyboard.LSHIFT) then
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

function Draw()
    sn.Graphics.setUniformData(1, uniform_data)
    sn.Graphics.drawModel(model, transform, material)
    sn.Graphics.drawModel(model, light_transform, material)
end
