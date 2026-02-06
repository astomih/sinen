local model = sn.Model.new()
model:load("DamagedHelmet.glb")
local transform = sn.Transform.new()
local light_transform = sn.Transform.new()
local vertex_shader = sn.Shader.new()
local fragment_shader = sn.Shader.new()
local pipeline3d = sn.GraphicsPipeline.new()
local light_pos = sn.Vec3.new(2, 0, 0)
local light_intensity = 2.5
local uniform_data = {}

function setup()
    transform.position = sn.Vec3.new(0, 0, 0)
    transform.rotation = sn.Vec3.new(90, 0, 0)
    transform.scale = sn.Vec3.new(1, 1, 1)

    light_transform.position = sn.Vec3.new(0)
    light_transform.rotation = sn.Vec3.new(90, 0, 0)
    light_transform.scale = sn.Vec3.new(1)


    local pos = sn.Vec3.new(0.7, 0.7, 2.1)
    local at = sn.Vec3.new(0)
    local up = sn.Vec3.new(0, 1, 0)
    sn.Graphics.getCamera3D():lookat(pos, at, up)

    vertex_shader:compileAndLoad("shader_custom.slang", sn.ShaderStage.Vertex)
    fragment_shader:compileAndLoad("shader_custom.slang", sn.ShaderStage.Fragment)
    pipeline3d:setVertexShader(vertex_shader)
    pipeline3d:setFragmentShader(fragment_shader)
    pipeline3d:setEnableTangent(true)
    pipeline3d:setEnableDepthTest(true)
    pipeline3d:build()

    uniform_data[1] = pos
    uniform_data[2] = light_pos
    uniform_data[3] = light_intensity
end

function update()
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
    uniform_data[2] = light_pos
    light_transform.position = light_pos
end

function draw()
    sn.Graphics.setGraphicsPipeline(pipeline3d)

    sn.Graphics.setTexture(1, model:getTexture(sn.TextureKey.Normal))
    sn.Graphics.setTexture(2, model:getTexture(sn.TextureKey.DiffuseRoughness))
    sn.Graphics.setTexture(3, model:getTexture(sn.TextureKey.Metalness))
    sn.Graphics.setTexture(4, model:getTexture(sn.TextureKey.Emissive))
    sn.Graphics.setTexture(5, model:getTexture(sn.TextureKey.LightMap))

    sn.Graphics.setUniformBuffer(1, sn.Buffer.new(uniform_data))
    sn.Graphics.drawModel(model, transform)
    sn.Graphics.drawModel(model, light_transform)
end
