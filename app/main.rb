$gtk.ffi_misc.gtk_dlopen("ext")

WIDTH  = FFI::MatoCore::terrain_size * FFI::MatoCore::get_zoom
HEIGHT = FFI::MatoCore::terrain_size * FFI::MatoCore::get_zoom

class GTK::Inputs
  def left_right
    return 0 if !self.left == !self.right
    return -1 if self.left
    return 1 if self.right
  end

  def up_down
    custom_up = self.up || self.keyboard.space
    return 0 if !custom_up == !self.down
    return 1 if custom_up
    return -1 if self.down
  end
end


# @param args [GTK::Args]
def tick in_args
  args                      = in_args || $args
  args.state.map_seed       ||= 5
  args.state.map_slice      ||= 0
  args.state.brush_material ||= "NONE"
  args.state.vert_move      ||= 0
  args.state.horz_move      ||= 0
  args.state.do_terrain_gen ||= false
  args.state.timer ||= Time.now
  if args.state.do_terrain_gen
    FFI::MatoCore::generate_terrain(args.state.map_seed.to_i, args.state.map_slice.to_i)
    args.state.map_slice      += 1
    args.state.do_terrain_gen = false
  end
  if Kernel.tick_count == 0 || args.inputs.keyboard.t || in_args == false
    args.outputs.labels << [640, 360, "LOADING", 10, 1]
    args.state.do_terrain_gen = true
    return
  else
    FFI::MatoCore::update_terrain
  end
  if args.inputs.keyboard.key_down.r && args.state.player_ptr
    FFI::MatoCore::despawn_player args.state.player_ptr
    args.state.player_ptr = nil
  end
  unless args.state.player_ptr
    args.state.player_ptr = FFI::MatoCore::spawn_player(32.0, 32.0, 1.0, 0.3, 0.3)
    args.state.sprite_x   = nil
    args.state.sprite_y   = nil
    args.state.has_moved  = false
  end
  args.state.sprite_x  ||= FFI::MatoCore::player_x_pos(args.state.player_ptr) - 640
  args.state.sprite_y  ||= FFI::MatoCore::player_y_pos(args.state.player_ptr) - 360

  args.state.sprite_x = (args.state.sprite_x * 0.8 + 0.2 * (FFI::MatoCore::player_x_pos(args.state.player_ptr) - 640)).to_i.clamp(0, WIDTH - 1280)
  args.state.sprite_y = (args.state.sprite_y * 0.8 + 0.2 * (FFI::MatoCore::player_y_pos(args.state.player_ptr) - 360)).to_i.clamp(0, HEIGHT - 720)

  vert_move_ratio      = 0
  horz_move_ratio      = 0.5
  args.state.vert_move = args.inputs.up_down #* (1 - vert_move_ratio) + args.state.vert_move * vert_move_ratio
  args.state.vert_move = 0 if args.inputs.up_down <= 0
  args.state.horz_move = args.inputs.left_right #* (1 - horz_move_ratio) + args.state.horz_move * horz_move_ratio

  args.state.vert_move = 0 if args.state.vert_move.abs < 0.1 && args.inputs.up_down == 0
  args.state.horz_move = 0 if args.state.horz_move.abs < 0.1 && args.inputs.left_right == 0

  if args.state.timer <= Time.now
      FFI::MatoCore::player_input(args.state.player_ptr, args.state.vert_move.to_f, args.state.horz_move.to_f)
      args.state.has_moved ||= args.inputs.up_down != 0 || args.inputs.left_right != 0
      args.state.timer     = Time.now unless args.state.has_moved
  end
  FFI::MatoCore::player_tick(args.state.player_ptr)

  args.state.player_x = FFI::MatoCore::player_x_pos_anchored(args.state.player_ptr, args.state.sprite_x.to_f, args.state.sprite_y.to_f) + 16
  args.state.player_y = FFI::MatoCore::player_y_pos_anchored(args.state.player_ptr, args.state.sprite_x.to_f, args.state.sprite_y.to_f) - 16

  mouse_angle = Math.atan2(args.inputs.mouse.y - args.state.player_y, args.inputs.mouse.x - args.state.player_x)
  mouse_dist  = Math.sqrt((args.state.player_y - args.inputs.mouse.y) ** 2 + (args.state.player_x - args.inputs.mouse.x) ** 2)
  mouse_dist  = mouse_dist.clamp(0, 20)

  FFI::MatoCore::destroy_terrain(
      (args.state.player_x + Math.cos(mouse_angle) * mouse_dist + args.state.sprite_x).to_i,
      (args.state.player_y + Math.sin(mouse_angle) * mouse_dist + args.state.sprite_y).to_i,
      10
  ) if args.inputs.mouse.button_right && Kernel.tick_count % 1 == 0
  FFI::MatoCore::create_terrain((args.inputs.mouse.x + args.state.sprite_x).to_i, (args.inputs.mouse.y + args.state.sprite_y).to_i, 10, args.state.brush_material) if args.inputs.mouse.button_left # && args.inputs.mouse.down

  FFI::MatoCore::draw_terrain(
      args.state.sprite_x.to_i,
      args.state.sprite_y.to_i
  )
  FFI::MatoCore::draw_player(
      args.state.player_ptr,
      args.state.sprite_x.to_i,
      args.state.sprite_y.to_i
  )

  # npc_test(args)
  FFI::MatoCore::screen_to_sprite("screen")
  args.outputs.primitives << [
      {x: 0, y: 0, w: 1280, h: 720, path: :screen}.sprite,
      [(args.state.player_x + Math.cos(mouse_angle) * mouse_dist) - 2, (args.state.player_y + Math.sin(mouse_angle) * mouse_dist) - 2, 4, 4, 200, 0, 0].solid,
      [640, 60, Kernel.sprintf("%02.2f", (Time.now - args.state.timer).to_f), 10, 1].label,
  ]
  if (Time.now - args.state.timer).to_f < 5 && args.outputs.static_labels.empty?
    args.outputs.primitives << [640, 80.from_top, ">>> GO RIGHT >>>", 10, 1, 0, 0, 0, 255 - 255 * (Time.now - args.state.timer).to_f / 5].label
  end
  args.outputs.debug << [
      [0, 40.from_top, 80, 40, 0, 0, 0, 255].solid,
      {x: 10, y: 10.from_top, text: "#{($gtk.current_framerate + 0.5).to_i} FPS", r: 255, g: 255, b: 255}.label,
  # [args.state.player_x + Math.cos(mouse_angle)*mouse_dist-20, args.state.player_y + Math.sin(mouse_angle)*mouse_dist-20, 40,40].border,
  # [args.state.player_x-8, args.state.player_y-16, 16,32].border,
  ]

  if WIDTH - FFI::MatoCore::player_x_pos($args.state.player_ptr) < 32
    str = "YOU WON IN " + Kernel.sprintf("%02.2f", (Time.now - args.state.timer).to_f) + " SECONDS!"
    args.outputs.static_labels << [640, 80.from_top, str, 10, 1]
    $gtk.schedule_callback(Kernel.tick_count + 120) { args.outputs.static_labels.clear }
    FFI::MatoCore::despawn_player args.state.player_ptr
    args.state.player_ptr = nil
    args.state.timer = Time.now+3
  end

  args.outputs.background_color = [51, 48, 36]
end

def npc_test args
  args.state.npc_pointers ||= [
      FFI::MatoCore::spawn_player((WIDTH / 2).to_f + 40.0 * 5.0, (HEIGHT / 2).to_f, 1.0, 0.3, 1.0),
      FFI::MatoCore::spawn_player((WIDTH / 2).to_f + 40.0 * 4.0, (HEIGHT / 2).to_f, 0.3, 0.3, 1.0),
      FFI::MatoCore::spawn_player((WIDTH / 2).to_f + 40.0 * 3.0, (HEIGHT / 2).to_f, 0.3, 0.9, 0.3),
      FFI::MatoCore::spawn_player((WIDTH / 2).to_f + 40.0 * 2.0, (HEIGHT / 2).to_f, 0.9, 0.9, 0.0),
      FFI::MatoCore::spawn_player((WIDTH / 2).to_f + 40.0 * 1.0, (HEIGHT / 2).to_f, 1.0, 0.5, 0.0),
  ]
  args.state.npc_pointers.each_with_index do |npc_ptr, idx|
    npc_movement_cycle_tick = args.state.tick_count + idx * 100 / args.state.npc_pointers.size
    npc_vert_thrust         = (npc_movement_cycle_tick % 120 <=> 90).greater(0).to_f
    FFI::MatoCore::player_input(npc_ptr, npc_vert_thrust, 0.0)
    FFI::MatoCore::player_tick(npc_ptr)
    FFI::MatoCore::draw_player(npc_ptr, args.state.sprite_x.to_i, args.state.sprite_y.to_i)
  end
end


GTK::Console.const_set("XHST", "XHST")
GTK::Console.const_set("SMKE", "SMKE")
GTK::Console.const_set("SAND", "SAND")
GTK::Console.const_set("DIRT", "DIRT")
GTK::Console.const_set("NONE", "NONE")

def brushtype material = ""
  old                        = $args.state.brush_material
  $args.state.brush_material = material
  if material == "XHST"
    "Terrain brush set to exhaust (XHST)"
  elsif material == "SMKE"
    "Terrain brush set to smoke (SMKE)"
  elsif material == "DIRT"
    "Terrain brush set to dirt (DIRT)"
  elsif material == "SAND"
    "Terrain brush set to sand (SAND)"
  elsif material == "NONE"
    "Terrain brush disabled"
  else
    $args.state.brush_material = old
    "USAGE: brushtype [MATERIAL]\n     Valid materials: XHST, SMKE, DIRT, SAND, NONE"
  end
end

def spawn_player
  FFI::MatoCore::spawn_player((Kernel.rand * (1280 - 32)).to_f, 360.to_f, Kernel.rand.to_f, Kernel.rand.to_f, Kernel.rand.to_f)
end