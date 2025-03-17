-- TODO: Shading
local texture = Texture()
texture:fill_color(Color(1, 1, 1, 1))
local model = Model()
model:load("brainstem.glb")
local draw3d = Draw3D(texture)
draw3d.position = Vector3(0, 0, 0)
draw3d.model = model

local vertex_shader = Shader()
vertex_shader:compile_and_load_vertex_shader("skinning.slang", 1)
local fragment_shader = Shader()
fragment_shader:compile_and_load_fragment_shader("skinning.slang", 0)

local pipeline = RenderPipeline3D()
pipeline:set_vertex_shader(vertex_shader)
pipeline:set_fragment_shader(fragment_shader)
pipeline:set_animation(true)
pipeline:build()


local pos = Vector3(1, 1, 3)
local speed = 100

function Update()
  if keyboard.is_down(keyboard.W) then
    pos.z = pos.z - speed
  end

  if keyboard.is_down(keyboard.S) then
    pos.z = pos.z + speed
  end

  if keyboard.is_down(keyboard.A) then
    pos.x = pos.x - speed
  end

  if keyboard.is_down(keyboard.D) then
    pos.x = pos.x + speed
  end

  if keyboard.is_down(keyboard.SPACE) then
    pos.y = pos.y + speed
  end

  if keyboard.is_down(keyboard.LSHIFT) then
    pos.y = pos.y - speed
  end

  scene.main_camera():lookat(pos, Vector3(0, 0, 0), Vector3(0, 1, 0))
end

function Draw()
  renderer.begin_pipeline3d(pipeline)
  renderer.set_uniform_data(1, model:bone_uniform_data())
  -- Draw texture
  draw3d:draw()
  renderer.end_pipeline()
end
