$gtk.ffi_misc.gtk_dlopen("ext")
# @param args [GTK::Args]
def tick args
  args.state.map_seed ||= 0
  args.state.map_slice ||= 0
  args.state.x ||= 640
  args.state.y ||= 360
  args.state.x, args.state.y = args.inputs.mouse.x-16, args.inputs.mouse.y-16 if args.inputs.mouse.button_left
  args.state.x += args.inputs.keyboard.left_right

  if Kernel.tick_count == 0 || args.inputs.keyboard.space
    FFI::MatoCore::generate_terrain(args.state.map_seed.to_i, args.state.map_slice.to_i)
    args.state.map_slice += 1
  else
    FFI::MatoCore::update_sand
  end

  FFI::MatoCore::destroy_terrain(args.inputs.mouse.x.to_i, args.inputs.mouse.y.to_i, 5) if args.inputs.mouse.button_right
  # Disabled since I'm currently using m1 for setting player position
  # FFI::MatoCore::create_terrain(args.inputs.mouse.x.to_i, args.inputs.mouse.y.to_i, 10) if args.inputs.mouse.button_left

  FFI::MatoCore::set_player1_pos(args.state.x.to_i, args.state.y.to_i)
  args.state.y = FFI::MatoCore::move_player1_to_surface

  FFI::MatoCore::draw_terrain
  FFI::MatoCore::draw_player1

  FFI::MatoCore::screen_to_sprite("screen")
  args.outputs.sprites << [0, 0, 1280, 720, :screen].sprite
end