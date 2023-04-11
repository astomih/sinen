local table = {
  update = function()
    local act = scene.get_actor("Player")
    if act:has_component("draw3d") then
      local comp = act:get_component("draw3d")
      local c = cconvert_draw3d(comp)
      c:set_vertex_name("BOX")
      c:set_draw_depth(true)
    end
  end
}
return table
