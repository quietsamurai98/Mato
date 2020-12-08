$gtk.ffi_misc.gtk_dlopen("ext")
# @param args [GTK::Args]
def tick args
  args.state.map_seed ||= 0
  args.state.map_slice ||= 0

  if Kernel.tick_count == 0 || args.inputs.keyboard.space
    FFI::MatoCore::generate_terrain(args.state.map_seed.to_i, args.state.map_slice.to_i)
    args.state.map_slice += 1
  else
    FFI::MatoCore::update_sand
  end

  args.state.player_ptr ||= FFI::MatoCore::spawn_player(640.0, 360.0, 1.0, 0.3, 0.3)

  FFI::MatoCore::destroy_terrain(args.inputs.mouse.x.to_i, args.inputs.mouse.y.to_i, 5) if args.inputs.mouse.button_right
  FFI::MatoCore::create_terrain(args.inputs.mouse.x.to_i, args.inputs.mouse.y.to_i, 10) if args.inputs.mouse.button_left

  FFI::MatoCore::player_input(args.state.player_ptr, args.inputs.up_down.to_f, args.inputs.left_right.to_f)
  FFI::MatoCore::player_tick(args.state.player_ptr, args.inputs.up_down.to_f, args.inputs.left_right.to_f)

  FFI::MatoCore::draw_terrain
  FFI::MatoCore::draw_player(args.state.player_ptr)

  FFI::MatoCore::screen_to_sprite("screen")
  args.outputs.sprites << [0, 0, 1280, 720, :screen].sprite
  args.outputs.debug << [
      [0,40.from_top,80, 40, 0, 0, 0, 255].solid,
      {x:10, y:10.from_top, text:"#{($gtk.current_framerate+0.5).to_s.to_i} FPS", r: 255, g: 255, b:255}.label
  ]
end

def spawn_player
  FFI::MatoCore::spawn_player((Kernel.rand*(1280-32)).to_f, 360.to_f, Kernel.rand.to_f, Kernel.rand.to_f, Kernel.rand.to_f)
end