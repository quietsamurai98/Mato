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

  FFI::MatoCore::create_terrain(args.inputs.mouse.x.to_i, args.inputs.mouse.y.to_i, 10) if args.inputs.mouse.button_left
  FFI::MatoCore::destroy_terrain(args.inputs.mouse.x.to_i, args.inputs.mouse.y.to_i, 5) if args.inputs.mouse.button_right
  FFI::MatoCore::draw_terrain
  FFI::MatoCore::screen_to_sprite("screen")
  args.outputs.sprites << [0, 0, 1280, 720, :screen].sprite
end