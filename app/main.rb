$gtk.ffi_misc.gtk_dlopen("matocore")

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

$devtools_enabled ||= false
$brush_material   ||= nil

# @param args [GTK::Args]
def tick in_args
  args                          = in_args || $args
  args.outputs.background_color = [51, 48, 36]
  args.state.map_seed           ||= 6
  args.state.vert_move          ||= 0
  args.state.horz_move          ||= 0
  args.state.do_terrain_gen     ||= false
  args.state.timer              ||= Kernel.tick_count
  args.state.npc_pointers       ||= []
  if args.state.do_terrain_gen
    FFI::MatoCore::generate_terrain(args.state.map_seed.to_i)
    if args.state.player_ptr
      FFI::MatoCore::player_surface_warp(args.state.player_ptr)
    end
    args.state.npc_pointers.each { |npc_ptr| FFI::MatoCore::player_surface_warp(npc_ptr) }
    args.state.do_terrain_gen = false
  end
  if Kernel.tick_count == 0 || args.inputs.keyboard.key_down.t || in_args == false
    args.outputs.background_color = [113, 78, 28]
    if Kernel.tick_count == 0
      puts "Welcome to the developer console!"
      puts "  Development commands include:"
      puts "    toggle_devtools"
      puts "    getseed"
      puts "    setseed"
      puts "    brushtype"
    end
    tips = loading_tips
    args.outputs.labels << [
        [640, 480, "GENERATING LEVEL", 10, 1],
        [640, 435, "~~~~Controls~~~~", 8, 1],
        [500, 400 - 30 * 0, "Move right  : →/D        ", 3, 0],
        [500, 400 - 30 * 1, "Move left   : ←/A        ", 3, 0],
        [500, 400 - 30 * 2, "Fly upwards : ↑/W/Space  ", 3, 0],
        [500, 400 - 30 * 3, "Reset player: R          ", 3, 0],
        [500, 400 - 30 * 4, "Reload level: T          ", 3, 0],
        [640, 400 - 30 * 6, tips.sample, 2, 1],
    ]
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
  args.state.sprite_x ||= FFI::MatoCore::player_x_pos(args.state.player_ptr) - 640
  args.state.sprite_y ||= FFI::MatoCore::player_y_pos(args.state.player_ptr) - 360

  args.state.sprite_x = (args.state.sprite_x * 0.8 + 0.2 * (FFI::MatoCore::player_x_pos(args.state.player_ptr) - 640)).to_i.clamp(0, WIDTH - 1280)
  args.state.sprite_y = (args.state.sprite_y * 0.8 + 0.2 * (FFI::MatoCore::player_y_pos(args.state.player_ptr) - 360)).to_i.clamp(0, HEIGHT - 720)

  vert_move_ratio      = 0
  horz_move_ratio      = 0.5
  args.state.vert_move = args.inputs.up_down #* (1 - vert_move_ratio) + args.state.vert_move * vert_move_ratio
  args.state.vert_move = 0 if args.inputs.up_down <= 0
  args.state.horz_move = args.inputs.left_right #* (1 - horz_move_ratio) + args.state.horz_move * horz_move_ratio

  args.state.vert_move = 0 if args.state.vert_move.abs < 0.1 && args.inputs.up_down == 0
  args.state.horz_move = 0 if args.state.horz_move.abs < 0.1 && args.inputs.left_right == 0

  if args.state.timer <= Kernel.tick_count
    FFI::MatoCore::player_input(args.state.player_ptr, args.state.vert_move.to_f, args.state.horz_move.to_f)
    args.state.has_moved ||= args.inputs.up_down != 0 || args.inputs.left_right != 0
    args.state.timer     = Kernel.tick_count unless args.state.has_moved
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
  ) if args.inputs.mouse.button_right && $devtools_enabled
  FFI::MatoCore::create_terrain((args.inputs.mouse.x + args.state.sprite_x).to_i, (args.inputs.mouse.y + args.state.sprite_y).to_i, 10, $brush_material) if args.inputs.mouse.button_left && $brush_material

  FFI::MatoCore::draw_terrain(
      args.state.sprite_x.to_i,
      args.state.sprite_y.to_i
  )
  FFI::MatoCore::draw_player(
      args.state.player_ptr,
      args.state.sprite_x.to_i,
      args.state.sprite_y.to_i
  )

  npc_logic args
  FFI::MatoCore::screen_to_sprite("screen")
  args.outputs.primitives << [
      {x: 0, y: 0, w: 1280, h: 720, path: :screen}.sprite,
      [(args.state.player_x + Math.cos(mouse_angle) * mouse_dist) - 2, (args.state.player_y + Math.sin(mouse_angle) * mouse_dist) - 2, 4, 4, 200, 0, 0].solid,
      [640, 60, Kernel.sprintf("%02.2f", (Kernel.tick_count - args.state.timer).fdiv(60).to_f), 10, 1].label,
  ]
  if (Kernel.tick_count - args.state.timer).fdiv(60).to_f < 5 && args.outputs.static_labels.empty?
    args.outputs.primitives << [640, 80.from_top, ">>> GO RIGHT >>>", 10, 1, 0, 0, 0, 255 - 255 * (Kernel.tick_count - args.state.timer).fdiv(60).to_f / 5].label
  end
  # args.outputs.debug << [
  #     [0, 40.from_top, 80, 40, 0, 0, 0, 255].solid,
  #     {x: 10, y: 10.from_top, text: "#{($gtk.current_framerate + 0.5).to_i} FPS", r: 255, g: 255, b: 255}.label,
  # # [args.state.player_x + Math.cos(mouse_angle)*mouse_dist-20, args.state.player_y + Math.sin(mouse_angle)*mouse_dist-20, 40,40].border,
  # # [args.state.player_x-8, args.state.player_y-16, 16,32].border,
  # ]

  if WIDTH - FFI::MatoCore::player_x_pos($args.state.player_ptr) < 32
    str = "YOU WON IN " + Kernel.sprintf("%02.2f", (Kernel.tick_count - args.state.timer).fdiv(60).to_f) + " SECONDS!"
    args.outputs.static_labels << [640, 80.from_top, str, 10, 1]
    $gtk.schedule_callback(Kernel.tick_count + 180) { args.outputs.static_labels.clear }
    FFI::MatoCore::despawn_player args.state.player_ptr
    args.state.player_ptr = nil
    if ((40 - (Kernel.tick_count - args.state.timer).fdiv(30)).to_i.greater(0) + 1) > args.state.npc_pointers.size
      ((40 - (Kernel.tick_count - args.state.timer).fdiv(30)).to_i.greater(0) + 1 - args.state.npc_pointers.size).times do
        add_npc args
      end
    end
    args.state.timer = Kernel.tick_count + 3 * 60
  end
end

def loading_tips
  return ["Goal: Get to the right side of the level. Par is 20 seconds."] if Kernel.global_tick_count == 0
  tips = []
  tips += ["Tip: There's no limit on your vertical speed, aside from hitting the ceiling."] * 4
  tips += ["Tip: Touching the floor dramatically slows you down, but touching the ceiling does not."] * 3
  tips += ["Tip: The level is very tall. Try flying higher if you can't find a good route closer to the ground."] * 2
  tips += ["Tip: Your jetpack gets less effective at horizontal boosting at high horizontal speeds."] * 2
  tips += ["Tip: You can slide off of sloped ceilings without a speed penalty."]
  tips += ["Tip: If you get stuck in terrain, press R to reset."]

  if Math.rand < 0.1
    # Rare loading "tips" that aren't really tips, but are neat to know.
    tips = [
        "Secret: Try pressing the key below ESC (~ on US keyboards)",
        "Trivia: Mato is Finnish for \"worm\".",
        "Trivia: Mato was originally created after the developer found Liero's controls too unintuitive.",
        "Trivia: Mato was inspired by games like Liero, Noita, and The Powder Toy.",
    ]
    if Math.rand < 0.1
      # Super rare loading """tip"""
      tips = ["PROTIP: To defeat the Cyberworm, shoot at it until it dies."]
    end
  end

  tips
end

def add_npc args
  args.state.npc_pointers << FFI::MatoCore::spawn_player((256 + 32 * args.state.npc_pointers.size).to_f, 100.0, Kernel.rand, Kernel.rand, Kernel.rand)
end

def npc_logic args
  args.state.npc_pointers.each_with_index do |npc_ptr, idx|
    npc_movement_cycle_tick = args.state.tick_count + idx * 100 / args.state.npc_pointers.size
    npc_vert_thrust         = (npc_movement_cycle_tick % 60 <=> 40).greater(0).to_f
    FFI::MatoCore::player_input(npc_ptr, npc_vert_thrust, 0.0)
    FFI::MatoCore::player_tick(npc_ptr)
    FFI::MatoCore::draw_player(npc_ptr, args.state.sprite_x.to_i, args.state.sprite_y.to_i)
  end
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
  old             = $brush_material
  $brush_material = material == "NONE" ? nil : material
  if material == "XHST"
    "Terrain brush set to exhaust (XHST)\nHold the left mouse button to paint the selected material."
  elsif material == "SMKE"
    "Terrain brush set to smoke (SMKE)\nHold the left mouse button to paint the selected material."
  elsif material == "DIRT"
    "Terrain brush set to dirt (DIRT)\nHold the left mouse button to paint the selected material."
  elsif material == "SAND"
    "Terrain brush set to sand (SAND)\nHold the left mouse button to paint the selected material."
  elsif material == "NONE"
    "Terrain brush disabled"
  else
    $args.state.brush_material = old
    "USAGE: brushtype [MATERIAL]\n     Valid materials: XHST, SMKE, DIRT, SAND, NONE\nEXAMPLE: brushtype SAND"
  end
end

def toggle_devtools
  $devtools_enabled = !$devtools_enabled
  if $devtools_enabled
    "Terrain carving ENABLED. Hold right click to dig, press T to reset the level."
  else
    "Terrain carving DISABLED."
  end
end

def setseed new_seed = nil
  if new_seed && new_seed.to_i == new_seed && new_seed >= 0
    if new_seed > 0xFFFFFFFF
      return "Pick a lower seed please ._."
    elsif new_seed == 0xFFFFFFFF
      puts "Very funny... -.-"
      new_seed = 68 # Generates the player stuck underground 57 67
    end
    $args.state.map_seed = new_seed
    $gtk.console.hide
    $gtk.schedule_callback(Kernel.tick_count + 33) {$args.inputs.keyboard.key_down.t = true}
    "Map seed set to #{new_seed}."
  else
    "USAGE: setseed [LEVEL_SEED]\n     NOTE: Level seed must be a positive integer.\n     EXAMPLE: setseed 6"
  end
end

def getseed
  puts "Current map seed:"
  $args.state.map_seed
end