function update()
  local act = scene.get_actor("Actor1")
  if act:has_component("draw3d") then
    local comp = act:get_component("draw3d")
    local c = cconvert_draw3d(comp)
    c:set_vertex_name("BOX")
    c:set_draw_depth(true)
  else
    act:add_component("draw3d")
  end
end
