local model = sn.Model.new()
model:load("DamagedHelmet.glb")
local material = model:getMaterial()

local LUT = sn.Texture.new()
LUT:load("citrus_orchard_road_puresky_2k/brdfLUT.png")
material:setTexture(LUT, 7)

local cubemap = sn.Cubemap.new()
cubemap:load("citrus_orchard_road_puresky_2k/citrus_orchard_road_puresky_2k.exr")
material:setCubemap(cubemap, 0) -- cubemap is set after texture


local irradiance = sn.Cubemap.new()
irradiance:load("citrus_orchard_road_puresky_2k/irradiance.exr")
material:setCubemap(irradiance, 1)

local prefiltered = sn.Cubemap.new()
prefiltered:load("citrus_orchard_road_puresky_2k/m1.exr")
material:setCubemap(prefiltered, 2)



local transform = sn.Transform.new()
transform.position = sn.Vec3.new(0, 0, 0)
transform.rotation = sn.Vec3.new(90, 0, 0)
transform.scale = sn.Vec3.new(1, 1, 1)

local light_transform = sn.Transform.new()
light_transform.position = sn.Vec3.new(0)
light_transform.rotation = sn.Vec3.new(90, 0, 0)
light_transform.scale = sn.Vec3.new(1)


local pos = sn.Vec3.new(0.7, 0.7, 2.1)
local at = sn.Vec3.new(0)
local up = sn.Vec3.new(0, 1, 0)
sn.Graphics.getCamera():lookat(pos, at, up)


local vertex_shader = sn.Shader.new()
vertex_shader:compileLoadVertexShader("shader_custom.slang")
local fragment_shader = sn.Shader.new()
fragment_shader:compileLoadFragmentShader("shader_custom.slang")
local vsCubemap = sn.Shader.new()
vsCubemap:compileLoadVertexShader("cubemap.slang")
local fsCubemap = sn.Shader.new()
fsCubemap:compileLoadFragmentShader("cubemap.slang")

local cubemapPipeline = sn.GraphicsPipeline.new()
cubemapPipeline:setVertexShader(vsCubemap)
cubemapPipeline:setFragmentShader(fsCubemap)
cubemapPipeline:setEnableDepthTest(false)
cubemapPipeline:build()


local pipeline3d = sn.GraphicsPipeline.new()
pipeline3d:setVertexShader(vertex_shader)
pipeline3d:setFragmentShader(fragment_shader)
pipeline3d:setEnableTangent(true)
pipeline3d:setEnableDepthTest(true)
pipeline3d:build()

local light_pos = sn.Vec3.new(2, 0, 0)
local light_intensity = 2.5

local uniform_data = sn.UniformData.new()
uniform_data:add(pos.x)
uniform_data:add(pos.y)
uniform_data:add(pos.z)
uniform_data:add(light_pos.x)
uniform_data:add(light_pos.y)
uniform_data:add(light_pos.z)
uniform_data:add(light_intensity)


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
    uniform_data:change(light_pos.x, 3)
    uniform_data:change(light_pos.y, 4)
    uniform_data:change(light_pos.z, 5)
    light_transform.position = light_pos
end

function draw()
    sn.Graphics.bindPipeline(cubemapPipeline)
    sn.Graphics.drawCubemap(cubemap)

    sn.Graphics.bindPipeline(pipeline3d)
    sn.Graphics.setUniformData(1, uniform_data)
    sn.Graphics.drawModel(model, transform, material)
    sn.Graphics.drawModel(model, light_transform, material)
end
