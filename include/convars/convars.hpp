// convars.hpp — CS2 ConVar / ConCommand catalogue (auto-generated)
// build: 14172
// 4018 convars, 1179 commands. Read-only snapshot of the tier0 CCvar registry.
#pragma once

// === ConVars ===
// name | type | value | flags | description
// CS_WarnFriendlyDamageInterval                             int32    3                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Defines how frequently the server notifies clients that a player damaged a friend
// Inferno_concav_plane_threshold                            float32  -10                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// _fov                                                      int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Automates fov command to server.
// adsp_alley_min                                            int32    122                   FCVAR_DEVELOPMENTONLY
// adsp_courtyard_min                                        int32    126                   FCVAR_DEVELOPMENTONLY
// adsp_debug                                                int32    0                     FCVAR_ARCHIVE
// adsp_door_height                                          int32    112                   FCVAR_DEVELOPMENTONLY
// adsp_duct_min                                             int32    106                   FCVAR_DEVELOPMENTONLY
// adsp_hall_min                                             int32    110                   FCVAR_DEVELOPMENTONLY
// adsp_low_ceiling                                          int32    108                   FCVAR_DEVELOPMENTONLY
// adsp_opencourtyard_min                                    int32    126                   FCVAR_DEVELOPMENTONLY
// adsp_openspace_min                                        int32    130                   FCVAR_DEVELOPMENTONLY
// adsp_openstreet_min                                       int32    118                   FCVAR_DEVELOPMENTONLY
// adsp_openwall_min                                         int32    130                   FCVAR_DEVELOPMENTONLY
// adsp_room_min                                             int32    102                   FCVAR_DEVELOPMENTONLY
// adsp_street_min                                           int32    118                   FCVAR_DEVELOPMENTONLY
// adsp_tunnel_min                                           int32    114                   FCVAR_DEVELOPMENTONLY
// adsp_wall_height                                          int32    128                   FCVAR_DEVELOPMENTONLY
// ag2_network_recipeshape_cache_size                        int32    32                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_DONTRECORD  // Cache size for recently-used pose recipe shapes.
// ag2_preserve_params_on_reload                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // When an animgraph is reloaded, should the underlying system restore all params?
// ag2_use_networked_serialization_context_demo              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_DONTRECORD  // Use networked compatibility serialization context in demo playback.
// ag2_use_networked_serialization_context_game              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Use networked compatibility serialization context in games.
// ai_debug_dyninteractions                                  int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Debug the NPC dynamic interaction system.
// ai_debug_los                                              int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // NPC Line-Of-Sight debug mode. If 1, solid entities that block NPC LOC will be highlighted with white bounding boxes. If 2, it'll show non-solid entities that would do it if they were solid.
// ai_debug_ragdoll_magnets                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ai_debug_scripted_sequence                                string   false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// ai_debug_shoot_positions                                  int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ai_debug_speech                                           int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ai_disabled                                               bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ai_force_serverside_ragdoll                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ai_off_nav_show_nearest                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// ai_sequence_debug                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// ai_use_visibility_cache                                   int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Sets whether or not NPCs can cache their Visibility checks against other entities. If set to 2, also tests to make sure that NPC->Target results match that of Target->NPC.
// ai_use_visibility_cache_reciprocation                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Sets whether or not the visibility check cache should be reciprocal.
// always_perform_full_spatial_partition_update              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// ammo_338mag_headshot_mult                                 float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_338mag_impulse                                       float32  2800                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_338mag_max                                           int32    30                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_357sig_headshot_mult                                 float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_357sig_impulse                                       float32  2000                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_357sig_max                                           int32    52                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_357sig_min_max                                       int32    12                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_357sig_p250_max                                      int32    26                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_357sig_small_max                                     int32    24                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_45acp_headshot_mult                                  float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_45acp_impulse                                        float32  2100                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_45acp_max                                            int32    100                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_50AE_headshot_mult                                   float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_50AE_impulse                                         float32  2400                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_50AE_max                                             int32    35                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_556mm_box_headshot_mult                              float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_556mm_box_impulse                                    float32  2400                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_556mm_box_max                                        int32    200                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_556mm_headshot_mult                                  float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_556mm_impulse                                        float32  2400                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_556mm_max                                            int32    90                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_556mm_small_max                                      int32    40                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_57mm_headshot_mult                                   float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_57mm_impulse                                         float32  2000                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_57mm_max                                             int32    100                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_762mm_headshot_mult                                  float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_762mm_impulse                                        float32  2400                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_762mm_max                                            int32    90                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_9mm_headshot_mult                                    float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_9mm_impulse                                          float32  2000                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_9mm_max                                              int32    120                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_buckshot_headshot_mult                               float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_buckshot_impulse                                     float32  600                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // You must enable tweaking via tweak_ammo_impulses to use this value.
// ammo_buckshot_max                                         int32    32                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_grenade_limit_default                                int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_grenade_limit_flashbang                              int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_grenade_limit_total                                  int32    3                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_item_limit_adrenaline                                int32    5                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// ammo_item_limit_healthshot                                int32    4                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// anim_decode_forcewritealltransforms                       bool     false                 FCVAR_DEVELOPMENTONLY  // Force BatchAnimationDecode to write transformations for all bones
// anim_disable                                              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// anim_resource_validate_on_load                            bool     true                  FCVAR_RELEASE  // Validates the animation group channel list against the animations on load for every animation
// animated_material_attributes                              bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// animgraph2_enable_parallel_update                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// animgraph2_serialize_pose_recipe_in_pre_pack_entities     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// animgraph_debug                                           bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Debug animation graph
// animgraph_debug_animevents                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Print info about animevents emitted by AnimGraph
// animgraph_debug_entindex                                  int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // The entity to specifically debug
// animgraph_debug_filterent                                 int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Filter setting for animgraph_debug_variables output. If set to -1, show debug for all entities. If set to 0, show debug for any NPCs that have been npc_selected. If set to >0, something other than 0, show debug for the entity with the matching entindex.
// animgraph_debug_max_poseop_count                          bool     false                 
// animgraph_debug_set_filter_params                         string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Comma separated list of params to filter against when drawing debug text overlays
// animgraph_debug_set_filter_tags                           string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Comma separated list of tags to filter against when drawing debug text overlays
// animgraph_debug_show_unreferenced_params                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// animgraph_debug_show_unreferenced_tags                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// animgraph_debug_tags                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// animgraph_debug_variables                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Turn on to see animgraph variable changes for entities passing animgraph_debug_filterent.
// animgraph_debug_variables_ignore_missing                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // If set, animgraph_debug_variables won't show debug for warnings about sets to missing variables.
// animgraph_debug_variables_ignore_nonchanges               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // If set, animgraph_debug_variables won't show debug for variable sets that don't change the value.
// animgraph_draw_traces                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// animgraph_enable                                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Enable animation graph
// animgraph_enable_dirty_netvar_optimization                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footlock_auto_ledge_detection                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED  // Attempt to detect when the foot is partially hanging off a ledge and stop it tilting to reach the bottom
// animgraph_footlock_auto_stair_detection                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED  // Attempt to detect when the foot is on a stair and will stop it from tilting to reach the next step
// animgraph_footlock_calculate_tilt                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footlock_debug_foot_index                       int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footlock_debug_type                             int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footlock_draw_footbase                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footlock_enabled                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED  // A master convar that effectively disables the entire footlock node.
// animgraph_footlock_ground_roll                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footlock_hip_offset_enable                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footlock_ik_enable                              bool     true                  FCVAR_REPLICATED|FCVAR_CHEAT  // Enable IK.
// animgraph_footlock_tilt_mode                              int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footlock_trace_ground_enabled                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED  // Convar for toggling foot lock ground tracking.
// animgraph_footlock_use_hip_shift                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_footstep_node_supresses_events                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// animgraph_force_full_network_updates                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_ik_debug                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_motionmatching_print_compressionstats           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_network_enable                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Enable animation graph networking. The setting is only read at graph creation time; to use please set on the command line.
// animgraph_parallel_postdataupdate                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// animgraph_slope_draw_raycasts                             bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// animgraph_slope_enable                                    bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// animgraph_slowdownonslopes_enabled                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// animgraph_trace_ignore_prop_physics                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// animgraph_trace_static_only                               bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// animgraph_verify_dirty_netvar_optimization                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// annotation_auto_load                                      bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// attached_output_stall_ms                                  float32  250                   FCVAR_DEVELOPMENTONLY
// audio_input_test_signal                                   bool     false                 FCVAR_DEVELOPMENTONLY  // For testing the audio input pathway with a sine tone instead of SDL3.
// audio_input_use_sdl_roles                                 bool     false                 FCVAR_DEVELOPMENTONLY
// autosave_fully_async                                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Set to 1 to have autosaves execute completely on the save thread, forces 'render only' mode while the save completes
// battery_saver                                             bool     false                 FCVAR_ARCHIVE  // OBSOLETE replaced by mobile_fps_* - Battery saver mode. 0=off, 1=on
// bot_allow_grenades                                        bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots may use grenades.
// bot_allow_machine_guns                                    bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots may use the machine gun.
// bot_allow_pistols                                         bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots may use pistols.
// bot_allow_rifles                                          bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots may use rifles.
// bot_allow_rogues                                          bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots may occasionally go 'rogue'. Rogue bots do not obey radio commands, nor pursue scenario goals.
// bot_allow_shotguns                                        bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots may use shotguns.
// bot_allow_snipers                                         bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots may use sniper rifles.
// bot_allow_sub_machine_guns                                bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots may use sub-machine guns.
// bot_auto_follow                                           bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots with high co-op may automatically follow a nearby human player.
// bot_auto_vacate                                           bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots will automatically leave to make room for human players.
// bot_autodifficulty_threshold_high                         float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Upper bound above Average Human Contribution Score that a bot must be above to change its difficulty
// bot_autodifficulty_threshold_low                          float32  -2                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Lower bound below Average Human Contribution Score that a bot must be below to change its difficulty
// bot_chatter                                               string   normal                FCVAR_GAMEDLL|FCVAR_RELEASE  // Control how bots talk. Allowed values: 'off', 'radio', 'minimal', or 'normal'.
// bot_chatter_use_rr                                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // 0 = Use old bot chatter system, 1 = Use response rules
// bot_controllable                                          bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Determines whether bots can be controlled by players
// bot_coop_idle_max_vision_distance                         float32  1400                  FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Max distance bots can see targets (in coop) when they are idle, dormant, hiding or asleep.
// bot_crouch                                                bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// bot_debug                                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // For internal testing purposes.
// bot_debug_target                                          int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // For internal testing purposes.
// bot_defense_rush_chance                                   float32  33                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Are the defense bots going to rush.
// bot_defer_to_human_goals                                  bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero and there is a human on the team, the bots will not do the scenario tasks.
// bot_defer_to_human_items                                  bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero and there is a human on the team, the bots will not get scenario items.
// bot_difficulty                                            int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Defines the skill of bots joining the game.  Values are: 0=easy, 1=normal, 2=hard, 3=expert.
// bot_dont_shoot                                            bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_RELEASE  // If nonzero, bots will not fire weapons (for debugging).
// bot_eco_limit                                             float32  2000                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots will not buy if their money falls below this amount.
// bot_flipout                                               bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots use no CPU for AI. Instead, they run around randomly.
// bot_force_duck                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// bot_freeze                                                bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// bot_ignore_enemies                                        bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // If nonzero, bots will ignore enemies (for debugging).
// bot_ignore_players                                        bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Bots will not see non-bot players.
// bot_join_after_player                                     bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots wait until a player joins before entering the game.
// bot_join_delay                                            int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Prevents bots from joining the server for this many seconds after a map change.
// bot_join_in_warmup                                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Prevents bots from joining the server while warmup phase is active.
// bot_join_team                                             string   any                   FCVAR_GAMEDLL|FCVAR_RELEASE  // Determines the team bots will join into. Allowed values: 'any', 'T', or 'CT'.
// bot_loadout                                               string                         FCVAR_GAMEDLL|FCVAR_CHEAT  // bots are given these items at round start
// bot_max_visible_smoke_length                              float32  200                   FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Bots will see players through smoke clouds up to this length.
// bot_max_vision_distance_override                          float32  -1                    FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Max distance bots can see targets.
// bot_mimic                                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Bot uses usercmd of player by index.
// bot_mimic_spec_buttons                                    bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // +attack, +jump etc are used for spectator control instead of being passed on to spectated bot
// bot_mimic_yaw_offset                                      float32  180                   FCVAR_GAMEDLL|FCVAR_CHEAT
// bot_prefix                                                string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // This string is prefixed to the name of all bots that join the game.
<difficulty> will be replaced with the bot's difficulty.
<weaponclass> will be replaced with the bot's desired weapon class.
<skill> will be replaced with a 0-100 representation of the bot
// bot_quota                                                 int32    10                    FCVAR_GAMEDLL|FCVAR_RELEASE  // Determines the total number of bots in the game.
// bot_quota_mode                                            string   fill                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Determines the type of quota.
Allowed values: 'normal', 'fill', and 'match'.
If 'fill', the server will adjust bots to keep N players in the game, where N is bot_quota.
If 'match', the server will maintain a 1:N ratio of humans to bots, where N is bot_quot
// bot_randombuy                                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // should bots ignore their prefered weapons and just buy weapons at random?
// bot_show_battlefront                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Show areas where rushing players will initially meet.
// bot_show_nav                                              bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // For internal testing purposes.
// bot_show_occupy_time                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Show when each nav area can first be reached by each team.
// bot_stop                                                  string   0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // bot_stop <1|all> | <not_bomber> | <t> | <ct>
// bot_strafe                                                float32  0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Strafe left and right (interval)
// bot_traceview                                             int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // For internal testing purposes.
// bot_walk                                                  bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // If nonzero, bots can only walk, not run.
// bot_zombie                                                bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // If nonzero, bots will stay in idle mode and not attack.
// break_damage_inherit_scale                                float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// break_invulnerable_spawn_duration                         float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// breakable_debug_spawn_transform_time                      float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Debug draw the spawn transform location.
// breakable_multiplayer                                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// buddha                                                    bool     false                 FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_CHEAT  // Player takes damage but won't die
// buddha_ignore_bots                                        bool     false                 FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_CHEAT  // Bots always buddha 0
// buddha_reset_hp                                           int32    1                     FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_CHEAT  // HP to set when damaged below zero in Buddha Mode
// bug_submitter_override                                    string                         FCVAR_ARCHIVE
// buildcubemaps_renderdoc_capture                           int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Capture a specific cubemap with RenderDoc during buildcubemaps.
// c_maxdistance                                             float32  200                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_maxpitch                                                float32  90                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_maxyaw                                                  float32  135                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_mindistance                                             float32  30                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_minpitch                                                float32  0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_minyaw                                                  float32  -135                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_orthoheight                                             float32  100                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_orthowidth                                              float32  100                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_thirdpersonshoulder                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_thirdpersonshoulderaimdist                              float32  120                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_thirdpersonshoulderdist                                 float32  40                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_thirdpersonshoulderheight                               float32  5                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// c_thirdpersonshoulderoffset                               float32  20                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cachedvalue_count_partybrowser                            int32    1784296970            FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cachedvalue_count_teammates                               int32    1784573455            FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cam_collision                                             int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // When in thirdperson and cam_collision is set to 1, an attempt is made to keep the camera from passing though walls.
// cam_idealdelta                                            float32  4                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Controls the speed when matching offset to ideal angles in thirdperson view
// cam_idealdist                                             float32  150                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cam_ideallag                                              float32  4                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Amount of lag used when matching offset to ideal angles in thirdperson view
// cam_idealpitch                                            float32  0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cam_idealyaw                                              float32  0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cam_showangles                                            bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // When in thirdperson, print viewangles/idealangles/cameraoffsets to the console.
// cam_snapto                                                bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// camera_datadriven_debug                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// camera_datadriven_disable_cache                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT
// camera_path_edit_mode                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// cash_player_bomb_defused                                  int32    200                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_bomb_planted                                  int32    200                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_damage_hostage                                int32    -30                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_drop_on_death                                 int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_drop_on_death_stack_value                     int32    250                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_get_killed                                    int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_interact_with_hostage                         int32    300                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_killed_enemy_default                          int32    200                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_killed_enemy_factor                           float32  0.5                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_killed_hostage                                int32    -1000                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_killed_teammate                               int32    -300                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_rescued_hostage                               int32    1000                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_player_respawn_amount                                int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_bonus_shorthanded                               int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_elimination_bomb_map                            int32    2700                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_elimination_hostage_map_ct                      int32    2300                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_elimination_hostage_map_t                       int32    2000                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_hostage_alive                                   int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_hostage_interaction                             int32    500                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_loser_bonus                                     int32    2400                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_loser_bonus_consecutive_rounds                  int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_per_dead_enemy                                  int32    50                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_planted_bomb_but_defused                        int32    200                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_rescued_hostage                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_terrorist_win_bomb                              int32    2700                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_win_by_defusing_bomb                            int32    2700                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_win_by_hostage_rescue                           int32    3000                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_win_by_time_running_out_bomb                    int32    2700                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_win_by_time_running_out_hostage                 int32    2000                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cash_team_winner_bonus_consecutive_rounds                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// cc_captiontrace                                           int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Show missing closecaptions (0 = no, 1 = devconsole, 2 = show in hud)
// cc_delay_time                                             float32  0.25                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Close caption delay before showing caption.
// cc_force_combine_chatter                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cc_lang                                                   string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Current close caption language (emtpy = use game UI language)
// cc_linger_time                                            float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Close caption linger time.
// cc_log                                                    int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Log caption names and contents (0 = off, 1 = found captions, 2 = unfound captions, 3 = all captions)
// cc_spectator_only                                         bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cc_subtitles                                              bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // If set, don't show sound effect captions, just voice overs (i.e., won't help hearing impaired players).
// cc_vr_caption_catchup_interval                            float32  0.3                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Duration it takes for attached caption to ideal point
// cc_vr_caption_speed                                       int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // 0 = slow, 1 = medium (default), 2 = fast
// cc_vr_debug                                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Debug visualization of VR closed caption placement
// cc_vr_depth_test                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Have closed caption Panorama panel perform depth testing against the scene
// cc_vr_epsilon                                             float32  2.5                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Epsilon to trigger movement of VR subtitle panel in world space
// cc_vr_font_size                                           int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // 0 = small, 1 = med (default), 2 = large
// cc_vr_forward_offset                                      float32  30                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Subtitle offset distance (forward, in front of player)
// cc_vr_vertical_offset                                     float32  -6.5                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Subtitle vertical offset distance (positive is up)
// cc_vr_width                                               int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // 0 = narrow, 1 = med (default), 2 = wide
// character_patches                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// check_transmit_dump_ents                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// chicken_stop                                              bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// cl_ShowBoneSetupEnts                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Show which entities are having their bones setup each frame.
// cl_access_all_missions                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_ag2_record_entity_graph                                string                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Automatically start AG2 recording when an entity with this name (wildcard) or id is created.
// cl_aggregate_particles                                    bool     false                 FCVAR_DEVELOPMENTONLY
// cl_allow_animated_avatars                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Whether or not to allow animated avatars
// cl_allow_multi_input_binds                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE
// cl_anglespeedkey                                          float32  0.67                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_async_client_shatter                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // spawn client glass shards asynchronously during demos or when remotely connected.
// cl_async_restore_server_authoritative_state               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_async_usercmd_send                                     bool     true                  FCVAR_DEVELOPMENTONLY
// cl_async_usercmd_send_recvmargin_min                      float32  1                     FCVAR_DEVELOPMENTONLY  // Min size of the recv margin queue when async usercmd send is disabled
// cl_auto_cursor_scale                                      bool     true                  FCVAR_ARCHIVE  // Automatic cursor size scaling.
// cl_autobuy                                                string   vesthelm vest m4a1 …  FCVAR_CLIENTDLL|FCVAR_RELEASE  // The order in which autobuy will attempt to purchase items
// cl_autohelp                                               bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO  // Auto-help
// cl_bake_bomb_damage_debug                                 int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_batch_entity_list_ops_during_latch                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Batch entity list adds / removes while latching interpolated variables to avoid mutex contention.
// cl_bone_cache_optimization                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_borrow_music_from_player_slot                          int32    -1                    FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_boxmove                                                int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // run in a square, # represents how many usercommands to run before turning.
// cl_boxmove_speed                                          float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // how fast to run (1 to use player max run speed).
// cl_buffer_incoming_net_messages                           bool     true                  FCVAR_RELEASE
// cl_buymenu_ct_nextround_high                              int32    5000                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER|FCVAR_RELEASE
// cl_buymenu_ct_nextround_low                               int32    1400                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER|FCVAR_RELEASE
// cl_buymenu_t_nextround_high                               int32    5000                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER|FCVAR_RELEASE
// cl_buymenu_t_nextround_low                                int32    1400                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER|FCVAR_RELEASE
// cl_buywheel_donate_key                                    int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER|FCVAR_RELEASE  // Set the key to use for donation in the buy menu. 0: Left Control; 1: Left Alt; 2: Left Shift.
// cl_buywheel_nonumberpurchasing                            bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER|FCVAR_RELEASE  // Set non-zero to prevent buy wheel from purchasing via number keys
// cl_cache_sendtable                                        bool     true                  FCVAR_DEVELOPMENTONLY  // Cache sendtables
// cl_cameraoverride_fade_in_amount                          float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_cameraoverride_shadow_depth_bias                       float32  0.006                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_cameraoverride_shadow_end                              float32  0.8                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_change_callback_limit                                  float32  0.2                   FCVAR_CLIENTDLL|FCVAR_RELEASE  // change callback msec warning limit
// cl_chat_active                                            int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_clanid                                                 string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO  // Current clan ID for name decoration
// cl_clock_buffer_ticks                                     float32  0                     FCVAR_DEVELOPMENTONLY  // Clock sync will try to maintain an additional margin of N ticks.  This is intended to smooth over packet loss, and is a replacement for cl_interp_ratio / cl_interp.  This value is simply added to cl_clock_recvmargin_desired
// cl_clock_buffer_ticks_spectator                           float32  2                     FCVAR_DEVELOPMENTONLY  // Additional margin (in ticks) to apply when spectating.
// cl_clock_correction                                       bool     true                  FCVAR_CHEAT  // Enable/disable clock correction on the client.
// cl_clock_recvmargin_adjust_limit_slowdown                 float32  93                    FCVAR_DEVELOPMENTONLY  // Clock sync will not slow down time slower than N%
// cl_clock_recvmargin_adjust_limit_speedup                  float32  106                   FCVAR_DEVELOPMENTONLY  // Clock sync will not speed up time faster than N%
// cl_clock_recvmargin_desired                               float32  5                     FCVAR_DEVELOPMENTONLY  // Clock sync will try to maintain N ms margin between tick arrival and polling network.  The effective value is the sum of this and the time implied by cl_clock_buffer_ticks
// cl_clock_recvmargin_spew_interval                         int32    0                     FCVAR_RELEASE
// cl_clock_recvmargin_timeconstant_slowdown                 float32  0.3                   FCVAR_DEVELOPMENTONLY  // Clock sync will remove 63.2% of the error in N seconds
// cl_clock_recvmargin_timeconstant_speedup                  float32  0.6                   FCVAR_DEVELOPMENTONLY  // Clock sync will remove 63.2% of the error in N seconds
// cl_clock_recvmargin_window                                float32  4                     FCVAR_DEVELOPMENTONLY  // Clock sync will use past N seconds
// cl_clockdbg                                               bool     false                 FCVAR_DEVELOPMENTONLY
// cl_clockdrift_max_ticks                                   int32    3                     FCVAR_RELEASE  // Maximum number of ticks the clock is allowed to drift before the client snaps its clock to the server's.
// cl_clutch_mode                                            bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // Silence voice and other distracting sounds until the end of round or next death.
// cl_color                                                  int32    2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO  // Preferred teammate color
// cl_connectionretrytime_p2p                                float32  20                    FCVAR_RELEASE  // Number of seconds over which to spread retry attempts for P2P.
// cl_cq_min_queue                                           int32    0                     FCVAR_USERINFO  // Used by the client to inform the server of their desired queue length.  Derived from cl_tickpacket_recvmargin_desired and cl_tickpacket_desired_queuelength
// cl_crosshair_drawoutline                                  bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Draws a black outline around the crosshair for better visibility
// cl_crosshair_dynamic_maxdist_splitratio                   float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // If using cl_crosshairstyle 2, this is the ratio used to determine how long the inner and outer xhair pips will be. [inner = cl_crosshairsize*(1-cl_crosshair_dynamic_maxdist_splitratio), outer = cl_crosshairsize*cl_crosshair_dynamic_maxdist_splitratio]  [0 
// cl_crosshair_dynamic_splitalpha_innermod                  float32  0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // If using cl_crosshairstyle 2, this is the alpha modification that will be used for the INNER crosshair pips once they've split. [0 - 1]
// cl_crosshair_dynamic_splitalpha_outermod                  float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // If using cl_crosshairstyle 2, this is the alpha modification that will be used for the OUTER crosshair pips once they've split. [0.3 - 1]
// cl_crosshair_dynamic_splitdist                            int32    3                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // If using cl_crosshairstyle 2, this is the distance that the crosshair pips will split into 2. (default is 7)
// cl_crosshair_friendly_warning                             int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // 0: off, 1: on
// cl_crosshair_outlinethickness                             float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Set how thick you want your crosshair outline to draw (0-3)
// cl_crosshair_recoil                                       bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshair_show_desynced_seeds_marker                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_crosshair_sniper_width                                 int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // If >1 sniper scope cross lines gain extra width (1 for single-pixel hairline)
// cl_crosshair_t                                            bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // T style crosshair
// cl_crosshairalpha                                         int32    200                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshaircolor                                         int32    5                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Set crosshair color as defined in game_options.consoles.txt
// cl_crosshaircolor_b                                       int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshaircolor_g                                       int32    255                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshaircolor_r                                       int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshairdot                                           bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshairgap                                           float32  -2.2                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshairgap_useweaponvalue                            bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // If set to 1, the gap will update dynamically based on which weapon is currently equipped
// cl_crosshairsize                                          float32  3.9                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshairstyle                                         int32    2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // 0 = DEFAULT (DISABLED), 1 = DEFAULT STATIC (DISABLED), 2 = DEFAULT (accurate recoil/spread feedback with a fixed inner part), 3 = ACCURATE DYNAMIC (DISABLED) (accurate recoil/spread feedback), 4 = DEFAULT STATIC, 5 = LEGACY (fake recoil - inaccurate feedba
// cl_crosshairthickness                                     float32  0.6                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_crosshairusealpha                                      bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cl_csgo_shoot_debugvis_rdp_text_l                         int32    10                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_csgo_shoot_debugvis_rdp_text_x                         int32    45                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_csgo_shoot_debugvis_show_los                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Show line of last shot.
// cl_csgo_shoot_debugvis_show_rdp                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_csgo_shoot_trim_input_frames                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_cursor_scale                                           float32  1                     FCVAR_ARCHIVE  // Cursor size scaling factor.
// cl_deathcam_audio_mix_phase1_fade_amount                  float32  0.15                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // Sets the amount of ducking to do on death cam fade out. When set to 1, full DeathFadeLayer is applied.
// cl_deathcam_audio_mix_phase1_fade_time                    float32  2                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Sets the amount of time we fade out over.
// cl_deathcam_audio_mix_phase2_fade_amount                  float32  0.5                   FCVAR_CLIENTDLL|FCVAR_RELEASE  // Sets the amount of ducking to do on death cam fade out. When set to 1, full DeathFadeLayer is applied.
// cl_deathcam_audio_mix_phase2_fade_time                    float32  0.4                   FCVAR_CLIENTDLL|FCVAR_RELEASE  // Sets the amount of time we fade out over.
// cl_deathcampanel_position_dynamic                         int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Turn on/off deathcam's kill panel dynamic Y movement
// cl_deathnotices_show_numbers                              int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // 0: default; 1: draw names as just numbers; 2: append number on killer and victim to the name
// cl_debounce_zoom                                          bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Whether or not to disable holding secondary fire to cycle zoom levels
// cl_debug_build_recvmargin_min                             float32  2                     FCVAR_DEVELOPMENTONLY  // Min size of the recv margin queue when in tools/debug mode
// cl_debug_force_push_to_talk                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_debug_overlay_fullposition                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_debug_overlays_broadcast                               bool     false                 FCVAR_RELEASE  // Render debug overlays from server.
// cl_debug_precipitation_surface_graph                      bool     false                 FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // When true, use the surface graph to pass in positions for rainfall.

// cl_debug_round_stat_submission                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_debugviewangle                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Plots view angles yaw at various stages of the frame/tick in Tracy.
// cl_demo_predict                                           int32    1                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Enable 'TrueView' when watching a demo, which attempts to recreate the client's experience more accurately.  0=disable, 1=only if demo version match, 2=always
// cl_demo_steadycam_blendframes                             int32    5                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // blend over this many frames
// cl_demo_steadycam_deflection                              float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // if camera orientation changes this much update orientation
// cl_demo_steadycam_enable                                  int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Stabilize camera orientation/position during demo playback.  1 == remove roll, 2 == steadycam
// cl_demo_steadycam_radius                                  float32  16                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // if camera moves this much from last anchor update anchor
// cl_demo_view_offset_left                                  float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // View offset during demo playback (+/- 1.25 is a good default for human average left/right eye offset)
// cl_demoviewoverride                                       float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Override view during demo playback
// cl_disable_deathcam_audio_mix_fade_out                    bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // When set to true, disables audio being silenced while the death cam fades out.
// cl_disable_postprocessing                                 bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_disable_ragdolls                                       bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_disable_round_end_report                               bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_disconnect_soundevent                                  string   StopSoundEvents.Sto…  FCVAR_DEVELOPMENTONLY  // This soundevent is called to stop the desired soundevents when the game is disconnected.
// cl_disconnect_voice_fade                                  float32  -1                    FCVAR_DEVELOPMENTONLY  // This is a fade of current voices that is called when the game is disconnected. -1.f for no fade on disconnect
// cl_display_flashbang_values                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_display_game_events                                    bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_display_player_visibilty                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_dm_buyrandomweapons                                    bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Player will automatically receive a random weapon on spawn in deathmatch if this is set to 1 (otherwise, they will receive the last weapon)
// cl_dormant_spew                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Spew state on when client entities become dormant or active.
// cl_draw_only_deathnotices                                 bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // For drawing only the crosshair and death notices (used for moviemaking)
// cl_draw_simulating_entities                               bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_draw_simulating_entities_distance                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_drawhud                                                bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Enable the rendering of the hud
// cl_drawhud_force_deathnotices                             int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // 0: default; 1: draw deathnotices even if hud disabled; -1: force no deathnotices
// cl_drawhud_force_radar                                    int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // 0: default; 1: draw radar even if hud disabled; -1: force no radar
// cl_drawhud_force_teamid_overhead                          int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // 0: default; 1: draw teamid even if hud disabled; -1: force no teamid
// cl_drawhud_specvote                                       bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // 1: default; 0: disables vote UI for spectators
// cl_embedded_stream_audio_volume                           float32  0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Embedded stream audio volume
// cl_embedded_stream_audio_volume_xmaster                   bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Whether embedded stream audio volume gets multiplied by master volume
// cl_embedded_stream_video_playing                          float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Embedded stream video playing state
// cl_enable_party_voice                                     bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_ent_attachment_filter_substrings                       string                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // If an attachment's name has any of the given substrings in it, it will be displayed. Substrings can be delimited by the ',' or '|' character.
// cl_ent_joint_axis_size                                    float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_ent_joint_filter_name                                  string                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // If a joint's entire name matches (case insensitive), it will be displayed.
// cl_ent_joint_filter_substrings                            string                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // If a joint's name has any of the given substrings in it, it will be displayed. Substrings can be delimited by the ',' or '|' character.
// cl_ent_joint_lines                                        bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Draw a line between a rendered joint and its parent.
// cl_ent_joint_names                                        bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Draw the name of a rendered joint.
// cl_ent_joint_only_ik_joints                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_ent_joint_use_bind_pose                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_ent_pivot_size                                         float32  20                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_CHEAT
// cl_ent_show_contexts                                      bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Show entity contexts in ent_text display
// cl_ent_showonlyattachment                                 string                         FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_ent_showonlyhitbox                                     int32    -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_ent_skeleton_only_ik_joints                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_ent_text_flags_active                                  int32    -1                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_CHEAT
// cl_ent_text_no_name_really_i_mean_it                      bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_error_report_time                                      float32  0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Minimum time in seconds that must elapse before printing prediction error summary. 0 to disable.
// cl_extrapolate                                            bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Enable/disable extrapolation if interpolation history runs out.
// cl_extrapolate_amount                                     float32  0.25                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Set how many seconds the client will extrapolate entities for.
// cl_fake_timeout                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_fasttempentcollision                                   int32    5                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_firstperson_legs                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_firstperson_legs_aoproxy                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_fixedcrosshairgap                                      float32  3                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // For crosshair style 1: How big to make the gap between the pips in the fixed crosshair
// cl_flushentitypacket                                      int32    0                     FCVAR_CHEAT  // For debugging. Force the engine to flush an entity packet.
// cl_force_next_signon_to_reset                             bool     false                 FCVAR_DEVELOPMENTONLY
// cl_force_spec_hud_color_to_team                           bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Spec hud color setting is always team/teammate
// cl_frametime_summary_report_detailed                      bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // When a perf report is dumped at the end of the session, should it be detailed?
// cl_generate_postdataupdatepreserved                       bool     true                  FCVAR_DEVELOPMENTONLY  // Do we invoke PostDataUpdatePreserved callbacks for entities that had no changes but are still in the PVS?
// cl_globallight_debug                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_depth_bias                                 float32  -999                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_expansion                                  float32  200                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_freeze                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_orig_calc_frustum                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_shadow_mode                                int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_slope_scale_depth_bias                     float32  -999                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_use_alt_focus_region                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_use_optimized_calc_frustum                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_use_shaadow_near_offset                    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_world_bottom_height                        float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_globallight_world_top_height                           float32  4096                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_glow_brightness                                        float32  1                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Brightness of player halos
// cl_glow_item_far_b                                        float32  1                     FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_glow_item_far_g                                        float32  0.4                   FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_glow_item_far_r                                        float32  0.3                   FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_graphics_driver_warning_dont_show_again                bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Graphics driver recommendation (NVIDIA 581.80 / AMD 23.11.1)
// cl_grenadecrosshair_decoy                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Is the grenade crosshair enabled
// cl_grenadecrosshair_explosive                             bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Is the grenade crosshair enabled
// cl_grenadecrosshair_fire                                  bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Is the grenade crosshair enabled
// cl_grenadecrosshair_flash                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Is the grenade crosshair enabled
// cl_grenadecrosshair_keepusercrosshair                     bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Keep the user's crosshair when the grenade crosshair is enabled
// cl_grenadecrosshair_smoke                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Is the grenade crosshair enabled
// cl_grenadecrosshair_tickinterval                          float32  10                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_grenadecrosshair_ticklabels                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_grenadecrosshair_tickscaling                           float32  1.1                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_grenadecrosshairdelay_decoy                            float32  2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // How long should the pin be pulled for before showing the grenade crosshair
// cl_grenadecrosshairdelay_explosive                        float32  2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // How long should the pin be pulled for before showing the grenade crosshair
// cl_grenadecrosshairdelay_fire                             float32  2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // How long should the pin be pulled for before showing the grenade crosshair
// cl_grenadecrosshairdelay_flash                            float32  2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // How long should the pin be pulled for before showing the grenade crosshair
// cl_grenadecrosshairdelay_smoke                            float32  2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // How long should the pin be pulled for before showing the grenade crosshair
// cl_hide_avatar_images                                     int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Hide avatar images for other players. 
	0 - Off.
	1 - Block All
	2 - Block all but friends
// cl_highlights_hud_playback                                int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Highlights hud playback
// cl_hitbox_debug                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_hold_game_events_force_delay_ticks                     int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Debugging convar to force late dispatch of game events.
// cl_hold_game_events_until_server_tick                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Holds game events until client has received the tick the event was fired on.
// cl_http_log_enable                                        bool     false                 FCVAR_CLIENTDLL|FCVAR_DONTRECORD|FCVAR_RELEASE|FCVAR_CLIENTCMD_CAN_EXECUTE  // Allows sending HTTP log from client main menu.
// cl_hud_color                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // 0 = team color, 1 =  white, 2 = bright white, 3 = light blue, 4 = blue, 5 = purple, 6 = red, 7 = orange, 8 = yellow, 9 = green, 10 = aqua, 11 = pink, 12 = teammate color.
// cl_hud_radar_background_alpha                             float32  0.627                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_hud_radar_blur_background                              bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Blurs the radar background.
// cl_hud_radar_map_additive                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Blend Hud radar map additively on top of background.
// cl_hud_radar_scale                                        float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_hud_telemetry_frametime_poor                           float32  100                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Frame time greater than this is considered 'poor'.
// cl_hud_telemetry_frametime_show                           int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Show frame time (FPS) in the HUD.  0=never, 1=only if poor, 2=always
// cl_hud_telemetry_net_detailed                             int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Show breakdown network misdelivery (loss, late delivery, and peak jitter).  0=never, 1=only in poor network conditions, 2=always
// cl_hud_telemetry_net_misdelivery_poor                     float32  2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Packet delivery anomaly rate (0..100) higher than this is considered 'poor'.
// cl_hud_telemetry_net_misdelivery_show                     int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Show percentage of user commands & server snapshots that are missed due to network conditions.  0=never, 1=only in poor conditions, 2=always
// cl_hud_telemetry_net_quality_graph_show                   int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Show packet jitter and netframe loss/reordering in the HUD.  0=never, 1=only in poor conditions, 2=always
// cl_hud_telemetry_ping_poor                                float32  100                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Ping higher than this (ms) is considered 'poor'.
// cl_hud_telemetry_ping_show                                int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Show ping in the HUD.  0=never, 1=only in poor conditions, 2=always
// cl_hud_telemetry_serverrecvmargin_graph_show              int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Show graph of the server recv margin in the HUD.  (How early/late user commands are arriving at the server before they are executed.)   0=never, 1=only when there are command queue problems, 2=always
// cl_ignorepackets                                          bool     false                 FCVAR_CHEAT  // Force client to ignore packets (for debugging).
// cl_import_csgo_config                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_inferno_bodyburn                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_input_enable_raw_keyboard                              bool     false                 FCVAR_RELEASE  // Enable raw keyboard input
// cl_instant_death_anim                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_interp_ag2_for_non_ag2_entities                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_interp_all                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Disable interpolation list optimizations.
// cl_interp_animationvars                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Interpolate LATCH_ANIMATION_BIT vars if interpolation interval is greater than simulation interval
// cl_interp_hermite                                         bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Set to zero do disable hermite interpolation.
// cl_interp_npcs                                            float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Interpolate NPC positions starting this many seconds in past (or the value as per cl_interp_ratio, if greater)
// cl_interp_parallel                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Run interpolation in parallel for entities with no children.
// cl_interp_ratio                                           float32  0                     FCVAR_CLIENTDLL|FCVAR_USERINFO  // Set number of client simulation interpolation ticks.
// cl_interp_simulationvars                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Interpolate LATCH_SIMULATION_BIT vars if interpolation interval is greater than animation interval
// cl_interp_threadmodeticks                                 int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Additional interpolation ticks to use when interpolating with threaded engine mode set.
// cl_interpolate                                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_USERINFO  // Interpolate entities on the client.
// cl_interpolate_report                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Enable to show interpolation profile timing

// cl_inv_volatile_limits                                    string   0:0                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cl_inventory_debug_tooltip                                bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_inventory_radial_immediate_select                      bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // In inventory selection radials. Select weapons the moment the cursor highlights them. Otherwise, only select the selected item on exit.
// cl_inventory_radial_tap_to_cycle                          bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // In inventory selection radials. Select weapons the moment the cursor highlights them. Otherwise, only select the selected item on exit.
// cl_inventory_saved_filter2                                string   all                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_inventory_saved_sort2                                  string   inv_sort_age          FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_invites_only_friends                                   bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If turned on, will ignore in-game invites from recent teammates or other non-friends
// cl_invites_only_mainmenu                                  bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If turned on, will ignore all invites when user is playing a match
// cl_ironsight_dot_scale                                    float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Ironsight dot scale
// cl_ironsight_filter_alpha                                 float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Ironsight filter alpha
// cl_ironsight_min_channel_color                            float32  0.3                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Ironsight min channel color value
// cl_ironsight_usecrosshaircolor                            bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Should the scope dot match the user's crosshair color
// cl_itemimages_dynamically_generated                       uint32   2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // 2: use render-targets; 0: disk assets only
// cl_jitter_bad_threshold_up                                float32  10                    FCVAR_USERINFO  // When upstream packet jitter in a frame exceeds this threshold (ms), the frame is considered to have 'irregular delivery'.  This is a derived value and should not be modified manually
// cl_joystick_enabled                                       bool     false                 FCVAR_ARCHIVE  // Enable joystick input
// cl_lagcompensation_test_auto_target                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Auto-pick value of cl_lagcompensation_test_target.
// cl_lagcompensation_test_target                            int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Player whose head is tracked to test lag compensation.
// cl_language                                               string   english               FCVAR_DEVELOPMENTONLY  // Language
// cl_latch_report                                           bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Enable to output stats about latching
// cl_leveloverview                                          float32  0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_lightquery_debug                                       bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_loadout_saved_sort                                     string   inv_sort_age          FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_lock_camera                                            bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_log_tick                                               bool     false                 FCVAR_DEVELOPMENTONLY  // Log when a tick is received

// cl_log_tick_skips                                         int32    0                     FCVAR_DEVELOPMENTONLY  // Log when the tick delta >= this
// cl_low_latency_vsync_recommendation_dont_show_again       bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_major_store_watch_list                                 string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cl_map_preview_debug_jitter                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_massreport                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_matchlist_controlroom_aid                              int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_max_particle_pvs_aabb_edge_length                      float32  0                     FCVAR_RELEASE
// cl_min_china_movie_time                                   float32  6                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_min_movie_time                                         float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_mute_all_but_friends_and_party                         int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Only allow communication from friends and matchmaking party members. Set to 1 to apply the in non-competitive game modes. Set to 2 will apply the setting in all modes.

// cl_mute_enemy_team                                        bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Block all communication from players on the enemy team.
// cl_mute_player_after_reporting_abuse                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Mute players reported for abuse automatically.
// cl_names_debug                                            bool     false                 FCVAR_DEVELOPMENTONLY
// cl_net_buffer_ticks                                       int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Number of ticks of delay for server snapshots and user commands.  This value controls the value of cl_interp_ratio, which you should not modify directly.
// cl_net_buffer_ticks_use_interp                            bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // If false, we smooth over packet loss by adjusting the clock synchronization to buffer packets.  If true, we process packets immediately and use cl_interp to delay their effects
// cl_net_showeventlisteners                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Show listening addition/removals
// cl_net_showevents                                         int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump game events to console (1=client only, 2=all).
// cl_new_user_phase                                         int32    -1                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // 0: Not Started, 1: Needs Training, 2: Training Complete, -1: Disabled
// cl_obs_interp_enable                                      bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Enables interpolation between observer targets
// cl_obs_interp_speed                                       float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Spectator camera interpolation speed
// cl_observed_bot_crosshair                                 int32    2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Control the crosshair shown when observing a bot. 0: Show player crosshair. 1: Show player crosshair only when bot can be taken over, otherwise show default.. 2: Always show default crosshair for bots.
// cl_paintkit_override                                      string                         FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE
// cl_panel_freeze_time_after_press                          float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // time to freeze mouse/pointer motion after a mouse button press
// cl_parallel_readpacketentities                            bool     true                  FCVAR_DEVELOPMENTONLY  // Set to 1 to use threading snapshot reading (if game supports and server is sending bitcounts).
// cl_parallel_readpacketentities_threshold                  int32    2                     FCVAR_DEVELOPMENTONLY  // Use parallel processing of snapshot reading if above this many entries.
// cl_particle_batch_mode                                    int32    1                     FCVAR_DEVELOPMENTONLY
// cl_particle_create_duplicate_work_for_profiling           int32    0                     FCVAR_DEVELOPMENTONLY  // Create and destroy N particle systems for every one created normally
// cl_particle_fallback_base                                 int32    0                     FCVAR_DEVELOPMENTONLY  // Base for falling back to cheaper effects under load.
// cl_particle_fallback_multiplier                           float32  0.5                   FCVAR_DEVELOPMENTONLY  // Multiplier for falling back to cheaper effects under load.
// cl_particle_log_creates                                   bool     false                 FCVAR_DEVELOPMENTONLY  // Print debug message every time a particle collection is created
// cl_particle_max_count                                     int32    0                     FCVAR_DEVELOPMENTONLY
// cl_particle_newinit                                       bool     true                  FCVAR_DEVELOPMENTONLY  // turn on optimized particle init
// cl_particle_retire_cost                                   float32  0                     FCVAR_CHEAT
// cl_particle_sim_fallback_base_multiplier                  float32  5                     FCVAR_DEVELOPMENTONLY  // How aggressive the switch to fallbacks will be depending on how far over the cl_particle_sim_fallback_threshold_ms the sim time is.  Higher numbers are more aggressive.
// cl_particle_sim_fallback_threshold_ms                     float32  6                     FCVAR_DEVELOPMENTONLY  // Amount of simulation time that can elapse before new systems start falling back to cheaper versions
// cl_particle_simulate                                      bool     true                  FCVAR_CHEAT  // Enables/Disables Particle Simulation
// cl_pclass                                                 string                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // Dump entity by prediction classname.
// cl_pdump                                                  int32    -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Dump info about this entity to screen.
// cl_phys_animated_hierarchy                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_phys_assume_fixed_tick_interval                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // If true, we assume the client uses a fixed tickrate like the server (which may not always be true). If false, we recalculate the number of physics substeps in each client tick based on the actual elapsed time in the tick.
// cl_phys_block_dist                                        float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_phys_block_fraction                                    float32  0.1                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_phys_debug_callback_entities                           bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Print all entities that get touch callbacks. Each entity is printed only once.
// cl_phys_enabled                                           bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Enable all physics simulation
// cl_phys_networked_start_sleep                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_phys_sleep_enable                                      bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Enable sleeping for dynamic physics bodies.
// cl_phys_sound_disable_impact_sounds_under_hard_threshold  bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // if true, impact sounds wont play if no soft impact sound is present and the impact is below the hard velocity threshold.
// cl_phys_stop_at_collision                                 string                         FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_phys_timescale                                         float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Scale time for physics
// cl_phys_visualize_awake                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_ping_fade_deadzone                                     float32  60                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Distance from the crosshair over which the ping is completely invisible
// cl_ping_fade_distance                                     float32  300                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Distance from the crosshair over which the ping fades
// cl_pitchdown                                              float32  89                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_pitchspeed                                             float32  225                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_pitchup                                                float32  89                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_playback_screenshots                                   bool     false                 FCVAR_DEVELOPMENTONLY  // Allows the client to playback screenshot and jpeg commands in demos.
// cl_player_ping_mute                                       int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If 1, player pinging will make a sound, if 0, pings will be silent
// cl_player_ragdolls_collide                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE
// cl_player_visibility_far                                  float32  700                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // distance at which proxy scale is maximized
// cl_player_visibility_far_scale                            float32  1.3                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // proxy scale multiplier at max dist (is 1.0 at mindist)
// cl_player_visibility_near                                 float32  200                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // cull characters nearer than this
// cl_player_visibility_show_stencil_proxy                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_player_visibility_stencil_bloat_amount                 float32  1.4                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_player_visibility_stencil_proxy_min_dist               float32  3                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_player_visibility_stencil_proxy_min_dist_box           float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_player_visibility_stencil_proxy_type                   int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // 0 - box, 1 - dodecahedron
// cl_playerslot_in_names                                    bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE  // prepend controller playerslot to names for debugging
// cl_poll_network_early                                     bool     true                  FCVAR_RELEASE  // Enable polling for network messages every frame, instead of every tick
// cl_pred_always_latch                                      bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_pred_build_verbose                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Verbose spew when building prediction optimized data runs.
// cl_pred_checkstuck                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Perform the additional 'stuck' traces on the client side during prediction.
// cl_pred_optimize                                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Optimize for not repredicting if there were no errors
// cl_pred_parallel_postnetwork                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_pred_print_every_cmd                                   bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // Print something every time we predict a command
// cl_predict_body_shot_fx                                   bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_predict_bomb_defusal                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_predict_head_shot_fx                                   bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_predict_kill_ragdolls                                  bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_predict_weapon_drop                                    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_prediction_savedata_postentitypacketreceived           bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // Experimental optimization.  If you are reading this in 2026, please delete this convar.
// cl_predictioncopy_runs                                    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_prefer_lefthanded                                      bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Left handed preference
// cl_promoted_settings_acknowledged                         string   0:0                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cl_quickinventory_filename                                string   radial_quickinvento…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_quickinventory_lastinv                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_quickinventory_line_update_speed                       float32  65                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radar_always_centered                                  bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If set to 0, the radar is maximally used. Otherwise the player is always centered, even at map extents.
// cl_radar_fast_transforms                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Faster way of placing icons on the mini map.
// cl_radar_icon_scale_min                                   float32  0.6                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Sets the minimum icon scale. Valid values are 0.4 to 1.25.
// cl_radar_rotate                                           bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // 1
// cl_radar_scale                                            float32  0.7                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Sets the radar scale. Valid values are 0.25 to 1.0.
// cl_radar_scale_alternate                                  float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Sets the alternate radar scale. Valid values are 0.25 to 1.0.
// cl_radar_scale_dynamic                                    bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Toggles between a radar that scales dynamically to encompass all the detected elements on the map.
// cl_radar_show_all_players_when_spectating                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Set all players visible on radar when spectating, regardless of whether they have been spotted.
// cl_radar_square_always                                    bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If set, the radar will always be square.
// cl_radar_square_when_spectating                           bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If set, the radar will be square when spectating.
// cl_radar_square_with_scoreboard                           bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If set, the radar will toggle to square when the scoreboard is visible.
// cl_radial_coyote_time                                     float32  0.15                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Selection lenience: How long in seconds the last selected radial segment is used if no segment is selected.
// cl_radial_menu_icon_radius                                float32  200                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_radial_menu_tap_duration                               float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // If nothing in a radial menu is selected, and the button engaging the radial menu is released within this duration, fallback on the radial's tap functionality
// cl_radial_radio_tab                                       int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_radial_radio_tab_0_text_1                              string   #Chatwheel_requests…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_0_text_2                              string   #Chatwheel_requestw…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_0_text_3                              string   #Chatwheel_bplan      FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_0_text_4                              string   #Chatwheel_followin…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_0_text_5                              string   #Chatwheel_midplan    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_0_text_6                              string   #Chatwheel_followme   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_0_text_7                              string   #Chatwheel_aplan      FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_0_text_8                              string   #Chatwheel_requeste…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_1_text_1                              string   #Chatwheel_enemyspo…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_1_text_2                              string   #Chatwheel_needback…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_1_text_3                              string   #Chatwheel_bplan      FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_1_text_4                              string   #Chatwheel_bombcarr…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_1_text_5                              string   #Chatwheel_multiple…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_1_text_6                              string   #Chatwheel_snipersp…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_1_text_7                              string   #Chatwheel_aplan      FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_1_text_8                              string   #Chatwheel_inpositi…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_2_text_1                              string   #Chatwheel_affirmat…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_2_text_2                              string   #Chatwheel_negative   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_2_text_3                              string   #Chatwheel_complime…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_2_text_4                              string   #Chatwheel_thanks     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_2_text_5                              string   #Chatwheel_cheer      FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_2_text_6                              string   #Chatwheel_peptalk    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_2_text_7                              string   #Chatwheel_sorry      FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tab_2_text_8                              string   #Chatwheel_sectorcl…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radial_radio_tap_to_ping                               bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // When tapping the radial radio button, leave a ping if nothing is selected within the time in seconds set in cl_radial_menu_tap_duration
// cl_radial_radio_version_reset                             int32    12                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_radialmenu_deadzone_size                               float32  0.4                   FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_radialmenu_deadzone_size_joystick                      float32  0.17                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_ragdoll_default_scale                                  float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_ragdoll_limit                                          int32    20                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Maximum number of ragdolls to show (-1 disables limit)
// cl_ragdoll_lru_debug                                      bool     false                 FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// cl_ragdoll_physics_enable                                 int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Enable/disable ragdoll physics.
// cl_ragdoll_reload                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_ragdoll_workaround_threshold                           float32  4                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Mainly cosmetic, client-only effect: when client doesn't know the last position of another player that spawns a ragdoll, the ragdoll creation is simplified and ragdoll is created in the right place. If you increase this significantly, ragdoll positions on 
// cl_random_taser_bone_y                                    float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // The Y position used for the random taser force.
// cl_random_taser_force_y                                   float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // The Y position used for the random taser force.
// cl_random_taser_power                                     float32  4000                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Power used when applying the taser effect.
// cl_rebuy                                                  string   Armor PrimaryWeapon…  FCVAR_CLIENTDLL|FCVAR_RELEASE  // The order in which rebuy will attempt to repurchase items
// cl_redemption_reset_timestamp                             string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_refresh_rate_recommendation_dont_show_again            bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_resend                                                 float32  0.5                   FCVAR_RELEASE  // Delay in seconds before the client will resend the 'connect' attempt
// cl_retire_low_priority_lights                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Low priority dlights are replaced by high priority ones
// cl_sanitize_muted_players                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // Hide names and avatars of muted players.
// cl_sanitize_player_names                                  bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Replace names of other players with something non-offensive.
// cl_sceneentity_debug                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Display all thinking scene entities and its data.
// cl_scoreboard_mouse_enable_binding                        string   +attack2              FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Name of the binding to enable mouse selection in the scoreboard
// cl_scoreboard_survivors_always_on                         bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_scoreboard_toggle_enable                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_screenmessage_notifytime                               float32  8                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // How long to display screen message text
// cl_script_attach_debugger_at_startup                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_script_break_in_native_debugger_on_error               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_search_key_token                                       string                         FCVAR_CLIENTDLL|FCVAR_RELEASE  // Development search key token.
// cl_sendtable_cache_filename                               string   sendtables.bin        FCVAR_DEVELOPMENTONLY  // Send tables cache file
// cl_sequence_debug                                         int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_sequence_debug2                                        int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_sequence_model_substring                               string                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_server_graphic1_enable                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // When enabled, 360x60 (<16kb) image file will be displayed to on-server spectators.
// cl_server_graphic2_enable                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // When enabled, 220x45 (<16kb) image file will be displayed to on-server spectators.
// cl_session                                                string   $0                    FCVAR_DEVELOPMENTONLY|FCVAR_SERVER_CAN_EXECUTE
// cl_show_bombs                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_show_camera_position                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_show_clan_in_death_notice                              bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Is set, the clan name will show next to player names in the death notices.
// cl_show_enemy_avatar_colors                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_show_equipment_value                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_show_equipped_character_for_player_avatars             bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// cl_show_head_trajectory                                   float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_show_matchmaking_stat_spew                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_show_observer_crosshair                                int32    2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Show the crosshair of the player being observed. 0: off 1: friends and party 2: everyone 
// cl_show_playernames_max_chars_console                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Shows all player names (including bots) as 16 W's.
// cl_show_quest_info                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_show_splashes                                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_showdemooverlay                                        float32  0                     FCVAR_DEVELOPMENTONLY  // How often to flash demo recording/playback overlay (0 - disable overlay, -1 - show always)
// cl_showerror                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Show prediction errors, 2 for above plus detailed field deltas, 3 to filter out serverside known prediction errors, -entindex for specific entity.
// cl_showfps                                                int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Draw fps meter at top of screen (1 = fps, 2 = smooth fps, 3 = server MS, 4 = Show FPS and Log to file )
// cl_showframenumber                                        bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // Show current framenumber
// cl_showloadout                                            bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Toggles display of current loadout.
// cl_showmem                                                int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Draw approximate memory use at top of screen
// cl_showpos                                                int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE  // Draw current position at top of screen
// cl_showtextmsg                                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Enable/disable text messages printing on the screen.
// cl_showtick                                               int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Show current tick/time values.  Bitmask:  1='render time'  2='GameTime'   4=time of predicted entities  8=offset of predicted entities    (-1 means 'everything')
// cl_showusercmd                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Show user command encoding
// cl_silencer_mode                                          int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // 0: cannot detach; 1: press secondary fire to detach
// cl_simulate_dormant_entities                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_skeleton_instance_smear_boneflags                      bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Smear boneflags across the model.  Costs computation, but tests to make sure your bone flags are consistent.
// cl_skip_hierarchy_update_for_unchanged_entities           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Skip updating hierarchy information in PostDataUpdate for entities that have not changed
// cl_skip_update_animations                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Enable to skip game animations
// cl_smoke_edge_feather                                     float32  21                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// cl_smoke_lower_speed                                      float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// cl_smoke_origin_height                                    float32  68                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// cl_smoke_torus_ring_radius                                float32  61                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// cl_smoke_torus_ring_subradius                             float32  88                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// cl_smoke_volume_growth                                    float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_smoke_volumeprop                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_smooth                                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Smooth view/eye origin after prediction errors
// cl_smooth_draw_debug                                      bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_smooth_root_catchup_factor                             float32  0.21                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_smooth_root_max_accel                                  float32  1000                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_smooth_root_origin_coeff                               float32  100                   FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_smooth_root_timehorizon                                float32  0.125                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_smooth_root_velocity_coeff                             float32  20                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_smooth_targetspeed                                     float32  150                   FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_smoothtime                                             float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Smooth client's view after prediction error over this many seconds
// cl_snd_cast_clear                                         bool     true                  FCVAR_DEVELOPMENTONLY
// cl_snd_cast_retrigger                                     bool     true                  FCVAR_DEVELOPMENTONLY
// cl_snd_new_visualize                                      bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays soundevent name played at it's 3d position
// cl_sniper_auto_rezoom                                     bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Auto-rezoom snipers after a shot
// cl_sniper_delay_unscope                                   bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_sniper_show_inaccuracy                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_spawngroup_log                                         bool     false                 FCVAR_DEVELOPMENTONLY  // Dump the contents of the next spawngroup manifest to file.
// cl_spawngroup_spewresources                               bool     false                 FCVAR_DEVELOPMENTONLY  // Spew all manifest add/updates.
// cl_spec_show_bindings                                     bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE|FCVAR_CLIENTCMD_CAN_EXECUTE  // Toggle the visibility of the spectator bindings.
// cl_spec_stats                                             bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_spec_use_tournament_content_standards                  bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// cl_streams_image_sfurl                                    string   img://loadjpeg:(640…  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Format of Scaleform image representing the stream
// cl_streams_mytwitchtv_channel                             string   http://www.twitch.t…  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Twitch.tv account channel URL
// cl_streams_mytwitchtv_nolink                              string   http://www.twitch.t…  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Twitch.tv account linking URL
// cl_streams_refresh_interval                               float32  300                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // How often to refresh streams list
// cl_streams_request_accept                                 string   application/vnd.twi…  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Header for api request
// cl_streams_request_url                                    string   https://api.twitch.…  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Number of streams requested for display
// cl_streams_write_response_file                            string                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // When set will save streams info file for diagnostics
// cl_svc_usercmds_delta_validate                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Validate consistency of delta-encoded user commands.  Requires server to have sv_cq_validate_encoded_svc_usercmds enabled.
// cl_teamcounter_playercount_instead_of_avatars             bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_teamid_overhead_colors_show                            bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Show team overhead id in teammate color
// cl_teamid_overhead_fade_near_crosshair                    float32  0.5                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // The amount to fade teamid when near the crosshair. Range is 0.0-1.0. 0: off
// cl_teamid_overhead_maxdist                                int32    6000                  FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_PER_USER  // max distance at which the overhead team id icons will show
// cl_teamid_overhead_maxdist_spec                           int32    4000                  FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_PER_USER  // max distance at which the overhead team id icons will show when a spectator
// cl_teamid_overhead_mode                                   int32    3                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Always show team id over teammates. 0 = off, 1 = pips; 2 = +name, 3 = +equipment
// cl_teammate_color_1                                       color    136 206 245 255       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_teammate_color_2                                       color    0 158 128 255         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_teammate_color_3                                       color    241 228 65 255        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_teammate_color_4                                       color    230 128 42 255        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_teammate_color_5                                       color    189 44 150 255        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_teammate_colors_show                                   int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // In competitive, 1 = show teammates as separate colors in the radar, scoreboard, etc., 2 = show colors and letters
// cl_tickpacket_desired_queuelength                         float32  0                     FCVAR_USERINFO  // This value, multiplied by the tick interval, is added to cl_tickpacket_recvmargin_desired to obtain the effective desired recv margin.
// cl_tickpacket_recvmargin_adjust_limit                     float32  5                     FCVAR_DEVELOPMENTONLY  // Recvmargin-based usercommand pacing will not speed up or slow down command pacing by more than N% compared to realtime
// cl_tickpacket_recvmargin_desired                          float32  5                     FCVAR_DEVELOPMENTONLY  // Recvmargin-based usercommand pacing will try to maintain N ms margin between user command arriving at the server and the server needing that user command.  See also cl_tickpacket_desired_queuelength.
// cl_tickpacket_recvmargin_minsamples                       int32    10                    FCVAR_DEVELOPMENTONLY  // Recvmargin-based usercommand pacing will not take action unless we have N samples
// cl_tickpacket_recvmargin_spew_interval                    int32    0                     FCVAR_RELEASE
// cl_tickpacket_recvmargin_timeconstant                     float32  0.4                   FCVAR_DEVELOPMENTONLY  // Recvmargin-based usercommand pacing will remove 63.2% of the error in N seconds
// cl_tickpacket_recvmargin_window                           float32  4                     FCVAR_DEVELOPMENTONLY  // Recvmargin-based usercommand pacing will use past N seconds
// cl_ticks_net_print_threshold                              float32  2                     FCVAR_RELEASE  // Print a message if network issues cause problems with server snapshots of user commands not being available when needed, if the percentage (0...100) exceeds this value.  A value of 0 will cause the message to always print each time it is calculated
// cl_ticks_warning_level                                    int32    0                     FCVAR_RELEASE  // Print a message about problems with ticks and interpolation.  0=never, 1=warnings, 2=all, even if hidden by interpolation
// cl_timeout                                                float32  30                    FCVAR_ARCHIVE  // After this many seconds without receiving a packet from the server, the client will disconnect itself
// cl_tracer_frequency_override                              int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Override tracer frequency (-1 to disable)
// cl_tracer_whiz_distance                                   float32  72                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_tracer_whiz_infront_distance                           float32  32                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_track_aim_head_log_closest                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Log when closest distance to head was reached and what it was
// cl_track_aim_head_threshold                               float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Notify render device when rendering a frame with enemy head within threshold distance
// cl_track_render_eye_angles                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Spew render eye angles
// cl_trueview_show_doa_predictions                          bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // If true, trueview will recreate the original player experience, including commands that were predicted clientside but never executed on the server because the player was dead when they arrived.
// cl_trueview_show_status                                   int32    2                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // 0=Never; 1=Only if there is a problem; 2=always
// cl_ui_particles_destroy_when_not_painting                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_use_entity_as_targetid                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_use_last_selected_weapon_slot_position                 bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Use the last selected weapon slot position when switching back to a weapon slot.
// cl_use_old_wearable_shoulddraw                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_use_opens_buy_menu                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Pressing the +use key will open the buy menu if in a buy zone (just as if you pressed the 'buy' key).
// cl_usercmd_dbg                                            int32    0                     FCVAR_DEVELOPMENTONLY  // show usercmd payload sizing info for packets with more than this many usercmds
// cl_usercmd_max_per_movemsg                                int32    4                     FCVAR_RELEASE  // max number of CUserCmds to send in one client move message
// cl_usercmd_showsize                                       bool     false                 FCVAR_DEVELOPMENTONLY
// cl_usesocketsforloopback                                  bool     false                 FCVAR_DEVELOPMENTONLY  // When connecting to local listen server (for example, using the 'map' command), default to loopback=false, which connects to '127.0.0.1' instead of 'loopback'.  This uses the network stack so that fake lag/loss can be simulated.
// cl_versus_intro                                           bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_view_near_hud_player_eye_dist                          float32  20                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_view_near_other_player_eye_dist                        float32  16                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_viewing_vanity_loadout                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_USERINFO
// cl_viewmodelsclonedasworld                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_voiceenabled                                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_voip_lobby_audio_volume                                int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Lobby voip stream audio volume
// cl_vrr_recommendation_dont_show_again                     bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_vsnd_morph_override_ease_enabled                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Controls whether the compiled in vsnd morph data ease in/out values are used or values set from the convars (cl_vsnd_morph_override_ease_in, cl_vsnd_morph_override_ease_out) are used
// cl_vsnd_morph_override_ease_in                            float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // If cl_enable_vsnd_morph_override_ease_enabled is true, ease into vsnd morph driven animation over the specified number of seconds.
// cl_vsnd_morph_override_ease_out                           float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // If cl_enable_vsnd_morph_override_ease_enabled is true, ease out of vsnd morph driven animation over the specified number of seconds.
// cl_wallbang_heavy_threshold                               int32    22                    FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE  // The Threshold where to switch from Light to Heavy Wallbang tracer
// cl_weapon_debug_print_accuracy                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// cl_weapon_debug_show_accuracy                             int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE  // Draws a circle representing the effective range with every shot.
// cl_weapon_debug_show_accuracy_duration                    float32  10                    FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE
// cl_weapon_selection_rarity_color                          bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// cl_workshop_map_download_timeout                          float32  120                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_yawspeed                                               float32  210                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// clear_debug_flags_on_death                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// clientport                                                int32    0                     FCVAR_RELEASE  // If non-zero, client binds port to specific address.  Usually you should leave this blank to use a different random system-assigned port for each connection.
// closecaption                                              bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO  // Enable close captioning.
// cloth_debug_draw                                          int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cloth_filter_transform_stateless                          bool     false                 FCVAR_DEVELOPMENTONLY  // Enable the new, stateless version of FilterTransform
// cloth_ground_plane_thickness                              float32  3                     FCVAR_DEVELOPMENTONLY  // Raise ground by this much for all cloth that traces the ground; should be 0 ideally
// cloth_hudmodel_presettle                                  int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cloth_hudmodel_presettle_log                              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cloth_interp_rot                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cloth_iv_dump                                             int32    4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cloth_iv_store_back                                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// cloth_sim_on_tick                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cloth_smooth_motion_correct                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cloth_smooth_motion_extrapolate                           float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cloth_update                                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cojob_lock_hold_warning_threshold_ms                      int32    10000                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How long in milliseconds before we warn about lock hold duration
// cojob_max_no_yield_time_us                                uint32   3000                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Will spew if a job takes longer than the specified number of microseconds
// commentary                                                bool     false                 FCVAR_GAMEDLL|FCVAR_ARCHIVE  // Desired commentary mode state.
// commentary_available                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Automatically set by the game when a commentary file is available for the current map.
// commentary_node_use_viewfacing                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// composite_material_cache_count_max                        int32    16                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// composite_material_dump_images                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// composite_material_save_to_disk                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// composite_material_use_bc7                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// composite_material_use_gpu                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// composite_material_use_gpu_endpoint_optimization          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// composite_material_use_gpu_perceptual_error_metric        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// compositematerial_showdebugwindow                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // Source2/Composite Material Debug
// con_enable                                                bool     false                 FCVAR_ARCHIVE|FCVAR_PER_USER  // Allows the console to be activated.
// con_logfile_suffix                                        string                         FCVAR_DEVELOPMENTONLY  // Suffix to append to the console log, may be changed to reopen the log
// connect_lobby                                             uint64   0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Sets the lobby ID to connect to on start.
// contributionscore_assist                                  int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score added for an assist
// contributionscore_assist_reqs                             int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // extra requirements to earn contribution score for an assist
// contributionscore_bomb_defuse_major                       int32    3                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score for defusing a bomb while at least one enemy remains alive
// contributionscore_bomb_defuse_minor                       int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score for defusing a bomb after eliminating enemy team
// contributionscore_bomb_exploded                           int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score awarded to bomb planter and terrorists remaining alive if bomb explosion wins the round
// contributionscore_bomb_planted                            int32    2                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score for planting a bomb
// contributionscore_cash_bundle                             int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score for picking up a cash bundle
// contributionscore_crate_break                             int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score for breaking an item crate
// contributionscore_hostage_kill                            int32    -2                    FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score for killing a hostage, normally negative
// contributionscore_hostage_rescue_major                    int32    3                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score added to rescuer per hostage rescued
// contributionscore_hostage_rescue_minor                    int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score added to all alive CTs per hostage rescued
// contributionscore_kill                                    int32    2                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score added for a kill
// contributionscore_kill_factor                             float32  0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // percentage of victim's contribution score to award to their killer as a bonus
// contributionscore_kill_reqs                               int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // extra requirements to earn contribution score for a kill
// contributionscore_objective_kill                          int32    3                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score added for an objective related kill
// contributionscore_participation                           int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score awarded to players for active participation in the round
// contributionscore_suicide                                 int32    -2                    FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score for a suicide, normally negative
// contributionscore_team_kill                               int32    -2                    FCVAR_GAMEDLL|FCVAR_RELEASE  // amount of contribution score for a team kill, normally negative
// convars_echo_toggle_changes                               bool     true                  FCVAR_DEVELOPMENTONLY  // Echo to the console changes caused by toggling.
// cpu_level                                                 int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // CPU Level - Default: High
// cq_buffer_bloat_msecs_max                                 float32  64                    FCVAR_REPLICATED|FCVAR_RELEASE  // Server will not allow the client to buffer up more than N ms of commands.
// cq_debug                                                  int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Verbose command queue logging.
// cq_dilation_percentage                                    float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // When speeding up slowing down, this is how much
// cq_enable                                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Run one usercmd per server tick and maintain a buffer.  Client speeds up/slows down it's usercmd tick rate to maintain server command queue buffering.
// cq_fake_starve                                            int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // if set, starve this many commands by discarding during process usercmds.
// cq_logging                                                bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // command queue logging of events.
// cq_logging_interval                                       float32  0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // command queue logging per player stats every N seconds, 0 to disable.
// cq_max_starved_substitute_commands                        int32    4                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Server will stop generating substitute commands if client hasn't sent one, after N in a row
// cq_print_every_command                                    bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // print every command as we execute it
// cq_runtests                                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// cq_runtests_broadcast_info                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // send message to remote client console when tests change.
// cq_runtests_interval                                      float32  30                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// crosshair                                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// cs2_bomb_damage_showdebugwindow                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // CSGO/Bomb Damage
// cs_AssistDamageThreshold                                  int32    25                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // cs_AssistDamageThreshold defines the amount of damage needed to score an assist
// cs_ShowStateTransitions                                   int32    -2                    FCVAR_GAMEDLL|FCVAR_CHEAT  // cs_ShowStateTransitions <ent index or -1 for all>. Show player state transitions.
// cs_hostage_near_rescue_music_distance                     float32  2000                  FCVAR_GAMEDLL|FCVAR_CHEAT
// cs_logtouchexpansion                                      int32    -2                    FCVAR_GAMEDLL|FCVAR_CHEAT  // cs_logtouchexpansion <ent index or -1 for all>. Log player touch expansion component.
// cs_minimap_create_output_size                             int32    1024                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // Size of minimap texture generated with cs_minimap_create (512 default)
// cs_minimap_renderdoc_capture_enabled                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// cs_minimap_rendering_msaa_mode                            int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // MSAA mode used for minimap rendering 0-none, 1-2xMSAA, 2-4xMSAA, 3-6X, 4-8X, etc
// cs_steamvideo_max_kills_per_multikill                     int32    5                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Max number of kills for a single multikill event
// cs_steamvideo_max_time_between_multikill_events           float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Maximum time in seconds between consecutive kills for them to be combined into a multikill event
// cs_steamvideo_multikill_padding_time                      float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Time in seconds to add before the first kill and after the last kill for multikill events
// csgo_3d_skybox                                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// csgo_demoui_playbeck_timestep_value                       int32    15                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Number of seconds to seek when using TimeStep buttons on demo playback controller.
// csgo_demoui_player_death_seek_lead_up_time                float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Seek to a moment this amount of seconds leading up to a player death instead of the exact time of the death.
// csgo_demoui_previous_event_search_offset                  float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Do not consider events that happened in the last specified number of seconds when a user clicks 'previous' on the UI.
// csgo_disable_preview_maps                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// csgo_fatdemo_enable                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// csgo_fatdemo_output                                       string   test.fatdem           FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// csgo_map_preview_scale                                    float32  2.5640001             FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// csgo_nav_jump_link_detour_threshold                       float32  1500                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // don't traverse a jump link if there's a detour that costs less than this amount
// csgo_use_fullsort_for_opaque                              bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // fullsort the opaque pass when there wasn't a depth prepass
// csm_bias_override_0                                       float32  1                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_bias_override_1                                       float32  1                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_bias_override_2                                       float32  1                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_bias_override_3                                       float32  1                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_cascade0_override_dist                                float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_cascade1_override_dist                                float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_cascade2_override_dist                                float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_cascade3_override_dist                                float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_cascade_viewdir_shadow_bias_scale                     float32  2                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_max_dist_between_caster_and_receiver                  float32  15000                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // default pushback
// csm_max_num_cascades_override                             int32    3                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Number of cascades in sunlight shadow
// csm_max_shadow_dist_override                              float32  560                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// csm_max_visible_dist                                      float32  7500                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_res_override_0                                        uint32   0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_res_override_1                                        uint32   0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_res_override_2                                        uint32   0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_res_override_3                                        uint32   0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_shadow_worldview_align_x_to_u                         bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_shadow_worldview_shear_align_z_to_v                   bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_sst_max_visible_dist                                  float32  2000                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_sst_pushback_distance                                 float32  1500                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // default pushback
// csm_sst_shadow_focus_region_maxz                          float32  15900                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_sst_shadow_focus_region_minz                          float32  -15900                FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_sst_shadow_focus_region_thin_compensation             float32  1500                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_viewdir_shadow_bias                                   float32  0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_viewmodel_max_shadow_dist                             float32  21                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_viewmodel_max_visible_dist                            float32  1000                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_viewmodel_nearz                                       float32  0.5                   FCVAR_CLIENTDLL|FCVAR_CHEAT
// csm_viewmodel_shadows                                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// custom_bot_difficulty                                     int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Bot difficulty for offline play.
// cv_bot_ai_bt_debug_target                                 int32    -1                    FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Draw the behavior tree of the given bot.
// cv_bot_ai_bt_hiding_spot_show                             bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Draw hiding spots.
// cv_bot_ai_bt_moveto_show_next_hiding_spot                 bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Draw the hiding spot the bot will check next.
// damage_impact_heavy                                       int32    40                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Damage ABOVE this value is considered heavy damage
// damage_impact_medium                                      int32    20                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Damage BELOW this value is considered light damage
// death_chase_distance                                      float32  76                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// death_panel_delay_time                                    float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// death_panel_travel_time                                   float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// debug_aim_angle                                           int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// debug_async_data_panel_override_state                     int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Force ALL async data panels to be in a specific state. -1:disabled, 0:failure, 1:loading, 2:success
// debug_chicken                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Chicken debug info
// debug_destructible_parts                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Draw debug information for destructible parts.
// debug_destructible_parts_enabled                          bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Toggle enabling/disabling the destructible parts system for debug.
// debug_destructible_parts_radius_damage                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// debug_destructible_parts_ttl                              float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How long the debug draws stick around for, unless they're per-tick.
// debug_draw_enable                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// debug_error_model                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// debug_font_name                                           string   Courier New           FCVAR_DEVELOPMENTONLY  // Debug font name
// debug_font_size                                           int32    14                    FCVAR_DEVELOPMENTONLY  // Font size for the debug font
// debug_hltv                                                int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CLIENTCMD_CAN_EXECUTE  // Print out hltv events
// debug_overlay_fullposition                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// debug_physimpact                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// debug_radial_damage                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// debug_shared_random                                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// debug_takedamage_summaries                                bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// debug_video_config_cvars                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// debug_visibility_monitor                                  int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// debugoverlay_circle_verts                                 int32    24                    FCVAR_CHEAT
// debugoverlay_enable_dotted_dashed                         bool     true                  FCVAR_CHEAT  // Toggle the use of dotted/dashed debugoverlay lines to indicate source
// debugoverlay_force_respect_ttl                            bool     false                 FCVAR_CHEAT  // Force respect TTL even when clearing scopes
// debugoverlay_show_text_outline                            bool     false                 FCVAR_CHEAT  // Toggle display of box around text
// debugoverlay_text_scale                                   float32  1                     FCVAR_ARCHIVE|FCVAR_CHEAT  // Scale of the text used for 3d display, but see also debug_font_{size,name}
// decalfrequency                                            float32  10                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_NOTIFY
// default_fov                                               float32  90                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// demo_allow_game_mismatch                                  bool     false                 FCVAR_DEVELOPMENTONLY  // Allow playback of demo even if game directories are not matched [may crash or fail to load].
// demo_debug                                                int32    0                     FCVAR_DEVELOPMENTONLY  // Turn on demo debug spew.
// demo_flush                                                bool     false                 FCVAR_ARCHIVE  // Flush writing the demo file every network update
// demo_highlight_fade_duration                              float32  0.25                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // Duration of the fade in and of the fade out transitions (fade in + fade out is 2x this value).
// demo_highlight_seconds_after                              float32  2                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // How many seconds after the actual highlight event to show when viewing highlights.
// demo_highlight_seconds_before                             float32  6                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // How many seconds before the actual highlight event to show when viewing highlights.
// demo_max_consecutive_skip_packets                         int32    2500                  FCVAR_DEVELOPMENTONLY  // Don't skip more than N messages in a row when skipping in a demo file.
// demo_mouse_enable_binding                                 string   drop                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Name of the binding to enable mouse on demo playback UI
// demo_pause_at_end                                         bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // Pause demo playback when the end of the file is reached, otherwise quit to main menu.
// demo_playback_override_settings                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// demo_quitafterplayback                                    bool     false                 FCVAR_RELEASE  // Quits game after demo playback.
// demo_recordcommands                                       bool     true                  FCVAR_CHEAT  // Record commands typed at console into .dem files.
// demo_skip_to_shot_seconds_before                          float32  2                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // How many seconds before the shot to skip to when skipping to a specific shot ID.
// demo_ui_mode                                              int32    2                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // UI mode for demo playback. 0 = disabled, 1 = minimal, 2 = full
// demo_usefastgoto                                          bool     true                  FCVAR_DEVELOPMENTONLY  // Use fast frame skipping when available for demo_goto commands.
// demo_writefullupdate_rate                                 int32    60                    FCVAR_DEVELOPMENTONLY  // Interval time in seconds to write full updates to demo.
// destructible_parts_destroy_parts_when_gibbing             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// dev_add_onground_on_spawn                                 bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Should we mess with the ground flag when we spawn? (I don't think we should). If we don't hit the assert in CCSPlayer_MovementServices::ProcessMovement, we should remove this by Dec 2022.
// dev_create_bhop_reports                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Whether we should create bhop reports when you jump. Reports are created for the client and server and are numbered monotonically
// dev_create_move_report                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Whether we should create move reports when you push movement keys. Reports are created for the server and are numbered monotonically
// dev_create_sensitivity_report                             float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// dev_create_smooth_motion_report                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// dev_cs_force_disable_move                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // forcibly prevent players from moving
// dev_cs_frame_firing_enable                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Enable that firing will pretend like it's happening on frames.
// dev_cs_frame_firing_insert_idle_pose_now                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Should we insert the idle pose at this time to make the animation interpolation punchier?
// dev_cs_frame_firing_play_animevents                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Should we play the animevents that animgraph will skip over?
// dev_cs_frame_firing_skip_first_frame_enable               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Should we skip the first frame of shooting to make the animation punchier?
// dev_cs_frame_firing_tick_offset_enable                    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Should we offset the current frame to the tick
// dev_cs_ragdoll_head_ankle_delta_z_threshold               float32  35                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// dev_cs_ragdoll_nudge_intensity                            float32  500                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// dev_cs_ragdoll_nudge_max_duration                         float32  1.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// dev_cs_ragdoll_progress_check_interval                    float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// dev_reportmoneychanges                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Displays money account changes for players in the console
// developer                                                 int32    0                     FCVAR_RELEASE  // Set developer message level.
// devonly_chicken_activity_debug                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Print chicken activity info to the console
// devonly_chicken_blocktimer                                float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Chicken blockertimer
// devonly_chicken_feeler_distance                           float32  30                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Chicken feeler distance
// devonly_chicken_feeler_height                             float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Chicken feeler height
// devonly_chicken_feeler_pitch                              float32  45                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Chicken feeler pitch
// diffcheck                                                 bool     true                  FCVAR_DEVELOPMENTONLY  // Activate diffcheck system.
// diffcheck_playerslot                                      int32    0                     FCVAR_DEVELOPMENTONLY
// diffcheck_spew                                            bool     true                  FCVAR_DEVELOPMENTONLY  // Actually show diffcheck results.
// diffcheck_spew_diff_filter                                string                         FCVAR_DEVELOPMENTONLY  // Show diff with matching filter substring only.
// diffcheck_spew_diff_only                                  bool     false                 FCVAR_DEVELOPMENTONLY  // Show diff only.
// disable_dynamic_prop_loading                              bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // If non-zero when a map loads, dynamic props won't be loaded
// disable_source_soundscape_trace                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Bypasses lookup of soundscapes for indvidual audio sources when enabled.
// display_convars_onscreen_in_big_text                      string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Display the convars on the screen in big text. Use semicolons to separate multiple convars
// display_game_events                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// dota_enable_spatial_audio                                 bool     false                 FCVAR_RELEASE  // Flag to enable spatial audio in Dota 2.
// dota_spatial_audio_mix                                    float32  1                     FCVAR_RELEASE  // Mix value to blend spatial and non-spatial audio in Dota 2.
// dsp_automatic                                             int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_DEMO
// dsp_db_min                                                int32    80                    FCVAR_DEVELOPMENTONLY|FCVAR_DEMO
// dsp_db_mixdrop                                            float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_DEMO
// dsp_dist_max                                              float32  1440                  FCVAR_CHEAT|FCVAR_DEMO
// dsp_dist_min                                              float32  0                     FCVAR_CHEAT|FCVAR_DEMO
// dsp_mix_max                                               float32  0.8                   FCVAR_DEVELOPMENTONLY|FCVAR_DEMO
// dsp_mix_min                                               float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_DEMO
// dsp_off                                                   bool     false                 FCVAR_CHEAT
// dsp_vol_2ch                                               float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_DEMO
// dsp_vol_4ch                                               float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_DEMO
// dsp_vol_5ch                                               float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_DEMO
// dsp_volume                                                float32  0.8                   FCVAR_ARCHIVE|FCVAR_DEMO
// dump_audio_input                                          bool     false                 FCVAR_DEVELOPMENTONLY
// econ_debug_loadout_ui                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Show debug data when players change their loadout.
// econ_enable_inventory_images                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // allow inventory image rendering for use by scaleform
// econ_inventory_image_pinboard                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// enable_boneflex                                           bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// engine_accurate_input_processing_delta_time               bool     false                 FCVAR_DEVELOPMENTONLY  // When true, elapsed time given to the input processing will be the time elapsed since the last input processing. This is only relevant when input is processed multiple times per frame ( i.e. multiple ticks per frame)
// engine_allow_multiple_simulates_per_frame                 bool     false                 FCVAR_DEVELOPMENTONLY  // When the client is catching up in low frame rate situations, should we run client simulate more than once a frame?
// engine_allow_multiple_ticks_per_frame                     bool     true                  FCVAR_DEVELOPMENTONLY  // When the client is catching up in low frame rate situations, should we run tick more than once a frame?
// engine_client_tick_pad_enable                             bool     false                 FCVAR_DEVELOPMENTONLY
// engine_cpu_info_extended                                  string   16 CPUs (8 cores), …  FCVAR_DEVELOPMENTONLY  // CPU the engine is running on.
// engine_frametime_amnesty_debug                            bool     false                 FCVAR_DEVELOPMENTONLY  // Enable logging about events that disable frame time warnings
// engine_frametime_warnings_enable                          bool     true                  FCVAR_DEVELOPMENTONLY  // Enable framerate-related warnings, such as sv_long_frame_ms.  Disabling warnings is useful when running in situations such a debug where a slow frame rate is expected
// engine_low_latency_sleep_after_client_tick                bool     false                 FCVAR_RELEASE  // When r_low_latency is enabled, this moves the low latency sleep on tick frames to happen after client simulation.
// engine_max_resource_system_update_time                    int32    5                     FCVAR_DEVELOPMENTONLY
// engine_max_ticks_to_simulate                              int32    -1                    FCVAR_DEVELOPMENTONLY  // Max number of ticks to simulate per frame, after which simulation will start to slow down compared to real time.
// engine_no_focus_sleep                                     int32    20                    FCVAR_ARCHIVE
// engine_no_focus_sleep_vconsole_suppress                   bool     true                  FCVAR_DEVELOPMENTONLY  // When VConsole is in the foreground, don't trigger engine_no_focus_sleep behavior
// engine_ostype                                             string   Windows 11            FCVAR_DEVELOPMENTONLY  // OS type the engine is running on.
// engine_phys_debug_limit_ticks                             bool     true                  FCVAR_DEVELOPMENTONLY
// engine_platform_name_extended                             string   pc64                  FCVAR_DEVELOPMENTONLY  // Platform the engine is running on.
// engine_relaunch_app_before_exiting                        bool     false                 FCVAR_RELEASE  // Use this to tell Steam to relaunch the app right after existing
// engine_render_only                                        bool     false                 FCVAR_DEVELOPMENTONLY
// engine_rendersystem_init                                  string   -dx11                 FCVAR_DEVELOPMENTONLY  // Rendersystem option requested (changing this does not change the rendersystem).
// engine_rendersystem_shader_model                          int32    111                   FCVAR_DEVELOPMENTONLY  // Rendersystem shader model in use (changing this does not change the shader model).
// engine_rendersystem_used                                  string   -dx11                 FCVAR_DEVELOPMENTONLY  // Rendersystem option in use (changing this does not change the rendersystem).
// engine_show_frame_dispatch                                bool     false                 FCVAR_DEVELOPMENTONLY  // show frame dispatch names.
// engine_show_frame_pacing                                  bool     false                 FCVAR_RELEASE
// engine_show_frame_ticks                                   bool     false                 FCVAR_DEVELOPMENTONLY
// engine_sse42                                              bool     true                  FCVAR_DEVELOPMENTONLY  // turn on sse4.2 optimizations in the engine
// engine_update_resource_system_during_low_latency_sleep    bool     true                  FCVAR_DEVELOPMENTONLY
// english                                                   bool     true                  FCVAR_CLIENTDLL|FCVAR_USERINFO  // If set to 1, running the english language set of assets.
// ent_actornames_font                                       string   Consolas              FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // ent_actornames font name
// ent_actornames_fontsize                                   int32    24                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // ent_actornames font size
// ent_attachment_filter_substrings                          string                         FCVAR_GAMEDLL|FCVAR_CHEAT  // If an attachment's name has any of the given substrings in it, it will be displayed. Substrings can be delimited by the ',' or '|' character.
// ent_bitvec_enable                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ent_debug_draw_thinkers                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// ent_joint_axis_size                                       float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ent_joint_filter_name                                     string                         FCVAR_GAMEDLL|FCVAR_CHEAT  // If a joint's entire name matches (case insensitive), it will be displayed.
// ent_joint_filter_substrings                               string                         FCVAR_GAMEDLL|FCVAR_CHEAT  // If a joint's name has any of the given substrings in it, it will be displayed. Substrings can be delimited by the ',' or '|' character.
// ent_joint_lines                                           bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw a line between a rendered joint and its parent.
// ent_joint_names                                           bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw the name of a rendered joint.
// ent_joint_only_ik_joints                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ent_joint_use_bind_pose                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ent_messages_draw                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Visualizes all entity input/output activity.
// ent_pivot_size                                            float32  20                    FCVAR_GAMEDLL|FCVAR_ARCHIVE|FCVAR_CHEAT
// ent_revert_dormancy_change                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// ent_show_contexts                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Show entity contexts in ent_text display
// ent_showonlyattachment                                    string                         FCVAR_GAMEDLL|FCVAR_CHEAT
// ent_skeleton_duration                                     float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Duration of ent_skeleton display
// ent_skeleton_only_ik_joints                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ent_skeleton_snapshot                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ent_steadystate_batchsize                                 int32    20                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Max number of entities to transmit to player
// ent_steadystate_delay                                     float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Time in seconds without network state changes until an entity is considered for trickle updates
// ent_steadystate_enable                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// ent_steadystate_interval                                  float32  0.1                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Rate at which entities can be trickled to players
// ent_test_interpolation                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// ent_text_flags_active                                     int32    -1                    FCVAR_GAMEDLL|FCVAR_ARCHIVE|FCVAR_CHEAT
// ent_text_no_name_really_i_mean_it                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// entity_log_load_unserialize                               int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Output unserialization of entities on map load. 0 - off, 1 - client/server, 2 - server, 3 - client
// eom_local_player_defeat_anim_enabled                      bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// execute_command_every_frame                               string                         FCVAR_CHEAT
// fade_debug_splitscreen_slot                               int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// ff_damage_bullet_penetration                              float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If friendly fire is off, this will scale the penetration power and damage a bullet does when penetrating another friendly player
// ff_damage_decoy_explosion                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Enables or disables team damage from decoy detonation
// ff_damage_reduction_bullets                               float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How much to reduce damage done to teammates when shot.  Range is from 0 - 1 (with 1 being damage equal to what is done to an enemy)
// ff_damage_reduction_grenade                               float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How much to reduce damage done to teammates by a thrown grenade.  Range is from 0 - 1 (with 1 being damage equal to what is done to an enemy)
// ff_damage_reduction_grenade_self                          float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How much to damage a player does to himself with his own grenade.  Range is from 0 - 1 (with 1 being damage equal to what is done to an enemy)
// ff_damage_reduction_other                                 float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How much to reduce damage done to teammates by things other than bullets and grenades.  Range is from 0 - 1 (with 1 being damage equal to what is done to an enemy)
// filesystem_buffer_size                                    int32    0                     FCVAR_DEVELOPMENTONLY  // Size of per file buffers. 0 for none
// filesystem_fake_latency                                   int32    0                     FCVAR_DEVELOPMENTONLY
// filesystem_max_stdio_read                                 int32    16                    FCVAR_DEVELOPMENTONLY
// filesystem_native                                         bool     true                  FCVAR_DEVELOPMENTONLY  // Use native FS or STDIO
// filesystem_report_buffered_io                             bool     false                 FCVAR_DEVELOPMENTONLY
// filesystem_unbuffered_io                                  bool     true                  FCVAR_DEVELOPMENTONLY
// filter_player_simulation_time                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// fire_use_modifier                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// fish_debug                                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Show debug info for fish
// fish_dormant                                              bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Turns off interactive fish behavior. Fish become immobile and unresponsive.
// fog_color                                                 vector3  -1 -1 -1              FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_colorskybox                                           vector3  -1 -1 -1              FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_enable                                                bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Enable fog
// fog_enableskybox                                          bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_end                                                   float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_endskybox                                             float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_hdrcolorscale                                         float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_hdrcolorscaleskybox                                   float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_maxdensity                                            float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_maxdensityskybox                                      float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_override                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Overrides the map's fog settings (-1 populates fog_ vars with map's values)
// fog_override_enable                                       bool     false                 FCVAR_CHEAT  // Use fog_override convars instead of world fog data
// fog_override_end                                          float32  3500                  FCVAR_CHEAT
// fog_override_exponent                                     float32  2                     FCVAR_CHEAT
// fog_override_max_density                                  float32  0.4                   FCVAR_CHEAT
// fog_override_start                                        float32  1000                  FCVAR_CHEAT
// fog_start                                                 float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_startskybox                                           float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// fog_volume_debug                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // If enabled, prints diagnostic information about the current fog volume
// font_show_glyph_miss                                      bool     false                 FCVAR_DEVELOPMENTONLY
// footstep_audible_threshold                                float32  0.55                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// footstep_debug                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// footstep_force_volume                                     float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// force_spectator_only_tools                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// fov_cs_debug                                              float32  0                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Sets the view fov if cheats are on.
// fov_cs_near_z                                             float32  6.5                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// fov_cs_super_ultrawide_near_z                             float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// fov_cs_ultrawide_near_z                                   float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// fov_desired                                               float32  75                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO  // Sets the base field-of-view.
// fps_max                                                   float32  400                   FCVAR_ARCHIVE|FCVAR_RELEASE  // Frame rate limiter.  0=no limit.  Does not apply to dedicated server.
// fps_max_tools                                             float32  120                   FCVAR_ARCHIVE  // Additional frame rate limit while in tools mode and a window other than the game window has focus. Note that fps_max still applies, this only allows the maximum frame rate for tools mode to be lower. 0=no tools specific limit.
// fps_max_ui                                                float32  200                   FCVAR_ARCHIVE  // Frame rate limiter while the game UI is displayed.  0=no limit.  Does not apply to dedicated server.
// frag_grenade_blip_frequency                               float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// freecamera_accel                                          float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Tweak this parameter to adjust Free Camera movement acceleration.
// freecamera_fog_end                                        float32  2500                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Fog end for Free Camera.
// freecamera_fog_start                                      float32  1800                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Fog start for Free Camera.
// freecamera_max_speed                                      float32  500                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Tweak this parameter to adjust Free Camera movement max speed.
// freecamera_rotation_multiplier                            float32  10                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Tweak this parameter to adjust Free Camera mouse rotation.
// freecamera_zfar                                           float32  4500                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Fog start for Free Camera.
// fs_async_threads                                          int32    -1                    FCVAR_DEVELOPMENTONLY  // Number of IO threads in async filesystem (-1 == auto)
// fs_fake_read_delay_ms                                     int32    0                     FCVAR_RELEASE  // Add N ms of delay to every low-level read operation, to simulate a slow disk
// fs_report_async_io                                        bool     false                 FCVAR_DEVELOPMENTONLY
// fs_report_long_reads                                      int32    0                     FCVAR_DEVELOPMENTONLY  // 0:Off, 1:All (for tracking accumulated duplicate read times), >1:Microsecond threashold
// fs_report_sync_opens                                      int32    0                     FCVAR_RELEASE  // 0:Off, 1:Always, 2:Not during load
// fs_warning_mode                                           int32    0                     FCVAR_DEVELOPMENTONLY  // 0:Off, 1:Warn main thread, 2:Warn other threads
// func_break_max_pieces                                     int32    15                    FCVAR_GAMEDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED
// func_break_reduction_factor                               float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_breakdmg_bullet                                      float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_breakdmg_club                                        float32  1.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_breakdmg_explosive                                   float32  1.25                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_async_movable_navmesh_updates                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_debug                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_debug_all                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_debug_follower                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_debug_parallel                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_debug_showtext                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_debug_verbose                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_force_transition_start_direction               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_get_speed_override                             float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_imgui_log_count                                int32    30                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_mover_run_parallel                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_rotator_debug                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// func_rotator_run_parallel                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// fx_drawmetalspark                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Draw metal spark effects.
// g_debug_angularsensor                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// g_debug_constraint_sounds                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Enable debug printing about constraint sounds.
// g_debug_doors                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// g_debug_ragdoll_visualize                                 bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// g_debug_transitions                                       int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // Set to 1 and restart the map to be warned if the map has no trigger_transition volumes. Set to 2 to see a dump of all entities & associated results during a transition.
// g_ragdoll_fadespeed                                       int32    600                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// g_ragdoll_important_maxcount                              int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// g_ragdoll_lvfadespeed                                     int32    100                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// g_ragdoll_maxcount                                        int32    5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// game_mode                                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The current game mode (based on game type). See GameModes.txt.
// game_online                                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The current game is online.
// game_particle_manager_requeue_messages                    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// game_public                                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The current game is public.
// game_type                                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The current game type. See GameModes.txt.
// gameevents_showeventlisteners                             bool     false                 FCVAR_DEVELOPMENTONLY  // Show listening addition/removals
// gameevents_showevents                                     int32    0                     FCVAR_DEVELOPMENTONLY  // Dump game events to console. (1 = Show Signaling, 2 = Show Posting also).
// gameinstructor_enable                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // Display in game lessons that teach new players.
// gameinstructor_find_errors                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Set to 1 and the game instructor will run EVERY scripted command to uncover errors.
// gameinstructor_start_sound_cooldown                       float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Number of seconds forced between similar lesson start sounds.
// gameinstructor_verbose                                    int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Set to 1 for standard debugging or 2 (in combo with gameinstructor_verbose_lesson) to show update actions.
// gameinstructor_verbose_lesson                             string                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // Display more verbose information for lessons have this name.
// gamestats_file_output_directory                           string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // When -gamestatsfileoutputonly is specified, file will be emitted here instead of to modpath

// gc_secret_key                                             string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_PROTECTED  // Secret key for authenticating with the GC

// gl_clear                                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// glow_chickens                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Glow chickens with a green outline.
// glow_outline_width                                        float32  6                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Width of glow outline effect in screen space.
// glow_use_tolerance                                        float32  0.85                  FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// gotv_theater_container                                    string                         FCVAR_CLIENTDLL|FCVAR_RELEASE  // Enables GOTV theater mode for the specified container, setting it to 'live' will play top live matches
// gpu_level                                                 int32    3                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // GPU Level - Default: High
// gpu_mem_level                                             int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Memory Level - Default: High
// hairsim_force_fixed_timestep                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// hairsim_reset                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// healthshot_allow_use_at_full                              bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// healthshot_health                                         int32    50                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// healthshot_healthboost_damage_multiplier                  float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// healthshot_healthboost_speed_multiplier                   float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// healthshot_healthboost_time                               float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// hidehud                                                   int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // bitmask: 1=weapon selection, 2=flashlight, 4=all, 8=health, 16=player dead, 32=needssuit, 64=misc, 128=chat, 256=crosshair, 512=vehicle crosshair, 1024=in vehicle
// hinttext_displaytime                                      float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// host_force_frametime_to_equal_tick_interval               bool     false                 FCVAR_DEVELOPMENTONLY
// host_force_max_frametime_to_tick_interval                 bool     false                 FCVAR_DEVELOPMENTONLY
// host_framerate                                            float32  0                     FCVAR_RELEASE  // Set to lock per-frame time elapse.
// host_readconfig_ignore_userconfig                         bool     false                 FCVAR_CHEAT  // Whether we should ignore the user config file for reading/writing.
// host_timescale                                            float32  1                     FCVAR_REPLICATED|FCVAR_CHEAT  // Prescale the clock by this amount.
// hostage_debug                                             int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Show hostage AI debug information
// hostage_drop_time                                         float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Time for the hostage before it fully drops to ground
// hostage_is_silent                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // When set, the hostage won't play any code driven response rules lines
// hostfile                                                  string   host.txt              FCVAR_GAMEDLL|FCVAR_RELEASE  // The HOST file to load.
// hostip                                                    int32    -1062717439           FCVAR_RELEASE  // Host game server ip
// hostname                                                  string   steven hawk           FCVAR_RELEASE  // Hostname for server.
// hostname_in_client_status                                 bool     false                 FCVAR_RELEASE  // Show server hostname in client status.
// hostport                                                  int32    27015                 FCVAR_RELEASE  // Host game server port
// hud_fastswitch                                            int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// hud_scaling                                               float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Scales hud elements
// hud_showtargetid                                          bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Enables display of target names
// hullivr_edge_merge_tan                                    float32  0.02                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Should we try to straighten two faces connected to this edge? (tangent)
// hullivr_faceisland_merge_disp                             float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Should we straighten face island if the displacement is this much? (inches)
// hullivr_faceisland_merge_tan                              float32  0.04                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Should we try to straighten an island of faces deviating from their average normal (tangent)?
// hullivr_version                                           int32    3                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// ik_constraints_enabled                                    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_all_chains_unique_color_per_chain                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_ccd                                              int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_chain_to_filter_by                               string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ik_debug_constraints                                      int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_dogleg3bone                                      int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_dogleg3bone_enabled                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_fabrik_backwards_enabled                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_fabrik_backwards_iterations                      int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_fabrik_forwards_enabled                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_fabrik_forwards_iterations                       int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_groundtraces                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Show IK trace related details
// ik_debug_perlin_solver                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// ik_debug_planetilt                                        int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_planetilt_axis_length                            float32  20                    FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_debug_targets                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_enable                                                 bool     true                  FCVAR_REPLICATED|FCVAR_CHEAT  // Enable IK.
// ik_fabrik_align_chain                                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_fabrik_backwards_enabled                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_fabrik_forwards_enabled                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_fabrik_override_num_iterations                         int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_final_fixup_enable                                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// ik_hinge_debug_bone_index                                 int32    -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ik_planetilt_enable                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// imgui_debug_draw_dashboard_toggle_pause                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // Dashboard/Pause Game When Activated
// imgui_debug_draw_dashboard_window                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // Dashboard/Show Dashboard
// imgui_debug_draw_dashboard_window_toggle_focus            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Dashboard toggle focus
// imgui_default_font_size                                   float32  20                    FCVAR_ARCHIVE|FCVAR_CHEAT  // Default imgui font size
// imgui_domain                                              int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT  // 1 == client, 2 == server
// imgui_enable                                              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // if imgui should display
// imgui_enable_input                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // if imgui should consume input
// imgui_ent_text_enable                                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // Show Entity Text in Window
// imgui_show_bullets                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // CSGO/Bullets
// imgui_show_cs2_worldmodel                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // CSGO/WorldModel
// imgui_show_grenades_window                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // CSGO/Show Grenades History
// imgui_temp_enable                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // if imgui should display temporarily
// in_button_double_press_window                             float32  0.22                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How short the time between presses needs to be for us to consider it a double-press
// in_spewbuttondelta                                        int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Spew button deltas, 0 = off, 1 = server, 2 = client, 3 = both
// in_spewbuttonhold                                         int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Spew button hold times, 0 = off, 1 = server, 2 = client, 3 = both
// in_spewent                                                int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Which entity should we spew input for? (Useful for debugging bot input)
// in_spewinput                                              int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Spew input, 0 = off, 1 = server, 2 = client, 3 = both
// inferno_batched_rays                                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// inferno_child_spawn_interval_multiplier                   float32  0.1                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Amount spawn interval increases for each child
// inferno_child_spawn_max_depth                             int32    4                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// inferno_ct_experiment                                     bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // enable ct incendiary experiment
// inferno_damage                                            float32  40                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Damage per second
// inferno_damage_ct                                         float32  40                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Damage per second from CT inferno
// inferno_damage_timer                                      float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How long between times for the inferno to deal damage.
// inferno_debug                                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// inferno_dlight_spacing                                    float32  7200                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Inferno dlights are at least this far apart
// inferno_dlights                                           float32  30                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Min FPS at which molotov dlights will be created
// inferno_fire                                              int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// inferno_flame_lifetime                                    float32  7                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Average lifetime of each flame in seconds
// inferno_flame_lifetime_incendiary                         float32  5.5                   FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Average lifetime of each flame in seconds (incgrenade)
// inferno_flame_spacing                                     float32  42                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Minimum distance between separate flame spawns
// inferno_forward_reduction_factor                          float32  0.9                   FCVAR_GAMEDLL|FCVAR_CHEAT
// inferno_friendly_fire_duration                            float32  6                     FCVAR_GAMEDLL|FCVAR_CHEAT  // For this long, FF is credited back to the thrower.
// inferno_initial_spawn_interval                            float32  0.02                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Time between spawning flames for first fire
// inferno_max_child_spawn_interval                          float32  0.5                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Largest time interval for child flame spawning
// inferno_max_flames                                        int32    16                    FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum number of flames that can be created
// inferno_max_range                                         float32  150                   FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum distance flames can spread from their initial ignition point
// inferno_max_range_ct                                      float32  110                   FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum distance flames can spread from their initial ignition point for an incendiary
// inferno_max_trace_per_tick                                int32    16                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// inferno_per_flame_spawn_duration                          float32  3                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Duration each new flame will attempt to spawn new flames
// inferno_smoke_volume_density                              float32  0.03                  FCVAR_GAMEDLL|FCVAR_CHEAT
// inferno_spawn_angle                                       float32  45                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Angular change from parent
// inferno_spread_speed_mult                                 float32  1                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Speed up the spreadrate of the Molotov until max number of nodes are created.  slowdown < 1 > Speedup
// inferno_spread_speed_mult_ct                              float32  10                    FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Speed up the spread rate of the Incendiary until max number of nodes are created. slowdown < 1 > Speedup
// inferno_surface_offset                                    float32  15                    FCVAR_GAMEDLL|FCVAR_CHEAT
// inferno_velocity_decay_factor                             float32  0.2                   FCVAR_GAMEDLL|FCVAR_CHEAT
// inferno_velocity_factor                                   float32  0.003                 FCVAR_GAMEDLL|FCVAR_CHEAT
// inferno_velocity_factor_ct                                float32  0.003                 FCVAR_GAMEDLL|FCVAR_CHEAT
// inferno_velocity_normal_factor                            float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// input_button_code_is_scan_code_scd                        bool     true                  FCVAR_ARCHIVE|FCVAR_PER_USER  // Bind keys based on keyboard position instead of key name
// input_downimpulsevalue                                    float32  0.7                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// input_filter_relative_analog_inputs                       bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// input_forceuser                                           int32    -1                    FCVAR_CHEAT  // Force user input to this split screen player.
// input_upimpulsevalue                                      float32  0.3                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// install_dlc_workshoptools_cvar                            string   0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // DLC Install Status
// instant_replay                                            bool     true                  FCVAR_DEVELOPMENTONLY  // Enable instant replay recording.
// instant_replay_history_limit                              int32    120                   FCVAR_DEVELOPMENTONLY  // Maximum amount of minutes to save history (0 is unlimited).
// ip                                                        string                         FCVAR_RELEASE  // Overrides IP for multihomed hosts
// iv_debugbone                                              string                         FCVAR_RELEASE  // Debug bone name for interpolation spew of CAnimationState.
// iv_parallel_latch                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// iv_parallel_restore                                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// iv_wrapped_parallel_latch                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_accel_filter                                          float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_accelmax                                              float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_accelscale                                            float32  0.6                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_advanced                                              bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_advaxisr                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_advaxisu                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_advaxisv                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_advaxisx                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_advaxisy                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_advaxisz                                              int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_autosprint                                            float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Automatically sprint when moving with an analog joystick
// joy_axisbutton_threshold                                  float32  0.3                   FCVAR_ARCHIVE  // Analog axis range before a button press is registered.
// joy_axisr_deadzone                                        float32  0.15                  FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisr_relative                                        bool     false                 FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisu_deadzone                                        float32  0.15                  FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisu_relative                                        bool     false                 FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisv_deadzone                                        float32  0.15                  FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisv_relative                                        bool     false                 FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisx_deadzone                                        float32  0.15                  FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisx_relative                                        bool     false                 FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisy_deadzone                                        float32  0.15                  FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisy_relative                                        bool     false                 FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisz_deadzone                                        float32  0.15                  FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_axisz_relative                                        bool     false                 FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_circle_correct_mode                                   int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_circle_correct_mode_vehicle                           int32    2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_display_input                                         bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_forward_sensitivity                                   float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_lowend                                                float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_lowmap                                                float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_movement_stick                                        bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Which stick controls movement (0 is left stick)
// joy_name                                                  string   joystick              FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_pegged                                                float32  0.75                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_pitch_sensitivity                                     float32  3                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_pitchsensitivity                                      float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_response_look                                         int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_response_move                                         int32    9                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_response_move_vehicle                                 int32    6                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_sensitive_step0                                       float32  0.1                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_sensitive_step1                                       float32  0.4                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_sensitive_step2                                       float32  0.9                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_side_sensitivity                                      float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_sidesensitivity                                       float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// joy_vehicle_turn_lowend                                   float32  0.7                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_vehicle_turn_lowmap                                   float32  0.4                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_virtual_peg                                           float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// joy_xcontroller_cfg_loaded                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // If 0, the 360controller.cfg file will be executed on startup & option changes.
// joy_yaw_sensitivity                                       float32  3                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joy_yawsensitivity                                        float32  -1                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// joystick                                                  bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // True if the joystick is enabled, false otherwise.
// jpeg_quality                                              int32    90                    FCVAR_DEVELOPMENTONLY  // Set jpeg screenshot quality. [1..100]
// key_bind_version                                          int32    5                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// keychain_animation_reactivity                             float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// keychain_preview_limit_step                               float32  0.125                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// keychain_reactivity                                       float32  0.05                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// keychain_wmul                                             float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// labelled_debug_helper_arc_segments                        int32    20                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// labelled_debug_helper_enabled                             bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// labelled_debug_helper_scale                               float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// labelled_debug_helper_show_position                       bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// labelled_debug_helper_show_text                           bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// labelled_debug_helper_skeleton_show_bone_names            bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// lb_allow_shadow_rotation                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Shadow Rotation
// lb_barnlight_shadow_use_precomputed_vis                   bool     true                  FCVAR_DEVELOPMENTONLY
// lb_barnlight_shadowmap_scale                              float32  0.75                  FCVAR_RELEASE  // Scale for computed barnlight shadowmap size
// lb_bin_slices                                             int32    8192                  FCVAR_DEVELOPMENTONLY
// lb_convert_to_barn_lights_falloff_match_point             float32  0.15                  FCVAR_DEVELOPMENTONLY
// lb_csm_cascade_size_override                              int32    1280                  FCVAR_DEVELOPMENTONLY  // Override width/height of individual cascades in the CSM
// lb_csm_cross_fade_override                                float32  0.1                   FCVAR_DEVELOPMENTONLY  // Override CSM cross fade amount
// lb_csm_distance_fade_override                             float32  0.05                  FCVAR_DEVELOPMENTONLY  // Override CSM distance fade
// lb_csm_draw_alpha_tested                                  bool     true                  FCVAR_DEVELOPMENTONLY
// lb_csm_draw_translucent                                   bool     true                  FCVAR_DEVELOPMENTONLY
// lb_csm_fov_override                                       float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// lb_csm_override_bulb_radius                               float32  -1                    FCVAR_DEVELOPMENTONLY  // Override bulb radius for CSM
// lb_csm_override_staticgeo_cascades                        bool     true                  FCVAR_DEVELOPMENTONLY  // Override Cascades that will render static objects with lb_csm_override_staticgeo_cascades_value
// lb_csm_override_staticgeo_cascades_animated_verts         bool     false                 FCVAR_DEVELOPMENTONLY  // If lb_csm_override_staticgeo_cascades, ensure only objects without animated verts, i.e. SCENEOBJECTFLAG_CAN_RENDER_INTO_SST flag will be excluded (as opposed to all static objects).
// lb_csm_override_staticgeo_cascades_value                  int32    2                     FCVAR_DEVELOPMENTONLY  // If lb_csm_override_staticgeo_cascades, override value used to determine which cascades render static objects
// lb_csm_receiver_plane_depth_bias                          float32  0.00001526            FCVAR_DEVELOPMENTONLY  // Shader depth bias applied to shadow receiver (Note this conflicts with renderstate depth bias, both now default to 0)
// lb_csm_receiver_plane_depth_bias_transmissive_backface    float32  0.00015               FCVAR_DEVELOPMENTONLY  // Depth bias applied to shadow receiver for transmissive backface geo (based on renderstate depthbias being 0)
// lb_cubemap_normalization_max                              float32  1                     FCVAR_DEVELOPMENTONLY
// lb_cubemap_normalization_roughness_begin                  float32  0.1                   FCVAR_DEVELOPMENTONLY
// lb_debug_light_bounds                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Debug Light Bounds
// lb_debug_shadow_atlas                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Debug Shadow Atlas
// lb_debug_shadowtile_atlas                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Debug ShadowTile Atlas
// lb_debug_silhouette                                       string                         FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Debug Silhouettes
// lb_debug_silhouette_sum                                   uint32   1                     FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // If we should draw normal silhouette or minkowski sum silhouette
// lb_debug_tiles                                            string                         FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Debug Tiles
// lb_debug_visualize_fog_shadowed_lights                    int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Debug Visualize Fog Shadowed Lights
// lb_debug_visualize_lights                                 int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Debug Visualize Lights
// lb_debug_visualize_shadowed_light_details                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// lb_debug_visualize_shadowed_lights                        int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Debug Visualize Shadowed Lights
// lb_dynamic_shadow_penumbra                                bool     true                  FCVAR_DEVELOPMENTONLY  // Adjust shadow penumbra based on light size
// lb_dynamic_shadow_resolution                              bool     true                  FCVAR_DEVELOPMENTONLY  // Dynamically adjust shadow resolution
// lb_dynamic_shadow_resolution_base                         float32  1536                  FCVAR_DEVELOPMENTONLY  // Base resolution for dynamic shadowmap sizing.  Shadowmap size of a screen sized light
// lb_dynamic_shadow_resolution_base_cmp_shadowmapsize       bool     false                 FCVAR_DEVELOPMENTONLY  // take min of lb_dynamic_shadow_resolution and barnlight shadowmapsize as base shadowmapsize
// lb_dynamic_shadow_resolution_delay                        float32  0.85                  FCVAR_DEVELOPMENTONLY  // Update delay for shadow size
// lb_dynamic_shadow_resolution_hysteresis                   float32  0.33                  FCVAR_DEVELOPMENTONLY  // Update hysteresis for shadow size
// lb_dynamic_shadow_resolution_quantization                 uint32   64                    FCVAR_DEVELOPMENTONLY  // Quantization of dynamically computed shadow size
// lb_enable_baked_shadows                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Enable Baked Shadows
// lb_enable_binning                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Enable Binning
// lb_enable_dynamic_lights                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Allows rendering dynamic lights
// lb_enable_envmaps                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Enable EnvMaps
// lb_enable_fog_mixed_shadows                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Enable Fog Mixed Shadows
// lb_enable_lights                                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Enable Lights
// lb_enable_shadow_casting                                  bool     true                  FCVAR_DEVELOPMENTONLY  // Allow stationary/dynamic lights to cast shadows.
// lb_enable_stationary_lights                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Allows rendering stationary/mixed lights
// lb_enable_sunlight                                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Enable Sunlight
// lb_low_quality_shader_fade_region_rescale                 float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // For envmaps in low quality shader mode, how much of the fade region to scale the envmap box by.
// lb_max_visible_barn_lights_override                       int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Override maximum visible barn lights
// lb_max_visible_envmaps_override                           int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Override maximum visible envmaps
// lb_mixed_shadows                                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Enable Mixed Shadows
// lb_override_barn_light_fade_sizes                         vector2  0.05 0.025            FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// lb_override_barn_light_fade_sizes_enable                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// lb_override_barn_light_shadow_fade_sizes                  vector2  0.1 0.05              FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// lb_precomputed_shadowmap_depth_bias                       float32  0.00035               FCVAR_DEVELOPMENTONLY
// lb_precomputed_shadowmap_enable                           bool     true                  FCVAR_DEVELOPMENTONLY
// lb_shadow_map_cull_empty_mixed                            bool     false                 FCVAR_CHEAT  // Don't render shadows for mixed shadowmaps with no dynamics objects in view
// lb_shadow_map_culling                                     bool     true                  FCVAR_CHEAT
// lb_shadow_texture_height_override                         int32    5248                  FCVAR_DEVELOPMENTONLY  // Override height of shadow atlas texture
// lb_shadow_texture_width_override                          int32    4352                  FCVAR_DEVELOPMENTONLY  // Override width of shadow atlas texture
// lb_ssss_importance_sample                                 bool     false                 FCVAR_DEVELOPMENTONLY
// lb_ssss_samples                                           int32    11                    FCVAR_DEVELOPMENTONLY  // Subsurface sample count
// lb_sun_csm_size_cull_threshold_texels                     float32  10                    FCVAR_DEVELOPMENTONLY  // Size, in texels, where we will cull an object in the shadowmap
// lb_tile_pixels                                            int32    8                     FCVAR_DEVELOPMENTONLY
// lb_time_sliced_shadow_map_reallocation_age                float32  0.7                   FCVAR_DEVELOPMENTONLY  // Age of cached allocation to be considered for re-allocation
// lb_time_sliced_shadow_map_reallocation_pct                float32  0.2                   FCVAR_DEVELOPMENTONLY  // Likelyhood we'll re-allocate a timesliced shadowmap (to try to improve packing)
// lb_time_sliced_shadow_map_rendering_enable                bool     true                  FCVAR_DEVELOPMENTONLY  // Allow time-sliced shadow buffer rendering when enabled via gameinfo.gi
// lb_timesliced_shadows_dynamic_size                        bool     true                  FCVAR_DEVELOPMENTONLY
// lb_use_ellipsoid_bounds                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// lb_use_illumination_silhouette                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/LightBinner/Use Illumination Bounds
// leaderboards_cache_duration                               int32    600                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// legacy_models_supported                                   bool     true                  FCVAR_DEVELOPMENTONLY  // Whether to support legacy (pre-modeldoc) models
// lightquery_debug_direct_lighting                          bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// lightquery_debug_indirect_lighting                        bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// lobby_default_privacy_bits2                               string   1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Lobby default permissions (0: private, 1: public)
// lobby_gamesearch_fake                                     int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// lobby_stats_fake                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// locator_topdown_style                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Topdown games set this to handle distance and offscreen location differently.
// lockMoveControllerRet                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// logaddress_token_secret                                   string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Set a secret string that will be hashed when using logaddress with explicit token hash.
// logic_entity_analyzer_debug                               bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// logic_npc_counter_debug                                   bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// lservercfgfile                                            string   listenserver.cfg      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// lua_assert_on_error                                       bool     false                 FCVAR_DEVELOPMENTONLY
// lua_is_currently_executing                                bool     false                 FCVAR_DONTRECORD|FCVAR_RELEASE
// lua_shipping_assert_on_error                              bool     false                 FCVAR_DEVELOPMENTONLY
// m_pitch                                                   float32  0.022                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Mouse pitch factor.
// m_yaw                                                     float32  0.022                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Mouse yaw factor.
// mapcyclefile                                              string   mapcycle.txt          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Name of the .txt file used to cycle the maps on multiplayer servers 
// mapoverview_allow_client_draw                             bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // Allow a client to draw on the map overview
// mapoverview_icon_scale                                    float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Sets the icon scale multiplier for the overview map. Valid values are 0.5 to 3.0.
// markup_volume_ref_cone_angle                              float32  135                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// mat_assert_on_error_shader_use                            bool     false                 FCVAR_DEVELOPMENTONLY
// mat_cache_and_skip_commandbuffers                         bool     true                  FCVAR_DEVELOPMENTONLY
// mat_cache_renderablepasses                                bool     true                  FCVAR_DEVELOPMENTONLY
// mat_colcorrection_disableentities                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Disable map color-correction entities
// mat_colcorrection_editor                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// mat_colcorrection_forceentitiesclientside                 bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Forces color correction entities to be updated on the client
// mat_colorcorrection                                       bool     true                  FCVAR_DEVELOPMENTONLY
// mat_depthbias_shadowmap                                   float32  0.0005                FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// mat_disable_normal_mapping                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// mat_execute_skipbuffers                                   bool     true                  FCVAR_DEVELOPMENTONLY
// mat_fullbright                                            int32    0                     FCVAR_CHEAT  // Debug rendering modes
// mat_hide_error_shader                                     bool     false                 FCVAR_DEVELOPMENTONLY
// mat_lpv_luxels                                            bool     false                 FCVAR_CHEAT
// mat_luxels                                                bool     false                 FCVAR_CHEAT
// mat_max_lighting_complexity                               float32  8                     FCVAR_CHEAT
// mat_overdraw                                              int32    0                     FCVAR_CHEAT  // Visualize overdraw
// mat_overdraw_color                                        vector3  0.075 0.15 0.3        FCVAR_CHEAT
// mat_shader_cache                                          bool     true                  FCVAR_DEVELOPMENTONLY
// mat_shading_complexity                                    bool     false                 FCVAR_CHEAT  // Visualize shading complexity
// mat_shading_complexity_color                              vector3  1 0.5 0.25            FCVAR_CHEAT
// mat_shading_complexity_max_instruction_count              float32  1024                  FCVAR_CHEAT
// mat_shading_complexity_max_register_count                 float32  128                   FCVAR_CHEAT
// mat_shadowmap_luxels                                      bool     false                 FCVAR_CHEAT
// mat_show_distance_field                                   int32    0                     FCVAR_CHEAT  // 0=Off, 1=Visualize trace from camera, 2=Visualize occlusion, 3=Visualize far field trace from camera
// mat_skip_static_const_eval                                bool     true                  FCVAR_DEVELOPMENTONLY
// mat_slopescaledepthbias_shadowmap                         float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// mat_tonemap_bloom_scale                                   float32  -1                    FCVAR_CHEAT
// mat_tonemap_bloom_start_value                             float32  -1                    FCVAR_CHEAT
// mat_tonemap_debug                                         int32    0                     FCVAR_DEVELOPMENTONLY
// mat_tonemap_force_accelerate_exposure_down                float32  -1                    FCVAR_CHEAT
// mat_tonemap_force_average_lum_min                         float32  -1                    FCVAR_CHEAT  // Override. Old default was 3.0
// mat_tonemap_force_log_lum_max                             float32  -1                    FCVAR_CHEAT
// mat_tonemap_force_log_lum_min                             float32  -1                    FCVAR_CHEAT
// mat_tonemap_force_max                                     float32  -1                    FCVAR_CHEAT
// mat_tonemap_force_min                                     float32  -1                    FCVAR_CHEAT
// mat_tonemap_force_percent_bright_pixels                   float32  -1                    FCVAR_CHEAT  // Override. Old value was 1.0
// mat_tonemap_force_percent_target                          float32  -1                    FCVAR_CHEAT  // Override. Old default was 45.
// mat_tonemap_force_rate                                    float32  -1                    FCVAR_CHEAT
// mat_tonemap_force_scale                                   float32  0                     FCVAR_CHEAT
// mat_tonemap_force_use_alpha                               int32    -1                    FCVAR_CHEAT
// mat_tonemap_uncap_exposure                                int32    0                     FCVAR_CHEAT
// mat_viewportscale                                         float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Scale down the main viewport (to reduce GPU impact on CPU profiling)
// mat_warn_bad_modes                                        bool     false                 FCVAR_DEVELOPMENTONLY
// mat_wireframe                                             int32    0                     FCVAR_CHEAT  // 0=Off, 1=Surface Wireframe, 2=Transparent Wireframe
// mem_level                                                 int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Memory Level - Default: High
// mem_test_each_frame                                       bool     false                 FCVAR_DEVELOPMENTONLY  // Run heap check at end of every frame
// mem_test_every_n_seconds                                  int32    0                     FCVAR_DEVELOPMENTONLY  // Run heap check at a specified interval
// mem_test_quiet                                            bool     false                 FCVAR_DEVELOPMENTONLY  // Don't print stats when memtesting
// mesh_calculate_curvature_smooth_invert                    bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// mesh_calculate_curvature_smooth_pass_count                int32    3                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// mesh_calculate_curvature_smooth_weight                    float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// mic_listen_while_nonfocused                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Enables the ability for the mic to remain open if the window loses focus such as when a caster tabs out to adjust settings
// mm_csgo_community_search_players_min                      int32    3                     FCVAR_ARCHIVE|FCVAR_RELEASE  // When performing CSGO community matchmaking look for servers with at least so many human players
// mm_debug_friend_rp                                        uint32   0                     FCVAR_DEVELOPMENTONLY
// mm_dedicated_allow                                        bool     true                  FCVAR_DEVELOPMENTONLY  // 1 = allow searches for dedicated servers
// mm_dedicated_fake                                         bool     false                 FCVAR_DEVELOPMENTONLY  // 1 = pretend like search is going, but abort after some time
// mm_dedicated_force_servers                                string                         FCVAR_RELEASE  // Comma delimited list of ip:port of servers used to search for dedicated servers instead of searching for public servers.
Use syntax `publicip1:port|privateip1:port,publicip2:port|privateip2:port` if your server is behind NAT.
If the server is behind NAT, y
// mm_dedicated_ip                                           string                         FCVAR_DEVELOPMENTONLY  // IP address of dedicated servers to consider available
// mm_dedicated_search_maxping                               int32    150                   FCVAR_ARCHIVE  // Longest preferred ping to dedicated servers for games
// mm_dedicated_search_maxresults                            int32    75                    FCVAR_DEVELOPMENTONLY
// mm_dedicated_timeout_request                              float32  20                    FCVAR_DEVELOPMENTONLY
// mm_dlcs_mask_extras                                       uint32   0                     FCVAR_DEVELOPMENTONLY
// mm_dlcs_mask_fake                                         string                         FCVAR_DEVELOPMENTONLY
// mm_events_listeners_validation                            bool     false                 FCVAR_DEVELOPMENTONLY
// mm_ignored_sessions_forget_pass                           int32    5                     FCVAR_DEVELOPMENTONLY
// mm_ignored_sessions_forget_time                           float32  600                   FCVAR_DEVELOPMENTONLY
// mm_player_search_count                                    int32    5                     FCVAR_DEVELOPMENTONLY
// mm_player_search_lan_ping_duration                        float32  3.5                   FCVAR_DEVELOPMENTONLY  // Duration of LAN discovery ping phase.
// mm_player_search_lan_ping_interval                        float32  0.5                   FCVAR_DEVELOPMENTONLY  // Interval between LAN discovery pings.
// mm_player_search_requests_limit                           int32    -1                    FCVAR_DEVELOPMENTONLY  // How many friend requests are displayed.
// mm_player_search_update_interval                          float32  10                    FCVAR_DEVELOPMENTONLY  // Interval between players searches.
// mm_session_search_num_results                             int32    10                    FCVAR_DEVELOPMENTONLY
// mm_session_search_qos_timeout                             float32  15                    FCVAR_RELEASE
// mm_session_sys_connect_timeout                            float32  8                     FCVAR_DEVELOPMENTONLY
// mm_session_sys_delay_create                               float32  0                     FCVAR_DEVELOPMENTONLY
// mm_session_sys_delay_create_host                          float32  1.2                   FCVAR_DEVELOPMENTONLY
// mm_session_sys_kick_ban_duration                          float32  180                   FCVAR_RELEASE
// mm_session_sys_pkey                                       string                         FCVAR_RELEASE
// mm_session_sys_ranking_timeout                            float32  12                    FCVAR_DEVELOPMENTONLY
// mm_session_sys_slots_guaranteed                           int32    10                    FCVAR_DEVELOPMENTONLY
// mm_session_team_res_timeout                               float32  30                    FCVAR_DEVELOPMENTONLY
// mm_session_voice_loading                                  bool     false                 FCVAR_DEVELOPMENTONLY
// mm_sv_load_test                                           bool     false                 FCVAR_DEVELOPMENTONLY
// mm_teamsearch_errortime                                   float32  3                     FCVAR_DEVELOPMENTONLY  // Time team search is in error state until it self-cancels
// mm_teamsearch_nostart                                     bool     false                 FCVAR_DEVELOPMENTONLY  // Team search will fake cancel before searching for server
// mm_title_debug_version                                    int32    0                     FCVAR_DEVELOPMENTONLY  // This matchmaking version will override .res file version for isolating matchmaking
// mm_tu_string                                              string   00000000              FCVAR_DEVELOPMENTONLY
// mm_use_p2p_for_listen_server                              bool     true                  FCVAR_DEVELOPMENTONLY
// mobile_fps_increase_during_charging                       bool     false                 FCVAR_ARCHIVE  // MOBILE_FPS_CONTROL: If true we increase framerate limit while charging
// mobile_fps_increase_during_hfr_animations                 bool     true                  FCVAR_DEVELOPMENTONLY  // MOBILE_FPS_CONTROL: If true we increase framerate limit during HFR-tagged animations and transitions.
// mobile_fps_increase_during_touch                          bool     true                  FCVAR_ARCHIVE  // MOBILE_FPS_CONTROL: If true we increase framerate limit during touch
// mobile_fps_limit                                          float32  30                    FCVAR_ARCHIVE  // MOBILE_FPS_CONTROL: Mobile FPS limit - 15, 30, 60
// model_default_preview_sequence_name                       string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED
// molotov_throw_detonate_time                               float32  2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// molotov_usethrow_direction                                bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// motdfile                                                  string   motd.txt              FCVAR_GAMEDLL|FCVAR_RELEASE  // The MOTD file to load.
// mouse_disableinput                                        bool     false                 FCVAR_DEVELOPMENTONLY  // Set to disable mouse input
// mouse_inverty                                             bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO
// movement_stats_debug_draw                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// movement_stats_force_calculate                            bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// mp_afterroundmoney                                        int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // amount of money awared to every player after each round
// mp_allowspectators                                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // toggles whether the server allows spectator mode or not
// mp_anyone_can_pickup_c4                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If set, everyone can pick up the c4, not just Ts.
// mp_autokick                                               bool     true                  FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Kick idle/team-killing/team-damaging players
// mp_autoteambalance                                        bool     true                  FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_RELEASE
// mp_backup_restore_load_autopause                          bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Whether to automatically pause the match after restoring round data from backup
// mp_backup_round_auto                                      bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // If enabled will keep in-memory backups to handle reconnecting players even if the backup files aren't written to disk
// mp_backup_round_file                                      string   backup                FCVAR_GAMEDLL|FCVAR_RELEASE  // If set then server will save all played rounds information to files filename_date_time_team1_team2_mapname_roundnum_score1_score2.txt
// mp_backup_round_file_last                                 string   backup_round11.txt    FCVAR_GAMEDLL|FCVAR_RELEASE  // Every time a backup file is written the value of this convar gets updated to hold the name of the backup file.
// mp_backup_round_file_pattern                              string   %prefix%_round%roun…  FCVAR_GAMEDLL|FCVAR_RELEASE  // If set then server will save all played rounds information to files named by this pattern, e.g.'%prefix%_%date%_%time%_%team1%_%team2%_%map%_round%round%_score_%score1%_%score2%.txt'
// mp_bot_ai_bt                                              string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Use the specified behavior tree file to drive the bot behavior.
// mp_buy_allow_grenades                                     bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether players can purchase grenades from the buy menu or not.
// mp_buy_allow_guns                                         int32    255                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether players can purchase guns: pistols (1), SMGs (2), rifles (4), shotguns (8), sniper rifles (16), heavy MGs (32).
// mp_buy_anywhere                                           int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // When set, players can buy anywhere, not only in buyzones. 0 = default. 1 = both teams. 2 = Terrorists. 3 = Counter-Terrorists.
// mp_buy_during_immunity                                    int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // When set, players can buy when immune, ignoring buytime. 0 = default. 1 = both teams. 2 = Terrorists. 3 = Counter-Terrorists.
// mp_buytime                                                float32  45                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How many seconds after round start players can buy items for.
// mp_c4_cannot_be_defused                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If set, the planted c4 cannot be defused.
// mp_c4timer                                                int32    40                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // how long from when the C4 is armed until it blows
// mp_chattime                                               int32    10                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // amount of time players can chat after the game is over
// mp_competitive_endofmatch_extra_time                      float32  15                    FCVAR_GAMEDLL|FCVAR_RELEASE  // After a competitive match finishes rematch voting extra time is given for rankings.
// mp_consecutive_loss_aversion                              int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How loss streak is affected with round win: 0 = win fully resets loss bonus, 1 = first win steps down loss bonus, 2 = first win holds loss bonus and step down starting with second win
// mp_consecutive_loss_max                                   int32    4                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// mp_coopmission_bot_difficulty_offset                      int32    0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The difficulty offset modifier for bots during coop missions.
// mp_ct_default_grenades                                    string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The default grenades that the CTs will spawn with.	 To give multiple grenades, separate each weapon class with a space like this: 'weapon_molotov weapon_hegrenade'
// mp_ct_default_melee                                       string   weapon_knife          FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The default melee weapon that the CTs will spawn with.	 Even if this is blank, a knife will be given.	To give a taser, it should look like this: 'weapon_knife weapon_taser'.	 Remember to set mp_weapons_allow_zeus to 1 if you want to give a taser!
// mp_ct_default_primary                                     string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The default primary (rifle) weapon that the CTs will spawn with
// mp_ct_default_secondary                                   string   weapon_hkp2000        FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The default secondary (pistol) weapon that the CTs will spawn with
// mp_damage_headshot_only                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether non-headshot hits do any damage.
// mp_damage_scale_ct_body                                   float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Scales the damage a CT player takes by this much when they take damage in the body. (1 == 100%, 0.5 == 50%)
// mp_damage_scale_ct_head                                   float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Scales the damage a CT player takes by this much when they take damage in the head (1 == 100%, 0.5 == 50%).  REMEMBER! headshots do 4x the damage of the body before this scaler is applied.
// mp_damage_scale_t_body                                    float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Scales the damage a T player takes by this much when they take damage in the body. (1 == 100%, 0.5 == 50%)
// mp_damage_scale_t_head                                    float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Scales the damage a T player takes by this much when they take damage in the head (1 == 100%, 0.5 == 50%).	 REMEMBER! headshots do 4x the damage of the body before this scaler is applied.
// mp_damage_vampiric_amount                                 float32  0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If Set to non-0, will determine the fraction of damage dealt that will be given to attacker.
// mp_death_drop_c4                                          bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether c4 is droppable
// mp_death_drop_defuser                                     bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Drop defuser on player death
// mp_death_drop_grenade                                     int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Which grenade to drop on player death: 0=none, 1=best, 2=current or best, 3=all grenades
// mp_death_drop_gun                                         int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Which gun to drop on player death: 0=none, 1=best, 2=current or best
// mp_death_drop_healthshot                                  bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Drop healthshot on player death
// mp_death_drop_taser                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Drop taser on player death
// mp_deathcam_skippable                                     bool     true                  FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether a player can early-out of the deathcam.
// mp_default_team_winner_no_objective                       int32    -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If the map doesn't define an objective (bomb, hostage, etc), the value of this convar will declare the winner when the time runs out in the round.
// mp_defuser_allocation                                     int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How to allocate defusers to CTs at start or round: 0=none, 1=random, 2=everyone
// mp_disconnect_kills_bots                                  bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // When a bot disconnects, kill them first.  Requires mp_disconnect_kills_players.
// mp_disconnect_kills_players                               bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // When a player disconnects, kill them first (triggering item drops, stats, etc.)
// mp_display_kill_assists                                   bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether to display and score player assists
// mp_dm_bonus_length_max                                    float32  30                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum time the bonus time will last (in seconds)
// mp_dm_bonus_length_min                                    float32  30                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Minimum time the bonus time will last (in seconds)
// mp_dm_bonus_percent                                       int32    50                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Percent of points additionally awarded when someone gets a kill with the bonus weapon during the bonus period.
// mp_dm_bonusweapon_dogtags                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Additional dogtags to drop when making a kill with the bonus weapon
// mp_dm_dogtag_score                                        int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Points to award for picking up a dogtag in deathmatch.
// mp_dm_healthshot_killcount                                int32    3                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Grant healthshots in deathmatch after n kills
// mp_dm_kill_base_score                                     int32    8                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of base points to award for a kill in deathmatch.  Cheaper weapons award 1 or 2 additional points.
// mp_dm_taser_bonus_streak_max                              int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum times to multiply the score for getting a streak of taser kills in a single life.
// mp_dm_teammode                                            int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // In deathmatch, enables team DM visuals & scoring (0: personal, 1: team mode, 2: use team contribution score)
// mp_dm_teammode_bonus_score                                int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Team deathmatch victory points to award for kill with bonus weapon
// mp_dm_teammode_dogtag_score                               int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Team deathmatch victory points to award for collecting enemy dogtags
// mp_dm_teammode_kill_score                                 int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Team deathmatch victory points to award for enemy kill
// mp_dm_time_between_bonus_max                              float32  40                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum time a bonus time will start after the round start or after the last bonus (in seconds)
// mp_dm_time_between_bonus_min                              float32  30                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Minimum time a bonus time will start after the round start or after the last bonus (in seconds)
// mp_dogtag_despawn_on_killer_death                         bool     true                  FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether dogtags should despawn when their killer dies
// mp_dogtag_despawn_time                                    float32  120                   FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How many seconds dogtags should stay around before despawning automatically (0 = infinite)
// mp_dogtag_pickup_rule                                     int32    0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Who is eligible to pick up a dogtag (0 = killer only, 1 = killer's team, 2 = victim's team, 3 = killer & victim's team, 4 = anyone)
// mp_drop_grenade_enable                                    bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Allows players to drop grenades.
// mp_drop_knife_enable                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Allows players to drop knives.
// mp_economy_reset_rounds                                   int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Reset all player money every N rounds (0 for never)
// mp_endmatch_votenextleveltime                             float32  20                    FCVAR_GAMEDLL|FCVAR_RELEASE  // If mp_endmatch_votenextmap is set, players have this much time to vote on the next map at match end.
// mp_endmatch_votenextmap                                   bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether or not players vote for the next map at the end of the match when the final scoreboard comes up
// mp_endmatch_votenextmap_keepcurrent                       bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If set, keeps the current map in the list of voting options.  If not set, the current map will not appear in the list of voting options.
// mp_endmatch_votenextmap_wargames_modes                    string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Modes available for endmatch voting during War Games. Separate names with spaces.
// mp_endmatch_votenextmap_wargames_nummaps                  int32    3                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Maximum number of maps to include in endmatch voting during War Games
// mp_endmatch_votenextmap_wargames_nummodes                 int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Maximum number of other War Games to include in endmatch voting during War Games
// mp_endwarmup_player_count                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of players required to be connected to end warmup early. 0 to require maximum players for mode.
// mp_equipment_reset_rounds                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Reset all player equipment every N rounds (0 for never)
// mp_fadetoblack                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED  // fade a player's screen to black when he dies
// mp_flinch_punch_scale                                     float32  3                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Scalar for first person view punch when getting hit.
// mp_footsteps_serverside                                   bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Makes the server always play footstep sounds. Clients never calculate footstep sounds locally, instead relying on the server.
// mp_force_pick_time                                        float32  15                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The amount of time a player has on the team screen to make a selection before being auto-teamed
// mp_forcecamera                                            int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Restricts spectator modes for dead players
// mp_forcerespawn                                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_NOTIFY
// mp_fraglimit                                              int32    0                     FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_RELEASE
// mp_free_armor                                             int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether kevlar (1+) and/or helmet (2+) are given automatically.
// mp_freezetime                                             int32    5                     FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // how many seconds to keep players frozen when the round starts
// mp_friendlyfire                                           bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Allows team members to injure other members of their team
// mp_give_player_c4                                         bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether this map should spawn a c4 bomb for a player or not.
// mp_global_damage_per_second                               float32  0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If above 0, deal non-lethal damage to players over time.
// mp_guardian_bomb_plant_custom_x_mark_location             string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // x,y,z to display an X for the bomb plant in guardian missions with custom bomb plant boundaries.
// mp_guardian_target_site                                   int32    -1                    FCVAR_GAMEDLL|FCVAR_RELEASE  // If set to the index of a bombsite, will cause random spawns to be only created near that site.
// mp_halftime                                               bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether the match switches sides in a halftime event.
// mp_halftime_duration                                      float32  15                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Target number of seconds that halftime lasts; shortened if team intros are active
// mp_halftime_pausematch                                    int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Set to 1 to pause match after halftime countdown elapses. Match must be resumed by vote or admin.
// mp_halftime_pausetimer                                    int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Set to 1 to stay in halftime indefinitely. Set to 0 to resume the timer.
// mp_hostages_max                                           int32    2                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum number of hostages to spawn.
// mp_hostages_rescuetime                                    float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Additional time added to round time if a hostage is reached by a CT.
// mp_hostages_rescuetowin                                   int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // 0 == all alive, any other number is the number the CT's need to rescue to win the round.
// mp_hostages_run_speed_modifier                            float32  1                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Default is 1.0, slow down hostages by setting this to < 1.0.
// mp_hostages_spawn_farthest                                bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When enabled will consistently force the farthest hostages to spawn.
// mp_hostages_spawn_force_positions                         string                         FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Comma separated list of zero based indices to force spawn positions, e.g. '0,2' or '1,6'
// mp_hostages_spawn_force_positions_xyz                     string                         FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Comma separated list of xyz locations to force spawn positions, e.g. 'x1 y1 z1,x2 y2 z2'
// mp_hostages_spawn_same_every_round                        bool     true                  FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // 0 = spawn hostages randomly every round, 1 = same spawns for entire match.
// mp_hostages_takedamage                                    bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether or not hostages can be hurt.
// mp_humanteam                                              string   any                   FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Restricts human players to a single team {any, CT, T}
// mp_ignore_round_win_conditions                            bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Ignore conditions which would end the current round
// mp_items_prohibited                                       string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Set this convar to a comma-delimited list of definition indices of weapons that should be prohibited from use.
// mp_join_grace_time                                        float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of seconds after round start to allow a player to join a game
// mp_limitteams                                             int32    2                     FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Max # of players 1 team can have over another (0 disables check)
// mp_logdetail                                              int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Logs attacks.  Values are: 0=off, 1=enemy, 2=teammate, 3=both)
// mp_logdetail_items                                        bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Logs a line any time a player acquires or loses an item.
// mp_logmoney                                               bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Enables money logging.  Values are: 0=off, 1=on
// mp_match_can_clinch                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Can a team clinch and end the match by being so far ahead that the other team has no way to catching up?
// mp_match_end_changelevel                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // At the end of the match, perform a changelevel even if next map is the same
// mp_match_end_restart                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // At the end of the match, perform a restart instead of loading a new map
// mp_match_restart_delay                                    int32    25                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Time (in seconds) until a match restarts.
// mp_max_armor                                              int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines the highest level of armor allowed to be purchased. (0) None, (1) Kevlar, (2) Helmet
// mp_maxmoney                                               int32    10000                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // maximum amount of money allowed in a player's account
// mp_maxrounds                                              int32    15                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // max number of rounds to play before server changes maps
// mp_min_halftime_duration                                  float32  8.5                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Minimum number of seconds that halftime lasts even if team intros are active
// mp_only_cts_rescue_hostages                               bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// mp_overtime_enable                                        bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If a match ends in a tie, use overtime rules to determine winner
// mp_overtime_halftime_pausetimer                           int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If set to 1 will set mp_halftime_pausetimer to 1 before every half of overtime. Set mp_halftime_pausetimer to 0 to resume the timer.
// mp_overtime_limit                                         int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When overtime is enabled, only so many overtimes can be played
// mp_overtime_maxrounds                                     int32    6                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When overtime is enabled play additional rounds to determine winner
// mp_overtime_startmoney                                    int32    10000                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Money assigned to all players at start of every overtime half
// mp_plant_c4_anywhere                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// mp_playercashawards                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Players can earn money by performing in-game actions
// mp_playerid                                               int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Controls what information player see in the status bar: 0 all names; 1 team names; 2 no names
// mp_playerid_delay                                         float32  0.4                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of seconds to delay showing information in the status bar
// mp_playerid_hold                                          float32  0.1                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of seconds to keep showing old information in the status bar
// mp_promoted_item_enabled                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Allow the purchasing of the promoted item.
// mp_randomspawn                                            int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether players are to spawn. 0 = default; 1 = both teams; 2 = Terrorists; 3 = CTs.
// mp_randomspawn_dist                                       int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If using mp_randomspawn, determines whether to test distance when selecting this spot.
// mp_randomspawn_los                                        bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If using mp_randomspawn, determines whether to test Line of Sight when spawning.
// mp_require_gun_use_to_acquire                             bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Whether guns must be +used to acquire or default is touch-to-pickup
// mp_respawn_immunitytime                                   float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How many seconds after respawn immunity lasts. Set to negative value to disable warmup immunity.
// mp_respawn_on_death_ct                                    bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When set to 1, counter-terrorists will respawn after dying.
// mp_respawn_on_death_t                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When set to 1, terrorists will respawn after dying.
// mp_respawnwavetime_ct                                     float32  10                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Time between respawn waves for CTs.
// mp_respawnwavetime_t                                      float32  10                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Time between respawn waves for Terrorists.
// mp_restartgame                                            int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // If non-zero, game will restart in the specified number of seconds
// mp_retake_ct_count                                        int32    4                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of CT's when playing retakes.
// mp_retake_ct_loadout_bonus_card                           string   #GameUI_Retake_Card…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // CT bonus card for full buy round when playing bomb site retake.
// mp_retake_ct_loadout_bonus_card_availability              string   1,2                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // CT bonus card availability pattern for full buy round when playing bomb site retake.
// mp_retake_ct_loadout_default_pistol_round                 string   1|3;#GameUI_Retake_…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // CT Loadouts for default pistol round when playing bomb site retake.
// mp_retake_ct_loadout_enemy_card                           string   #GameUI_Retake_Card…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // CT enemy card for full buy round when playing bomb site retake.
// mp_retake_ct_loadout_full_buy_round                       string   4|2;#GameUI_Retake_…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // CT Loadouts for full buy round when playing bomb site retake.
// mp_retake_ct_loadout_light_buy_round                      string   3|2;#GameUI_Retake_…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // CT Loadouts for force buy round when playing bomb site retake.
// mp_retake_ct_loadout_upgraded_pistol_round                string   2|2;#GameUI_Retake_…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // CT Loadouts for upgraded pistol round when playing bomb site retake.
// mp_retake_max_consecutive_rounds_same_target_site         int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Limit the number of consecutive rounds targeting the same site.
// mp_retake_t_count                                         int32    3                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of terrorists when playing retakes.
// mp_retake_t_loadout_bonus_card                            string   #GameUI_Retake_Card…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // T bonus card for full buy round when playing bomb site retake.
// mp_retake_t_loadout_bonus_card_availability               string   1,1,2                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // T bonus card availability pattern for full buy round when playing bomb site retake.
// mp_retake_t_loadout_default_pistol_round                  string   0|3;#GameUI_Retake_…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // T Loadouts for default pistol round when playing bomb site retake.
// mp_retake_t_loadout_enemy_card                            string   #GameUI_Retake_Card…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // T enemy card for full buy round when playing bomb site retake.
// mp_retake_t_loadout_full_buy_round                        string   0|2;#GameUI_Retake_…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // T Loadouts for full buy round when playing bomb site retake.
// mp_retake_t_loadout_light_buy_round                       string   0|2;#GameUI_Retake_…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // T Loadouts for force buy round when playing bomb site retake.
// mp_retake_t_loadout_upgraded_pistol_round                 string   0|2;#GameUI_Retake_…  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // T Loadouts for upgraded pistol round when playing bomb site retake.
// mp_round_restart_delay                                    float32  10                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of seconds to delay before restarting a round after a win
// mp_roundtime                                              float32  3                     FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // How many minutes each round takes.
// mp_roundtime_defuse                                       float32  2.25                  FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // How many minutes each round of Bomb Defuse takes. If 0 then use mp_roundtime instead.
// mp_roundtime_hostage                                      float32  2                     FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // How many minutes each round of Hostage Rescue takes. If 0 then use mp_roundtime instead.
// mp_shoot_dropped_grenades                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Dropped grenades detonate when shot.
// mp_shorthanded_cash_bonus_ignore_kicked                   bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether kicked players are included in the assessment for short-handedness
// mp_shorthanded_cash_bonus_round_delay                     int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // number of previous rounds that a team needs to have been shorthanded before they are eligible for the short-handed bonus
// mp_solid_enemies                                          int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How solid are enemies: 0 = transparent; 1 = fully solid
// mp_solid_teammates                                        int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How solid are teammates: 0 = transparent; 1 = fully solid; 2 = can stand on top of heads
// mp_spawnprotectiontime                                    int32    5                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Kick players who team-kill within this many seconds of a round restart.
// mp_spectators_max                                         int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How many spectators are allowed in a match.
// mp_starting_losses                                        int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines what the initial loss streak is.
// mp_startmoney                                             int32    1000                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // amount of money each player gets when they reset
// mp_suicide_penalty                                        bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Punish players for suicides
// mp_t_default_grenades                                     string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The default grenades that the Ts will spawn with.	To give multiple grenades, separate each weapon class with a space like this: 'weapon_molotov weapon_hegrenade'
// mp_t_default_melee                                        string   weapon_knife          FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The default melee weapon that the Ts will spawn with
// mp_t_default_primary                                      string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The default primary (rifle) weapon that the Ts will spawn with
// mp_t_default_secondary                                    string   weapon_glock          FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The default secondary (pistol) weapon that the Ts will spawn with
// mp_tagging_scale                                          float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Scalar for player tagging modifier when hit. Lower values for greater tagging.
// mp_taser_recharge_time                                    float32  30                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines recharge time for taser. -1 = disabled.
// mp_td_dmgtokick                                           int32    300                   FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The damage threshhold players have to exceed in a match to get kicked.
// mp_td_dmgtowarn                                           int32    200                   FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The damage threshhold players have to exceed in a match to get warned that they are about to be kicked.
// mp_td_spawndmgthreshold                                   int32    50                    FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The damage threshold players have to exceed at the start of the round to be warned/kick.
// mp_team_intro_time                                        float32  6.5                   FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // How many seconds for team intro
// mp_team_timeout_max                                       int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of timeouts each team gets per match.
// mp_team_timeout_ot_add_each                               int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of timeouts to add for each team when match goes to 2nd and each next overtime.
// mp_team_timeout_ot_add_once                               int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Number of timeouts to add for each team when regulation time ends and match goes to overtime.
// mp_team_timeout_ot_max                                    int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Max number of timeouts each team can have per OT after all OT timeouts got added.
// mp_team_timeout_time                                      int32    60                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Duration of each timeout.
// mp_teamcashawards                                         bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Teams can earn money by performing in-game actions
// mp_teamflag_1                                             string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Enter a country's alpha 2 code to show that flag next to team 1's name in the spectator scoreboard.
// mp_teamflag_2                                             string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Enter a country's alpha 2 code to show that flag next to team 2's name in the spectator scoreboard.
// mp_teamlogo_1                                             string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Enter a team's shorthand image name to display their logo. Images can be found here: 'resource/flash/econ/tournaments/teams'
// mp_teamlogo_2                                             string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Enter a team's shorthand image name to display their logo. Images can be found here: 'resource/flash/econ/tournaments/teams'
// mp_teammatchstat_1                                        string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // A non-empty string sets first team's match stat.
// mp_teammatchstat_2                                        string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // A non-empty string sets second team's match stat.
// mp_teammatchstat_cycletime                                float32  45                    FCVAR_GAMEDLL|FCVAR_RELEASE  // Cycle match stats after so many seconds
// mp_teammatchstat_holdtime                                 float32  5                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Decide on a match stat and hold it additionally for at least so many seconds
// mp_teammatchstat_txt                                      string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // A non-empty string sets the match stat description, e.g. 'Match 2 of 3'.
// mp_teammates_are_enemies                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // When set, your teammates act as enemies and all players are valid targets.
// mp_teamname_1                                             string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // A non-empty string overrides the first team's name.
// mp_teamname_2                                             string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // A non-empty string overrides the second team's name.
// mp_teamplay                                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_NOTIFY
// mp_teamprediction_pct                                     int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // A value between 1 and 99 will show predictions in favor of CT team.
// mp_teamprediction_txt                                     string   #SFUIHUD_Spectate_P…  FCVAR_GAMEDLL|FCVAR_RELEASE  // A value between 1 and 99 will set predictions in favor of first team.
// mp_teamscore_1                                            int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // A non-empty string for best-of-N maps won by the first team.
// mp_teamscore_2                                            int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // A non-empty string for best-of-N maps won by the second team.
// mp_teamscore_max                                          int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // How many maps to win the series (bo3 max=2; bo5 max=3; bo7 max=4)
// mp_technical_timeout_duration_s                           int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How many seconds is a full technical timeout?
// mp_technical_timeout_per_team                             int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How many technical timeouts are there per team?
// mp_timelimit                                              float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // game time per map in minutes
// mp_tkpunish                                               int32    0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Will TK'ers and team damagers be punished in the next round?  {0=no,  1=yes}
// mp_tournament                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED
// mp_tournament_whitelist                                   string   item_whitelist.txt    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Specifies the item whitelist file to use.
// mp_use_respawn_waves                                      int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When set to 1, and that player's team is set to respawn, they will respawn in waves. If set to 2, teams will respawn when the whole team is dead.
// mp_verbose_changelevel_spew                               int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// mp_warmup_items_drop_policy                               int32    247                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Which items can drop during warmup (bitfield, 1=gun, 2=c4, 4=nade, 8=defuser, 16=taser, 32=healthshot)
// mp_warmup_items_nocost                                    bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether weapons are free to buy during warmup.
// mp_warmup_items_nocount_policy                            int32    42                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Which items are unlimited during warmup (bitfield, 1=gun, 2=c4, 4=nade, 8=defuser/kevlar, 16=taser, 32=healthshot)
// mp_warmup_jointeam_cooldown                               float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// mp_warmup_offline_enabled                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether or not to do a warmup period at the start of a match in an offline (bot) match.
// mp_warmup_online_enabled                                  bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether or not to do a warmup period at the start of an online match.
// mp_warmup_pausetimer                                      int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Set to 1 to stay in warmup indefinitely. Set to 0 to resume the timer.
// mp_warmuptime                                             float32  90                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // How long the warmup period lasts. Changing this value resets warmup.
// mp_warmuptime_all_players_connected                       float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Warmup time to use when all players have connected. 0 to disable.
// mp_warmuptime_match_cancelled                             float32  5                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Warmup time to use when the match will be cancelled (eg. due to a live VAC ban).
// mp_weapon_next_owner_touch_time                           float32  1.3                   FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_RELEASE
// mp_weapon_prev_owner_touch_time                           float32  1.5                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// mp_weapon_self_inflict_amount                             float32  0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If Set to non-0, will hurt the attacker by the specified fraction of max damage if they miss.
// mp_weapons_allow_heavy                                    int32    -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines which team, if any, can purchase Heavy guns. -1 = any; 0 = non; 2 = Ts; 3 = CTs.
// mp_weapons_allow_map_placed                               bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If this convar is set, when a match starts, the game will not delete weapons placed in the map.
// mp_weapons_allow_pistols                                  int32    -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines which team, if any, can purchase Pistols. -1 = any; 0 = non; 2 = Ts; 3 = CTs.
// mp_weapons_allow_rifles                                   int32    -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines which team, if any, can purchase Rifles. -1 = any; 0 = non; 2 = Ts; 3 = CTs.
// mp_weapons_allow_smgs                                     int32    -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines which team, if any, can purchase SMGs. -1 = any; 0 = non; 2 = Ts; 3 = CTs.
// mp_weapons_allow_typecount                                int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines how many purchases of each weapon type allowed per player per round (0 to disallow purchasing, -1 to have no limit).
// mp_weapons_allow_zeus                                     int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines how many Zeus purchases a player can make per round (0 to disallow, -1 to have no limit).
// mp_weapons_max_gun_purchases_per_weapon_per_match         int32    -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Max number of times a player may purchase any weapon per match
// mp_weaponstay                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_NOTIFY
// mp_win_panel_display_time                                 float32  3                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The amount of time to show the win panel between matches / halfs
// mp_winlimit                                               int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED  // Max score one team can reach before server changes maps
// multigpu_skip_semaphores                                  bool     false                 FCVAR_DEVELOPMENTONLY
// multigpu_skip_transfers                                   bool     false                 FCVAR_DEVELOPMENTONLY
// muzzle_flash_debug                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// name                                                      string   steven hawk           FCVAR_ARCHIVE|FCVAR_PER_USER
// nav_approach_points_area_size_threshold                   float32  200                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Ignore nav areas with at least one side smaller than this amount during approach point calculation.
// nav_attribute_obstacle_draw                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_attribute_obstacle_draw_attribute                     string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_attribute_obstacle_draw_elements                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_bfs_debug                                             int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_corner_adjust_adjacent                                float32  18                    FCVAR_CHEAT  // radius used to raise/lower corners in nearby areas when raising/lowering corners.
// nav_create_indirect_connection_set_from                   vector3  0 0 0                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Set the 'from' location of an indirect connection.
// nav_create_indirect_connection_set_to                     vector3  0 0 0                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Set the 'to' location of an indirect connection.
// nav_curve_alt                                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_curve_iter                                            int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_curve_lock                                            int32    -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_curve_max_step                                        float32  10                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_curve_set                                             int32    -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_curve_step                                            float32  0.02                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_debug_blocked                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_drag_selection_volume_zmax_offset                     int32    32                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // The offset of the nav drag volume top from center
// nav_drag_selection_volume_zmin_offset                     int32    32                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // The offset of the nav drag volume bottom from center
// nav_draw_area_connections                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_filled                                      bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_gravity                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_ground                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_hull_support                                bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_ids                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_inset_margin                                float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_normal                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_should_be_destroyed                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_split_by_obstacle_mgr                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_area_ztest                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_attribute_dynamic                                string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Draw all nav areas with this dynamic attribute
// nav_draw_attribute_game                                   string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Draw all nav areas with this game attribute
// nav_draw_attribute_space                                  string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Draw only nav blocks with this attribute
// nav_draw_blocked                                          bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_blocked_connections                              bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_boundary_areas                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_connected_area_radius                            float32  1000                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_dangerareas                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_dormant_movable_meshes                           bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw dormant movable meshes.
// nav_draw_externally_created                               bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_flow_map                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_hidingspots                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_indirect_connections                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_jump_links                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_limit                                            int32    300                   FCVAR_GAMEDLL|FCVAR_CHEAT  // The maximum number of areas to draw in edit mode
// nav_draw_link_alignment                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_links                                            bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_markup                                           bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_mesh                                             bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_draw_mesh_grid                                        bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw the mesh's spatial grid structure around the edit cursor position.
// nav_draw_mesh_offset                                      float32  1                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Vertical offset for drawing the mesh (useful for flat planes where the mesh is often a fixed offset from the physical ground
// nav_draw_space_boundary                                   int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Draw the boundaries of the 3d nav space. 1 = draw flying space, 2 = draw swimming space
// nav_draw_space_cells                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_draw_space_fly                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_draw_space_neighbors                                  int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_draw_space_portals                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_draw_space_radius                                     float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_draw_space_swim                                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_draw_space_transitions                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_edit                                                  int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Set to one to interactively edit the Navigation Mesh. Set to zero to leave edit mode.
// nav_edit_draw_navlinks                                    bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_edit_use_camera                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_edit_validate                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Validate navmesh structures.
// nav_find_occluded_node_nozup_use_raycast                  bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_flow_map_enabled                                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_gen_add_jumps                                         bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_agent_radius_buffer                               float32  0.75                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Buffer to add to agent radius before passing to nav gen
// nav_gen_clip_polys_to_clearance                           bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_clip_polys_to_clearance_debug                     bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_connect_allow_multiple                            bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_connect_angle                                     float32  0.75                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_connect_angle_ignore_z                            bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_connect_dist_a                                    float32  1                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_connect_dist_b                                    float32  1.5                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_connect_dist_z_mult                               float32  0.5                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_connect_overlap                                   float32  0.5                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_degen_limit                                       float32  0.001                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_false                                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Always false
// nav_gen_island_removal                                    bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_island_removal_all_hulls                          bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_join_nonzup                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_jump_connection_min_overlap_ratio                 float32  0.1                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Minimum edge overlap required for jump connection consideration as a percentage of agent radius
// nav_gen_markup_split_expand                               float32  2                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_markup_split_tol_base                             float32  1                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_markup_split_tol_nonav                            float32  1                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_markup_split_tol_nonentity                        float32  8                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_max_bottleneck_width                              float32  128                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_max_bottleneck_width_do_clip                      bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_max_edge_len                                      float32  512                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_max_edge_len_do_clip                              bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_max_edge_len_split_tol                            float32  24                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_opt_to_quads                                      bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_opt_to_quads_angle_limit                          float32  8                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_opt_to_quads_num_steps                            int32    6                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_opt_to_quads_planar_deviation_limit               float32  4                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_opt_to_quads_se_limit_end                         float32  0.1                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_opt_to_quads_se_limit_start                       float32  0.00001               FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_opt_to_quads_weld_limit_end                       float32  0.01                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_opt_to_quads_weld_limit_start                     float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_oriented_angle_tol                                float32  15                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Max abrupt orientation difference an NPC can tolerate when moving through the mesh (degrees).
// nav_gen_oriented_max_region_range                         float32  15                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Max orientation range allowed within a region before it gets further split.
// nav_gen_remove_vertical_polys                             bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_split_boundary_polys                              bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_split_multi_connection_polys                      bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_split_multi_connection_polys_tol                  float32  0.01                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_gen_true                                              bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Always true
// nav_gen_vertical_limit                                    float32  88                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_genrt_debug                                           bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_ignore_vpk_navdata                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // For testing using legacy nav data
// nav_max_view_distance                                     float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Maximum range for precomputed nav mesh visibility (0 = default 1500 units)
// nav_max_vis_delta_list_length                             int32    64                    FCVAR_CHEAT
// nav_obstacle_validate                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_obstruction_async_update                              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_obstruction_draw                                      float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_obstruction_draw_change                               bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_obstruction_draw_dist                                 float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_obstruction_draw_island                               float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_obstruction_draw_island_hull                          int32    -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_obstruction_draw_movefail_blocking                    bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_debug                                            bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_areas                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_arrow                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_climb_segments                              bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_connected_areas                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_ground_segments                             bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_jump_segments                               bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_ladder_segments                             bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_link_segments                               bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_draw_tick                                        float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_fixup_climb_up_segments                          bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_fixup_gap_segments                               bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_jump_process_debug                               bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_optimize                                         bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_optimize_portals                                 bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_path_optimizer_debug                                  int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_pathfind_debug_log                                    int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_pathfind_draw                                         float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_pathfind_draw_blocked                                 float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_pathfind_draw_costs                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_pathfind_draw_fail                                    float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_pathfind_draw_total_costs                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_pathfind_inadmissable_heuristic_factor                float32  1                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_pathfind_multithread                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_potentially_visible_dot_tolerance                     float32  0.98                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_recorder_enabled                                      bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_select_allow_blocked                                  bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // When selecting an area under nav_edit, allow area marked as blocked.
// nav_select_area_id                                        int32    -1                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Select nav area with matching ID.
// nav_select_block_id                                       int32    -1                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Select nav space block with matching ID.
// nav_select_hull                                           int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Restrict area selection to areas that can support a hull of the given category
// nav_show_area_connections                                 bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Show connections to selected area when true
// nav_show_area_verts                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Show area vertex positions
// nav_show_area_water_info                                  bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_show_elem_info                                        bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_show_elem_info_font                                   string   Consolas              FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_show_elem_info_font_size                              float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_show_elem_info_font_voffset                           float32  -11                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_show_potentially_visible                              int32    0                     FCVAR_CHEAT  // Show areas that are potentially visible from the current nav area
// nav_smooth_constrain_spline                               bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_constrain_spline_relax                         float32  0.006                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_constrain_spring                               int32    2                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_constrain_spring_relax                         float32  0.01                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_draw_boundary                                  float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_draw_calc                                      int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_draw_constraint_spline                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_draw_constraint_spring                         float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_draw_speed                                     float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_enable                                         int32    1                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_push_from_walls                                float32  12                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_smooth_relax                                          bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_relax_use_timesteps                            bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_const_override                          float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_enable                                  bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_factor_deriv                            float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_factor_dist                             float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_factor_speed                            float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_forward_dist_base                       float32  50                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_forward_dist_time_limit                 float32  1                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_max_dist                                float32  36                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_tension_max_override                    float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_timestep_factor_accel                   float32  100                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_timestep_factor_speed                   float32  100                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_timestep_max                            float32  0.5                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_timestep_min                            float32  0.1                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_yaw_rotation_speed                      float32  50                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_smooth_spring_yaw_threshold                           float32  20                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_space_select_dist                                     float32  1000                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_split_show_line                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Show the free split line.
// nav_test_area_gravity                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_lattice_dist_0                               float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_lattice_dist_1                               float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_lattice_dist_2                               float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_lattice_hex                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Demonstrates searching hexagonal lattice over nav mesh.
// nav_test_bfs_lattice_mark                                 float32  2                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_lattice_simple                               bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_lattice_spacing_0                            float32  24                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_lattice_spacing_1                            float32  48                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_lattice_spacing_2                            float32  96                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_bfs_simple                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_boundary_zone_circle                             float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_boundary_zone_force                              bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_boundary_zone_grid_dim                           float32  90                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_boundary_zone_path                               float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_boundary_zone_rays                               float32  100                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_boundary_zone_rays_margin                        float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_boundary_zone_rays_random                        bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_boundary_zone_sphere                             float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_curve_opt                                        int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_detour                                           bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_find_nearest                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Calculate the nearest point on the navmesh to the trace point.  Uses selection from nav_select_hull.
// nav_test_find_nearest_clear                               bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Calculate the nearest point on the navmesh to the trace point.  Uses selection from nav_select_hull.
// nav_test_find_random_connected                            bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Demonstrates finding random points that are connected in the nav mesh to the start point.
// nav_test_find_random_connected_dist_max                   float32  1000                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_find_random_connected_dist_min                   float32  100                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_find_z                                           float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_force_npc_repath                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_genrt                                            bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_genrt_place                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_genrt_tile_removal_extent                        float32  50                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_genrt_tile_removal_place                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_getareaoverlapping_gravity                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_getnearestnav_gravity                            bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_multi_connection                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_npc_area                                         int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_npc_collision                                    int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_npc_collision_range                              float32  250                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_npc_collision_show_geometry                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_path                                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Calculate and draw a path from player/camera position to the test position.
// nav_test_path_lock_goal                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Lock the pathfinding goal to the current intersection point.
// nav_test_path_lock_start                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Lock the pathfinding start to the current intersection point.
// nav_test_path_move                                        bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_path_opt                                         bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Enable path optimization for nav_edit_path paths.
// nav_test_path_opt_transitions                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_path_return                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Calculate a return path from cursor position to the path calculated by nav_test_path.
// nav_test_path_space                                       int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Should nav_test_path test 3d navigation?  1 = space to space, 2 = multi-modal space/ground
// nav_test_path_space_fly                                   bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Test flight paths
// nav_test_path_space_swim                                  bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Test swim paths
// nav_test_pos_name                                         string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_test_pos_place                                        int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// nav_test_ray_space                                        int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_rays                                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_rays_use_npc_move                                bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_search_on_path                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Test the 'on path' search mode for tactical searches. Requires a selected NPC with a current path.
// nav_test_search_on_path_boundary_edges_only               bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Activate the 'boundary edges only' constraint when testing the 'on path' search mode for tactical searches.
// nav_test_search_on_path_setgoal                           bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Test the 'on path' search mode for tactical searches using SetGoal w/ possible path clipping (flips between 2 searches). Requires a selected NPC.
// nav_test_smooth                                           bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_smooth_extern_push                               float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_smooth_in_speed                                  float32  120                   FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_smooth_in_yaw                                    float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_smooth_path_speed                                float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_smooth_separating_dist                           float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_smooth_spring_const                              float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_smooth_spring_tension_max                        float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_spline                                           int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_split_obstacle                                   int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_split_obstacle_dirty                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_split_obstacle_leave                             bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_split_obstacle_size                              float32  30                    FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_test_split_obstacle_update_pos                        bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_validate                                              int32    0                     FCVAR_CHEAT  // Level of validation for nav system.  Higher will be slower.
// nav_volume_debug                                          int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw or print debug information about nav volume queries.
// navspace_create_water_smooth_connections                  bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// navspace_create_water_transition_connections              bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// navspace_debug_pathfind                                   float32  -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// navspace_debug_stringpull                                 float32  1                     FCVAR_GAMEDLL|FCVAR_CHEAT
// navspace_debug_trace                                      float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// navspace_debug_transition_calc                            float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT
// navspace_draw_changes_blocks                              float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw blocks when they change
// navspace_draw_changes_waters                              float32  0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw water volumes when they change
// navspace_path_use_water_level_locator                     bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// net_async_clientconnect                                   bool     true                  FCVAR_DEVELOPMENTONLY  // Enable async client connect optimization
// net_async_job_random_sleep                                float32  0                     FCVAR_DEVELOPMENTONLY  // Sleep randomly 0..net_async_job_random_sleep ms in the parallel server jobs; sleep is per job
// net_client_steamdatagram_enable_override                  int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // 0: Use connect method requested by GC.  >0: Always use SDR if possible.  <0: Always use direct UDP if possible
// net_compresspackets_minsize                               int32    1024                  FCVAR_DEVELOPMENTONLY  // Don't bother compressing packets below this size.
// net_culloptimization                                      bool     true                  FCVAR_DEVELOPMENTONLY  // Enable optimization of slow path that makes HLTV CPU consumption high in AnimGraph-using mods. Will switch to this on by default soon.
// net_debug_to_file                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// net_detailed_canpacket_log                                bool     false                 FCVAR_DEVELOPMENTONLY
// net_filelogging                                           bool     false                 FCVAR_DEVELOPMENTONLY  // Log packets to files
// net_fs_showindirections                                   bool     false                 FCVAR_DEVELOPMENTONLY
// net_limit_sv_recv_max_message_size_kb                     int32    32                    FCVAR_RELEASE  // Server will reject message larger than N kb
// net_limit_sv_recv_segments_per_packet                     int32    50                    FCVAR_RELEASE  // Server will reject packets with more than N segments
// net_limit_sv_recvbuffer_kb                                int32    128                   FCVAR_RELEASE  // Server will not buffer more than N kb from connected clients
// net_limit_sv_recvbuffer_msg                               int32    100                   FCVAR_RELEASE  // Server will not buffer more than N messages from connected clients
// net_log_processing                                        bool     false                 FCVAR_DEVELOPMENTONLY  // Log network processing
// net_max_message_process_count                             int32    0                     FCVAR_DEVELOPMENTONLY  // Maximum number of messages to process from a client in a single frame (0 == no limit).
// net_max_message_queue_size                                int32    0                     FCVAR_DEVELOPMENTONLY  // Maximum number of messages to allow waiting in queue after processing; exceeding this disconnects the client. 0 == no limit
// net_max_polymorphic_spew                                  int32    5                     FCVAR_DEVELOPMENTONLY  // Max polymorphic variants to spew when spewing a flattened serializer.
// net_p2p_listen_dedicated                                  bool     false                 FCVAR_DEVELOPMENTONLY  // Should dedicated server listen for new-style P2P?
// net_public_adr                                            string                         FCVAR_RELEASE  // For servers behind NAT/DHCP meant to be exposed to the public internet, this is the public facing ip address string: ("x.x.x.x" )
// net_qosinterval_spew                                      bool     false                 FCVAR_DEVELOPMENTONLY  // Spew QoS interval data as we gather it
// net_qospacketloss_percentage_threshold                    float32  5                     FCVAR_DEVELOPMENTONLY  // Spew a warning if packet loss percentage is above this threshold
// net_restrict_showmsg_socket                               string                         FCVAR_DEVELOPMENTONLY  // If set, only net_showmsg spew for data inbound on this socket name e.g. client, server, etc.
// net_showdrop                                              bool     false                 FCVAR_DEVELOPMENTONLY  // Show dropped packets in console
// net_showeventlisteners                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Show listening addition/removals
// net_showevents                                            int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Dump game events to console (1=client only, 2=all).
// net_showmsg                                               string   0                     FCVAR_DEVELOPMENTONLY  // Show incoming message: <0|1|2|name> where 1 == all and 2 == all except net_NOP
// net_showoob                                               bool     false                 FCVAR_DEVELOPMENTONLY  // Show connectionless UDP traffic.
// net_showpeaks                                             int32    0                     FCVAR_DEVELOPMENTONLY  // Show messages for large packets only: <size>
// net_showreliable                                          string   0                     FCVAR_DEVELOPMENTONLY  // Like net_showmsg, but only spew reliable messages
// net_showudp                                               bool     false                 FCVAR_RELEASE  // Dump UDP packets summary to console
// net_showudp_remoteonly                                    bool     true                  FCVAR_RELEASE  // Dump non-loopback udp only
// net_skip_redundant_change_callbacks                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// net_use_packet_compression                                bool     true                  FCVAR_DEVELOPMENTONLY  // Compress network traffic
// nextlevel                                                 string                         FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_RELEASE  // If set to a valid map name, will trigger a changelevel to the specified map at the end of the round
// nextmap_print_enabled                                     bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // When enabled prints next map to clients
// nextmode                                                  string                         FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Sets the game mode to be played when the next level loads
// noclip_fixup                                              bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT
// npc_record_snapshot_data                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_attract_draw                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_constraint_nav                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_constraint_npc                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_drag_linear                                      float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_forward                                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_forward_const                                    float32  30000                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_forward_dist                                     float32  200                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_forward_margin                                   float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_path_close_const                                 float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_path_close_max_tension                           float32  100                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_path_lookahead_const                             float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_path_lookahead_dist                              float32  100                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_path_vel_const                                   float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_separation                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_separation_const                                 float32  10000                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_separation_dist                                  float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_separation_draw                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_separation_jitter                                float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// npcsolve_separation_r2                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// option_duck_method                                        bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Input toggle control
// option_speed_method                                       bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Input toggle control
// opus_decode_test_signal                                   bool     false                 FCVAR_DEVELOPMENTONLY
// opus_encode_test_signal                                   bool     false                 FCVAR_DEVELOPMENTONLY
// opus_unittest_test_signal                                 bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_2d_translate_no_comp_layer                       bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_alignment_fixes                                  bool     true                  FCVAR_DEVELOPMENTONLY  // Fix alignment issues
// panorama_allow_texture_composition_layer_fast_path        bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_allow_transitions                                bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_apply_styles_for_invisible_parents               bool     false                 FCVAR_DEVELOPMENTONLY  // Specifies whether to short circuit applying styles when a parent is invisible.
// panorama_assert_loading_panel_type                        bool     false                 FCVAR_DEVELOPMENTONLY  // Force style invalidation of the entire panel subtree when adding / removing classes.
// panorama_async_compute_mipgen                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // use asynchronous compute for mipmap generation.
// panorama_box_shadow_no_comp_layer                         bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_cache_command_list_repaint_threshold             float32  0.25                  FCVAR_DEVELOPMENTONLY
// panorama_cache_command_list_size_threshold                uint32   384                   FCVAR_DEVELOPMENTONLY
// panorama_classes_force_invalidate                         bool     false                 FCVAR_DEVELOPMENTONLY  // Force style invalidation of the entire panel subtree when adding / removing classes.
// panorama_clear_frames_on_device_restore                   int32    2                     FCVAR_DEVELOPMENTONLY
// panorama_comp_layer_lru_lifetime                          float32  1                     FCVAR_DEVELOPMENTONLY
// panorama_composition_atlas                                bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_console_max_autocomplete                         int32    100                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_console_max_history                              int32    100                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_console_max_lines                                int32    2000                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_console_position_and_size                        string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// panorama_content_size_fixes                               bool     true                  FCVAR_DEVELOPMENTONLY  // Fix content size issues
// panorama_daisy_wheel                                      string   ABXY                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Daisy wheel input mode: RS | ABXY
// panorama_dash_gap_ratio                                   float32  0.5                   FCVAR_DEVELOPMENTONLY
// panorama_dash_len                                         float32  20                    FCVAR_DEVELOPMENTONLY
// panorama_debug_movies                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_debug_overlay_opacity                            float32  0.25                  FCVAR_ARCHIVE
// panorama_debug_overlay_opacity_max                        float32  0.25                  FCVAR_ARCHIVE
// panorama_debug_overlay_opacity_min                        float32  0.01                  FCVAR_ARCHIVE
// panorama_debug_ready_for_display                          bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_debugger_theme                                   string   Light                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// panorama_disable_blur                                     bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disable_box_shadow                               bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disable_descendant_filtering                     bool     false                 FCVAR_DEVELOPMENTONLY  // Disable descendant selector filtering
// panorama_disable_draw_fancy_quad                          bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disable_draw_text                                bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disable_draw_text_shadow                         bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disable_layer_cache                              bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disable_layer_clear                              bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disable_render_callbacks                         bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disable_render_target_cache                      bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_disallow_hover_styles                            bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_dragscroll_affordance                            int32    20                    FCVAR_DEVELOPMENTONLY  // Minimum mouse movement in pixels before a move is treated as a drag scroll
// panorama_dragscroll_maxflickvelocity                      float32  8000                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Maximum velocity for a drag scroll flick
// panorama_dragscroll_minflickvelocity                      float32  60                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Minimum velocity that the mouse must be moving as mouse up time to qualify as a drag scroll flick
// panorama_dragscroll_mintime                               float32  0.02                  FCVAR_DEVELOPMENTONLY  // Minimum time that the mouse button must be down before a move is treated as a drag scroll
// panorama_dragscroll_velocitymultiplier                    float32  0.5                   FCVAR_DEVELOPMENTONLY  // Multiplier for flick velocity off of actual measured velocity
// panorama_draw_fast_path_img_shadow                        bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_draw_text_fast_path                              bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_draw_text_fast_path_text_shadow                  bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_enable_secondary_layout_pass                     bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_focus_world_panels                               bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // when set request key focus when a world panel is enabled
// panorama_force_active_controller_type                     int32    -1                    FCVAR_DEVELOPMENTONLY
// panorama_force_desired_layout_traverse                    bool     false                 FCVAR_DEVELOPMENTONLY  // Force desired layout traverse, even if the cached values are up to date.
// panorama_highlight_bad_opacity_masks                      bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_highlight_composition_layers                     bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_highlight_slow_operations                        bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_hsbc_through_fast_path                           bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_joystick_axis_repeat_curve_time                  float32  1                     FCVAR_DEVELOPMENTONLY
// panorama_joystick_axis_repeat_interval_end                float32  0.05                  FCVAR_DEVELOPMENTONLY
// panorama_joystick_axis_repeat_interval_start              float32  0.22                  FCVAR_DEVELOPMENTONLY
// panorama_joystick_button_repeat_curve_time                float32  1.2                   FCVAR_DEVELOPMENTONLY
// panorama_joystick_button_repeat_interval_end              float32  0.1                   FCVAR_DEVELOPMENTONLY
// panorama_joystick_button_repeat_interval_start            float32  0.48                  FCVAR_DEVELOPMENTONLY
// panorama_joystick_enabled                                 bool     false                 FCVAR_ARCHIVE  // Enable panorama joystick input
// panorama_js_minidumps                                     bool     true                  FCVAR_DEVELOPMENTONLY  // Enable sending minidumps on JS Exceptions.
// panorama_label_draw_rects                                 int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // When labels paint, draw the rectangles for the character ranges. 0 = none, 1 = all, 2 = text only, 3 = inline objects only
// panorama_label_wrap_before_shrink                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Should labels try to wrap text before using text-overflow: shrink
// panorama_large_dispatch_event_queue                       int32    0                     FCVAR_DEVELOPMENTONLY
// panorama_max_text_shadow_strength                         float32  10                    FCVAR_DEVELOPMENTONLY
// panorama_might_scroll_no_comp_layer                       bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_min_comp_layer_cache_cost                        int32    4096                  FCVAR_DEVELOPMENTONLY
// panorama_movie_async_load_size_bytes                      int32    20971520              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_movie_force_not_ready_behavior                   int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_panel_occlusion                                  bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_print_render_tree                                string                         FCVAR_DEVELOPMENTONLY  // Print the RenderOperation_t tree for the given root window; set to * to print all
// panorama_reload_animations                                int32    2                     FCVAR_DEVELOPMENTONLY
// panorama_render_target_cache_max_size                     int32    31457280              FCVAR_DEVELOPMENTONLY
// panorama_script_cache_enabled                             bool     true                  FCVAR_DEVELOPMENTONLY  // Enable script caching to speed up recompiling scripts multiple times.
// panorama_show_fps                                         bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_show_fps_scale                                   float32  1                     FCVAR_DEVELOPMENTONLY
// panorama_simple_borders_no_comp_layer                     bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_skip_composition_layer_content_paint             bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_skip_composition_layer_content_paint_tint        bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_spew_async_event_substring                       string                         FCVAR_DEVELOPMENTONLY  // If non-empty, print debug info about async event queue and dispatch behavior for events containing the substring.
// panorama_spew_layout_invalidates                          bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_stats_log_time                                   float32  0                     FCVAR_DEVELOPMENTONLY
// panorama_streaming_load_local_images                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_style_flag_force_invalidate                      bool     false                 FCVAR_DEVELOPMENTONLY  // Force style invalidation of the entire panel subtree when adding / removing style flags.
// panorama_suspend_animation                                bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_suspend_paint                                    bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_temp_comp_layer_min_dimension                    float32  512                   FCVAR_DEVELOPMENTONLY
// panorama_toggledebugger_mode                              int32    1                     FCVAR_ARCHIVE  // Toggledebugger key operation : 0 = open/inspect, 1 = open/close
// panorama_track_render_commands                            bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_transform_parents_no_layer_for_perspective       bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_transforms_no_comp_layer                         bool     false                 FCVAR_DEVELOPMENTONLY
// panorama_transition_time_factor                           float32  1                     FCVAR_DEVELOPMENTONLY  // A float representing a scale factor for transitions. 1.0 is normal, 2.0 would be twice as fast as normal, 0.5 half as fast
// panorama_unlink_from_render_tree                          bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_use_backbuffer_directly                          bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_use_composite_cmd_for_cached_layers              bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_use_new_occlusion_invalidation                   bool     true                  FCVAR_DEVELOPMENTONLY
// panorama_worldpanel_update_cull_distance                  float32  1000                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_worldpanel_update_cull_size_threshold            float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// panorama_worldpanel_update_culling                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// parallel_perform_invalidate_physics                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// parallel_update_surrounding_bounds_in_spatial_partition_update  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// particle_cluster_debug                                    int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// particle_cluster_manager_search_dist                      float32  256                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// particle_cluster_nodraw                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// particle_cluster_use_collision_hulls                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// particle_debug_creation_filter                            string                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// particle_layer_id_whitelist                               string                         FCVAR_DEVELOPMENTONLY
// particle_powsimd_random_range_exp                         bool     true                  FCVAR_DEVELOPMENTONLY
// particle_profile_filter                                   string                         FCVAR_DEVELOPMENTONLY  // Profile particle filter
// particle_snapshot_allow_combined_models                   bool     false                 FCVAR_DEVELOPMENTONLY
// particle_test_attach_attachment                           int32    0                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Attachment index for attachment mode
// particle_test_attach_mode                                 string   follow_attachment     FCVAR_GAMEDLL|FCVAR_CHEAT  // Possible Values: 'start_at_attachment', 'follow_attachment', 'start_at_origin', 'follow_origin'
// particle_test_file                                        string                         FCVAR_GAMEDLL|FCVAR_CHEAT  // Name of the particle system to dynamically spawn
// partybrowser_throttle_data                                float32  0.15                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// partybrowser_timeout                                      float32  15                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// password                                                  string                         FCVAR_ARCHIVE|FCVAR_DONTRECORD|FCVAR_SERVER_CANNOT_QUERY  // Current server access password
// path_closest_point_debug                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// path_node_evaluation_debug                                float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// pawn_mimic_all                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phonemedelay                                              float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Phoneme delay to account for sound system latency.
// phonemefilter                                             float32  0.08                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Time duration of box filter to pass over phonemes.
// phonemesnap                                               int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Lod at level at which visemes stops always considering two phonemes, regardless of duration.
// phys_batch_ray_test                                       int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// phys_continuous_kinematic_update                          float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_cull_internal_mesh_contacts                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// phys_dynamic_scaling                                      bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// phys_expensive_shape_threshold                            int32    6                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// phys_force_controller_debug                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// phys_headshotscale                                        float32  1.3                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Modifier for the headshot impulse hits on players
// phys_highlight_expensive_objects                          bool     false                 FCVAR_CHEAT  // Highlight expensive physics objects
// phys_highlight_expensive_objects_strength                 float32  0.02                  FCVAR_CHEAT  // Highlight expensive physics objects strength
// phys_impactforcescale                                     float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// phys_joint_teleport                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Teleport joint anchors if connected to world
// phys_length_damping_ratio                                 float32  2                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Spring damping ratio for length constraint
// phys_length_frequency                                     float32  5                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Spring stiffness for length constraint
// phys_log_updaters                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_log_updaters_exclude                                 string   weapon pistol rifle…  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_log_updaters_include                                 string   limbs                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_min_motion_controller_count_to_run_in_job            int32    8                     FCVAR_DEVELOPMENTONLY
// phys_multithreading_enabled                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Enable/Disable Multithreading in VPhysics
// phys_playerscale                                          float32  10                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // This multiplies the bullet impact impuse on players for more dramatic results when players are shot.
// phys_powered_ragdoll_debug                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_pushscale                                            float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_shoot_angular_speed                                  float32  3600                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// phys_shoot_speed                                          float32  250                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// phys_show_stats                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_step_threaded                                        bool     true                  FCVAR_DEVELOPMENTONLY
// phys_stressbodyweights                                    float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// phys_threaded_cloth_bone_update                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_threaded_kinematic_bone_update                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_threaded_transform_update                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_timescale                                            float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Scale time for physics
// phys_upimpactforcescale                                   float32  0.375                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// phys_use_block_solver                                     bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Use block solving for constraint entities
// phys_vehicleimpactforcescale                              float32  1.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// phys_visualize_awake_dynamic_only                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_visualize_awake_unattached_only                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// phys_wind_force_scale                                     float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Scale on the force wind applies to physics bodies
// pickup_check_period                                       float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// player0_using_joystick                                    bool     false                 FCVAR_ARCHIVE
// player_botdifflast_s                                      string   2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// player_competitive_maplist_2v2_10_0_D684D4E1              string   mg_de_debris,mg_de_…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// player_competitive_maplist_8_10_0_A062AC6A                string   mg_de_dust2,mg_de_t…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// player_debug_off_nav                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// player_debug_print_damage                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // When true, print amount and type of all damage received by player to console.
// player_nevershow_communityservermessage                   string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// player_ping_token_cooldown                                float32  20                    FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_RELEASE  // Cooldown for how long it takes for a player's ping token to refresh allowing them to ping again (they get 5 tokens).
// player_survival_list_10_0_303                             string   mg_dz_blacksite,mg_…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// player_teamplayedlast                                     int32    2                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER
// player_use_radius                                         float32  80                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// player_wargames_list2_10_0_0                              string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// population_distribution_debug                             float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// pred_cloth_pos_max                                        float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// pred_cloth_pos_multiplier                                 float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// pred_cloth_pos_strength                                   float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// pred_cloth_rot_high                                       float32  0.1                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// pred_cloth_rot_low                                        float32  0.01                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// pred_cloth_rot_multiplier                                 float32  0.3                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// pred_cloth_smooth_motion                                  int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// pred_cloth_vmax                                           float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// pred_cloth_vw                                             float32  0.05                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// presettle_cloth_iterations                                int32    30                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// prop_debug_collision                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Highlights props based on their collision group: COLLISION_GROUP_PROPS(white), COLLISION_GROUP_INTERACTIVE_DEBRIS(green), COLLISION_GROUP_DEBRIS and will return to COLLISION_GROUP_INTERACTIVE_DEBRIS on sleeping(bright red), COLLISION_GROUP_DEBRIS permanent
// prop_nav_ignore_edge_len                                  float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_ignore_mass                                      float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_obstacle_avoid_mass                              float32  100.1                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_obstacle_avoid_use_connection_blocker            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_obstacle_block_edge_min_                         float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_obstacle_block_edge_min_a                        float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_obstacle_block_edge_min_c                        float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_obstacle_block_mass_a                            float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_obstacle_block_mass_b                            float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// prop_nav_obstacle_block_mass_c                            float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// props_break_apply_radial_forces                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// props_break_max_pieces_perframe                           int32    16                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Maximum prop breakable piece count per frame (-1 = model default)
// props_break_radial_force_ratio                            float32  0.33                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// pulse_save_execution_history                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Keep a history of all instructions run on a per graph basis.
// pulse_save_execution_history_limit                        int32    10000                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Keep a history of all instructions run on a per graph basis.
// pvs_debugentity                                           int32    -1                    FCVAR_GAMEDLL|FCVAR_RELEASE  // Verbose spew for this entity when doing IsInPVS computation.
// pvs_flowtype                                              int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Flow through spawn groups for vis (0 == default, 1 == always visible, 2 == never visible.
// pwatchent                                                 int32    -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Entity to watch for prediction system changes.
// pwatchvar                                                 string                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // Entity variable to watch in prediction system for changes.
// r_AirboatViewDampenDamp                                   float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT
// r_AirboatViewDampenFreq                                   float32  7                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT
// r_AirboatViewZHeight                                      float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT
// r_JeepViewDampenDamp                                      float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT
// r_JeepViewDampenFreq                                      float32  7                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT
// r_JeepViewZHeight                                         float32  10                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_CHEAT
// r_RainAllowInSplitScreen                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Allows rain in splitscreen
// r_RainParticleDensity                                     float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Density of Particle Rain 0-1
// r_add_views_in_pre_output                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_allow_onesweep_gpusort                                  bool     true                  FCVAR_DEVELOPMENTONLY
// r_always_render_all_windows                               bool     false                 FCVAR_DEVELOPMENTONLY  // Always force all engine & tools to render
// r_aoproxy_cull_dist                                       float32  12                    FCVAR_DEVELOPMENTONLY  // Distance to cull the AO proxy as a factor of size
// r_aoproxy_debug                                           bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_aoproxy_default_ambient_strength                        float32  0.2                   FCVAR_DEVELOPMENTONLY
// r_aoproxy_default_light_cone_angles                       vector4  0.3 0.3 0.3 0.3       FCVAR_DEVELOPMENTONLY
// r_aoproxy_default_light_cone_strengths                    vector4  1 1 1 1               FCVAR_DEVELOPMENTONLY
// r_aoproxy_default_light_position_0                        vector3  1 0 1.5               FCVAR_DEVELOPMENTONLY
// r_aoproxy_default_light_position_1                        vector3  -1 0 1.5              FCVAR_DEVELOPMENTONLY
// r_aoproxy_default_light_position_2                        vector3  0 1 1.5               FCVAR_DEVELOPMENTONLY
// r_aoproxy_default_light_position_3                        vector3  0 -1 1.5              FCVAR_DEVELOPMENTONLY
// r_aoproxy_enable                                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_aoproxy_min_dist                                        float32  3                     FCVAR_DEVELOPMENTONLY
// r_aoproxy_min_dist_box                                    float32  1                     FCVAR_DEVELOPMENTONLY
// r_aoproxy_show                                            bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_aspectratio                                             float32  0                     FCVAR_DEVELOPMENTONLY
// r_async_shader_compile_notify_frequency                   int32    10                    FCVAR_DEVELOPMENTONLY
// r_bloom_tent_filter_radius                                float32  3.1                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // bloom mip up-sample filtering radius (using 3x3 tent filter, radius in mip level texels), 0.0 radius => box (2x2) filter with (fixed) 1.0 radius
// r_character_decal_monitor_draw_frustum                    bool     false                 FCVAR_DEVELOPMENTONLY
// r_character_decal_monitor_emissive                        bool     false                 FCVAR_DEVELOPMENTONLY
// r_character_decal_monitor_render_res                      int32    512                   FCVAR_DEVELOPMENTONLY
// r_character_decal_renderdoc_capture                       bool     false                 FCVAR_DEVELOPMENTONLY
// r_character_decal_resolution                              int32    1024                  FCVAR_DEVELOPMENTONLY  // Resolution of character decal texture.
// r_cs2_show_icon_editor                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // CSGO/Icon Editor
// r_csgo_bloom_threshold_all_samples                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Execute bloom threshold once per sample during downsample (default enabled, higher quality, less bloom aliasing)
// r_csgo_bloom_threshold_downsample_jimenez                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Custom downsample based on Jimenez14, (default enabled, higher quality, decreases bloom aliasing further)
// r_csgo_cable_pixel_radius_clamp                           float32  1.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Minimum clamped size in pixels of a cable (if using F_CLAMP_MIN_RADIUS 1 in cable material)
// r_csgo_cmaa_debug_edges                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // debug visualize edges
// r_csgo_cmaa_enable                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_cmaa_extra_sharp                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // trade more sharpness for reduced antialiasing
// r_csgo_cmaa_quality                                       int32    3                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // 0=low, 1=medium, 2=high, 3=ultra
// r_csgo_csm_max_visible_distance                           float32  7500                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_csm_max_visible_distance_preview                   float32  2000                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_csm_override_staticgeo_cascades_alphatest          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // If lb_csm_override_staticgeo_cascades true, ensure objects with SCENEOBJECTFLAG_ALPHA_TESTED flag will be rendered into cascade.
// r_csgo_csm_pushback_distance                              float32  7000                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_csm_pushback_distance_preview                      float32  1500                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_cubemap_normalization                              bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_decal_debug                                        bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_decals_use_msaa                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_depth_prepass                                      bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_depth_prepass_cull_threshold                       float32  60                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_depth_prepass_skybox_alpha_tested                  bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_depth_prepass_skybox_large                         bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_depth_prepass_skybox_small                         bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_depth_prepass_skybox_small_cull_threshold          float32  5                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_depth_prepass_small_cull_threshold                 float32  10                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_depth_prepass_viewmodel                            bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_directional_lightmaps                              bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_effects_bloom                                      bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_effects_bloom_when_smoked                          bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_enable_cubemap_fog                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_enable_glows                                       bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_enable_gradient_fog                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_enable_high_precision_lighting                     bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_enable_sunlight_check                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Enable vis tests for sunlight.
// r_csgo_enable_tonemapping                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_enable_translucent_screen_space                    bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_enable_volume_fog                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_firstpersonlegs_nearz_offset                       float32  0.1                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_fsr_enable_mip_bias                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Apply negative mip bias when rendering with FSR.
// r_csgo_fsr_rcas_sharpness                                 float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // RCAS sharpness when using FSR + RCAS upsample.
// r_csgo_fsr_upsample                                       int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // 0 == bilinear upsampe, 1 == FSR upsample, 2 == FSR + RCAS upsample
// r_csgo_gpu_culling                                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_MENUBAR_ITEM  // CSGO/Graphics/GPU Culling
// r_csgo_gpu_culling_camera_offset                          float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_gpu_culling_shadows                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_MENUBAR_ITEM  // CSGO/Graphics/GPU Cull Shadow Views
// r_csgo_gpu_culling_shadows_min_cascade                    int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // If r_csgo_gpu_culling_shadows is true, this defines min cascade for which gpu culling is used
// r_csgo_gpu_culling_two_pass                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_MENUBAR_ITEM  // CSGO/Graphics/GPU Culling (Two Pass)
// r_csgo_gpu_debug_draw                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_gpu_opt_downsample_depth_using_resolved_depth      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // use already resolved depth as input to downsample depth layer
// r_csgo_gpu_opt_firstpersonlegs_visible_angle              float32  40                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // avoid overhead of firstpersonlegs layers if not looking down enough to see them
// r_csgo_gpu_opt_prepass_characters                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // only depth prepass nearby characters (see r_csgo_gpu_opt_prepass_characters_cull_threshold to control threshold)
// r_csgo_gpu_opt_prepass_characters_cull_threshold          float32  15                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // use with r_csgo_gpu_opt_prepass_characters
// r_csgo_gpu_opt_resolve_depth_for_decals_on_translucent    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // optimize layers for decals on translucent geo, avoid one resolve and some fullscreen passes
// r_csgo_gpu_opt_resolve_depth_no_characters                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // remove unused resolve
// r_csgo_gpu_opt_use_aoproxy_depth_for_depth_pyramid        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // if ao proxies enabled, use ao proxy downsampled depth as input to generate the depth pyramid for gpu culling
// r_csgo_gpu_opt_viewmodel_stencil                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_gpu_opt_water_refraction_resolve                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // copy already resolved depth for use by waterrefraction layers, instead of resolving main depth again (avoids msaa samples)
// r_csgo_gpu_optimizations                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // temporary cvar to control new GPU optimzations (depth resolves, etc)
// r_csgo_joint_upscale_sigma                                float32  0.002                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_max_barnlight_shadow_scale_preview                 float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_mboit                                              bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_mboit_bias                                         float32  0.000005              FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_mboit_debug                                        bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_mboit_force_mixed_resolution                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_mboit_overestimation                               float32  0.01                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_mboit_upscale_cs                                   bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_mboit_use_4_moments                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_merge_resolve_with_histogram                       bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_microshadowing                                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_mixed_resolution_color_slices                      bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_mixed_resolution_particles                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_mixed_resolution_particles_minmax                  bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_mixed_resolution_particles_scale                   int32    2                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_mouse_trace_coord                                  bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_msaa_resolve_apply_exposure_scale                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // 0 - before, 1 - after fix for a2c fringing
// r_csgo_multiscattering                                    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_no_shader_resolve                                  bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_opaquerefract_viewmodel_depthcopy                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Copy depth in viewmodel for opaquerefract 
// r_csgo_opaquerefract_viewmodel_quality                    int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Opaque refract quality in viewmodel: 0 = no background copy, no depth, 1= background copy, depth if enabled 
// r_csgo_outline_glow_scaledenom                            int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_player_occlusion_query                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_player_occlusion_query_debug                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_postprocess_enable                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_r11g11b10_dither_mode                              int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // 0 - disabled, 1 - regular dither noise, 2 - blue noise dither
// r_csgo_readonly_depth_stencil_enable                      bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_reconstruct_normals                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_reconstruct_normals_method                         int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_decals                                      bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_decals_on_translucent                       bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_dither_scale                                float32  1                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_dynamic_objects                             bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_inferno_decals                              bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_opaque                                      bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_overlays                                    bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_post_bloom                                  int32    1                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_post_bloom_strength                         float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_post_colorcorrection                        int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_post_film_grain                             int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_post_fxaa                                   int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_render_post_local_contrast                         bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_post_mirror_horizontal                      int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_post_mirror_vertical                        int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_render_translucent                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_resolve_dither_bluenoise_amount                    float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Equivalent to r_csgo_render_dither_scale, but purely to control bluenoise for R11G11B10 downsample dither (if r_csgo_r11g11b10_dither_mode = 2)
// r_csgo_resolve_dither_noise_amount                        float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Amount of screen space dither noise to apply during resolve (used/essential with R11G11B10_FLOAT RT)
// r_csgo_shader_feature_test_value                          float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_shader_perf_test                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_shadow_map_allocation_failure_policy               int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // What happens when a shadow map fails allocation? 0 = don't render, 1 = render unshadowed
// r_csgo_shadows_debug                                      int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_smoke_avoid_flat                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_smoke_clip_sniper                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_smoke_fullres_enhance                              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Enhance edges of smokes to eliminate bad pixels
// r_csgo_smoke_fullres_pass                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Does a full res pass to cover holes and artifacts in smoke low res
// r_csgo_smoke_overlay_min_dt                               float32  0.015686275           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_smoke_shadow                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_smoke_upscale_discard_pixels_behind                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // When upsampling smoke discard pixels behind solid depth to avoid pixelated artifacts
// r_csgo_stencil_sniper_zoom                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_test1                                              bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// r_csgo_tools_vis_cubemap_roughness                        float32  0.0001                FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_upscale_depth_threshold                            float32  3                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_use_compute_bloom                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_viewmodel_csm_pushback_distance                    float32  1500                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_viewmodel_envmap_position_bias                     float32  0.85                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_viewmodel_far_plane                                float32  100                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_viewmodel_near_plane                               float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_csgo_viewmodel_probe_clamp_plane_distance               float32  16                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_volume_mboit_optimization                          bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_water_effects                                      bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_water_refraction                                   bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_csgo_water_skybox_depth                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_cubemap_debug_colors                                    int32    0                     FCVAR_CHEAT
// r_dashboard_render_quality                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_debug_depth_holes                                       bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_debug_draw_safe_area_insets                             bool     false                 FCVAR_DEVELOPMENTONLY  // Render safe area insets as wireframe.
// r_debug_particle_shadows                                  bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_debug_precipitation                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Show precipitation volumes
// r_decal_hit_confirmation                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_decals                                                  int32    2048                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_decals_additional_offset                                float32  0.01                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_decals_default_fade_duration                            float32  3                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_decals_default_start_fade                               float32  30                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_decals_max_on_deformables                               int32    512                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_decals_overide_fadestarttime_params                     float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_decals_overlap_threshold                                int32    6                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_depth_of_field                                          int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // 0 = off, 1 = enabled (high quality, circular bokeh, HDR)
// r_directional_lightmaps                                   bool     true                  FCVAR_DEVELOPMENTONLY
// r_directlighting                                          bool     true                  FCVAR_CHEAT  // Set to use direct lighting
// r_dlss_preset                                             int32    5                     FCVAR_DEVELOPMENTONLY
// r_dof2_maxblursize                                        float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_dof2_radiusscale                                        float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_dof_override                                            bool     false                 FCVAR_CHEAT
// r_dof_override_far_blurry                                 float32  2000                  FCVAR_CHEAT
// r_dof_override_far_crisp                                  float32  180                   FCVAR_CHEAT
// r_dof_override_near_blurry                                float32  -100                  FCVAR_CHEAT
// r_dof_override_near_crisp                                 float32  0                     FCVAR_CHEAT
// r_dof_override_tilt_to_ground                             float32  0.5                   FCVAR_CHEAT
// r_dopixelvisibility                                       bool     true                  FCVAR_CHEAT
// r_draw3dskybox                                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_draw_first_tri_only                                     bool     false                 FCVAR_CHEAT
// r_draw_instances                                          bool     true                  FCVAR_CHEAT
// r_draw_overlays                                           bool     true                  FCVAR_DEVELOPMENTONLY
// r_draw_particle_children_with_parents                     int32    -1                    FCVAR_CHEAT  // Draw particle children with parents (-1=use gameinfo, 0=no, 1=yes)
// r_drawblankworld                                          bool     false                 FCVAR_CHEAT  // Render blank instead of the game world
// r_drawchickens                                            bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Render chickens
// r_drawcsplayers                                           bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Render CS players
// r_drawdecals                                              bool     true                  FCVAR_CHEAT  // Set to render decals
// r_drawdevvisualizers                                      bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Render dev visualizers
// r_drawpanorama                                            bool     true                  FCVAR_CHEAT  // Enable the rendering of panorama UI
// r_drawparticles                                           bool     true                  FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Particles/Draw Particles
// r_drawpixelvisibility                                     bool     false                 FCVAR_DEVELOPMENTONLY  // Show the occlusion proxies
// r_drawropes                                               bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_drawskybox                                              bool     true                  FCVAR_CHEAT  // Render the 2d skybox.
// r_drawtracers                                             bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_drawtracers_firstperson                                 bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Toggle visibility of first person weapon tracers
// r_drawviewmodel                                           bool     true                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Render view model
// r_drawworld                                               bool     true                  FCVAR_CHEAT  // Render the world.
// r_dx11_debug_clean                                        bool     false                 FCVAR_RELEASE  // Aggressively unbind bound resources to cleanup DX11 debug warnings.
// r_dx11_software_cmd_lists                                 bool     true                  FCVAR_DEVELOPMENTONLY  // Enable Software Command lists for DX11 (Avoid using deferred contexts)
// r_enable_rigid_animation                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_experimental_lag_limiter                                bool     false                 FCVAR_DEVELOPMENTONLY
// r_extra_render_frames                                     int32    0                     FCVAR_CHEAT
// r_fallback_texture_lod_scale                              float32  2                     FCVAR_CHEAT  // Scale factor for requested texture size (texture streaming) - used for geo that doesn't have a precomputed UV density measure
// r_farz                                                    float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Override the far clipping plane. -1 means to use the value in env_fog_controller.
// r_flashlightambient                                       float32  0                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlightbacktraceoffset                               float32  0.4                   FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlightbrightness                                    float32  1                     FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightconstant                                      float32  0                     FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightfar                                           float32  1500                  FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightfov                                           float32  53                    FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightladderdist                                    float32  40                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlightlinear                                        float32  100                   FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightlockposition                                  bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlightmuzzleflashfov                                float32  120                   FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlightnear                                          float32  4                     FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightnearoffsetscale                               float32  1                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlightoffsetforward                                 float32  0                     FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightoffsetright                                   float32  5                     FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightoffsetup                                      float32  -5                    FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightquadratic                                     float32  0                     FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_flashlightshadowatten                                   float32  0.35                  FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlighttracedistcutoff                               float32  128                   FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlighttracedistwatercutoff                          float32  80                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_flashlightvisualizetrace                                bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_force_no_present                                        bool     false                 FCVAR_CHEAT  // Force the render device to not present frames.
// r_force_render_frame_count                                int32    5                     FCVAR_DEVELOPMENTONLY  // The number of frames to render when a
// r_force_thick_hair                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// r_frame_sync_enable                                       bool     true                  FCVAR_DEVELOPMENTONLY
// r_freeze_sceneobjects                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_freezeparticles                                         bool     false                 FCVAR_CHEAT  // Pause particle simulation
// r_fullscreen_gamma                                        float32  2.2                   FCVAR_ARCHIVE  // Screen Gamma (only in fullscreen modes)
// r_fullscreen_quad_single_triangle                         bool     true                  FCVAR_DEVELOPMENTONLY
// r_gpu_debug_draw_freeze                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_grass_allow_flattening                                  bool     false                 FCVAR_DEVELOPMENTONLY
// r_grass_alpha_test                                        int32    1                     FCVAR_DEVELOPMENTONLY
// r_grass_density_mode                                      int32    1                     FCVAR_DEVELOPMENTONLY  // 0 = Density corresponds to blade existance, 1 = Density corresponds to blade height, 2 = Both 0 and 1
// r_grass_end_fade                                          float32  3900                  FCVAR_DEVELOPMENTONLY
// r_grass_max_brightness_change                             float32  75                    FCVAR_DEVELOPMENTONLY
// r_grass_quality                                           int32    3                     FCVAR_DEVELOPMENTONLY  // 0 = Off, 1 = Low, 2 = Med, 3 = high, 4 = ultra
// r_grass_start_fade                                        float32  3000                  FCVAR_DEVELOPMENTONLY
// r_grass_vertex_lighting                                   int32    0                     FCVAR_DEVELOPMENTONLY
// r_hair_ao                                                 bool     true                  FCVAR_DEVELOPMENTONLY
// r_hair_debug_guides                                       int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // 1: Highlight guide hairs, 2: draw only guide hairs
// r_hair_indirect_transmittance                             bool     true                  FCVAR_DEVELOPMENTONLY
// r_hair_meshshader                                         int32    0                     FCVAR_DEVELOPMENTONLY
// r_hair_shadowtile                                         bool     true                  FCVAR_DEVELOPMENTONLY
// r_hair_voxels                                             int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// r_hair_wind_global_scale                                  float32  0.3                   FCVAR_DEVELOPMENTONLY
// r_hair_wind_min_noise_speed                               float32  20                    FCVAR_DEVELOPMENTONLY
// r_hair_wind_motion_scale                                  float32  0.07                  FCVAR_DEVELOPMENTONLY
// r_hair_wind_noise                                         float32  0.2                   FCVAR_DEVELOPMENTONLY
// r_hair_wind_noise_occlusion                               float32  1                     FCVAR_DEVELOPMENTONLY
// r_hair_wind_noise_size                                    float32  10                    FCVAR_DEVELOPMENTONLY
// r_hair_wind_occlusion                                     float32  2                     FCVAR_DEVELOPMENTONLY
// r_haircull_percent                                        float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// r_hairsort                                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// r_icon_csm_pushback_distance                              float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // csm pushback distance, should be much shorter/disabled for icon rendering
// r_icon_custommaterial_maxres                              int32    512                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // maxres for custommaterials when rendering icons
// r_icon_generate_offline_mips                              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // generate mips via sidecar file for offline
// r_icon_generate_runtime_mips                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // generate mips for runtime
// r_icon_highcontrast_postprocessing_weight                 float32  0.375                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // if using high contrast postprocessing, use this weight (weight = 1.0 for characters)
// r_icon_image_cache_to_disk                                bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // 1
// r_icon_max_mip_width                                      int32    128                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // r_icon_max_mip_width
// r_icon_player_equip_gloves_from_loadout                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // equip gloves on player for icon rendering from loadout, or use default gloves
// r_icon_rendering_height                                   int32    384                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // icon rendering height
// r_icon_rendering_width                                    int32    512                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // icon rendering width
// r_icon_show_timing                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // show timing in output
// r_icon_use_kv3_camera                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // use test kv3 data for camera
// r_impact_ricochet_chance                                  float32  0.3                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_impacts_alt_orientation                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_impacts_decal_grazing_incidence_cutoff                  float32  0.55                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_impacts_decal_grazing_incidence_variance                float32  0.1                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_indirectlighting                                        bool     true                  FCVAR_CHEAT  // Set to use indirect lighting
// r_late_particle_job_sync                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_legacy_vsync                                            bool     false                 FCVAR_DEVELOPMENTONLY  // Use legacy vsync mode -- for testing for a couple user machines.
// r_light_flickering_enabled                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_light_probe_volume_debug_colors                         bool     false                 FCVAR_CHEAT
// r_light_probe_volume_debug_grid                           int32    0                     FCVAR_CHEAT  // Show LPV debug grid, 0: off, 1: closest only 2: closest and keep 3: all
// r_light_probe_volume_debug_grid_albedo                    color    128 128 128 255       FCVAR_CHEAT  // albedo for LPV debug grid
// r_light_probe_volume_debug_grid_bbox                      bool     true                  FCVAR_CHEAT  // Show LPV bounding box when debug grid is on, 0: off, 1: on
// r_light_probe_volume_debug_grid_metalness                 float32  0                     FCVAR_CHEAT  // metalness for LPV debug grid
// r_light_probe_volume_debug_grid_prim                      int32    0                     FCVAR_CHEAT  // 0: spheres, 1: cubes
// r_light_probe_volume_debug_grid_roughness                 float32  0.5                   FCVAR_CHEAT  // roughness for LPV debug grid
// r_light_probe_volume_debug_grid_samplesize                float32  4                     FCVAR_CHEAT  // sphere radius (world) for LPV debug grid
// r_lightmap_set                                            string   lightmaps             FCVAR_CHEAT  // Lightmap set to use, only works on map load
// r_lightmap_size                                           int32    65536                 FCVAR_DEVELOPMENTONLY  // Maximum lightmap resolution.
// r_lightmap_size_directional_irradiance                    int32    -1                    FCVAR_DEVELOPMENTONLY  // Maximum lightmap resolution for directional_irradiance channel. -1 = use value of r_lightmap_size
// r_limit_particle_job_duration                             bool     false                 FCVAR_DEVELOPMENTONLY
// r_low_latency                                             int32    1                     FCVAR_DEVELOPMENTONLY  // NVIDIA Low Latency/AMD Anti-Lag 2 (0 = off, 1 = on, 2 = NV-only, on + boost)
// r_low_latency_trigger_flash                               bool     true                  FCVAR_DEVELOPMENTONLY  // NVIDIA Low Latency Trigger Flash
// r_mapextents                                              float32  16384                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Set the max dimension for the map.  This determines the far clipping plane
// r_max_texture_pool_size                                   int32    0                     FCVAR_DEVELOPMENTONLY  // Upper limit on texture pool size.
// r_mixed_shadows_fade_in_time                              float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_mixed_shadows_fade_out_time                             float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// r_monitor_3dskybox                                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_morphing_enabled                                        bool     true                  FCVAR_CHEAT
// r_multigpu_num_gpus_found                                 int32    1                     FCVAR_DEVELOPMENTONLY
// r_multigpu_num_gpus_used                                  int32    1                     FCVAR_DEVELOPMENTONLY
// r_muzzleflashbrightness                                   float32  0.4                   FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_muzzleflashlinear                                       float32  0.05                  FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// r_nearz                                                   float32  -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Override the near clipping plane. -1 means use the default.
// r_particle_allowprerender                                 bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_batch_collections                              bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_batch_in_fullsort                              bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_cables_cast_shadows                            bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_cables_culling                                 int32    1                     FCVAR_DEVELOPMENTONLY
// r_particle_cables_culling_bounds_scale                    float32  1.2                   FCVAR_DEVELOPMENTONLY
// r_particle_cables_dynamic_roundness                       bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_cables_dynamic_roundness_threshold             int32    20                    FCVAR_DEVELOPMENTONLY
// r_particle_cables_render                                  bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_cables_render_meshlets                         bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_cables_visualize_roundness                     bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_debug_filter                                   string                         FCVAR_DEVELOPMENTONLY  // Limit debug visualizations to substring match of effect name
// r_particle_debug_force_simulation                         int32    0                     FCVAR_DEVELOPMENTONLY  // -1 for all asleep, 1 for all awake
// r_particle_debug_randomseeds                              bool     false                 FCVAR_DEVELOPMENTONLY  // Use random seeds in debug
// r_particle_debug_show_attribute                           int32    -1                    FCVAR_DEVELOPMENTONLY  // Show specific attribute when debugging particle systems
// r_particle_debug_show_control_points                      bool     false                 FCVAR_DEVELOPMENTONLY  // Show all used controlpoints
// r_particle_debug_show_rope_segments                       int32    0                     FCVAR_DEVELOPMENTONLY  // Show rope segments when debugging particle systems - specify a number to isolate to that segment id
// r_particle_debug_show_sort_position                       bool     false                 FCVAR_DEVELOPMENTONLY  // Show the sorting position when debugging particle systems
// r_particle_enable_fastpath                                bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_explicit_fetch                                 bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_fixedrandomseeds                               bool     false                 FCVAR_DEVELOPMENTONLY  // Use fixed seeds for easier debugging
// r_particle_gpu_implicit                                   bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_gpu_implicit_cull_columns                      bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_gpu_implicit_debug_bricks                      bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_gpu_implicit_debug_stats                       bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_gpu_implicit_debug_wireframe                   bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_gpu_implicit_lds_cache                         bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_max_detail_level                               int32    2                     FCVAR_DEVELOPMENTONLY  // The maximum detail level of particle to create
// r_particle_max_draw_distance                              float32  1000000               FCVAR_CHEAT  // The maximum distance that particles will render
// r_particle_max_size_cull                                  float32  1200                  FCVAR_DEVELOPMENTONLY  // Particle systems larger than this in every dimension skip culling to save CPU.  They will be drawn anyway.
// r_particle_max_texture_layers                             int32    -1                    FCVAR_DEVELOPMENTONLY
// r_particle_min_timestep                                   float32  0                     FCVAR_DEVELOPMENTONLY  // A minimum on particle simulation time, particle simulation happening more frequently than this will lerp.
// r_particle_mixed_resolution_viewstart                     float32  500                   FCVAR_DEVELOPMENTONLY
// r_particle_model_new                                      bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_model_new8                                     bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_model_per_thread_count                         int32    32                    FCVAR_DEVELOPMENTONLY
// r_particle_multiplier                                     int32    1                     FCVAR_CHEAT  // Render each particle system N times for perf testing
// r_particle_newinput                                       bool     false                 FCVAR_DEVELOPMENTONLY  // Enable input path in particle ops
// r_particle_render_refreshes_sleep_timer                   bool     true                  FCVAR_DEVELOPMENTONLY  // Disable to get a better look at what's happening offscreen
// r_particle_render_sprites_issue_vizquery                  bool     true                  FCVAR_DEVELOPMENTONLY
// r_particle_render_test                                    bool     false                 FCVAR_DEVELOPMENTONLY  // render particles 100 times and show perf
// r_particle_shadow_map_texture_size                        int32    1536                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_particle_shadows                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_particle_shadows_cast_on_particles                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_particle_shadows_cast_on_particles_scale                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_particle_shadows_cast_on_world                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_particle_shadows_compute                                bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE
// r_particle_skip_postsim                                   bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_testbuffers                                    bool     false                 FCVAR_DEVELOPMENTONLY
// r_particle_timescale                                      float32  1                     FCVAR_DEVELOPMENTONLY
// r_particle_warn_threshold_ms                              float32  0                     FCVAR_DEVELOPMENTONLY  // Threshold to warn about when rendering particles.
// r_particles_memset_at_init                                int32    1                     FCVAR_DEVELOPMENTONLY  // 0=don't clear particle attrs at init 1=clear to zero 2=clear to 0xdb -1=clear to zero at first sim
// r_physics_particle_op_spawn_scale                         float32  1                     FCVAR_DEVELOPMENTONLY
// r_pipeline_stats_command_flush                            bool     false                 FCVAR_DEVELOPMENTONLY  // Experimental: Set to 1 to enable full GPU pipeline flushing after each command list.
// r_pipeline_stats_flush_before_sleeping                    bool     false                 FCVAR_DEVELOPMENTONLY  // Experimental: Set to 1 to enable GPU pipeline flushes right before the render thread sleeps to wait for more work.
// r_pipeline_stats_present_flush                            bool     false                 FCVAR_DEVELOPMENTONLY  // Experimental: Set to 1 to enable full GPU pipeline flushing after each present.
// r_pipeline_stats_use_flush_api                            bool     true                  FCVAR_DEVELOPMENTONLY  // Experimental: Set to 1 to use the ID3D11DeviceContext11::Flush() to flush the GPU pipeline instead of queries.
// r_pixelvisibility_partial                                 bool     true                  FCVAR_CHEAT
// r_pixelvisibility_spew                                    bool     false                 FCVAR_CHEAT
// r_player_fog_distance_multiplier                          float32  1.7                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_player_fog_maxdensity_multiplier                        float32  0.6                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_player_visibility_mode                                  int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// r_player_visibility_stencil                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_player_visibility_strength                              float32  1.1                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_post_bloom_debug                                        int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // 1 = bloom output (before thresholding), 2 = quarter res downsample, 3 = quarter res effects bloom 4 = quarter res effects raw
// r_propsmaxdist                                            float32  1200                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Maximum visible distance
// r_render_hair                                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// r_render_to_cubemap_begin_mixing_roughness                float32  0.25                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_render_to_cubemap_debug                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_render_world_node_bounds                                bool     false                 FCVAR_CHEAT  // Render world node bounds
// r_renderdoc_auto_shader_pdbs                              bool     true                  FCVAR_DEVELOPMENTONLY  // Automatically generate shader debug info on capture
// r_renderdoc_open_captures                                 bool     true                  FCVAR_DEVELOPMENTONLY
// r_renderdoc_validation_error_capture_limit                int32    5                     FCVAR_DEVELOPMENTONLY
// r_rendersun                                               bool     true                  FCVAR_CHEAT  // Render sun lighting
// r_replay_post_effect                                      int32    -1                    FCVAR_CLIENTDLL|FCVAR_CHEAT
// r_reset_character_decals                                  bool     false                 FCVAR_DEVELOPMENTONLY
// r_ropetranslucent                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_screen_size_expansion                                   int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_shadows                                                 bool     true                  FCVAR_CHEAT
// r_shadowtile_waveops                                      bool     false                 
// r_show_build_info                                         bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Build information. Leave this enabled when submitting bug screenshots and videos, please!
// r_show_time_info                                          bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // Show real time, large.
// r_showdebugoverlays                                       bool     false                 FCVAR_CHEAT  // Set to render debug overlays
// r_showsceneobjectbounds                                   bool     false                 FCVAR_CHEAT  // Show scenesystem object bounding boxes
// r_size_cull_threshold                                     float32  0.33                  FCVAR_DEVELOPMENTONLY  // Threshold of screen size percentage below which objects get culled
// r_size_cull_threshold_fade                                float32  7.5                   FCVAR_DEVELOPMENTONLY  // % above the screen size percentage where we will start fading out (==0 will disable fading).
// r_size_cull_threshold_shadow                              float32  0.2                   FCVAR_CHEAT  // Threshold of shadow map size percentage below which objects get culled
// r_skinning_enabled                                        bool     true                  FCVAR_CHEAT
// r_skip_precache_validation_check                          bool     true                  FCVAR_DEVELOPMENTONLY
// r_smooth_morph_normals                                    bool     false                 FCVAR_RELEASE
// r_spectator_flashbang_opacity                             float32  0.6                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Spectator flash opacity
// r_ssao                                                    bool     true                  FCVAR_DEVELOPMENTONLY  // Set to use screen-space ambient occlusion
// r_ssao_bias                                               float32  2.5                   FCVAR_DEVELOPMENTONLY
// r_ssao_blur                                               bool     true                  FCVAR_DEVELOPMENTONLY
// r_ssao_radius                                             float32  8                     FCVAR_DEVELOPMENTONLY
// r_ssao_strength                                           float32  3                     FCVAR_DEVELOPMENTONLY
// r_strip_invisible_during_sceneobject_update               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_test1_maximum_wait_ms                                   float32  10                    FCVAR_CLIENTDLL|FCVAR_RELEASE
// r_texture_budget_dynamic                                  bool     true                  FCVAR_DEVELOPMENTONLY  // Dynamically adjust texture streaming budget based on GPU memory usage.
// r_texture_budget_threshold                                float32  0.9                   FCVAR_DEVELOPMENTONLY  // Reduce texture memory pool size when this percentage of the budget is full.
// r_texture_budget_update_period                            float32  0.1                   FCVAR_DEVELOPMENTONLY  // Time (in seconds) between updating texture memory budget.
// r_texture_eager_eviction                                  bool     false                 FCVAR_DEVELOPMENTONLY
// r_texture_hookup_uses_threadpool                          bool     true                  FCVAR_DEVELOPMENTONLY  // Async Texture hookup uses its own threadpool instead of the global pool.
// r_texture_lod_scale                                       float32  1                     FCVAR_CHEAT  // Scale factor for requested texture size (texture streaming)
// r_texture_nonstreaming_load                               bool     true                  FCVAR_DEVELOPMENTONLY  // Allow immediately loading mips of textures (when possible) when their headers are loaded, saving IO & reducing latency.
// r_texture_pool_increase_rate                              float32  64                    FCVAR_DEVELOPMENTONLY  // Increase texture memory pool size by this many MB / s when under budget.
// r_texture_pool_reduce_rate                                float32  256                   FCVAR_DEVELOPMENTONLY  // Reduce texture memory pool size by this many MB / s when over budget.
// r_texture_pool_size                                       int32    7036                  FCVAR_DEVELOPMENTONLY  // Total size of the texture pool in MB
// r_texture_stream_max_resolution                           int32    8192                  FCVAR_DEVELOPMENTONLY  // Maximum resolution for top mip level in streaming textures
// r_texture_stream_mip_bias                                 int32    0                     FCVAR_DEVELOPMENTONLY  // Biases the mip level the texture streaming system choses to stream for each texture.
// r_texture_stream_resolution_bias_decrease_rate            float32  0.1                   FCVAR_DEVELOPMENTONLY
// r_texture_stream_resolution_bias_increase_rate            float32  0.05                  FCVAR_DEVELOPMENTONLY
// r_texture_stream_resolution_bias_min                      float32  1                     FCVAR_DEVELOPMENTONLY
// r_texture_stream_resolution_bias_update_period            float32  0.5                   FCVAR_DEVELOPMENTONLY
// r_texture_stream_throttle_amount                          float32  10                    FCVAR_DEVELOPMENTONLY
// r_texture_stream_throttle_count                           int32    3                     FCVAR_DEVELOPMENTONLY
// r_texture_stream_throttle_count_over_budget               int32    1                     FCVAR_DEVELOPMENTONLY
// r_texture_streaming_timesliced                            bool     true                  FCVAR_DEVELOPMENTONLY
// r_texture_streamout_unthrottle_ms                         float32  0.2                   FCVAR_DEVELOPMENTONLY  // After hitting throttling limits for streamout, allow it to continue up to this number of milliseconds.
// r_texturefilteringquality                                 int32    3                     FCVAR_DEVELOPMENTONLY  // 0: Bilinear, 1: Trilinear, 2: Aniso 2x, 3: Aniso 4x, 4: Aniso 8x, 5: Aniso 16x
// r_threaded_particle_creation                              bool     true                  FCVAR_DEVELOPMENTONLY
// r_threaded_particles                                      bool     true                  FCVAR_DEVELOPMENTONLY
// r_threaded_scene_object_update                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_timestamp_query_multiplier                              float32  1                     FCVAR_DEVELOPMENTONLY  // Set the TIMESTAMP query cycle multiplier, for drivers that lie
// r_translucent                                             bool     true                  FCVAR_CHEAT  // Enable rendering of translucent geometry
// r_ui_update_parallel_with_server                          bool     false                 FCVAR_DEVELOPMENTONLY
// r_update_particles_on_render_only_frames                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// r_use_memory_budget_model                                 bool     false                 FCVAR_DEVELOPMENTONLY  // Use a model of GPU memory use to determine budget rather than querying the OS.
// r_validate_texture_streaming                              bool     false                 FCVAR_DEVELOPMENTONLY  // Dumps state of texture streaming at the next frame boundary.
// r_vconsole_foregroundforcerender                          bool     true                  FCVAR_DEVELOPMENTONLY  // When VConsole is in the foreground, force all engine & tools to render
// r_wait_on_present                                         bool     false                 FCVAR_DEVELOPMENTONLY
// r_world_frame_load_threshold_ms                           float32  10                    FCVAR_DEVELOPMENTONLY
// r_world_wind_dir                                          vector3  0.707 0.707 0         FCVAR_DEVELOPMENTONLY
// r_world_wind_frequency_grass                              float32  0.03                  FCVAR_DEVELOPMENTONLY
// r_world_wind_frequency_trees                              float32  0.003                 FCVAR_DEVELOPMENTONLY
// r_world_wind_offset_speed                                 vector3  0.25 0.3 0.2          FCVAR_DEVELOPMENTONLY
// r_world_wind_smooth_time                                  float32  2                     FCVAR_DEVELOPMENTONLY
// r_world_wind_strength                                     float32  40                    FCVAR_DEVELOPMENTONLY
// radarvisdistance                                          float32  1000                  FCVAR_GAMEDLL|FCVAR_CHEAT  // at this distance and beyond you need to be point right at someone to see them
// radarvismaxdot                                            float32  0.996                 FCVAR_GAMEDLL|FCVAR_CHEAT  // how closely you have to point at someone to see them beyond max distance
// radarvismethod                                            int32    1                     FCVAR_GAMEDLL|FCVAR_CHEAT  // 0 for traditional method, 1 for more realistic method
// radarvispow                                               float32  0.4                   FCVAR_GAMEDLL|FCVAR_CHEAT  // the degree to which you can point away from a target, and still see them on radar.
// ragdoll_debug_item_detachment                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// ragdoll_fixup_joint_limits                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Adjusts bone transforms so that physics joints don't appear violated (limits)
// ragdoll_fixup_joint_limits_max_height                     int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Disable ragdoll_fixup_joint_limits on joints too high in the hierarchy because long chains tend to depend on violating limits
// ragdoll_fixup_joint_orientation                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Adjusts bone transforms so that physics joints don't appear violated (orientation)
// ragdoll_fixup_joint_orientation_max_height                int32    10                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Disable ragdoll_fixup_joint_orientation on joints too high in the hierarchy because small differences can massively accumulate (e.g. long chains)
// ragdoll_fixup_joint_translation                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Adjusts bone transforms so that physics joints don't appear violated (translation)
// ragdoll_friction_scale                                    float32  0.6                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ragdoll_gravity_scale                                     float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ragdoll_impact_strength                                   float32  500                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// ragdoll_lru_debug_removal                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ragdoll_lru_min_age                                       float32  10                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ragdoll_move_entity                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ragdoll_override_root_orientation                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// ragdoll_parallel_pose_control                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// ragdoll_prop_settle                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Enable more aggressive ragdoll settling
// ragdoll_prop_sleepaftertime                               float32  4                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // After this many seconds of being basically stationary, the ragdoll will go to sleep.
// ragdoll_prop_sleepdisabletime                             float32  1.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Ragdoll is not allowed to physically sleep until this timer has elapsed.
// ragdoll_resolve_initial_conflict                          bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ragdoll_resolve_separation                                bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ragdoll_scale_sleep_tolerance                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// ragdoll_update_from_weights                               bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// ragdoll_visualize_creation_skeleton                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// ragdoll_vphysics_scale                                    float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // How much we scale physics impacts against the ragdoll.
// rate                                                      int32    786432                FCVAR_ARCHIVE|FCVAR_USERINFO  // Min bytes/sec the host can receive data
// rcon_address                                              string                         FCVAR_DONTRECORD|FCVAR_RELEASE|FCVAR_SERVER_CANNOT_QUERY  // Address of remote server if sending unconnected rcon commands (format x.x.x.x:p) 
// rcon_connected_clients_allow                              bool     true                  FCVAR_REPLICATED|FCVAR_RELEASE  // Allow clients to use rcon commands on server.
// rcon_password                                             string                         FCVAR_DONTRECORD|FCVAR_RELEASE|FCVAR_SERVER_CANNOT_QUERY  // remote console password.
// recast_mark_overhang                                      bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Enable/disable overhang detection
// recast_partitioning                                       int32    0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // 0 = watershed, 1 = monotone, 2 = layers
// replay_debug                                              int32    0                     FCVAR_REPLICATED|FCVAR_RELEASE
// report_cliententitysim                                    bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // List all clientside simulations and time - will report and turn itself off.
// report_clientthinklist                                    bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // List all clientside entities thinking and time - will report and turn itself off.
// report_connection_failure_percentage                      float32  0                     FCVAR_DEVELOPMENTONLY
// reset_voice_on_input_stallout                             bool     false                 FCVAR_USERINFO  // If true, resets the input device when there was a long enough hitch between callbacks.
// rope_averagelight                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Makes ropes use average of cubemap lighting instead of max intensity.
// rope_collide                                              int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Collide rope with the world
// rope_shake                                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// rope_smooth_enlarge                                       float32  1.4                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // How much to enlarge ropes in screen space for antialiasing effect
// rope_smooth_maxalpha                                      float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Alpha for rope antialiasing effect
// rope_smooth_maxalphawidth                                 float32  1.75                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// rope_smooth_minalpha                                      float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Alpha for rope antialiasing effect
// rope_smooth_minwidth                                      float32  0.3                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // When using smoothing, this is the min screenspace width it lets a rope shrink to
// rope_subdiv                                               int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Rope subdivision amount
// rope_wind_dist                                            float32  1000                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Don't use CPU applying small wind gusts to ropes when they're past this distance.
// rr_debugclassname                                         string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // If set, rr_debugclassname will print only response tests where 'classname' corresponds to this variable. Use to filter for a specific character.
// rr_debugresponseconcept                                   string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // If set, rr_debugresponseconcept will print only responses testing for the specified concept
// rr_debugresponses                                         int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Show verbose matching output (1 for simple, 2 for rule scoring, 3 for noisy). If set to 4, it will only show response success/failure for npc_selected NPCs.
// rr_debugrule                                              string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // If set to the name of the rule, that rule's score will be shown whenever a concept is passed into the response rules system.
// rr_followup_maxdist                                       float32  1800                  FCVAR_GAMEDLL|FCVAR_CHEAT  // 'then ANY' or 'then ALL' response followups will be dispatched only to characters within this distance.
// rr_thenany_score_slop                                     float32  0                     FCVAR_GAMEDLL|FCVAR_ARCHIVE|FCVAR_CHEAT  // When computing respondents for a 'THEN ANY' rule, all rule-matching scores within this much of the best score will be considered.
// rtx_dynamic_blas                                          bool     true                  FCVAR_DEVELOPMENTONLY  // Allow dynamic BLAS creation for geometry going through the compute shader skinning path.
// rtx_dynamic_blas_caching                                  bool     true                  FCVAR_DEVELOPMENTONLY  // Cache dynamic BLAS if geometry has not changed
// rtx_force_default_hitgroup                                bool     false                 FCVAR_DEVELOPMENTONLY  // Forces all ray traced geometry to use default hit shaders instead of specialized ones.
// rtx_texture_resolution                                    uint32   512                   FCVAR_DEVELOPMENTONLY  // Sets the texture resolution the raytracer will mark to stream in
// run_voicecontainer_async                                  bool     false                 FCVAR_DEVELOPMENTONLY
// safezonex                                                 float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // The percentage of the screen width that is considered safe from overscan. Cannot result in a width less than the height.
// safezoney                                                 float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // The percentage of the screen height that is considered safe from overscan
// save_async                                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// save_debug_snapshots                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Save/Load debug snapshot data
// save_fake_hitch                                           int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Force a busy wait for the specified number of milliseconds during save to simulate a hitch
// save_history_count                                        int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Keep this many old copies in history of autosaves and quicksaves.
// save_maxarray_spew                                        int32    10                    FCVAR_GAMEDLL|FCVAR_RELEASE  // Max number of array entries to spew when using SaveRestoreIO spewing.
// save_parallel                                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// save_screenshot                                           int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // 0 = none, 1 = non-autosave, 2 = always, 3 = bug_only
// save_spew                                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// save_write_kv3                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Write the KV3 entity data as a text file in the save directory
// saving_enabled                                            bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sc_aggregate_bvh                                          bool     true                  FCVAR_DEVELOPMENTONLY
// sc_aggregate_bvh_threshold                                uint32   128                   FCVAR_DEVELOPMENTONLY
// sc_aggregate_debug_draw_meshlets                          uint32   0                     FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/Aggregates/Visualize Meshlets
// sc_aggregate_debug_draw_meshlets_bounds                   bool     false                 FCVAR_DEVELOPMENTONLY  // Visualize meshlet bounds and cone axis. Mesh shader only.
// sc_aggregate_debug_visualizer                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Aggregates/Debug Visualizer
// sc_aggregate_fragment_merging                             bool     true                  FCVAR_DEVELOPMENTONLY
// sc_aggregate_gpu_culling                                  bool     true                  FCVAR_DEVELOPMENTONLY  // Toggles GPU culling of aggregate meshes
// sc_aggregate_gpu_culling_conservative_bounds              bool     false                 FCVAR_DEVELOPMENTONLY
// sc_aggregate_gpu_culling_show_culled                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/Aggregates/Show GPU Culled Meshes
// sc_aggregate_gpu_occlusion_culling                        bool     true                  FCVAR_DEVELOPMENTONLY
// sc_aggregate_indirect_draw_compaction                     bool     true                  FCVAR_RELEASE  // Use multidrawindirect...count if the driver/hardware supports it
// sc_aggregate_indirect_draw_compaction_threshold           uint32   8                     FCVAR_RELEASE  // Threshold of indirect draws when we will do compaction
// sc_aggregate_instance_streams                             bool     true                  FCVAR_DEVELOPMENTONLY  // Enable instance streams
// sc_aggregate_material_solo                                string                         FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// sc_aggregate_render_mesh_shader                           bool     true                  FCVAR_DEVELOPMENTONLY  // Using mesh shaders if available instead of drawcalls
// sc_aggregate_rtproxy_debug_visualizer                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Aggregates/RT Proxy Debug Visualizer
// sc_aggregate_rtproxy_instanced_geo                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// sc_aggregate_rtproxy_unique_geo                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// sc_aggregate_rtproxy_visualize                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// sc_aggregate_show_outside_vis                             bool     false                 FCVAR_DEVELOPMENTONLY
// sc_allow_dithered_lod                                     bool     true                  FCVAR_DEVELOPMENTONLY  // Allow use of dithered lod transitions
// sc_allow_dynamic_constant_batching                        bool     true                  FCVAR_DEVELOPMENTONLY
// sc_allow_precomputed_vismembers                           bool     true                  FCVAR_DEVELOPMENTONLY
// sc_allow_write_depth_before_blend                         bool     true                  FCVAR_DEVELOPMENTONLY
// sc_barnlight_enable_precomputed_vis                       bool     true                  FCVAR_DEVELOPMENTONLY  // Enable use of precomputed vis membership for lights (requires map restart)
// sc_batch_layer_cb_updates                                 bool     true                  FCVAR_DEVELOPMENTONLY
// sc_cache_envmap_lpv_lookup                                bool     true                  FCVAR_DEVELOPMENTONLY
// sc_clutter_density_full_size                              float32  0.0075                FCVAR_DEVELOPMENTONLY  // Screen-size where clutter will be full density
// sc_clutter_density_none_size                              float32  0.0035                FCVAR_DEVELOPMENTONLY  // Screen-size where clutter will be gone
// sc_clutter_desity_override                                bool     false                 FCVAR_DEVELOPMENTONLY
// sc_clutter_enable                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/Clutter/Draw Clutter
// sc_disableThreading                                       bool     false                 FCVAR_CHEAT
// sc_disable_baked_lighting                                 bool     false                 FCVAR_DEVELOPMENTONLY
// sc_disable_culling_boxes                                  bool     false                 FCVAR_CHEAT
// sc_disable_procedural_layer_rendering                     bool     false                 FCVAR_CHEAT
// sc_disable_shadow_fastpath                                bool     false                 FCVAR_CHEAT
// sc_disable_spotlight_shadows                              bool     false                 FCVAR_CHEAT
// sc_disable_world_materials                                bool     false                 FCVAR_CHEAT
// sc_distancefield_visualize_atlas                          bool     false                 FCVAR_DEVELOPMENTONLY
// sc_dithered_lod_transition_amt                            float32  0.075                 FCVAR_DEVELOPMENTONLY  // Percentage of the transition between two lods we will apply a dither
// sc_draw_aggregate_meshes                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/Aggregates/Draw Aggregates
// sc_dump_lists                                             string                         FCVAR_CHEAT
// sc_enable_discard                                         bool     true                  FCVAR_DEVELOPMENTONLY
// sc_extended_stats                                         bool     false                 FCVAR_CHEAT
// sc_fade_distance_scale_override                           float32  -1                    FCVAR_CHEAT
// sc_force_lod_level                                        int32    -1                    FCVAR_CHEAT
// sc_force_materials_batchable                              bool     false                 FCVAR_CHEAT
// sc_force_single_display_list_per_layer                    bool     false                 FCVAR_DEVELOPMENTONLY
// sc_force_translation_in_projection                        bool     false                 FCVAR_CHEAT  // If enabled, the camera's translation will be included in the projection matrix.
// sc_hdr_enabled_override                                   int32    -1                    FCVAR_DEVELOPMENTONLY  // Override default setting for HDR rendering. -1 default, 0 NoHdr, 1 Hdr, 2 Hdr 1010102 3 Hdr 111110
// sc_imgui_show_debug_log                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Imgui/Show Debug Log
// sc_imgui_show_id_stack                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Imgui/Show ID Stack Tool
// sc_imgui_show_metrics                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Imgui/Show Metrics
// sc_instanced_debug_visualizer                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Instanced/Debug Visualizer
// sc_instanced_gpu_culling_show_culled                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/Instanced/Show GPU Culled Meshlets
// sc_instanced_material_solo                                string                         FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// sc_instanced_mesh_enable                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Instanced/Draw Instanced
// sc_instanced_mesh_gpu_culling                             bool     true                  FCVAR_DEVELOPMENTONLY  // Toggles GPU culling of instanced meshes
// sc_instanced_mesh_gpu_density_culling                     bool     true                  FCVAR_DEVELOPMENTONLY  // Toggles density culling (if enabled)
// sc_instanced_mesh_gpu_occlusion_culling                   bool     true                  FCVAR_DEVELOPMENTONLY  // Toggles GPU occlusion of instanced meshes
// sc_instanced_mesh_gpu_vis_culling                         bool     true                  FCVAR_DEVELOPMENTONLY  // Toggles GPU vis of instanced meshes
// sc_instanced_mesh_lod_bias                                float32  1.25                  FCVAR_DEVELOPMENTONLY  // Bias for LOD selection of instanced meshes
// sc_instanced_mesh_lod_bias_shadow                         float32  1.75                  FCVAR_DEVELOPMENTONLY  // Bias for LOD selection of instanced meshes in shadowmaps
// sc_instanced_mesh_motion_vectors                          bool     true                  FCVAR_DEVELOPMENTONLY  // Toggles motion vector support for instanced meshes
// sc_instanced_mesh_opaque_fade                             bool     true                  FCVAR_DEVELOPMENTONLY  // Toggles fade support for instanced meshes
// sc_instanced_mesh_size_cull_bias                          float32  1.5                   FCVAR_DEVELOPMENTONLY  // Bias for size culling of instanced meshes
// sc_instanced_mesh_size_cull_bias_shadow                   float32  2                     FCVAR_DEVELOPMENTONLY  // Bias for size culling instanced meshes in shadowmaps
// sc_instanced_mesh_solo                                    string                         FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// sc_keep_all_layers                                        bool     false                 FCVAR_DEVELOPMENTONLY
// sc_layer_batch_threshold                                  int32    128                   FCVAR_DEVELOPMENTONLY
// sc_layer_batch_threshold_fullsort                         int32    80                    FCVAR_DEVELOPMENTONLY
// sc_max_framebuffer_copies_per_layer                       int32    1                     FCVAR_DEVELOPMENTONLY
// sc_mesh_backface_culling                                  bool     true                  FCVAR_DEVELOPMENTONLY
// sc_mesh_gpu_occlusion_culling                             bool     true                  FCVAR_DEVELOPMENTONLY
// sc_mesh_gpu_volume_culling                                bool     true                  FCVAR_DEVELOPMENTONLY
// sc_mesh_mesh_shaders                                      bool     false                 FCVAR_DEVELOPMENTONLY
// sc_mesh_shadows_batch_across_materials                    bool     true                  FCVAR_DEVELOPMENTONLY
// sc_mesh_use_pmb                                           bool     false                 FCVAR_DEVELOPMENTONLY
// sc_no_cull                                                bool     false                 FCVAR_DEVELOPMENTONLY
// sc_no_vis                                                 bool     false                 FCVAR_DEVELOPMENTONLY
// sc_only_render_opaque                                     bool     false                 FCVAR_CHEAT
// sc_only_render_shadowcasters                              bool     false                 FCVAR_CHEAT
// sc_particle_debug_visualizer                              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Particles/Debug Visualizer
// sc_reject_all_objects                                     bool     false                 FCVAR_CHEAT
// sc_rendergraph_debug_visualizer                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/RenderGraph Visualizer
// sc_screen_size_lod_scale_override                         float32  -1                    FCVAR_CHEAT
// sc_shadow_depth_bias                                      int32    256                   FCVAR_DEVELOPMENTONLY
// sc_shadow_depth_bias_clamp                                float32  0                     FCVAR_DEVELOPMENTONLY
// sc_shadow_depth_bias_state_override                       bool     false                 FCVAR_DEVELOPMENTONLY
// sc_shadow_slopescale_depth_bias                           float32  2.13                  FCVAR_DEVELOPMENTONLY
// sc_show_cs_skinning_stats                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Compute Skinning Stats
// sc_show_gpu_profiler                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/GPU Profiler
// sc_show_hair_debug_ui                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Hair Debug UI
// sc_show_object_browser                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/SceneObject Browser
// sc_show_texture_visualizer                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/Texture Visualizer
// sc_show_view_profiler                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // SceneSystem/View Profiler
// sc_skip_traversal                                         bool     false                 FCVAR_CHEAT
// sc_spew_cmt_usage                                         bool     false                 FCVAR_DEVELOPMENTONLY
// sc_throw_away_all_layers                                  bool     false                 FCVAR_DEVELOPMENTONLY
// sc_use_clear_subrect                                      bool     false                 FCVAR_DEVELOPMENTONLY
// sc_view_profiler_frame_averaging                          int32    10                    FCVAR_DEVELOPMENTONLY
// sc_visualize_batches                                      int32    0                     FCVAR_DEVELOPMENTONLY  // color per batch
// sc_visualize_sceneobjects                                 string   SCENEOBJECT_VIS_NONE  FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/Visualize SceneObject Mode
// sc_visualize_sceneobjects_meshlets                        string   SCENEOBJECT_MESHLET…  FCVAR_DEVELOPMENTONLY|FCVAR_MENUBAR_ITEM  // SceneSystem/Visualize Base SceneObject Meshlets
// scene_clientflex                                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Do client side flex animation.
// scene_print                                               int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // When playing back a scene, print timing and event info to console.
// scene_vcdautosave                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Create a savegame before VCD playback
// screenmessage_notifytime                                  float32  8                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // How long to display screen message text
// screenmessage_show                                        int32    -1                    FCVAR_CHEAT  // Enable display of console messages on screen. 1 = Enabled, 0 = Disabled, -1 = Enabled if vgui is not present
// screenshot_height                                         int32    -1                    FCVAR_DEVELOPMENTONLY  // Screenshot height. -1 for screen height.
// screenshot_prefix                                         string   shot                  FCVAR_DEVELOPMENTONLY  // Set the screenshot auto naming prefix.
// screenshot_subdir                                         string   screenshots           FCVAR_DEVELOPMENTONLY  // Set the screenshot directory.
// screenshot_width                                          int32    -1                    FCVAR_DEVELOPMENTONLY  // Screenshot width. -1 for screen width.
// script_attach_debugger_at_startup                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// script_break_in_native_debugger_on_error                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sensitivity                                               float32  1.25                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Mouse sensitivity.
// sensitivity_y_scale                                       float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Multiplies the mouse Y axis for finer pitch vs yaw aim
// servercfgfile                                             string   server.cfg            FCVAR_GAMEDLL|FCVAR_RELEASE
// shake_show                                                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Displays a list of the active screen shakes.
// shatterglass_cleanup                                      bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// shatterglass_cleanup_max                                  int32    200                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// shatterglass_debug                                        bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// shatterglass_hit_tolerance                                float32  2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// shatterglass_shard_lifetime                               float32  15                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// show_visibility_boxes                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Enable or Disable debug display of visibility boxes
// silence_dsp                                               bool     false                 FCVAR_CHEAT  // When on, silences all DSP mixes.
// sk_autoaim_mode                                           int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED
// sk_player_arm                                             float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sk_player_chest                                           float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sk_player_head                                            float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sk_player_leg                                             float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sk_player_stomach                                         float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// skel_constraints_enable                                   bool     true                  FCVAR_REPLICATED|FCVAR_CHEAT
// skel_debug                                                string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// skeleton_instance_debug_bodygroups                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Debug bodygroups
// skeleton_instance_lod_optimization                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Compute LOD mask internally like since 2016, i.e. force all LOD groups' bones to compute
// skeleton_instance_scaleset_enable                         bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// skeleton_instance_smear_boneflags                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Smear boneflags across the model.  Costs computation, but tests to make sure your bone flags are consistent.
// skeleton_physics_joint_fixup                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// skill                                                     int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED|FCVAR_PER_USER  // Game skill level.
// slope_drop_enable                                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Toggles a test dropping the view offset based on the slope
// slope_drop_max_offset                                     float32  16                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The maximum distance to adjust the view height
// slope_drop_off_ground_blend_speed                         float32  160                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The speed with which the slope drop is blended out when the entity leaves the ground
// smoke_grenade_ct_color                                    vector3  75 127 155            FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT
// smoke_grenade_t_color                                     vector3  180 129 50            FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT
// smoke_param1                                              float32  6.26                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// smoke_param2                                              float32  8.27                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// smoke_param3                                              float32  0.13                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// smoke_param4                                              float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// smoke_param5                                              int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// smoke_use_noise_texture                                   bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// smoke_volume_lod_ratio_change                             float32  0.6                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// smoothstairs                                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Smooth player eye z coordinate when traversing stairs.
// snd_async_spew_blocking                                   int32    0                     FCVAR_DEVELOPMENTONLY  // Spew message to console any time async sound loading blocks on file i/o.
// snd_autodetect_latency                                    bool     true                  FCVAR_ARCHIVE
// snd_beatpattern_show_bpm                                  bool     false                 FCVAR_CHEAT
// snd_beatpattern_show_events                               bool     false                 FCVAR_CHEAT
// snd_beatpattern_show_quantize_queue                       bool     false                 FCVAR_CHEAT
// snd_beatpattern_use_lookahead                             bool     false                 FCVAR_CHEAT
// snd_boxverb_simd                                          bool     true                  FCVAR_DEVELOPMENTONLY  // Enable SIMD code path for shoebox reverb processor.
// snd_boxverb_simd_svf                                      int32    1                     FCVAR_DEVELOPMENTONLY  // 0 = use biquad instead of svf, 1 = use vectorized svf, 2 = use scalar svf
// snd_break_on_start_soundevent                             string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Use to debug break on any soundevent that is started matching this name
// snd_compare_KV_convert                                    bool     false                 FCVAR_DEVELOPMENTONLY
// snd_deathcamera_volume                                    float32  0.16                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of Deathcam Timers
// snd_delay_sound_ms_max                                    float32  40                    FCVAR_DEVELOPMENTONLY  // Sound device synchronization max delay (ms)
// snd_delay_sound_ms_shift                                  float32  23                    FCVAR_DEVELOPMENTONLY  // Sound device synchronization shift (ms)
// snd_diffusor_simd                                         bool     false                 FCVAR_DEVELOPMENTONLY  // Enable SIMD code path for diffusor processor.
// snd_disable_mixer_duck                                    bool     false                 FCVAR_CHEAT
// snd_disable_mixer_solo                                    bool     false                 FCVAR_CHEAT
// snd_disable_radar_visualize                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// snd_dsp_distance_max                                      float32  2000                  FCVAR_CHEAT
// snd_dsp_distance_min                                      float32  20                    FCVAR_CHEAT
// snd_duckerattacktime                                      float32  0.5                   FCVAR_ARCHIVE
// snd_duckerreleasetime                                     float32  2.5                   FCVAR_ARCHIVE
// snd_duckerthreshold                                       float32  0.15                  FCVAR_ARCHIVE
// snd_ducktovolume                                          float32  0.55                  FCVAR_ARCHIVE
// snd_enable_imgui                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_ARCHIVE|FCVAR_CHEAT|FCVAR_MENUBAR_ITEM  // Game/Sound System Debugger
// snd_enable_subgraph_corenull_passthrough                  bool     true                  FCVAR_DEVELOPMENTONLY
// snd_enable_subgraph_log                                   bool     false                 FCVAR_DEVELOPMENTONLY
// snd_envelope_rate                                         float32  0.9                   FCVAR_CHEAT
// snd_eq_arms_race                                          int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_eq_casual                                             int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_eq_competitive                                        int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_eq_deathmatch                                         int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_eq_spectator                                          int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_eq_warmup                                             int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_event_cone_debug                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_event_oriented_box_debug                              bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_event_oriented_lerp_max_distance                      float32  64                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_event_oriented_lerp_min_distance                      float32  24                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_filter                                                string                         FCVAR_CHEAT
// snd_foliage_db_loss                                       float32  4                     FCVAR_GAMEDLL|FCVAR_CHEAT  // foliage dB loss per 1200 units
// snd_gain                                                  float32  1                     FCVAR_ARCHIVE
// snd_gain_max                                              float32  1                     FCVAR_CHEAT
// snd_gain_min                                              float32  0.01                  FCVAR_CHEAT
// snd_gamevoicevolume                                       float32  1                     FCVAR_ARCHIVE  // Game v.o. volume
// snd_gamevolume                                            float32  1                     FCVAR_ARCHIVE  // Game volume
// snd_group_cluster_debug                                   bool     false                 FCVAR_REPLICATED|FCVAR_CHEAT
// snd_group_occlusion_debug                                 bool     false                 FCVAR_DEVELOPMENTONLY
// snd_group_priority_debug                                  bool     false                 FCVAR_REPLICATED|FCVAR_CHEAT
// snd_group_priority_max_tolerance                          float32  0.05                  FCVAR_REPLICATED|FCVAR_CHEAT
// snd_headphone_eq                                          int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_CLIENTCMD_CAN_EXECUTE  // Select Headphone EQ Preset
// snd_headphone_eq_active                                   int32    0                     FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Select Headphone EQ Preset
// snd_hrtf_distance_behind                                  float32  50                    FCVAR_DEVELOPMENTONLY  // HRTF calculations will calculate the player as being this far behind the camera.
// snd_list                                                  string                         FCVAR_CHEAT
// snd_log_empty_event_entities                              bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Logs the sound event entities that have empty names.
// snd_mainmenu_music_break_time_max                         int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Maximum amount of time to pause between playing main menu music
// snd_mainmenu_music_break_time_min                         int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Minimum amount of time to pause between playing main menu music
// snd_mapobjective_volume                                   float32  0.04                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of Map Objective Music
// snd_max_pitch_shift_inaccuracy                            float32  0.08                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// snd_menumap_volume                                        float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of background sounds for maps
// snd_menumusic_volume                                      float32  0.04                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of Menu / Non-gameplay music
// snd_mergemethod                                           int32    1                     FCVAR_DEVELOPMENTONLY  // Sound merge method (0 == sum and clip, 1 == max, 2 == avg).
// snd_min_latency                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_mix_async                                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_mixahead                                              float32  0.001                 FCVAR_ARCHIVE
// snd_mixer_master_dsp                                      float32  1                     FCVAR_CHEAT
// snd_mixer_master_level                                    float32  1                     FCVAR_CHEAT
// snd_musicvolume                                           float32  1                     FCVAR_ARCHIVE  // Music volume
// snd_mute_losefocus                                        bool     true                  FCVAR_ARCHIVE
// snd_mute_mvp_music_live_players                           bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If set, MVP music is muted if players from both teams are still alive.
// snd_mvp_volume                                            float32  0.16                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of MVP Music
// snd_new_visualize                                         bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays soundevent name played at it's 3d position
// snd_occlusion_bounces                                     int32    1                     FCVAR_REPLICATED|FCVAR_CHEAT
// snd_occlusion_debug                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_occlusion_debug_listener_pos                          string                         FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_occlusion_indirect_max                                float32  0.7                   FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_occlusion_indirect_min                                float32  0.01                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_occlusion_indirect_radius                             float32  120                   FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_occlusion_min_wall_thickness                          float32  4                     FCVAR_REPLICATED|FCVAR_CHEAT
// snd_occlusion_override                                    float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_occlusion_rays                                        int32    4                     FCVAR_REPLICATED|FCVAR_CHEAT
// snd_occlusion_report                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_occlusion_visualize                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_op_test_convar                                        float32  720                   FCVAR_CHEAT
// snd_opvar_set_point_debug                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_opvar_set_point_update_interval                       float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// snd_opvar_set_point_update_interval_fast                  float32  0.0333                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// snd_rear_stereo_scale                                     float32  1                     FCVAR_REPLICATED|FCVAR_CHEAT
// snd_refdb                                                 float32  60                    FCVAR_CHEAT  // Reference dB at snd_refdist
// snd_refdist                                               float32  36                    FCVAR_CHEAT  // Reference distance for snd_refdb
// snd_report_audio_nan                                      bool     false                 FCVAR_RELEASE
// snd_report_c4_sounds                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT
// snd_report_verbose_error                                  bool     false                 FCVAR_CHEAT  // If set to 1, report more error found when playing sounds.

// snd_roundaction_volume                                    float32  0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of Move Action Music
// snd_roundend_volume                                       float32  0.16                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of Won/Lost Music
// snd_roundstart_volume                                     float32  0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of Round Start Music
// snd_showclassname                                         int32    0                     FCVAR_CHEAT
// snd_showstart                                             int32    0                     FCVAR_CHEAT
// snd_sos_beatpattern_show_operator_updates                 bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_block_global_stack                                bool     false                 FCVAR_CHEAT
// snd_sos_block_stop_global_stack                           bool     true                  FCVAR_CHEAT
// snd_sos_calc_angle_debug                                  bool     false                 FCVAR_REPLICATED|FCVAR_CHEAT
// snd_sos_debug_trigger_opvar                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// snd_sos_enable_nan_check                                  bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_hide_simple_parameter_overwrite_warnings          bool     true                  FCVAR_DEVELOPMENTONLY
// snd_sos_ingame_debug                                      bool     false                 FCVAR_CHEAT
// snd_sos_limit_self                                        bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_list_operator_updates                             bool     false                 FCVAR_CHEAT
// snd_sos_max_event_base_depth                              int32    4                     FCVAR_DEVELOPMENTONLY
// snd_sos_opvar_debug                                       bool     false                 FCVAR_CHEAT
// snd_sos_pause_system                                      bool     false                 FCVAR_CHEAT
// snd_sos_print_addfield_dupes                              bool     false                 FCVAR_CHEAT
// snd_sos_print_field_references                            bool     false                 FCVAR_CHEAT
// snd_sos_print_fps                                         bool     false                 FCVAR_CHEAT
// snd_sos_print_frametime                                   bool     false                 FCVAR_CHEAT
// snd_sos_print_full_field_info                             bool     false                 FCVAR_CHEAT
// snd_sos_print_table_arrays                                bool     false                 FCVAR_CHEAT
// snd_sos_report_entity_deleted                             bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_show_block_debug                                  bool     false                 FCVAR_CHEAT  // Spew data about the list of block entries.
// snd_sos_show_entry_match_free                             bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_show_mixgroup_path_errors                         bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_show_operator_event_and_stack                     bool     true                  FCVAR_CHEAT
// snd_sos_show_operator_event_filter                        string                         FCVAR_CHEAT
// snd_sos_show_operator_field_filter                        string                         FCVAR_CHEAT
// snd_sos_show_operator_init                                bool     false                 FCVAR_CHEAT
// snd_sos_show_operator_not_executing                       bool     true                  FCVAR_CHEAT
// snd_sos_show_operator_operator_filter                     string                         FCVAR_CHEAT
// snd_sos_show_operator_pause_entry                         bool     false                 FCVAR_CHEAT
// snd_sos_show_operator_shutdown                            bool     false                 FCVAR_CHEAT
// snd_sos_show_operator_stop_entry                          bool     false                 FCVAR_CHEAT
// snd_sos_show_operator_updates                             bool     false                 FCVAR_CHEAT
// snd_sos_show_opfield_cache_updates                        bool     false                 FCVAR_CHEAT
// snd_sos_show_opvar_updates                                bool     false                 FCVAR_CHEAT
// snd_sos_show_opvar_updates_filter                         string                         FCVAR_CHEAT
// snd_sos_show_parameter_overwrite_value_comparisons        bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_show_parameter_overwrite_warnings                 bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_show_queuetotrack                                 bool     false                 FCVAR_CHEAT
// snd_sos_show_soundevent_overwrites                        bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_show_soundevent_param_overwrite                   bool     false                 FCVAR_CHEAT
// snd_sos_show_soundevent_start                             bool     false                 FCVAR_CHEAT
// snd_sos_show_track_list                                   bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_show_voice_elapsed_time                           bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_soundevent_constellation_debug                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_sos_soundevent_constellation_replenish_max_fraction   float32  0.3                   FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_sos_soundevent_deferred_interval_time                 float32  0.1                   FCVAR_DEVELOPMENTONLY
// snd_sos_soundevent_filter                                 string                         FCVAR_CHEAT
// snd_sos_soundevent_max_deferred_time                      float32  5                     FCVAR_DEVELOPMENTONLY
// snd_sos_soundevent_show_deferral_warning                  bool     false                 FCVAR_DEVELOPMENTONLY
// snd_sos_tools_detailed_debugging                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_ARCHIVE
// snd_sound_areas_debug                                     bool     false                 FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_sound_areas_debug_interval                            float32  0.2                   FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// snd_soundmixer                                            string   Default_Mix           FCVAR_DEVELOPMENTONLY
// snd_soundmixer_update_maximum_frame_rate                  int32    10                    FCVAR_CHEAT
// snd_soundmixer_version                                    int32    2                     FCVAR_DEVELOPMENTONLY
// snd_spatialize_lerp                                       float32  0                     FCVAR_ARCHIVE|FCVAR_RELEASE
// snd_steamaudio_active_hrtf                                uint32   0                     FCVAR_DEVELOPMENTONLY  // Index of active HRTF.
// snd_steamaudio_ambisonics_correction_type                 int32    1                     FCVAR_DEVELOPMENTONLY  // 0: Use order-truncated Ambisonics HRTF without correction. 1: Use diffuse field equalization to correct order-truncated Ambisonics HRTFs.
// snd_steamaudio_baked_occlusion_mode                       int32    4                     FCVAR_CHEAT  // 0: distance ratio only. 1: deviation only (1/r). 2: deviation only (linear). 3: Mode 0 and Mode 1, 4: Mode 0 and Mode 2
// snd_steamaudio_baked_occlusion_pathing_exponent           float32  0.2                   FCVAR_CHEAT  // Adjust scaling of pathing energy by exponentiall scaling with pathing delta.
// snd_steamaudio_baked_occlusion_reflection_factor          float32  1                     FCVAR_CHEAT  // Adjust scaling for reflection energy when combining it to calculate occlusion value.
// snd_steamaudio_default_hrtf_volume_gain                   float32  0                     FCVAR_DEVELOPMENTONLY  // Adjust overall volume of the default HRTF by the specified gain (dB).
// snd_steamaudio_display_dimension_data_inside              bool     true                  FCVAR_DEVELOPMENTONLY  // When visualizing dimensions data at runtime, draw the inside direction.
// snd_steamaudio_display_dimension_data_outside             bool     true                  FCVAR_DEVELOPMENTONLY  // When visualizing dimensions data at runtime, draw the outisde direction.
// snd_steamaudio_display_dimension_data_size                bool     true                  FCVAR_DEVELOPMENTONLY  // When visualizing dimensions data at runtime, draw the size of the space.
// snd_steamaudio_dynamicpathing_max_samples                 int32    16                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// snd_steamaudio_enable_compressed_reverb_lookup            bool     true                  FCVAR_DEVELOPMENTONLY  // Use compressed reverb data if available.
// snd_steamaudio_enable_custom_hrtf                         bool     false                 FCVAR_DEVELOPMENTONLY  // Enable custom HRTF loading.
// snd_steamaudio_enable_editor_profiling                    bool     false                 FCVAR_DEVELOPMENTONLY  // Enables tracy profiling when baking steam audio data in Hammer.
// snd_steamaudio_enable_occlusion_caching                   bool     true                  FCVAR_DEVELOPMENTONLY  // Continue using previous occlusion if probe lookup for occlusion fails.
// snd_steamaudio_enable_ordertruncated_ambisonics_hrtf      bool     false                 FCVAR_DEVELOPMENTONLY  // Enable order-truncated Ambisonics HRTF loading.
// snd_steamaudio_enable_pathing                             bool     true                  FCVAR_CHEAT  // This variable is checked by soundstack to globally enabling pathing for audio processing.
// snd_steamaudio_enable_perspective_correction              bool     true                  FCVAR_ARCHIVE|FCVAR_RELEASE  // Enable perspective correction for 3D audio.
// snd_steamaudio_enable_probeneighborhood_caching           bool     true                  FCVAR_DEVELOPMENTONLY  // Enable caching listener probe neighborhood for pathing.
// snd_steamaudio_enable_reverb                              float32  0                     FCVAR_RELEASE  // Enable Steam Audio Reverb processor.
// snd_steamaudio_enable_reverb_caching                      bool     true                  FCVAR_DEVELOPMENTONLY  // Continue using previous reverb if probe lookup for reverb fails.
// snd_steamaudio_hybrid_reverb_overlap                      float32  0.25                  FCVAR_DEVELOPMENTONLY  // Set the overlap fraction (0 to 1) for hybrid reverb.
// snd_steamaudio_hybrid_reverb_transition_time              float32  1                     FCVAR_DEVELOPMENTONLY  // Set the transition time (in seconds) between convolution and parametric reverb.
// snd_steamaudio_invalid_path_length                        float32  0                     FCVAR_DEVELOPMENTONLY  // Path length of invalid path in inches. Set this to .0 to use direct distance as path length.
// snd_steamaudio_ir_duration                                float32  2                     FCVAR_CHEAT  // The time delay between a sound being emitted and the last audible reflection in Steam Audio.
// snd_steamaudio_irradiancemindistance_reverb               float32  1                     FCVAR_DEVELOPMENTONLY  // Minimum distance (in meters) of a source from a surface for the purposes of energy calculations.
// snd_steamaudio_load_dimensions_data                       bool     false                 FCVAR_DEVELOPMENTONLY  // If set, baked dimensions data is loaded.
// snd_steamaudio_load_materials_data                        bool     false                 FCVAR_DEVELOPMENTONLY  // If set, baked materials data is loaded.
// snd_steamaudio_load_occlusion_data                        bool     false                 FCVAR_DEVELOPMENTONLY  // If set, baked occlusion data is loaded.
// snd_steamaudio_load_pathing_data                          bool     false                 FCVAR_DEVELOPMENTONLY  // If set, baked pathing data is loaded. Steam Audio Hammer entities can successfully use pathing in this case.
// snd_steamaudio_load_reverb_data                           bool     false                 FCVAR_DEVELOPMENTONLY  // If set, baked reverb data is loaded. Reset it to zero during an format changes of baked data until all data is updated.
// snd_steamaudio_max_ambisonics_order                       int32    3                     FCVAR_DEVELOPMENTONLY  // Maximum order for Ambisonics binaural rendering and Ambisonics panning.
// snd_steamaudio_max_convolution_sources                    int32    4                     FCVAR_CHEAT  // The maximum number of simultaneous sources that can be modeled by Steam Audio.
// snd_steamaudio_max_hrtf_normalization_gain_db             float32  6                     FCVAR_DEVELOPMENTONLY  // Maximum gain any HRTF could have during volume normalization.
// snd_steamaudio_max_occlusion_samples                      int32    64                    FCVAR_CHEAT  // The maximum number of rays that can be traced for volumetric occlusion by Steam Audio.
// snd_steamaudio_max_probes_customdata                      uint32   12000                 FCVAR_DEVELOPMENTONLY  // Maximum number of probes to create when baking custom data (occlusion).
// snd_steamaudio_max_probes_customdata_dimensions           uint32   4294967295            FCVAR_DEVELOPMENTONLY  // Maximum number of probes to create when baking custom data (dimensions).
// snd_steamaudio_max_probes_customdata_materials            uint32   4294967295            FCVAR_DEVELOPMENTONLY  // Maximum number of probes to create when baking custom data (occlusion).
// snd_steamaudio_max_probes_pathing                         uint32   4294967295            FCVAR_DEVELOPMENTONLY  // Maximum number of probes to create when baking paths.
// snd_steamaudio_max_probes_reverb                          uint32   4294967295            FCVAR_DEVELOPMENTONLY  // Maximum number of probes to create when baking reverb.
// snd_steamaudio_normalize_default_hrtf_volume              bool     true                  FCVAR_DEVELOPMENTONLY  // Normalize volume of default HRTF dataset across all directions.
// snd_steamaudio_num_bounces                                int32    128                   FCVAR_CHEAT  // The maximum number of times any ray can bounce when using Steam Audio.
// snd_steamaudio_num_diffuse_samples                        int32    2048                  FCVAR_CHEAT  // The number of directions considered for ray bounce by Steam Audio.
// snd_steamaudio_num_rays                                   int32    65536                 FCVAR_CHEAT  // The number of rays to trace for reflection modeling by Steam Audio.
// snd_steamaudio_num_threads                                int32    2                     FCVAR_CHEAT  // Sets the number of threads used for realtime reflection by Steam Audio.
// snd_steamaudio_pathing_caching_threshold                  float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// snd_steamaudio_pathing_enable_caching                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// snd_steamaudio_pathing_enable_source_probe_interp         bool     false                 FCVAR_DEVELOPMENTONLY  // If set, all the probes near a source withing probe range are used to find paths instead of nearest probe.
// snd_steamaudio_pathing_order                              int32    1                     FCVAR_CHEAT  // The amount of directional detail in the simulated by Steam Audio.
// snd_steamaudio_pathing_order_rendering                    int32    1                     FCVAR_CHEAT  // The amount of directional detail in the rendered audio by Steam Audio.
// snd_steamaudio_perspective_correction_factor              float32  1                     FCVAR_DEVELOPMENTONLY  // Perspective correction factor, ratio of screen size and view depth from screen, for 3D audio.
// snd_steamaudio_perspective_correction_front_only          bool     true                  FCVAR_DEVELOPMENTONLY  // Use perspective correction for 3D audio only in the frontal directions.
// snd_steamaudio_reverb_level_db                            float32  -3                    FCVAR_RELEASE  // Adjust overall volume (dB) of the output from Steam Audio Reverb processor.
// snd_steamaudio_reverb_order                               int32    1                     FCVAR_DEVELOPMENTONLY  // Ambisonics order to use for simulating reverb.
// snd_steamaudio_reverb_order_rendering                     int32    1                     FCVAR_DEVELOPMENTONLY  // Ambisonics order to use for convolution reverb. 0th order = 1 channel, 1st order = 4 channels.
// snd_steamaudio_reverb_update_rate                         float32  30                    FCVAR_DEVELOPMENTONLY  // Set the maximum update rate (in Hz) for reverb.
// snd_steamaudio_source_pathing_debug                       bool     false                 FCVAR_ARCHIVE  // Enable path visualization for steam_audio_source operator.
// snd_steamaudio_source_pathing_debug_duration              float32  0.01                  FCVAR_DEVELOPMENTONLY  // Duration for which path remains visible. Should be close to update rate of the sound operator stack.
// snd_steamaudio_source_pathing_enable_validation           bool     false                 FCVAR_DEVELOPMENTONLY  // Enable real-time pathing validation against dynamic geometry.
// snd_steamaudio_use_soundblocking_shapes_only              bool     false                 FCVAR_DEVELOPMENTONLY  // If set, OBJ export and Hammer visualization only shows sound blocking geometry used for baking simulation and runtime queries.
// snd_surf_volume_inair                                     float32  0.5                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // The volume of the wind when surfing.
// snd_surf_volume_map                                       float32  0.3                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // The volume of ambient sounds when surfing is enabled.
// snd_surf_volume_slide                                     float32  0.5                   FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // The volume of sliding along surfaces when surfing.
// snd_tensecondwarning_volume                               float32  0.04                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Volume of Ten Second Warnings
// snd_toolvolume                                            float32  0.9                   FCVAR_ARCHIVE  // Volume of sounds in tools (e.g. Hammer, SFM)
// snd_ui_positional                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_ui_spatialization_spread                              float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT
// snd_use_baked_occlusion                                   float32  0                     FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// snd_vmix_override_mix_decay_time                          float32  -1                    FCVAR_CHEAT  // If set > 0, overrides how long the decay time is on all mix graphs (in seconds).

// snd_voipvolume                                            float32  1                     FCVAR_ARCHIVE  // Voice volume
// snd_vol_arms_race                                         float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_vol_casual                                            float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_vol_competitive                                       float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_vol_deathmatch                                        float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_vol_per_game_mode                                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_vol_spectator                                         float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// snd_vol_warmup                                            float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_ARCHIVE
// sos_debug_emit                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sos_use_guid_filter                                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sound_device_override                                     string                         FCVAR_ARCHIVE|FCVAR_RELEASE  // ID of the sound device to use
// soundevent_check_networked_entity                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// soundpatch_captionlength                                  float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How long looping soundpatch captions should display for.
// soundscape_debug                                          bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // When on, draws lines to all env_soundscape entities. Green lines show the active soundscape, red lines show soundscapes that aren't in range, and white lines show soundscapes that are in range, but not the active soundscape.
// soundscape_fadetime                                       float32  3                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Time to crossfade sound effects between soundscapes
// soundscape_message                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// soundscape_radius_debug                                   bool     false                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Prints current volume of radius sounds
// soundscape_update_include_bots                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // Enable to calculate soundscape audio params for bots.
// soundsystem_device_used                                   string   HISENSE (NVIDIA Hig…  FCVAR_DEVELOPMENTONLY  // Sound device in use (changing this does not change the soundsystem).
// soundsystem_update_async                                  bool     true                  FCVAR_DEVELOPMENTONLY
// sparseshadowtree_cascade_mask                             int32    4                     FCVAR_DEVELOPMENTONLY  // Bitfield describing which cascades to generate/use SST for. (OR'd 1UL<<cascadeIndex, default is 1UL<<2 only, i.e. just cascade 2)
// sparseshadowtree_copy_to_shadow_atlas_ps                  bool     true                  FCVAR_DEVELOPMENTONLY  // Copy layer from CS output to shadow atlas uses PS copy (vs CopyTexture).
// sparseshadowtree_cs_debug_colors                          bool     false                 FCVAR_DEVELOPMENTONLY  // Output debug colors for SST CS.
// sparseshadowtree_cs_exclude_next_cascade_region           bool     true                  FCVAR_DEVELOPMENTONLY  // Exclude the inner region of a cascade during CS unpack if there is a higher resolution cascade that will cover that area.
// sparseshadowtree_cs_unpack_mode                           int32    1                     FCVAR_DEVELOPMENTONLY  // Unpack mode in cs, 0 - one leaf per thread (16 output pixels), 1 (default) - one leaf row per thread (4 output pixels), 2 - one pixel out per thread.
// sparseshadowtree_debug_tile_range_xmax                    int32    1                     FCVAR_DEVELOPMENTONLY  // SST Tile range for renderdoc/debug capturing.
// sparseshadowtree_debug_tile_range_xmin                    int32    0                     FCVAR_DEVELOPMENTONLY  // SST Tile range for renderdoc/debug capturing.
// sparseshadowtree_debug_tile_range_ymax                    int32    1                     FCVAR_DEVELOPMENTONLY  // SST Tile range for renderdoc/debug capturing.
// sparseshadowtree_debug_tile_range_ymin                    int32    0                     FCVAR_DEVELOPMENTONLY  // SST Tile range for renderdoc/debug capturing.
// sparseshadowtree_disable_add_layers                       bool     false                 FCVAR_DEVELOPMENTONLY  // Disable SST runtime layers, for debugging (will exclude geo that CAN render into SST if SST otherwise enabled)
// sparseshadowtree_disable_for_viewmodel                    bool     true                  FCVAR_DEVELOPMENTONLY  // Disable SST generation and runtime for viewmodel (use original CSM rendering).
// sparseshadowtree_enable_rendering                         bool     true                  FCVAR_DEVELOPMENTONLY  // Enable use of SST at runtime (static geo rendered into cascades via SST).
// sparseshadowtree_leaf_compress_scaleoffset                bool     true                  FCVAR_DEVELOPMENTONLY  // Compress leaf node depths using scale & offset.
// sparseshadowtree_leaf_precision                           float32  0.0004                FCVAR_DEVELOPMENTONLY  // precision for depth compression at SST leaf nodes.
// sparseshadowtree_leaf_precision_viewmodel                 float32  0.0005                FCVAR_DEVELOPMENTONLY  // (viewmodel) precision for depth compression at SST leaf nodes.
// sparseshadowtree_parallel_generation                      int32    2                     FCVAR_DEVELOPMENTONLY  // Split SST tile generation into threadjobs (0 - disabled, 1 - wait on readpixels for job batch, 2 - async readpixels).
// sparseshadowtree_plane_incr_per_step                      float32  0.0001                FCVAR_DEVELOPMENTONLY  // depth to increment candidate plane values per iteration to satisfy selection.
// sparseshadowtree_plane_incr_per_step_viewmodel            float32  0.0025                FCVAR_DEVELOPMENTONLY  // (viewmodel) depth to increment candidate plane values per iteration to satisfy selection.
// sparseshadowtree_plane_max_error                          float32  0.0004                FCVAR_DEVELOPMENTONLY  // max error (distance away in depth) candidate plane is allowed before rejecting.
// sparseshadowtree_plane_max_error_viewmodel                float32  0.01                  FCVAR_DEVELOPMENTONLY  // (viewmodel) max error (distance away in depth) candidate plane is allowed before rejecting.
// sparseshadowtree_plane_num_iter                           int32    5                     FCVAR_DEVELOPMENTONLY  // number of steps to push candidate plane behind depths.
// sparseshadowtree_render_cables                            bool     false                 FCVAR_DEVELOPMENTONLY  // Render cables into SST.
// sparseshadowtree_renderdoc_capture_generation             bool     false                 FCVAR_DEVELOPMENTONLY  // Capture dual shadow maps during sparseshadowtree generation.
// sparseshadowtree_unpack_direct_to_shadow_atlas            bool     false                 FCVAR_DEVELOPMENTONLY  // unpack SST directly into shadow atlas cascade vs via staging texture PS copy (NOTE - rendersystem fix reqd for AMD + driver fix required for NV + VK only.
// sparseshadowtree_uv_frac_offset_x                         float32  0                     FCVAR_DEVELOPMENTONLY  // uv x offset during copy to cascade.
// sparseshadowtree_uv_frac_offset_y                         float32  0                     FCVAR_DEVELOPMENTONLY  // uv y offset during copy to cascade.
// spawngroup_ignore_timeouts                                bool     false                 FCVAR_DEVELOPMENTONLY
// speaker_config                                            int32    -1                    FCVAR_ARCHIVE
// spec_autodirector                                         bool     false                 FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Auto-director chooses best view modes while spectating
// spec_autodirector_cameraman                               int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// spec_centerchasecam                                       bool     false                 FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Looks at the target player's center, instead of his eye position, in chase came mode
// spec_chasedistance                                        float32  96                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Chase cam's ideal distance from target
// spec_chasedistancespeed                                   float32  144                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Chase cam's ideal distance from target
// spec_death_panel_replay_position                          float32  0.75                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// spec_freeze_deathanim_time                                float32  0.8                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The time that the death cam will spend watching the player's ragdoll before going into the freeze death cam.
// spec_freeze_time                                          float32  3                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Time spend frozen in observer freeze cam.
// spec_freeze_time_lock                                     float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Time players are prevented from skipping the freeze cam
// spec_freeze_traveltime                                    float32  0.3                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Time taken to zoom in to frame a target in observer freeze cam.
// spec_glow_decay_time                                      float32  2                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Time to decay glow from 1.0 to spec_glow_silent_factor after spec_glow_full_time.
// spec_glow_full_time                                       float32  1                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Noisy players stay at full brightness for this long.
// spec_glow_silent_factor                                   float32  0.4                   FCVAR_CLIENTDLL|FCVAR_RELEASE  // Lurking player xray glow scaling.
// spec_glow_spike_factor                                    float32  1.2                   FCVAR_CLIENTDLL|FCVAR_RELEASE  // Noisy player xray glow scaling (pop when noise is made).  Make >1 to add a 'spike' to noise-making players
// spec_glow_spike_time                                      float32  0                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Time for noisy player glow 'spike' to show that they made noise very recently.
// spec_lock_to_accountid                                    uint32   0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // As an observer, lock the spectator target to the given accountid.
// spec_replay_autostart                                     bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Auto-start Killer Replay when available
// spec_replay_bot                                           bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Enable Spectator Hltv Replay when killed by bot
// spec_replay_cache_ragdolls                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // when set to 0, ragdolls will settle dynamically before and after Killer Replay
// spec_replay_colorcorrection                               float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Amount of color correction in deathcam replay
// spec_replay_enable                                        int32    0                     FCVAR_REPLICATED|FCVAR_RELEASE  // Enable Killer Replay, requires hltv server running (0:off, 1:default, 2:force)
// spec_replay_fadein                                        float32  0.75                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Amount of time in seconds it takes to visually fade into replay, or into real-time after replay
// spec_replay_fadeout                                       float32  0.75                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Amount of time in seconds it takes to visually fade out of replay, or out of real-time before replay
// spec_replay_fullframe                                     bool     true                  FCVAR_DEVELOPMENTONLY  // Send full frame on every hltv replay transition
// spec_replay_leadup_time                                   float32  5.3438                FCVAR_REPLICATED|FCVAR_RELEASE  // Replay time in seconds before the highlighted event
// spec_replay_message_time                                  float32  9.5                   FCVAR_REPLICATED|FCVAR_RELEASE  // How long to show the message about Killer Replay after death. The best setting is a bit shorter than spec_replay_autostart_delay + spec_replay_leadup_time + spec_replay_winddown_time
// spec_replay_on_death                                      bool     false                 FCVAR_REPLICATED|FCVAR_RELEASE  // When > 0, sets the mode whereas players see delayed replay, and are segregated into a domain of chat and voice separate from the alive players
// spec_replay_others_experimental                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Replay the last death of the round, if possible. Disabled on official servers by default. Experimental.
// spec_replay_outline                                       int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Enable outline selecting victim in hltv replay: 0 - none; 1 - ouline YOU; 2 - outline YOU, with red ragdoll outline; 3 - normal spectator outlines
// spec_replay_rate_base                                     float32  1                     FCVAR_REPLICATED|FCVAR_RELEASE  // Base time scale of Killer Replay.Experimental.
// spec_replay_rate_limit                                    float32  3                     FCVAR_REPLICATED|FCVAR_RELEASE  // Minimum allowable pause between replay requests in seconds
// spec_replay_rate_slowdown                                 float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // The part of Killer Replay right before death is played at this rate
// spec_replay_rate_slowdown_length                          float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // The part of Killer Replay right before death is played at this rate
// spec_replay_review_sound                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // When set to non-0, a sound effect is played during Killer Replay
// spec_replay_round_delay                                   float32  0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Round can be delayed by this much due to someone watching a replay; must be at least 3-4 seconds, otherwise the last replay will always be interrupted by round start, assuming normal pause between round_end and round_start events (7 seconds) and freezecam 
// spec_replay_sound_fadein                                  float32  0.05                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Amount of time in seconds it takes to fade in the audio before or after replay
// spec_replay_sound_fadeout                                 float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Amount of time in seconds it takes to fade out the audio before or after replay
// spec_replay_victim_pov                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Killer Replay - replay from victim's point of view (1); the default is killer's (0). Experimental.
// spec_replay_winddown_time                                 float32  2                     FCVAR_GAMEDLL|FCVAR_RELEASE  // The trailing time, in seconds, of replay past the event, including fade-out
// spec_show_xray                                            int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // If set to 1, you can see player outlines and name IDs through walls - who you can see depends on your team and mode
// spec_track                                                int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Tracks an entity in spec mode
// spec_usenumberkeys_nobinds                                bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // If set to 1, map voting and spectator view use the raw number keys instead of the weapon binds (slot1, slot2, etc).
// splitscreen_mode                                          int32    0                     FCVAR_ARCHIVE|FCVAR_CHEAT
// ss_mimic                                                  int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // Split screen users mimic base player's CUserCmds
// ss_voice_hearpartner                                      bool     false                 FCVAR_DEVELOPMENTONLY  // Route voice between splitscreen players on same system.
// stats_collect_gpu                                         bool     false                 FCVAR_DEVELOPMENTONLY  // While doing stats_display, collect GPU perf counters. Used for stats_print_gpu.
// stats_display                                             int32    0                     FCVAR_DEVELOPMENTONLY  // Displays perf statistics information
// stats_highlight_interval                                  float32  10                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Interval between hightlight screens in the transition stats panel
// steam_controller_haptics                                  bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE
// steamaudio_customdata_dimensions_numrays                  int32    32768                 FCVAR_DEVELOPMENTONLY  // Number of rays to trace for estimating inside outside status of a probe.
// steamworks_sessionid_client                               string   8945948787629         FCVAR_CLIENTDLL|FCVAR_USERINFO  // The client session ID for the new steamworks gamestats.
// steamworks_sessionid_server                               string   8945948782929         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The server session ID for the new steamworks gamestats.
// sticky_tooltips                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Don't ever hide tooltips. Helpful when debugging complicated tooltip layouts.
// surf_speed_fast                                           float32  3000                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Speed above which a player is considered to be going fast.
// surf_speed_med                                            float32  2000                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Speed above which a player is considered to be going medium.
// surf_speed_slow                                           float32  50                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Speed above which a player is considered to be going slow.
// suspicious_hit_odds_threshold                             float32  0.01                  FCVAR_GAMEDLL|FCVAR_RELEASE
// suspicious_hit_player_radius                              float32  8                     FCVAR_GAMEDLL|FCVAR_RELEASE
// suspicious_hit_strategy                                   uint32   0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // What to do about suspicious hits. 0: Nothing. 1: Skip the bullet. 2: Skip the bullet and re-roll a new bullet.
// sv_accelerate                                             float32  5.5                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_accelerate_debug_speed                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_accelerate_use_weapon_speed                            bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_ag2_record_entity_graph                                string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Automatically start AG2 recording when an entity with this name (wildcard) or id is created.
// sv_air_max_wishspeed                                      float32  30                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_airaccelerate                                          float32  12                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_allchat                                                bool     true                  FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_RELEASE  // Players can receive all other players' text chat, no death restrictions
// sv_allow_annotations_access_level                         int32    1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // 0:off | 1: view-only | 2: edit.
// sv_allow_ground_weapon_pickup                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_allow_switching_weapon_handedness                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_allow_votes                                            bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Allow voting?
// sv_alltalk                                                bool     false                 FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_RELEASE  // Players can hear all other players' voice communication, no team restrictions
// sv_annotation_limits_max_rounds_per_half                  int32    -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Hard limit on maximum number of rounds (per half) that annotations can be seen in a live match
// sv_auto_adjust_bot_difficulty                             bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Adjust the difficulty of bots each round based on contribution score.
// sv_auto_cstrafe_attempt_window                            int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // The length of the window of trailing counter-strafe attempts considered during input automation detection.
// sv_auto_cstrafe_kick                                      bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Whether or not to kick players when counter-strafe input automation is detected.
// sv_auto_cstrafe_logging                                   int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // 0: never, 1: every time counter-strafe input automation is detected, 2: every counter-strafe
// sv_auto_cstrafe_lower_overlap_pct_threshold               float32  0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // The percentage of overlapping attempts in the attempt window below which input automation detection is triggered at the success threshold.
// sv_auto_cstrafe_min_attempts                              int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // The minimum number of counter-strafe attempts required for input automation detection. The player must be moving more than 135.2 units/s for their counter-strafe to be considered an attempt. An attempt is either considered a success (counter-strafing took 
// sv_auto_cstrafe_sequence_length                           int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // The length of sequential counter-strafe attempts evaluated relative to the success threshold. Input automation detection considers the best sequence within the larger attempt window.
// sv_auto_cstrafe_success_threshold                         int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // The minimum number of successful counter-strafes within a best sequence that will trigger input automation detection. The number of successes that trigger input automation detection is interpolated between the success threshold and a 'perfect' sequence (al
// sv_auto_cstrafe_upper_overlap_pct_threshold               float32  0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // The percentage of overlapping attempts in the attempt window below which input automation detection is triggered when all counter-strafes in a sequence are successes.
// sv_auto_full_alltalk_during_warmup_half_end               bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // When enabled will automatically turn on full all talk mode in warmup, at halftime and at the end of the match
// sv_autobunnyhopping                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Players automatically re-jump while holding jump button
// sv_autobuyammo                                            bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Enable automatic ammo purchase when inside buy zones during buy periods
// sv_autoexec_mapname_cfg                                   bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Execute a mapname cfg file on the server automatically in custom game modes that require it.
// sv_autosave                                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // Set to 1 to autosave game on level transition. Does not affect autosave triggers.
// sv_backspeed                                              float32  0.6                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How much to slow down backwards motion
// sv_banid_enabled                                          bool     true                  FCVAR_RELEASE  // Whether server supports banid command
// sv_bhop_time_window                                       float32  0.0078125             FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // sv_legacy_jump disabled only: The time window (in seconds) around landing where a jump press is considered a bhop attempt.
// sv_bot_buy_decoy_weight                                   float32  1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Given a bot will buy a grenade, controls the odds of the grenade type. Proportional to all other sv_bot_buy_*_weight convars.
// sv_bot_buy_flash_weight                                   float32  1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Given a bot will buy a grenade, controls the odds of the grenade type. Proportional to all other sv_bot_buy_*_weight convars.
// sv_bot_buy_grenade_chance                                 float32  33                    FCVAR_GAMEDLL|FCVAR_RELEASE  // Chance bots will buy a grenade with leftover money (after prim, sec and armor). Input as percent (0-100.0)
// sv_bot_buy_hegrenade_weight                               float32  6                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Given a bot will buy a grenade, controls the odds of the grenade type. Proportional to all other sv_bot_buy_*_weight convars.
// sv_bot_buy_molotov_weight                                 float32  1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Given a bot will buy a grenade, controls the odds of the grenade type. Proportional to all other sv_bot_buy_*_weight convars.
// sv_bot_buy_smoke_weight                                   float32  1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Given a bot will buy a grenade, controls the odds of the grenade type. Proportional to all other sv_bot_buy_*_weight convars.
// sv_bot_difficulty_kbm                                     float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Bot difficulty while playing with Keyboard/Mouse device
// sv_bot_parallel_threat_detection                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Perform bot threat detection in parallel
// sv_bots_get_easier_each_win                               int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // If > 0, some # of bots will lower thier difficulty each time they win. The argument defines how many will lower their difficulty each time.
// sv_bounce                                                 float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Bounce multiplier for when physically simulated objects collide with other objects.
// sv_buy_status_override                                    int32    -1                    FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Override for buy status map info. 0 = everyone can buy, 1 = ct only, 2 = t only 3 = nobody
// sv_buymenu_open_prevents_opportunistic_pickup             bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE
// sv_c4_upright_constraint_damping                          float32  0.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Controls how much velocity is damped on the constraint. 0 = undamped wobbly spring, 1 = critically damped no wobble fast converge, >1 = over damped
// sv_c4_upright_constraint_enabled                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Use a constraint to keep C4 pointed upright when thrown
// sv_c4_upright_constraint_strength                         float32  0.6                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // How quickly the constraint converges
// sv_chat_proximity                                         float32  -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_cheats                                                 bool     false                 FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Allow cheats on server
// sv_client_max_interp_ratio                                float32  5                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // This can be used to limit the value of cl_interp_ratio for connected clients (only while they are connected).
// sv_client_min_interp_ratio                                float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // This can be used to limit the value of cl_interp_ratio for connected clients (only while they are connected).

// sv_clip_penetration_traces_to_players                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_clockcorrection_msecs                                  float32  30                    FCVAR_GAMEDLL|FCVAR_RELEASE  // The server tries to keep each player's m_nTickBase withing this many msecs of the server absolute tickcount
// sv_clockdbg                                               bool     false                 FCVAR_DEVELOPMENTONLY  // Print spew related to server clock and ticking
// sv_cloth_interp_rot                                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_cluster                                                int32    0                     FCVAR_RELEASE  // Data center cluster this server lives in.
// sv_coaching_enabled                                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Allows spectating and communicating with a team ( 'coach t' or 'coach ct' )
// sv_competitive_minspec                                    bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Enable to force certain client convars to minimum/maximum values to help prevent competitive advantages.
// sv_compute_per_bot_difficulty                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // 0 = compute all bot difficulties equally, 1 = compute unique bot difficulty for each bot 
// sv_condense_late_buttons                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // When condensing late commands. Should we compress multiple moves of button presses into the target move?
// sv_cq_delta_encode_svc_usercmds                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Delta encode svc_UserCmds message
// sv_cq_min_queue                                           int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED  // Server min buffer size.
// sv_cq_trim_bloat_remainder                                int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // When trimming a bloated CQ, leave at least N more commands than the minimum
// sv_cq_trim_bloat_space                                    int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // When trimming a bloated CQ, try to leave room for N more commands to be added.  0 will trim only what is needed to remove the immediate bloat, a very large value will reset the whole queue.
// sv_cq_trim_catchup_remainder                              int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE  // When trimming an overful CQ due to app 'catchup' request, leave at least N more commands than the minimum
// sv_cq_validate_encoded_svc_usercmds                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // VERY EXPENSIVE: serialize non-delta-encoded commands along with delta-encoded for validation
// sv_cs_player_speed_has_hostage                            float32  200                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_csgo_gpu_culling_skybox                                bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_csgo_shoot_assert_lagcompensation_error                bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_csgo_shoot_force_full_interp                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_csgo_shoot_force_use_target_time                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_csgo_shoot_lagcompensation_max_error                   float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Warn if lag compensated head hitbox position doesn't match that on client.
// sv_csgo_shoot_log                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_csgo_shoot_log_attack_cmds_only                        bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_csgo_shoot_use_full_interp                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_csgo_shoot_verify                                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_csgo_shoot_verify_on_attack_only                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Only run lag compensation error check when primary attack goes down.
// sv_damage_prediction_allowed                              bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_deadtalk                                               bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Dead players can speak (voice, text) to the living
// sv_debug_client_not_in_pvs                                bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // If set, draw failed client PVS checks with red box
// sv_debug_overlays_bandwidth                               int32    65536                 FCVAR_RELEASE  // Broadcast server debug overlays traffic
// sv_debug_overlays_broadcast                               bool     false                 FCVAR_NOTIFY|FCVAR_CHEAT|FCVAR_RELEASE  // Broadcast server debug overlays
// sv_debug_player_use                                       int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Visualizes +use logic. Green cross=trace success, Red cross=trace too far, Green box=radius success
// sv_debugroundstats                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_deltaticks_enforce                                     int32    2                     FCVAR_RELEASE  // By default, player must ack delta ticks in order. How to enforce it: 2 = kick all clients, 1 = kick only TV clients, 0 = do not kick.
// sv_deltaticks_log                                         int32    2                     FCVAR_RELEASE  // Whether diagnostic logging is enabled when clients ack delta ticks out of order. Policy: 2 = log all out of order acks, 1 = log only when disconnect is triggered, 0 = do not log.
// sv_dev_damage_use_netvars                                 bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Whether we should use network vars (true) or legacy messages (false).
// sv_dev_entitydeltapadding_extra_max                       int32    0                     FCVAR_DEVELOPMENTONLY  // When encoding entity deltas, append on a random number of extra bytes.  This happens after sv_dev_entitydeltapadding_min_size.
// sv_dev_entitydeltapadding_extra_min                       int32    0                     FCVAR_DEVELOPMENTONLY  // When encoding entity deltas, append on a random number of extra bytes.  This happens after sv_dev_entitydeltapadding_min_size.
// sv_dev_entitydeltapadding_min_size                        int32    0                     FCVAR_DEVELOPMENTONLY  // When encoding entity deltas, if the delta size is < N bytes, then shove in N dummy bytes.  This happens before sv_dev_entitydeltapadding_extra_min/sv_dev_entitydeltapadding_extra_max
// sv_disable_immunity_alpha                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If set, clients won't slam the player model render settings each frame for immunity [mod authors use this]
// sv_disable_networkable_loadouts                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_disable_observer_interpolation                         bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Disallow interpolating between observer targets on this server.
// sv_disable_querycache                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // debug - disable trace query cache
// sv_disable_radar                                          int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // 0: regular radar; 1: always disabled; 2: disabled in warmup
// sv_disable_reliable_delta_retransmit                      bool     true                  FCVAR_DEVELOPMENTONLY  // Assume that a reliable entity delta will be ack'ed and send future deltas relative to the last reliable delta.
// sv_disable_teamselect_menu                                bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Disable teamselect menu on clients
// sv_disconnected_player_data_hold_time                     int32    60                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Duration, in seconds, to hold onto the data of disconnected players, for scoreboard display.
// sv_disconnected_players_cleanup_delay                     int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Delay between player disconnecting and their corpse getting cleaned up.
// sv_early_network_message_processing                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Processes network messages on the server before entities think, instead of at the end of the tick.
// sv_enable_alternate_baselines                             int32    1                     FCVAR_RELEASE  // Allow alternate baseline system, set to 2 for debugging spew.
// sv_enable_donttransmit                                    bool     true                  FCVAR_DEVELOPMENTONLY  // When encoding entity deltas, instead of unreliably deducing explicit deletions, actually send list of existing but not networked entities (dont_transmit list) to each client.
// sv_enable_removearrayelementsoutsidemetadatabounds        bool     false                 FCVAR_RELEASE
// sv_enablebunnyhopping                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Allow jump speed to exceed 1.1x max speed
// sv_endmatch_item_drop_interval                            float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The time between drops on the end match scoreboard 
// sv_endmatch_item_drop_interval_ancient                    float32  3.5                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The time between drops on the end match scoreboard for ancient items 
// sv_endmatch_item_drop_interval_legendary                  float32  2                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The time between drops on the end match scoreboard for legendary items 
// sv_endmatch_item_drop_interval_mythical                   float32  1.25                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The time between drops on the end match scoreboard for mythical items 
// sv_endmatch_item_drop_interval_rare                       float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // The time between drops on the end match scoreboard for rare items 
// sv_ent_showonlyhitbox                                     int32    -1                    FCVAR_GAMEDLL|FCVAR_CHEAT
// sv_extra_client_connect_time                              float32  15                    FCVAR_DEVELOPMENTONLY  // Seconds after client connect during which extra frames are buffered to prevent non-delta'd update
// sv_extract_ammo_from_dropped_weapons                      bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_extreme_strafe_accuracy_fishtail                       float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Number of degrees of aim 'fishtail' when making an extreme strafe direction change
// sv_fade_player_visibility_farz                            bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_falldamage_scale                                       float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_falldamage_to_below_player_multiplier                  float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Scale damage when distributed across two players
// sv_falldamage_to_below_player_ratio                       float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Landing on a another player's head gives them this ratio of the damage.
// sv_filterban                                              int32    1                     FCVAR_DEVELOPMENTONLY  // Set packet filtering by IP mode
// sv_flashed_amount_for_blind_kill                          float32  0.7                   FCVAR_GAMEDLL|FCVAR_RELEASE  // Minimum flashed alpha value for a player to be awarded a blind kill on the kill feed.
// sv_footsteps                                              float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED  // Play footstep sound for players
// sv_force_team_intro_random                                int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_force_team_intro_variant                               int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_force_transmit_ents                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Will transmit all entities to client, regardless of PVS conditions (will still skip based on transmit flags, however).
// sv_fps_max                                                float32  0                     FCVAR_DEVELOPMENTONLY  // Dedicated server frame rate limiter. 0=tick rate. Only applies to the dedicated server.
// sv_freeze_camera_angles                                   vector3  0 0 0                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_freeze_camera_enabled                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_freeze_camera_min_remaining                            int32    3                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_freeze_camera_position                                 vector3  0 0 0                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_friction                                               float32  5.2                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // World friction.
// sv_full_alltalk                                           bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Any player (including Spectator team) can speak to any other player
// sv_game_mode_flags                                        int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Dedicated server game mode flags to run
// sv_gameinstructor_disable                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Force all clients to disable their game instructors.
// sv_gameinstructor_enable                                  bool     false                 FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Force all clients to enable their game instructors.
// sv_give_item                                              string                         FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE  // Player's extra item to give
// sv_gravity                                                float32  800                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // World gravity.
// sv_grenade_collision_sphere                               bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_grenade_collision_sphere_radius                        float32  2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_grenade_trajectory_prac_pipreview                      bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Shows grenade trajectory practice picture-in-picture preview.
// sv_grenade_trajectory_prac_trailtime                      float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Shows grenade trajectory practice visualization for this number of seconds.
// sv_grenade_trajectory_time_spectator                      float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Length of time grenade trajectory remains visible as a spectator.
// sv_guardian_extra_equipment_ct                            string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Extra starting equipment for CT players in guardian modes
// sv_guardian_extra_equipment_t                             string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Extra starting equipment for Terrorist players in guardian modes
// sv_guardian_refresh_ammo_for_items_on_waves               string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // List of additional weapons to refill ammo on waves.
// sv_guardian_spawn_health_ct                               int32    100                   FCVAR_GAMEDLL|FCVAR_RELEASE  // Starting health in guardian modes.
// sv_guardian_spawn_health_t                                int32    100                   FCVAR_GAMEDLL|FCVAR_RELEASE  // Starting health in guardian modes.
// sv_health_approach_enabled                                bool     true                  FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_health_approach_speed                                  float32  10                    FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_hegrenade_damage_multiplier                            float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_hegrenade_radius_multiplier                            float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_hibernate_postgame_delay                               float32  5                     FCVAR_RELEASE  // # of seconds to wait after final client leaves before hibernating.
// sv_hibernate_when_empty                                   bool     true                  FCVAR_RELEASE  // Puts the server into extremely low CPU usage mode when no clients connected
// sv_hide_ent_in_pvs                                        int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_hide_roundtime_until_seconds                           int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_highlight_distance                                     float32  500                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_highlight_duration                                     float32  3.5                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_hitbox_debug                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_hosting_lobby                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_REPLICATED
// sv_hoststate_quit_syscall                                 bool     false                 FCVAR_RELEASE  // When enabled, game server will quit immediately via syscall instead of running host states shutdown sequence
// sv_human_autojoin_team                                    int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Force human players on to a team. 0 to disable.
// sv_ignoregrenaderadio                                     bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Turn off Fire in the hole messages
// sv_infinite_ammo                                          int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE  // Player's active weapon will never run out of ammo
// sv_instancebaselines                                      bool     true                  FCVAR_DEVELOPMENTONLY  // Enable instanced baselines. Saves network overhead.
// sv_invites_only_mainmenu                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If turned on, will ignore all invites when user is playing a match
// sv_jump_impulse                                           float32  301.99338             FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Initial upward velocity for player jumps; sqrt(2*gravity*height).
// sv_jump_precision_enable                                  bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Enable jump precision. Some game modes benefit from being able to turn this off.
// sv_jump_spam_penalty_time                                 float32  0.015625              FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // For subtick jumps, if this much time or less has elapsed since the last time the user has pressed the jump key, pretend they hadn't. Lowering this makes bunnyhopping easier.
// sv_kick_ban_duration                                      float32  15                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // How long should a kick ban from the server should last (in minutes)
// sv_kick_players_with_cooldown                             int32    1                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // (0: do not kick on insecure servers; 1: kick players with Untrusted status or convicted by Overwatch; 2: kick players with any cooldown)
// sv_kill_players_at_coord_min                              bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Kill players with fall damage at negative coord min
// sv_ladder_angle                                           float32  -0.707                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Cos of angle of incidence to ladder perpendicular for applying ladder_dampen
// sv_ladder_dampen                                          float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Amount to dampen perpendicular movement on a ladder
// sv_ladder_scale_speed                                     float32  0.78                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Scale top speed on ladders
// sv_ladder_slack_z_mult                                    float32  0.026                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Difference in Z increases toward the middle of the slack ladder.

// sv_lagcomp_filterbyviewangle                              bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // If true, player pawn will filter lag compensation targets by their view angle.
// sv_lagcompensationforcerestore                            bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Don't test validity of a lag comp restore, just do it.
// sv_lan                                                    bool     false                 FCVAR_RELEASE  // Server is a lan server ( no heartbeat, no authentication, no non-class C addresses )
// sv_late_commands_allowed                                  int32    5                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Allow N late commands to run at 0 timescale prior to running an on-time command. Negative values for network round trip based calculation with a hard cap of the of absolute value
// sv_legacy_jump                                            bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether or not to use the pre-2026 jump code.
// sv_lightquery_debug                                       bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// sv_limit_buyrandom_per_life                               bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Enable to limit buyrandom command to only run once per player life
// sv_listen_directudp                                       bool     true                  FCVAR_RELEASE  // Server will listen for direct UDP connections on the configured port.  This can be turned off to only listen for P2P/SDR connections.
// sv_log_http_record_before_any_listeners                   bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE
// sv_log_onefile                                            bool     false                 FCVAR_ARCHIVE|FCVAR_RELEASE  // Log server information to only one file.
// sv_log_roundstats                                         bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE
// sv_logbans                                                bool     false                 FCVAR_ARCHIVE|FCVAR_RELEASE  // Log server bans in the server logs.
// sv_logblocks                                              bool     false                 FCVAR_RELEASE  // If true when log when a query is blocked (can cause very large log files)
// sv_logecho                                                bool     true                  FCVAR_ARCHIVE|FCVAR_RELEASE  // Echo log information to the console.
// sv_logfile                                                bool     false                 FCVAR_ARCHIVE|FCVAR_RELEASE  // Log server information in the log file.
// sv_logflush                                               bool     false                 FCVAR_ARCHIVE|FCVAR_RELEASE  // Flush the log file to disk on each write (slow).
// sv_logsdir                                                string   logs                  FCVAR_ARCHIVE|FCVAR_RELEASE  // Folder in the game directory where server logs will be stored.
// sv_long_frame_ms                                          float32  15                    FCVAR_DEVELOPMENTONLY  // If a server frame takes longer than N ms, complain about it.  (Dedicated server only.)  See also engine_frametime_warnings_enable.
// sv_mapvetopickvote_maps                                   string   de_cache,de_anubis,…  FCVAR_GAMEDLL|FCVAR_RELEASE  // Which maps are used for map veto pick sequence
// sv_mapvetopickvote_phase_duration                         string   [1:5][2:15][3:20][4…  FCVAR_GAMEDLL|FCVAR_RELEASE  // How many seconds each phase lasts
// sv_mapvetopickvote_rnd                                    bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // When enabled will shuffle veto pick maps list order every time
// sv_massreport                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_matchend_drops_enabled                                 bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Rewards gameplay time is always accumulated for players, but drops at the end of the match can be prevented
// sv_matchpause_auto_5v5                                    bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When enabled will automatically pause the match at next freeze time if less than 5 players are connected on each team.
// sv_matchperfstats_maxclientperfsamples                    int32    100                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Don't retain more than N perf samples for any given client
// sv_max_deathmatch_respawns_per_tick                       int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE
// sv_max_distance_transmit_footsteps                        float32  1250                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Maximum distance to transmit footstep sound effects.
// sv_max_queries_sec                                        float32  3                     FCVAR_RELEASE  // Maximum queries per second to respond to from a single IP address.
// sv_max_queries_sec_global                                 float32  60                    FCVAR_RELEASE  // Maximum queries per second to respond to from anywhere.
// sv_max_queries_window                                     float32  30                    FCVAR_RELEASE  // Window over which to average queries per second averages.
// sv_max_unreliable_delta_size                              int32    4096                  FCVAR_DEVELOPMENTONLY  // Maximum allowable entity delta size over unreliable delivery.
// sv_maxclientframes                                        int32    128                   FCVAR_DEVELOPMENTONLY
// sv_maxrate                                                int32    0                     FCVAR_REPLICATED|FCVAR_RELEASE  // Max bandwidth rate allowed on server, 0 == unlimited
// sv_maxreplay                                              float32  0                     FCVAR_DEVELOPMENTONLY  // Maximum replay time in seconds
// sv_maxspeed                                               float32  320                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_maxunlag                                               float32  0.2                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum lag compensation in seconds
// sv_maxunlag_player                                        float32  -1                    FCVAR_GAMEDLL|FCVAR_RELEASE  // If >0, maximumum lag compensation used for other human pawns. Applied after sv_maxunlag!
// sv_maxuptimelimit                                         float32  0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Number of hours to operate before trying sv_shutdown.
// sv_maxvelocity                                            float32  3500                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum speed any ballistically moving object is allowed to attain per axis.
// sv_memlimit                                               int32    0                     FCVAR_CHEAT|FCVAR_RELEASE  // If set, whenever a game ends, if the total memory used by the server is greater than this # of megabytes, the server will exit.
// sv_merge_changes_after_tick_with_calcdelta                int32    1                     FCVAR_RELEASE  // This fixes bugs where pure calcdelta is used due to recipient changing but it doesn't pick up a field change where the value was changed back to same value as the from snapshot even though the destination fields change list does note the change. Set to 2 t
// sv_min_jump_landing_sound                                 float32  260                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_minimum_desired_chicken_count                          int32    0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Minimum number of chickens to attempt to spawn in the map
// sv_minrate                                                int32    98304                 FCVAR_REPLICATED|FCVAR_RELEASE  // Min bandwidth rate allowed on server, 0 == unlimited
// sv_mmqueue_reservation                                    string                         FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Server queue reservation
// sv_mmqueue_reservation_extended_timeout                   int32    21                    FCVAR_DEVELOPMENTONLY  // Extended time in seconds before mmqueue reservation expires.
// sv_mmqueue_reservation_timeout                            int32    21                    FCVAR_DEVELOPMENTONLY  // Time in seconds before mmqueue reservation expires.
// sv_mover_maxslope                                         float32  0.7                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED  // The maximum slope the player can overcome [-]
// sv_mover_pogodampingratio                                 float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED  // The capsule pogo stick damping ratio [-]
// sv_mover_pogofrequency                                    float32  10                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED  // The capsule pogo stick frequency [hz].
// sv_mute_players_with_social_penalties                     bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_networkvar_log_fullchanges                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Log FUL_FULL_EDICT_CHANGED calls.
// sv_networkvar_perfieldtracking                            bool     true                  FCVAR_RELEASE  // Track individual field offset changes, rather than a single dirty flag for the whole entity.
// sv_no_navmesh                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // Block loading of the navmesh. Unplayable, only used for memory sampling.
// sv_noclipaccelerate                                       float32  5                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_NOTIFY|FCVAR_REPLICATED
// sv_noclipduringpause                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // If cheats are enabled, then you can noclip with the game paused (for doing screenshots, etc.).
// sv_noclipfriction                                         float32  4                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_NOTIFY|FCVAR_REPLICATED  // Friction during noclip move.
// sv_noclipspeed                                            float32  1200                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_NOTIFY|FCVAR_REPLICATED
// sv_noclipspeedscaleonshift                                float32  0.5                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_NOTIFY|FCVAR_REPLICATED
// sv_nomvp                                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Disable MVP awards.
// sv_nonemesis                                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Disable nemesis and revenge.
// sv_nowinpanel                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Turn on/off win panel on server
// sv_optimizedmovement                                      bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_outofammo_indicator                                    bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_outofpvsentityupdates                                  bool     false                 FCVAR_DEVELOPMENTONLY
// sv_override_max_health                                    int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE
// sv_parallel_checktransmit                                 int32    2                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Set to 1 to use threaded checkentities for transmit/pvs on listen servers, 2 for dedicated servers.
// sv_parallel_packentities                                  int32    2                     FCVAR_RELEASE  // Set to 1 to use threaded snapshot sending on listen servers, 2 for dedicated servers.
// sv_parallel_prepare_client_updates                        bool     false                 FCVAR_DEVELOPMENTONLY
// sv_parallel_sendsnapshot                                  int32    2                     FCVAR_RELEASE  // 0: run all send jobs on main thread; 1: send jobs run asynchronously (except on dedicated server); 2: send jobs asynchronously; 3: send jobs run in parallel but block to not overlap the next tick; 4: main server clients' send jobs run in parallel, then HLT
// sv_party_mode                                             bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Party!!
// sv_password                                               string                         FCVAR_PROTECTED|FCVAR_NOTIFY|FCVAR_DONTRECORD|FCVAR_RELEASE  // Server password for entry into multiplayer games
// sv_pausable                                               int32    0                     FCVAR_RELEASE  // Is the server pausable.
// sv_pausable_dev                                           bool     true                  FCVAR_DEVELOPMENTONLY  // Whether listen server is pausable when running -dev and playing solo against bots
// sv_pausable_dev_ds                                        bool     false                 FCVAR_DEVELOPMENTONLY  // Whether dedicated server is pausable when running -dev and playing solo against bots
// sv_pause_on_console_open                                  bool     false                 FCVAR_ARCHIVE  // 1 = Pause the game when pressing ~ to open the console. CTRL+~ opens the console without pause.
// sv_pause_on_tick                                          int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Tick count to pause on
// sv_phys_animated_hierarchy                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_phys_async_buoyancy_update                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED  // If true, server buoyancy motion controllers are updated in an async job after the tick has completed.
// sv_phys_debug_callback_entities                           bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Print all entities that get touch callbacks. Each entity is printed only once.
// sv_phys_enabled                                           bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Enable all physics simulation
// sv_phys_sleep_enable                                      bool     true                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Enable sleeping for dynamic physics bodies.
// sv_phys_sound_disable_impact_sounds_under_hard_threshold  bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // if true, impact sounds wont play if no soft impact sound is present and the impact is below the hard velocity threshold.
// sv_phys_stop_at_collision                                 string                         FCVAR_GAMEDLL|FCVAR_CHEAT
// sv_phys_visualize_awake                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_player_search_range                                    float32  64                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_playerradio_use_allowlist                              bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // playerradio commands may only use responses from an allow list of commands.
// sv_predictable_damage_tag_ticks                           int32    2                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Delay player slowdown when damaged by # ticks to reduce misprediction effects
// sv_prime_accounts_only                                    bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // When this setting is enabled only prime users can connect to this game server.
// sv_pushaway_clientside                                    int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Clientside physics push away (0=off, 1=only localplayer, 1=all players)
// sv_pushaway_clientside_size                               float32  15                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Minimum size of pushback objects
// sv_pushaway_force                                         float32  300000                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How hard physics objects are pushed away from the players on the server.
// sv_pushaway_hostage_force                                 float32  20000                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // How hard the hostage is pushed away from physics objects (falls off with inverse square of distance).
// sv_pushaway_max_force                                     float32  2000                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Maximum amount of force applied to physics objects by players.
// sv_pushaway_max_hostage_force                             float32  1000                  FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Maximum of how hard the hostage is pushed away from physics objects.
// sv_pushaway_max_player_force                              float32  20                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Maximum of how hard the player is pushed away from physics objects.
// sv_pushaway_min_player_speed                              float32  75                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // If a player is moving slower than this, don't push away physics objects (enables ducking behind things).
// sv_pushaway_player_force                                  float32  450                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // How hard the player is pushed away from physics objects (falls off with inverse square of distance).
// sv_pvs_cache_query_inflate_amount                         int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_pvs_entity                                             int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // If set, only allows this ent index to network (other than players and things that force sending).
// sv_pvs_max_distance                                       float32  0                     FCVAR_REPLICATED|FCVAR_RELEASE  // if set, adds a maximum range to PVS/PAS checks
// sv_pvs_random                                             bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // If set, objects blink in/out of pvs randomly.
// sv_pvs_shadows_include_env                                bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_quantize_movement_input                                bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Quantize movement input values. Enabling this restricts players from using analog input to move at fractional speeds normally impossible with digital button input.
// sv_radio_throttle_window                                  float32  10                    FCVAR_GAMEDLL|FCVAR_RELEASE  // The number of seconds before radio command tokens refresh.
// sv_ragdoll_lru_debug                                      bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// sv_rcon_banpenalty                                        int32    0                     FCVAR_DEVELOPMENTONLY  // Number of minutes to ban users who fail rcon authentication
// sv_rcon_log                                               bool     true                  FCVAR_DEVELOPMENTONLY  // Enable/disable rcon logging.
// sv_rcon_maxfailures                                       int32    10                    FCVAR_DEVELOPMENTONLY  // Max number of times a user can fail rcon authentication before being banned
// sv_rcon_minfailures                                       int32    5                     FCVAR_DEVELOPMENTONLY  // Number of times a user can fail rcon authentication in sv_rcon_minfailuretime before being banned
// sv_rcon_minfailuretime                                    float32  30                    FCVAR_DEVELOPMENTONLY  // Number of seconds to track failed rcon authentications
// sv_record_item_time_data                                  bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Turn on recording of per player item time data into the server log.
// sv_recvbuf_messages                                       int32    1024                  FCVAR_DEVELOPMENTONLY  // Max number of messages that can be queued in a netchan receive buffer for an ordinary connection from a client.
// sv_regeneration_force_on                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Cheat to test regenerative health systems
// sv_regeneration_wait_time                                 float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// sv_region                                                 int32    -1                    FCVAR_RELEASE  // The region of the world to report this server in.
// sv_reliableavatardata                                     bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Use server overrides for steam avatars
// sv_remapper_loopsoundfix                                  bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_remapper_range_multiplier                              float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_remove_ent_from_pvs                                    int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_replay_group_id                                        int32    0                     FCVAR_RELEASE  // The replay group that this server will be uploading files to
// sv_replaysdir                                             string   replays               FCVAR_DEVELOPMENTONLY  // Directory to store replays in
// sv_reserve_slots_for_reconnecting_players_kick_prior      bool     true                  FCVAR_DEVELOPMENTONLY  // Kick a previously connected player with the same steamID if a replacement comes along
// sv_rollangle                                              float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED  // Max view roll angle
// sv_rollspeed                                              float32  200                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED
// sv_runcmds                                                bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_script_think_interval                                  float32  0.1                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// sv_search_key                                             string                         FCVAR_RELEASE
// sv_search_team_key                                        string   public                FCVAR_RELEASE  // When initiating team search, set this key to match with known opponents team
// sv_sellback_enabled                                       bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether players can undo purchases in the buy menu
// sv_sendtables                                             int32    1                     FCVAR_DEVELOPMENTONLY  // Force full sendtable sending path.
// sv_sequence_debug                                         int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_sequence_debug2                                        int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_sequence_model_substring                               string                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_server_graphic1                                        string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // A 360x60 (<16kb) image file in /csgo/ that will be displayed to spectators.
// sv_server_graphic2                                        string                         FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // A 220x45 (<16kb) image file in /csgo/ that will be displayed to spectators.
// sv_server_verify_blood_on_player                          bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// sv_shared_team_pvs                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // PVS is shared between teams
// sv_show_bot_difficulty_in_name                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // 0 = hide bot difficulty in bot name, 1 = show bot difficulty in bot name
// sv_show_move_collisions                                   bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Enable this to visualize collisions between player and geometry.
// sv_show_team_equipment_force_on                           bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Force on if not prohibited
// sv_show_team_equipment_prohibit                           bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether +cl_show_team_equipment is prohibited.
// sv_show_teammate_death_notification                       bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Show chat notification upon teammate death
// sv_show_voip_indicator_for_enemies                        bool     false                 FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Makes it so the voip icon is shown over enemies as well as allies when they are talking
// sv_showbullethits                                         int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // 1=show hits and near misses, 2=show hits only
// sv_showhitregistration                                    int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Display lag_compensated hitboxes. 0 = off, 1 = server only, 2 = client only, 3 = both server and client
// sv_showimpacts                                            int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Shows client (red) and server (blue) bullet impact point (1=both, 2=client-only, 3=server-only)
// sv_showimpacts_penetration                                int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Shows extra data when bullets penetrate. (use sv_showimpacts_time to increase time shown)
// sv_showimpacts_time                                       float32  4                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Duration bullet impact indicators remain before disappearing
// sv_showladders                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Show bbox and dismount points for all ladders (must be set before level load.)

// sv_showlagcompensation_rec                                float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // If > 0, show lag compensation hitboxes as they're recorded. Value is for how long.
// sv_showplayerhitboxes                                     int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Show lag compensated hitboxes for the specified player index whenever a player fires.
// sv_shutdown_immediately_on_request                        bool     false                 FCVAR_DEVELOPMENTONLY  // The server will always shutdown on receiving the shutdown request, even if not hibernating
// sv_skel_constraints_enable                                bool     false                 FCVAR_REPLICATED|FCVAR_CHEAT
// sv_skip_update_animations                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Enable to skip game animations
// sv_skirmish_id                                            int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Dedicated server skirmish id to run
// sv_skyname                                                string   sky_urb01             FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED  // Current name of the skybox texture
// sv_smoke_volume_blind_start                               float32  0.2                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// sv_snapshot_unlimited                                     bool     false                 FCVAR_REPLICATED|FCVAR_RELEASE  // For debugging, don't throw away old snapshots so that if you break in debugger (on remote client or server) it won't require an uncompressed update to resume.  You may run out of memory of course...
// sv_sniper_tracer_innacuracy                               float32  0.085                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How inaccurate a sniper shot can be before we trip sv_sniper_tracer_mode behavior.
// sv_sniper_tracer_innacuracy_length                        float32  200                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // How far should the tracer draw if we trip sv_sniper_tracer_mode behavior.
// sv_sniper_tracer_mode                                     int32    1                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Mode for sniper tracers. 0: legacy, 1: hide when more than sv_sniper_tracer_innacuracy inaccurate.
// sv_spawn_afk_bomb_drop_time                               float32  15                    FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Players that have never moved since they spawned will drop the bomb after this amount of time.
// sv_spec_hear                                              int32    3                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines who spectators can hear: 0: only spectators; 1: all players; 2: spectated team; 3: self only; 4: nobody
// sv_spec_use_tournament_content_standards                  bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_specaccelerate                                         float32  5                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_NOTIFY|FCVAR_REPLICATED
// sv_specnoclip                                             bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_NOTIFY|FCVAR_REPLICATED
// sv_specspeed                                              float32  1200                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_NOTIFY|FCVAR_REPLICATED
// sv_staminajumpcost                                        float32  0.08                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // sv_legacy_jump only: Stamina penalty for jumping
// sv_staminalandcost                                        float32  0.05                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // sv_legacy_jump only: Stamina penalty for landing
// sv_staminamax                                             float32  80                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum stamina penalty
// sv_staminarecoveryrate                                    float32  60                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Rate at which stamina recovers (units/sec)
// sv_standable_normal                                       float32  0.7                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// sv_stats                                                  bool     true                  FCVAR_DEVELOPMENTONLY  // Collect CPU usage stats
// sv_steamauth_correct                                      bool     false                 FCVAR_RELEASE  // Correct behavior
// sv_steamauth_enforce                                      int32    2                     FCVAR_RELEASE  // By default, player must maintain a reliable connection to Steam servers. When player Steam session drops, enforce it: 2 = instantly kick, 1 = kick at next spawn, 0 = do not kick.
// sv_steamauth_ignore_localhost                             bool     true                  FCVAR_RELEASE  // Ignore VAC and auth errors for client connected via localhost address or in-engine loopback
// sv_steamgroup                                             string                         FCVAR_NOTIFY|FCVAR_RELEASE  // The ID of the steam group that this server belongs to. You can find your group's ID on the admin profile page in the steam community.
// sv_steamgroup_exclusive                                   bool     false                 FCVAR_RELEASE  // If set, only members of Steam group will be able to join the server when it's empty, public people will be able to join the server only if it has players.
// sv_step_move_vel_min                                      float32  64                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Min velocity for step move.
// sv_stepsize                                               float32  18                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED
// sv_stopspeed                                              float32  80                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Minimum stopping speed when on ground.
// sv_strafing_inaccuracy_bias                               float32  0.5                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// sv_strafing_inaccuracy_enabled                            bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// sv_strafing_inaccuracy_scale                              float32  0.1                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// sv_stressbots                                             bool     false                 FCVAR_RELEASE  // If set to 1, the server calculates data and fills packets to bots. Used for perf testing.
// sv_strict_notarget                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // If set, notarget will cause entities to never think they are in the pvs
// sv_subtick_movement_view_angles                           bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Whether or not subtick view angles are taken into account during movement.
// sv_suppress_friendlyfire_decals                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_suppress_viewpunch                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// sv_surf_sounds                                            bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Should we try to play sounds for surf?
// sv_tags                                                   string                         FCVAR_NOTIFY|FCVAR_RELEASE  // Server tags. Used to provide extra information to clients when they're browsing for servers. Separate tags with a comma.
// sv_talk_after_dying_time                                  float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // The number of seconds a player can continue talking after dying as if they were still alive
// sv_talk_enemy_dead                                        bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Dead players can hear all dead enemy communication (voice, chat)
// sv_talk_enemy_living                                      bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Living players can hear all living enemy communication (voice, chat)
// sv_teamid_overhead                                        bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Shows teamID over player's heads.  0 = off, 1 = on
// sv_teamid_overhead_always_prohibit                        bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Determines whether cl_teamid_overhead_always is prohibited.
// sv_teamid_overhead_maxdist                                int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If >0, server will override cl_teamid_overhead_maxdist
// sv_teamid_overhead_maxdist_spec                           int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If >0, server will override cl_teamid_overhead_maxdist_spec
// sv_temp_baseline_string_table_buffer_size                 int32    524288                FCVAR_DEVELOPMENTONLY  // Buffer size for writing string table baselines
// sv_tick_parallel_with_client                              bool     false                 FCVAR_DEVELOPMENTONLY  // Runs the final server tick of the frame in parallel with client work
// sv_tick_snapshot_sort_entities                            bool     true                  FCVAR_DEVELOPMENTONLY
// sv_timebetweenducks                                       float32  0.4                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Minimum time before recognizing consecutive duck key
// sv_timeout                                                float32  20                    FCVAR_DEVELOPMENTONLY  // After this many seconds without a message from fully connected client, the client is dropped
// sv_turbophysics                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Turns on turbo physics
// sv_turning_inaccuracy_angle_min                           float32  4                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// sv_turning_inaccuracy_decay                               float32  0.8                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// sv_turning_inaccuracy_enabled                             bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// sv_unlag                                                  bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Enables player lag compensation
// sv_unlag_debug                                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_unlag_fixstuck                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Disallow backtracking a player for lag compensation if it will cause them to become stuck
// sv_unlockedchapters                                       int32    1                     FCVAR_ARCHIVE  // Highest unlocked game chapter.
// sv_unpause_on_console_close                               bool     false                 FCVAR_ARCHIVE  // 1 = Unpause the game when pressing ~ to close the console. 0 = Leave the game paused.
// sv_use_hi_pri_context_switch_time                         float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // +use search behaves as though high priority items are usable for this long after they become unusable to avoid players accidentally performing a different action.
// sv_use_playercache                                        bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Cache off player bounds for traces.
// sv_use_pvs_cache                                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// sv_usenetworkvars                                         bool     true                  FCVAR_DEVELOPMENTONLY  // Use networkvar system.
// sv_usercmd_custom_random_seed                             bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // When enabled server will populate an additional random seed independent of the client
// sv_usercmd_execute_warning_ms                             float32  5                     FCVAR_GAMEDLL|FCVAR_ARCHIVE  // Emit a warning if we spend more than N ms executing user commands for a single player
// sv_vac_webapi_auth_key                                    string                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Key for when posting to vac related webapis.
// sv_versus_screen_scene_id                                 int32    0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Determines which scene is used for the versus screen.
// sv_visiblemaxplayers                                      int32    -1                    FCVAR_RELEASE  // Overrides the max players reported to prospective clients
// sv_voice_proximity                                        float32  -1                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_voicecodec                                             string   vaudio_speex          FCVAR_RELEASE  // Specifies which voice codec DLL to use in a game. Set to the name of the DLL without the extension.
// sv_voiceenable                                            bool     true                  FCVAR_ARCHIVE|FCVAR_NOTIFY|FCVAR_RELEASE
// sv_vote_allow_in_warmup                                   bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Allow voting during warmup?
// sv_vote_allow_spectators                                  bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Allow spectators to initiate votes?
// sv_vote_command_delay                                     float32  2                     FCVAR_GAMEDLL|FCVAR_RELEASE  // How long after a vote passes until the action happens
// sv_vote_count_spectator_votes                             bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Allow spectators to vote on issues?
// sv_vote_creation_timer                                    float32  120                   FCVAR_GAMEDLL|FCVAR_RELEASE  // How often someone can individually call a vote.
// sv_vote_disallow_kick_on_match_point                      bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Disallow vote kicking on the match point round.
// sv_vote_failure_timer                                     float32  300                   FCVAR_GAMEDLL|FCVAR_RELEASE  // A vote that fails cannot be re-submitted for this long
// sv_vote_issue_changelevel_allowed                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Can people hold votes to change levels?
// sv_vote_issue_kick_allowed                                bool     true                  FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Can people hold votes to kick players from the server?
// sv_vote_issue_loadbackup_allowed                          bool     true                  FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // Can people hold votes to load match from backup?
// sv_vote_issue_loadbackup_spec_authoritative               bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // When enabled, admins load match from backup without players vote
// sv_vote_issue_loadbackup_spec_only                        bool     false                 FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // When enabled, only admins load match from backup
// sv_vote_issue_loadbackup_spec_safe                        bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // When enabled, admins load match from backup in safe time of the round only
// sv_vote_issue_matchready_allowed                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Can people hold votes to ready/unready the match?
// sv_vote_issue_nextlevel_allowed                           bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Can people hold votes to set the next level?
// sv_vote_issue_nextlevel_allowextend                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Allow players to extend the current map?
// sv_vote_issue_nextlevel_choicesmode                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Present players with a list of lowest playtime maps to choose from?
// sv_vote_issue_nextlevel_prevent_change                    bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Not allowed to vote for a nextlevel if one has already been set.
// sv_vote_issue_pause_match_allowed                         bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Can people hold votes to pause/unpause the match?
// sv_vote_issue_pause_match_spec_only                       bool     false                 FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // When enabled, only admins start technical pause
// sv_vote_issue_restart_game_allowed                        bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Can people hold votes to restart the game?
// sv_vote_issue_scramble_teams_allowed                      bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Can people hold votes to scramble the teams?
// sv_vote_issue_surrrender_allowed                          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Can people hold votes to surrender?
// sv_vote_issue_swap_teams_allowed                          bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Can people hold votes to swap the teams?
// sv_vote_issue_timeout_allowed                             bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Can people hold votes to time out?
// sv_vote_kick_ban_duration                                 float32  15                    FCVAR_GAMEDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE  // How long should a kick vote ban someone from the server? (in minutes)
// sv_vote_quorum_ratio                                      float32  0.501                 FCVAR_GAMEDLL|FCVAR_RELEASE  // The minimum ratio of players needed to vote on an issue to resolve it.
// sv_vote_timer_duration                                    float32  15                    FCVAR_GAMEDLL|FCVAR_RELEASE  // How long to allow voting on an issue
// sv_vote_to_changelevel_before_match_point                 bool     true                  FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Restricts vote to change level to rounds prior to match point (default 0, vote is never disallowed)
// sv_vote_to_changelevel_rndmin                             int32    0                     FCVAR_GAMEDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When non-zero, restricts vote to change level to this many first rounds or minutes of the match (default 0, vote is not disallowed)
// sv_walkable_normal                                        float32  0.7                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE
// sv_warmup_to_freezetime_delay                             int32    4                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Delay between end of warmup and start of match.
// sv_water_slow_amount                                      float32  0.9                   FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_wateraccelerate                                        float32  10                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_waterdist                                              float32  12                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Vertical view fixup when eyes are near water plane.
// sv_waterfriction                                          float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_NOTIFY|FCVAR_REPLICATED|FCVAR_RELEASE
// sv_weapon_require_use_grace_period                        float32  1                     FCVAR_GAMEDLL|FCVAR_RELEASE
// sv_weapon_swap_difficulty_near_hi_pri                     int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // 0 = Cone searches easily reach past high priority items to swap weapons. 1 = Cone searches are narrowed and require that the weapon is strictly closer. 2 = cone searches are disabled near high priority items
// sv_workshop_allow_other_maps                              bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // When hosting a workshop collection, users can play other workshop map on this server when it is empty and then mapcycle into this server collection.
// sv_workshop_map_save_data_max_filesize_mb                 int32    1                     FCVAR_GAMEDLL|FCVAR_RELEASE
// sys_minidumpexpandedspew                                  bool     true                  FCVAR_DEVELOPMENTONLY
// sys_minidumpspewlines                                     int32    2000                  FCVAR_RELEASE  // Lines of crash dump console spew to keep.
// target_scan_use_query_cache                               bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// teleport_trigger_debug                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// testscript_debug                                          bool     false                 FCVAR_DEVELOPMENTONLY  // Debug test scripts.
// think_limit                                               float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Maximum think time in milliseconds, warning is printed if this is exceeded.
// thread_pool_option                                        int32    3                     FCVAR_RELEASE  // Thread pool option
// throttle_expensive_ai                                     bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// timedemo_end                                              string   -1                    FCVAR_RELEASE  // Ends timedemo on given tick.
// timedemo_start                                            string   -1                    FCVAR_RELEASE  // Starts timedemo on given tick.
// toast_manager_override_duration                           float32  -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// tool_spawned_model_scales                                 vector3  1 1 1                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// tools_stall_monitor_break_on_unknown_cause                bool     false                 FCVAR_DEVELOPMENTONLY  // Break on unknown stall cause
// trigger_fan_debug                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// trigger_fan_player_windblock_debug                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// trusted_launch                                            int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Trusted launch status
// tv_advertise_watchable                                    bool     false                 FCVAR_PROTECTED|FCVAR_NOTIFY|FCVAR_DONTRECORD|FCVAR_RELEASE  // GOTV advertises the match as watchable via game UI, clients watching via UI will not need to type password
// tv_allow_autorecording_index                              int32    -1                    FCVAR_GAMEDLL|FCVAR_RELEASE  // When >=0 restricts autorecording only to the specified TV index
// tv_allow_camera_man                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Auto director allows spectators to become camera man
// tv_allow_camera_man_steamid                               uint64   0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Allows tournament production cameraman to run csgo.exe -interactivecaster on SteamID 7650123456XXX and be the camera man.
// tv_allow_camera_man_steamid2                              uint64   0                     FCVAR_GAMEDLL|FCVAR_RELEASE  // Allows tournament production tv cameraman to run csgo.exe -interactivecaster on SteamID 7650123456XXX and be the tv camera man.
// tv_allow_static_shots                                     bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Auto director uses fixed level cameras for shots
// tv_autorecord                                             bool     false                 FCVAR_RELEASE  // Automatically records all games as SourceTV demos.
// tv_autoretry                                              bool     true                  FCVAR_RELEASE  // Relay proxies retry connection after network timeout
// tv_broadcast                                              bool     false                 FCVAR_RELEASE  // Automatically broadcasts all games as GOTV demos through Steam.
// tv_broadcast1                                             bool     false                 FCVAR_RELEASE  // Automatically broadcasts all games as GOTV[1] demos through Steam.
// tv_broadcast_drop_fragments                               int32    0                     FCVAR_RELEASE  // Drop every Nth fragment
// tv_broadcast_keyframe_interval                            float32  3                     FCVAR_RELEASE  // The frequency, in seconds, of sending keyframes and delta fragments to the broadcast relay server
// tv_broadcast_keyframe_interval1                           float32  3                     FCVAR_RELEASE  // The frequency, in seconds, of sending keyframes and delta fragments to the broadcast1 relay server
// tv_broadcast_max_requests                                 int32    20                    FCVAR_RELEASE  // Max number of broadcast http requests in flight. If there is a network issue, the requests may start piling up, degrading server performance. If more than the specified number of requests are in flight, the new requests are dropped.
// tv_broadcast_max_requests1                                int32    20                    FCVAR_RELEASE  // Max number of broadcast1 http requests in flight. If there is a network issue, the requests may start piling up, degrading server performance. If more than the specified number of requests are in flight, the new requests are dropped.
// tv_broadcast_origin_auth                                  string   gocastauth            FCVAR_RELEASE  // X-Origin-Auth header of the broadcast POSTs
// tv_broadcast_origin_auth1                                 string   gocastauth            FCVAR_RELEASE  // X-Origin-Auth header of the broadcast1 POSTs
// tv_broadcast_origin_delay                                 float32  0                     FCVAR_RELEASE  // Injection delay request for CDN rebroadcast frameworks, seconds
// tv_broadcast_spew_threshold                               float32  0.1                   FCVAR_RELEASE  // The threshold, in seconds, that we'll spew about the snapshot tick
// tv_broadcast_startup_resend_interval                      float32  10                    FCVAR_RELEASE  // The interval, in seconds, of re-sending startup data to the broadcast relay server (useful in case relay crashes, restarts or startup data http request fails)
// tv_broadcast_terminate                                    bool     true                  FCVAR_RELEASE  // Terminate every broadcast with a stop command
// tv_broadcast_url                                          string   http://localhost:80…  FCVAR_RELEASE  // URL of the broadcast relay
// tv_broadcast_url1                                         string   http://localhost:80…  FCVAR_RELEASE  // URL of the broadcast relay1
// tv_chatgroupsize                                          int32    0                     FCVAR_RELEASE  // Set the default chat group size
// tv_chattimelimit                                          float32  0.2                   FCVAR_RELEASE  // Limits spectators to chat only every n seconds
// tv_debug                                                  int32    0                     FCVAR_RELEASE  // SourceTV debug info.
// tv_delay                                                  int32    30                    FCVAR_GAMEDLL|FCVAR_RELEASE  // SourceTV broadcast delay in seconds
// tv_delay1                                                 int32    15                    FCVAR_GAMEDLL|FCVAR_RELEASE  // SourceTV[instance 1] broadcast delay in seconds
// tv_delaymapchange                                         bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Delays map change until broadcast is complete
// tv_deltacache                                             int32    2                     FCVAR_RELEASE  // Enable delta entity bit stream cache
// tv_demo_starttick                                         int32    0                     FCVAR_DEVELOPMENTONLY
// tv_dispatchmode                                           int32    1                     FCVAR_RELEASE  // Dispatch clients to relay proxies: 0=never, 1=if appropriate, 2=always
// tv_enable                                                 bool     false                 FCVAR_NOTIFY|FCVAR_RELEASE  // Activates SourceTV on server.
// tv_enable1                                                bool     false                 FCVAR_NOTIFY|FCVAR_RELEASE  // Activates SourceTV[1] on server.
// tv_enable_delta_frames                                    bool     true                  FCVAR_RELEASE  // Indicates whether or not the tv should use delta frames for storage of intermediate frames. This takes more CPU but significantly less memory.
// tv_enable_dynamic                                         bool     false                 FCVAR_NOTIFY|FCVAR_RELEASE  // When enabled, changes in tv_enable convars cause immediate startup or shutdown of hltv server
// tv_extended_logging                                       bool     false                 FCVAR_DEVELOPMENTONLY
// tv_grouprelaydatareliable                                 bool     false                 FCVAR_DEVELOPMENTONLY  // When enabled, this will collect all information for relay sending into a single datagram to ensure that the data stays together through a potentially large number of relays
// tv_grouprelaydataunreliable                               bool     false                 FCVAR_DEVELOPMENTONLY  // When enabled, this will collect all information for relay sending into a single datagram to ensure that the data stays together through a potentially large number of relays
// tv_grouprelaydatavoice                                    bool     false                 FCVAR_DEVELOPMENTONLY  // Similar to tv_grouprelaydata, but controls whether or not the voice channels should be routed into the grouped data for the relays
// tv_include_usercommands                                   bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE  // HLTV streams will include player usercommands each tick
// tv_instant_replay_full_frame                              bool     true                  FCVAR_DEVELOPMENTONLY  // Send embedded full frames
// tv_instant_replay_full_frame_build_threaded               bool     false                 FCVAR_DEVELOPMENTONLY  // Build the full frames on a seperate job thread
// tv_instant_replay_full_frame_time                         int32    30                    FCVAR_DEVELOPMENTONLY  // Seconds between full frame embeddeds
// tv_listen_voice_indices                                   int32    0                     FCVAR_CLIENTDLL|FCVAR_USERINFO  // Bitfield of playerslots to listen to voice messages from when connected to SourceTV, default is none
// tv_listen_voice_indices_h                                 int32    0                     FCVAR_CLIENTDLL|FCVAR_USERINFO  // High 32 bits of bitfield of playerslots to listen to voice messages from when connected to SourceTV, default is none
// tv_log_director_events                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Log game events being considered by the director
// tv_maxclients                                             int32    128                   FCVAR_RELEASE  // Maximum client number on SourceTV server.
// tv_maxclients_relayreserved                               int32    0                     FCVAR_RELEASE  // This number of relay client connections are reserved for SourceTV relays.
// tv_maxrate                                                int32    0                     FCVAR_RELEASE  // Max SourceTV spectator bandwidth rate allowed, 0 == unlimited
// tv_name                                                   string   SourceTV              FCVAR_RELEASE  // SourceTV host name
// tv_nochat                                                 bool     false                 FCVAR_ARCHIVE|FCVAR_USERINFO  // Don't receive chat messages from other SourceTV spectators
// tv_overridemaster                                         bool     false                 FCVAR_RELEASE  // Overrides the SourceTV master root address.
// tv_password                                               string                         FCVAR_PROTECTED|FCVAR_NOTIFY|FCVAR_DONTRECORD|FCVAR_RELEASE  // SourceTV password for all clients of CSTV[0]
// tv_password1                                              string                         FCVAR_PROTECTED|FCVAR_NOTIFY|FCVAR_DONTRECORD|FCVAR_RELEASE  // SourceTV password for all clients of CSTV[1]. If empty, tv_password is used
// tv_playcast_delay_prediction                              bool     true                  FCVAR_RELEASE
// tv_playcast_delay_resync                                  float32  0                     FCVAR_RELEASE  // To alleviate intermittent network connectivity problems, this is the number of seconds to wait before actually re-syncing the stream after failure
// tv_playcast_fragment_cache_history_length                 float32  120                   FCVAR_RELEASE  // How far back before our current playback head in seconds to hold onto fragments.
// tv_playcast_http_delta_fragment_timeout_s                 int32    3                     FCVAR_RELEASE
// tv_playcast_max_rcvage                                    float32  15                    FCVAR_RELEASE
// tv_playcast_max_rtdelay                                   float32  300                   FCVAR_RELEASE
// tv_playcast_origin_auth                                   string                         FCVAR_RELEASE  // Get request X-Origin-Auth string
// tv_playcast_retry_timeout                                 float32  25                    FCVAR_RELEASE  // In case of intermittent network problems, how long should playcast retry fragment retrieval before resorting to resync
// tv_playcast_showerrors                                    string                         FCVAR_RELEASE  // Set to display headers upon error (e.g. "CF-Ray,CF-Cache-Status,Body" )
// tv_playcast_slow_playback_when_fragment_requests_fail     bool     true                  FCVAR_RELEASE  // Whether or not we slow playback rate if we start running out of buffered stream fragments.
// tv_port                                                   int32    27020                 FCVAR_RELEASE  // Host SourceTV[0] port
// tv_port1                                                  int32    27021                 FCVAR_RELEASE  // Host SourceTV[1] port
// tv_rate_multiplier                                        float32  2                     FCVAR_DEVELOPMENTONLY  // Multiply requested rate by this value to adjust Dota TV send rate
// tv_record_immediate                                       int32    0                     FCVAR_RELEASE  // tv_record starting the moment tv_record was executed, not tv_delay earlier
// tv_relay_hard_shutdown                                    bool     false                 FCVAR_DEVELOPMENTONLY
// tv_relay_quit_after_game                                  bool     true                  FCVAR_DEVELOPMENTONLY  // Quit after a game has been relayed, do not hibernate
// tv_relay_rate                                             int32    500000                FCVAR_DEVELOPMENTONLY  // default rate for relays
// tv_relay_secret_code                                      bool     false                 FCVAR_DEVELOPMENTONLY  // When enabled, this will use a uniquely generated server code to authenticate relay to relay connections. This code is coordinated via the GC or some external means rather than by clients directly
// tv_relaypassword                                          string                         FCVAR_PROTECTED|FCVAR_NOTIFY|FCVAR_DONTRECORD|FCVAR_RELEASE  // SourceTV password for relay proxies
// tv_relayradio                                             bool     false                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Relay team radio commands to TV: 0=off, 1=on
// tv_relayvoice                                             bool     true                  FCVAR_RELEASE  // Relay voice data: 0=off, 1=on
// tv_secret_code                                            bool     false                 FCVAR_DEVELOPMENTONLY  // When enabled, this will use a uniquely generated server code to authenticate relay connections. This code is coordinated via the GC or some external means rather than by clients directly
// tv_secure_bypass                                          bool     false                 FCVAR_RELEASE  // Bypass secure challenge on TV port
// tv_show_allchat                                           bool     true                  FCVAR_GAMEDLL|FCVAR_RELEASE
// tv_spectator_port_offset                                  int32    0                     FCVAR_CLIENTDLL|FCVAR_RELEASE
// tv_threaded_merge_entity_deltas                           bool     true                  FCVAR_DEVELOPMENTONLY  // Enable SourceTV threading of delta merging
// tv_timeout                                                float32  20                    FCVAR_RELEASE  // SourceTV connection timeout in seconds.
// tv_title                                                  string   SourceTV              FCVAR_RELEASE  // Set title for SourceTV spectator UI
// tv_transmitall                                            bool     false                 FCVAR_REPLICATED|FCVAR_RELEASE  // Transmit all entities (not only director view)
// tv_update_hibernation_enabled                             bool     false                 FCVAR_DEVELOPMENTONLY  // Allow SourceTV to control server hibernation state.
// tv_window_size                                            float32  16                    FCVAR_RELEASE  // Specifies the number of seconds worth of frames that the tv replay system should keep in memory. Increasing this greatly increases the amount of memory consumed by the TV system
// ui_deepstats_radio_heat_figurine                          string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_deepstats_radio_heat_tab                               string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_deepstats_radio_heat_team                              string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_deepstats_toplevel_mode                                string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_hud_dist                                               float32  24                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // distance from the player to the hud
// ui_inspect_bkgnd_map_C2AEBB5E                             string   warehouse             FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Inspect background map
// ui_inventorysettings_recently_acknowledged                string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_leaderboards_top_public_appid                          int32    730                   FCVAR_CLIENTDLL|FCVAR_RELEASE
// ui_lobby_draft_enabled                                    bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// ui_mainmenu_bkgnd_movie_C2AEBB5E                          string   de_cache              FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Main menu background movie
// ui_nearbylobbies_filter3                                  string   competitive           FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_news_last_read_link                                    string   701021228894257508    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_news_last_read_link2                                   string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_notification_tb_snooze                                 string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_party_msg_sound_enabled                                bool     true                  FCVAR_CLIENTDLL|FCVAR_RELEASE  // When enabled, lobby messages will play a short sound
// ui_playsettings_custom_preset                             string   mg_de_overpass,mg_d…  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_directchallengekey                        string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_listen_casual                       string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_listen_competitive                  string   16                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_listen_cooperative                  string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_listen_deathmatch                   string   32                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_listen_retakes                      string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_listen_scrimcomp2v2                 string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_listen_skirmish                     string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_listen_survival                     string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_official_casual                     string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_official_competitive                string   16                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_official_cooperative                string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_official_deathmatch                 string   32                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_official_retakes                    string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_official_scrimcomp2v2               string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_official_skirmish                   string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_flags_official_survival                   string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_listen_annotations                        string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_listen_grenades                           string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_listen_infammo                            string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_listen_infwarmup                          string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_listen_casual                        string   mg_cs_shelter         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_listen_competitive                   string   mg_de_boulder         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_listen_deathmatch                    string   mg_de_mirage          FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_listen_gungameprogressive            string   mg_ar_baggage         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_listen_retakes                       string   mg_de_dust2           FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_listen_scrimcomp2v2                  string   mg_de_vertigo         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_official_casual                      string   mg_casualalpha        FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_official_deathmatch                  string   mg_casualcharlie      FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_official_gungameprogressive          string   mg_armsrace           FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_official_retakes                     string   mg_casualalpha        FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_maps_workshop                             string                         FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_mode_listen                               string   casual                FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_mode_official_v20                         string   scrimcomp2v2          FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_survival_solo                             string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_playsettings_warmup_map_name                           string   de_mirage             FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_popup_weaponupdate_version                             string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_render_task_clips_label                                string   dealt_damage          FCVAR_CLIENTDLL|FCVAR_RELEASE
// ui_render_task_file                                       string   rendertask            FCVAR_CLIENTDLL|FCVAR_RELEASE
// ui_render_task_fps                                        int32    60                    FCVAR_CLIENTDLL|FCVAR_RELEASE
// ui_render_task_generate_clips                             bool     false                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// ui_setting_advertiseforhire_auto                          int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Whether users will automatically advertise for invites (0: off; 1: last; 2: auto)
// ui_setting_advertiseforhire_auto_last                     string   /competitive          FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE  // Which game mode users last used to advertise for invites
// ui_show_subscription_alert                                string   0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_show_unlock_competitive_alert                          string   1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_steam_overlay_notification_position                    string   bottomleft            FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Steam overlay notification position
// ui_steam_overlay_notification_position_horz               int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Steam overlay notification position horizontal offset
// ui_steam_overlay_notification_position_vert               int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // Steam overlay notification position vertical offset
// ui_vanitysetting_loadoutslot_ct                           string   rifle3                FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_vanitysetting_loadoutslot_t                            string   c4                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// ui_vanitysetting_team                                     string   t                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE
// update_all_keyframed_in_spatial_partition_update          bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// update_voices_low_priority                                bool     false                 FCVAR_DEVELOPMENTONLY
// v8_jitless                                                bool     true                  FCVAR_DEVELOPMENTONLY  // Disable runtime allocation of executable memory for V8.
// v8_maximum_heap_size_mb                                   int32    512                   FCVAR_DEVELOPMENTONLY  // Hard limit for the v8 heap size (in mBytes)
// v8_stack_size                                             int32    384                   FCVAR_DEVELOPMENTONLY  // Default size of stack region v8 is allowed to use (in kBytes)
// vconsole_rcon_server_details                              string                         FCVAR_DONTRECORD|FCVAR_RELEASE|FCVAR_SERVER_CANNOT_QUERY  // when non-empty allows for easy vconsole connection to the dedicated server.
// vehicle_debug_impact_damage                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// videocfg_ao_detail                                        int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// videocfg_fsr_detail                                       int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// videocfg_hdr_detail                                       int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// videocfg_particle_detail                                  int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// videocfg_shadow_quality                                   int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// videocfg_texture_detail                                   int32    2                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// view_punch_decay                                          float32  18                    FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE  // Decay factor exponent for view punch
// viewmodel_fov                                             float32  60                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // Viewmodel FOV
// viewmodel_offset_x                                        float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // viewmodel_offset_x
// viewmodel_offset_y                                        float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // viewmodel_offset_y
// viewmodel_offset_z                                        float32  -1                    FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_USERINFO|FCVAR_PER_USER  // viewmodel_offset_z
// viewmodel_presetpos                                       int32    1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // 1:"Desktop", 2:"Classic" 
// violence_ablood                                           bool     true                  FCVAR_ARCHIVE  // Draw alien blood
// violence_agibs                                            bool     true                  FCVAR_ARCHIVE  // Show alien gib entities
// violence_hblood                                           bool     true                  FCVAR_ARCHIVE  // Draw human blood
// violence_hgibs                                            bool     true                  FCVAR_ARCHIVE  // Show human gib entities
// vis_enable                                                bool     true                  FCVAR_DEVELOPMENTONLY  // Enable precomputed visibility when true
// vis_force                                                 bool     false                 FCVAR_GAMEDLL|FCVAR_CHEAT
// vis_sunlight_enable                                       bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Toggle whether to use sunlight PVS for sunlight views (0 = sky PVS, 1 = sunlight PVS)
// vismon_poll_frequency                                     float32  0.5                   FCVAR_GAMEDLL|FCVAR_CHEAT
// vismon_trace_limit                                        int32    12                    FCVAR_GAMEDLL|FCVAR_CHEAT
// voice_all_icons                                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Draw all players' voice icons
// voice_always_sample_mic                                   bool     false                 FCVAR_ARCHIVE  // When enabled, open the voip audio input stream when the application launches.
// voice_bypass_noise_gate                                   bool     false                 FCVAR_DEVELOPMENTONLY
// voice_clientdebug                                         int32    0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// voice_debugfeedbackfrom                                   bool     false                 FCVAR_DEVELOPMENTONLY
// voice_device_override                                     string                         FCVAR_ARCHIVE|FCVAR_RELEASE  // Default device used for voice capture.
// voice_fadeouttime                                         float32  0.005                 FCVAR_DEVELOPMENTONLY
// voice_in_process                                          bool     true                  FCVAR_DEVELOPMENTONLY
// voice_initial_buffer_ms                                   int32    200                   FCVAR_DEVELOPMENTONLY
// voice_input_stallout                                      float32  2                     FCVAR_USERINFO  // Time before we consider a mic stalled out and need to reset it.
// voice_loopback                                            bool     false                 FCVAR_USERINFO
// voice_loopback_no_networking                              bool     false                 FCVAR_USERINFO
// voice_min_buffer_ms                                       int32    100                   FCVAR_DEVELOPMENTONLY
// voice_modenable                                           bool     true                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_RELEASE|FCVAR_CLIENTCMD_CAN_EXECUTE  // Enable/disable voice in this mod.
// voice_noise_supression                                    bool     false                 FCVAR_DEVELOPMENTONLY
// voice_player_speaking_delay_threshold                     float32  0.5                   FCVAR_GAMEDLL|FCVAR_CHEAT
// voice_sequence_maximum_wait_time                          float32  0.5                   FCVAR_DEVELOPMENTONLY  // When receiving packets out of sequence, wait this many seconds for missing sequences to arrive
// voice_serverdebug                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// voice_stall_ms                                            float32  250                   FCVAR_DEVELOPMENTONLY
// voice_test_log_send                                       bool     false                 FCVAR_RELEASE
// voice_threshold                                           float32  -120                  FCVAR_CLIENTDLL|FCVAR_ARCHIVE  // decibel threshold for how loud the talker's input signal is before we think they are talking.
// voice_threshold_attack                                    float32  0.3                   FCVAR_DEVELOPMENTONLY  // Amount of time we buffer outgoing audio to detect an onset.
// voice_threshold_delay                                     float32  0.7                   FCVAR_DEVELOPMENTONLY  // Amount of time the talker is silent before we infer that they are no longer talking.
// voice_threshold_hold                                      float32  0.2                   FCVAR_DEVELOPMENTONLY  // Amount of time after the talker starts talking we should keep listening regardless of how loud they are speaking.
// voice_threshold_ramp_min_db                               float32  -60                   FCVAR_DEVELOPMENTONLY  // A dB floor of when to stop transmitting packets, the volume between this and voice_threshold will still transmit packets to allow for volume ramping.
// voice_vox                                                 int32    0                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Voice chat uses a vox-style always on
// voice_vox_current_peak                                    float32  0                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Current peak value (out of 64k) of the incoming voice stream
// volume                                                    float32  0.021137              FCVAR_ARCHIVE  // Sound volume
// volume_fog_debug_volumes                                  bool     false                 FCVAR_CHEAT
// volume_fog_density_scale                                  float32  1                     FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Scale global volume fog density
// volume_fog_depth                                          int32    128                   FCVAR_DEVELOPMENTONLY  // Depth of volume fog texture
// volume_fog_depth_warp                                     float32  7                     FCVAR_DEVELOPMENTONLY
// volume_fog_depth_warp_debug                               bool     false                 FCVAR_DEVELOPMENTONLY
// volume_fog_dither_scale                                   float32  1                     FCVAR_CHEAT
// volume_fog_enable_jitter                                  bool     true                  FCVAR_CHEAT
// volume_fog_height                                         int32    160                   FCVAR_DEVELOPMENTONLY  // Height of volume fog texture
// volume_fog_intermediate_textures_hdr                      bool     true                  FCVAR_DEVELOPMENTONLY
// volume_fog_shadow_penumbra_multiplier                     float32  3                     FCVAR_DEVELOPMENTONLY  // Penumbra size multiplier for shadow sampling, reduces fog shadow aliasing
// volume_fog_temporal_filter                                bool     true                  FCVAR_DEVELOPMENTONLY
// volume_fog_temporal_weight                                float32  0.9                   FCVAR_DEVELOPMENTONLY  // Temporal filtering weight
// volume_fog_width                                          int32    240                   FCVAR_DEVELOPMENTONLY  // Width of volume fog texture
// vprof_counters                                            int32    0                     FCVAR_DEVELOPMENTONLY
// vprof_counters_show_minmax                                bool     false                 FCVAR_DEVELOPMENTONLY
// vprof_scope_entity_clientthink                            bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Does nothing whatsoever.
// vprof_scope_entity_thinks                                 bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// vprof_think_limit                                         bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// vt_sim_streaming_delay_ms                                 float32  500                   FCVAR_DEVELOPMENTONLY
// weapon_accuracy_forcespread                               float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Force spread to the specified value.
// weapon_accuracy_logging                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_REPLICATED
// weapon_accuracy_nospread                                  bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Disable weapon inaccuracy spread
// weapon_accuracy_reset_on_deploy                           bool     false                 FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE  // On deploy, forcibly reset weapon accuracy to zero.
// weapon_accuracy_shotgun_spread_patterns                   bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE
// weapon_accuracy_stack_boost_limit                         int32    2                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Apply ladder inaccuracy to players boosted by a stack of this many (or more) players
// weapon_air_spread_scale                                   float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // Scale factor for jumping inaccuracy, set to 0 to make jumping accuracy equal to standing
// weapon_all_nametag                                        bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// weapon_all_stattrak                                       bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// weapon_auto_cleanup_time                                  float32  0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If set to non-zero, weapons will delete themselves after the specified time (in seconds) if no players are near.
// weapon_debug_inaccuracy_only_up                           bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Force weapon inaccuracy to be in exactly the up direction
// weapon_debug_max_inaccuracy                               bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // Force all shots to have maximum inaccuracy
// weapon_debug_spread_gap                                   float32  0.67                  FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_PER_USER
// weapon_debug_spread_show                                  int32    0                     FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_PER_USER  // Enables display of weapon accuracy; 1: show accuracy box, 3: show accuracy with dynamic crosshair
// weapon_land_dip_amt                                       float32  20                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT  // The amount the gun should dip when the player lands after a jump.
// weapon_max_before_cleanup                                 int32    0                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // If set to non-zero, will remove the oldest dropped weapon to maintain the specified number of dropped weapons in the world.
// weapon_molotov_maxdetonateslope                           float32  30                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED  // Maximum angle of slope on which the molotov will detonate
// weapon_near_empty_sound                                   bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT
// weapon_random_stickers                                    bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// weapon_reticle_knife_show                                 bool     true                  FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_RELEASE  // When enabled will show knife reticle on clients. Used for game modes requiring target id display when holding a knife.
// weapon_skin_force_legacy                                  int32    -1                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED
// weapon_skins                                              bool     true                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// weapon_skins_on_default                                   bool     false                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// weapon_sound_falloff_multiplier                           float32  1                     FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_RELEASE  // Scaling for falloff of weapon firing sounds
// webapi_values_init_buffer_size                            int32    65536                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Initial buffer size for buffers in the WebAPIValues buffer pool
// webapi_values_max_pool_size_mb                            uint32   400                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Maximum size in bytes of the WebAPIValues buffer pool
// wind_system_debug_volumes                                 bool     false                 FCVAR_DEVELOPMENTONLY
// wind_system_default_resolution_xy                         int32    256                   FCVAR_DEVELOPMENTONLY
// wind_system_default_resolution_z                          int32    32                    FCVAR_DEVELOPMENTONLY
// wind_system_default_sample_min_spacing                    float32  12                    FCVAR_DEVELOPMENTONLY
// wind_system_temporal_smoothing                            bool     true                  FCVAR_DEVELOPMENTONLY
// zoom_sensitivity_ratio                                    float32  1                     FCVAR_CLIENTDLL|FCVAR_ARCHIVE|FCVAR_PER_USER  // Additional mouse sensitivity scale factor applied when FOV is zoomed in.

// === ConCommands ===
// name | flags | description
// +bugvoice                                                 FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Start recording bug voice attachment.
// +camdistance                                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// +cammousemove                                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// +cl_show_team_equipment                                   FCVAR_CLIENTDLL|FCVAR_RELEASE
// +lookatweapon                                             FCVAR_CLIENTDLL|FCVAR_RELEASE
// +quickbuyradial                                           FCVAR_CLIENTDLL|FCVAR_RELEASE
// +quickgearradial                                          FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// +quickgrenaderadial                                       FCVAR_CLIENTDLL|FCVAR_RELEASE
// +quickinv                                                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// +radialradio                                              FCVAR_CLIENTDLL|FCVAR_RELEASE
// +radialradio2                                             FCVAR_CLIENTDLL|FCVAR_RELEASE
// +radialradio3                                             FCVAR_CLIENTDLL|FCVAR_RELEASE
// +spray_menu                                               FCVAR_CLIENTDLL|FCVAR_RELEASE
// -bugvoice                                                 FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Finish recording bug voice attachment.
// -camdistance                                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// -cammousemove                                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// -cl_show_team_equipment                                   FCVAR_CLIENTDLL|FCVAR_RELEASE
// -lookatweapon                                             FCVAR_CLIENTDLL|FCVAR_RELEASE
// -quickbuyradial                                           FCVAR_CLIENTDLL|FCVAR_RELEASE
// -quickgearradial                                          FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// -quickgrenaderadial                                       FCVAR_CLIENTDLL|FCVAR_RELEASE
// -quickinv                                                 FCVAR_CLIENTDLL|FCVAR_RELEASE
// -radialradio                                              FCVAR_CLIENTDLL|FCVAR_RELEASE
// -radialradio2                                             FCVAR_CLIENTDLL|FCVAR_RELEASE
// -radialradio3                                             FCVAR_CLIENTDLL|FCVAR_RELEASE
// -spray_menu                                               FCVAR_CLIENTDLL|FCVAR_RELEASE
// ShowSteamStatsSessionID                                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Prints out the game stats session ID's (developer convar must be set to non-zero).
// StackStats_Dump                                           FCVAR_DEVELOPMENTONLY  // Dump a named stackstats structure to disk. Usage: stackstats_dump "structname" ["filename"]
// Test_Checkpoint                                           FCVAR_DEVELOPMENTONLY  // Indicate to a test script that a checkpoint has been reached
// Test_CreateEntity                                         FCVAR_GAMEDLL|FCVAR_CHEAT
// Test_EHandle                                              FCVAR_GAMEDLL|FCVAR_CHEAT
// Test_ExitProcess                                          FCVAR_CHEAT  // Test_ExitProcess <exit code> - immediately kill the process.
// Test_Loop                                                 FCVAR_DEVELOPMENTONLY  // Test_Loop <loop name> - loop back to the specified loop start point unconditionally.
// Test_LoopCount                                            FCVAR_DEVELOPMENTONLY  // Test_LoopCount <loop name> <count> - loop back to the specified loop start point the specified # of times.
// Test_LoopForNumSeconds                                    FCVAR_DEVELOPMENTONLY  // Test_LoopForNumSeconds <loop name> <time> - loop back to the specified start point for the specified # of seconds.
// Test_RandomChance                                         FCVAR_DEVELOPMENTONLY  // Test_RandomChance <percent chance, 0-100> <token1> <token2...> - Roll the dice and maybe run the command following the percentage chance.
// Test_RandomPlayerPosition                                 FCVAR_GAMEDLL|FCVAR_CHEAT
// Test_StartLoop                                            FCVAR_DEVELOPMENTONLY  // Test_StartLoop <loop name> - Denote the start of a loop. Really just defines a named point you can jump to.
// Test_StartScript                                          FCVAR_DEVELOPMENTONLY  // Start a test script running..
// _record                                                   FCVAR_CHEAT|FCVAR_DONTRECORD|FCVAR_RELEASE  // Record a demo incrementally.
// _resetgamestats                                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Erases current game stats and writes out a blank stats file
// addip                                                     FCVAR_DEVELOPMENTONLY  // Add an IP address to the ban list.
// addons                                                    FCVAR_DEVELOPMENTONLY  // list current addon info.
// adjacent_levels                                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // List adjacent levels
// adjacent_preload                                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Preload adjacennt level data - will override regular changelevel code -- PROTOTYPE/WIP
// alias                                                     FCVAR_RELEASE  // Alias a command.
// anim_eval_stats                                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Displays stats about how many EvaluatePose calls are unused
// animevents_dump                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // List all the currently registered anim events.

// animgraph_dump_update_list                                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Displays stats about which animations are updating
// annotation_append                                         FCVAR_CLIENTDLL|FCVAR_RELEASE  // Load annotation to a file without clearing existing annotations
// annotation_clear                                          FCVAR_CLIENTDLL|FCVAR_RELEASE  // Clear all annotation
// annotation_create                                         FCVAR_CLIENTDLL|FCVAR_RELEASE  // Creates an annotation
// annotation_delete_previous_node_set                       FCVAR_CLIENTDLL|FCVAR_RELEASE  // Delete the last node set created
// annotation_load                                           FCVAR_CLIENTDLL|FCVAR_RELEASE  // Load annotation to a file after first clearing existing annotations
// annotation_reload                                         FCVAR_CLIENTDLL|FCVAR_RELEASE  // Reload the annotation file
// annotation_reload_language_file                           FCVAR_CLIENTDLL|FCVAR_RELEASE  // Creates an annotation
// annotation_save                                           FCVAR_CLIENTDLL|FCVAR_RELEASE  // Save annotation to a file
// apply_crosshair_code                                      FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Apply a crosshair code to the current crosshair settings.
// auto_bug                                                  FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // auto_bug : create non-interactive bug report.
// autobuy                                                   FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Attempt to purchase items with the order listed in cl_autobuy
// autosave                                                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Autosave
// autosavedangerous                                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // AutoSaveDangerous
// autosavedangerousissafe                                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// axis                                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw an axis
	Arguments:  x y z pitch yaw roll <lifetime = 10.0> <r g b a>

// bake_bomb_damage_render_visualization                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// banid                                                     FCVAR_DEVELOPMENTONLY  // Add a user ID to the ban list.
// banip                                                     FCVAR_DEVELOPMENTONLY  // Add an IP address to the ban list.
// benchframe                                                FCVAR_RELEASE  // Takes a snapshot of a particular frame in a time demo.
// bind                                                      FCVAR_RELEASE  // Bind a key.
// binddefaults                                              FCVAR_RELEASE  // Bind all keys to their default values.
// bindss                                                    FCVAR_RELEASE  // Bind a key for a particular splitscreen player.
// bindtoggle                                                FCVAR_DEVELOPMENTONLY  // Performs a bind <key> "increment var <cvar> 0 1 1".
// blink                                                     FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Blink specified convar value between two values at the specified duration.
// bot_add                                                   FCVAR_GAMEDLL|FCVAR_RELEASE  // bot_add <t|ct> <type> <difficulty> <name> - Adds a bot matching the given criteria.
// bot_add_ct                                                FCVAR_GAMEDLL|FCVAR_RELEASE  // bot_add_ct <type> <difficulty> <name> - Adds a Counter-Terrorist bot matching the given criteria.
// bot_add_t                                                 FCVAR_GAMEDLL|FCVAR_RELEASE  // bot_add_t <type> <difficulty> <name> - Adds a terrorist bot matching the given criteria.
// bot_all_weapons                                           FCVAR_GAMEDLL|FCVAR_RELEASE  // Allows the bots to use all weapons
// bot_goto_mark                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Sends a bot to the marked nav area (useful for testing navigation meshes)
// bot_goto_selected                                         FCVAR_GAMEDLL|FCVAR_CHEAT  // Sends a bot to the selected nav area (useful for testing navigation meshes)
// bot_hurt                                                  FCVAR_GAMEDLL|FCVAR_CHEAT
// bot_kick                                                  FCVAR_GAMEDLL|FCVAR_RELEASE  // bot_kick <all> <t|ct> <type> <difficulty> <name> - Kicks a specific bot, or all bots, matching the given criteria.
// bot_kill                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // bot_kill <all> <t|ct> <type> <difficulty> <name> - Kills a specific bot, or all bots, matching the given criteria.
// bot_knives_only                                           FCVAR_GAMEDLL|FCVAR_RELEASE  // Restricts the bots to only using knives
// bot_path                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // bot_path <all> <t|ct> <type> <difficulty> <name> - Tells a specific bot to follow a human path, matching the given criteria.
// bot_pistols_only                                          FCVAR_GAMEDLL|FCVAR_RELEASE  // Restricts the bots to only using pistols
// bot_place                                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // bot_place - Places a bot from the map at where the local player is pointing.
// bot_snipers_only                                          FCVAR_GAMEDLL|FCVAR_RELEASE  // Restricts the bots to only using sniper rifles
// box                                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Draw a bbox
	Arguments:  minx miny miny maxx maxy maxz <lifetime = 10.0> <r g b a>

// breakable_force_break                                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Force a breakable to break
// bug                                                       FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // bug [auto_fill_tokens] [-title <text>] [-noscreenshot] : Activate the bug reporter.
// bugbug                                                    FCVAR_CLIENTDLL|FCVAR_RELEASE  // bugbug
// bugvoice_clear                                            FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Clear voice attachment data.
// bugvoice_save                                             FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Write buffered voice attachment data to file.
// buildcubemaps                                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Build Cubemaps
// buildsparseshadowtree                                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Build Sparse Shadow Tree
// button_info                                               FCVAR_RELEASE  // Display information about the specified key or button.
// buymenu                                                   FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE  // Show or hide main buy menu
// buyrandom                                                 FCVAR_GAMEDLL  // Buy random primary and secondary. Primarily for deathmatch where cost is not an issue.
// callvote                                                  FCVAR_GAMEDLL  // Start a vote on an issue.
// cam_command                                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Tells camera to change modes
// camera_cut_to_datadriven_camera                           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// camera_path_add                                           FCVAR_CLIENTDLL|FCVAR_CHEAT
// camera_path_clear_all                                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// camera_path_delete                                        FCVAR_CLIENTDLL|FCVAR_CHEAT
// camera_path_demo                                          FCVAR_CLIENTDLL|FCVAR_CHEAT
// camera_path_load                                          FCVAR_CLIENTDLL|FCVAR_CHEAT
// camera_path_save                                          FCVAR_CLIENTDLL|FCVAR_CHEAT
// camerazoomin                                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// camerazoomout                                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// camortho                                                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Switch to orthographic camera.
// cancelselect                                              FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// capturecubemap                                            FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Capture Cubemap
// cast_aabb                                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests box collision detection
// cast_bullet                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests bullet cast
// cast_capsule                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests capsule collision detection
// cast_convex                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests convex hull collision detection
// cast_cylinder                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests cylinder collision detection
// cast_intervals                                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests interval ray cast
// cast_obb                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests cylinder collision detection
// cast_physics                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests physics shape collision detection
// cast_ray                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests ray cast
// cast_sphere                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Tests sphere cast
// cc_emit                                                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Emits a closed caption
// changelevel                                               FCVAR_RELEASE  // changelevel <mapname> :Multiplayer change level.
// check_nofilefd                                            FCVAR_DEVELOPMENTONLY  // Print the current number of FDs reported by getrlimit
// cl_anim_eval_stats                                        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Displays stats about how many EvaluatePose calls are unused
// cl_animgraph_dump_update_list                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Displays stats about which animations are updating
// cl_avatar_convert_png                                     FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE  // Converts all rgb avatars in the avatars directory to png
// cl_avatar_convert_rgb                                     FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_RELEASE  // Converts all png avatars in the avatars directory to rgb
// cl_axis                                                   FCVAR_CLIENTDLL|FCVAR_CHEAT  // Draw an axis
	Arguments:  x y z pitch yaw roll <lifetime = 10.0> <r g b a>

// cl_box                                                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Draw a bbox
	Arguments:  minx miny miny maxx maxy maxz <lifetime = 10.0> <r g b a>

// cl_break_on_missing_resource                              FCVAR_DEVELOPMENTONLY  // Break in debugger when missing resource match is found.
Format: cl_break_on_missing_resource <substring> /(empty to break on all) / 0 to turn off.

// cl_checkdeclareclasses                                    FCVAR_CHEAT  // Check game code serializers
// cl_cs_dump_econ_item_stringtable                          FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // cl_cs_dump_econ_item_stringtable
// cl_debugoverlay_cycle_domain                              FCVAR_CLIENTDLL|FCVAR_CHEAT  // Toggles visibility of the debug overlay system.
// cl_debugoverlay_cycle_state                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Toggles visibility of the debug overlay system.
// cl_debugoverlay_dashboard                                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Makes the debug overlay dashboard visible.
// cl_debugoverlay_hide_imgui                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // Hides the overlay.
// cl_debugoverlay_toggle                                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Toggles visibility of the debug overlay system.
// cl_destroy_ragdolls                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Destroys all client-side ragdolls
// cl_dev_decaltrace_blood                                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // Shoot out a decal spray that shoots blood.
// cl_drawcross                                              FCVAR_CLIENTDLL|FCVAR_CHEAT  // Draws a cross at the given location
	Arguments: x y z
// cl_drawline                                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Draws line between two 3D Points.
	Green if no collision
	Red is collides with something
	Arguments: x1 y1 z1 x2 y2 z2
// cl_dump_projected_texture_count                           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Print out number of active projected textures
// cl_dump_response_symbols                                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // print all response symbols to the console
// cl_dumpentity                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Dumps info about an entity
// cl_dumpsplithacks                                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump split screen workarounds.
// cl_ent_absbox                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the total bounding box for the given entity(s) in green.  Some entites will also display entity specific overlays.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_actornames                                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the entity name for all entities that have ShouldDisplayInActorNames true in code
// cl_ent_animgraph2_open_graph                              FCVAR_CLIENTDLL|FCVAR_CHEAT  // Opens the graph and starts live debugging the AG2 graph for a given entity
	Arguments: entityName
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_animgraph2_record                                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Starts live debugging & recording the AG2 graph for a given entity
	Arguments: entityName
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_animgraph_debug                                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays debug draws about the given entity(ies) animgraph
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_attachments                                        FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the attachment points on an entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_bbox                                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the movement bounding box for the given entity(ies) in orange.  Some entites will also display entity specific overlays.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_call                                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // ent_call <funcname> <option:entname> calls function on current look target or filtername, checks on ent, then root, then mode, then map scope
// cl_ent_clear_debug_overlays                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Clears all debug overlays
// cl_ent_find                                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Find and list all entities with classnames or targetnames that contain the specified substrings.
Format: find_ent <substring>

// cl_ent_find_index                                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // Display data for entity matching specified index.
Format: find_ent_index <index>

// cl_ent_grab                                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // grabs the object in front of the player. Options: -loose -multiple -toggle
// cl_ent_hierarchy                                          FCVAR_CLIENTDLL|FCVAR_CHEAT  // Prints the entity hierarchy tree rooted at the specified ent(s)
// cl_ent_hitbox                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the hitboxes for the given entity(ies).
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_joints                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the joint names + axes an entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_messages                                           FCVAR_CLIENTDLL|FCVAR_CHEAT  // Toggles input/output message display for the selected entity(ies).  The name of the entity will be displayed as well as any messages that it sends or receives.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player i
// cl_ent_name                                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the entity name
// cl_ent_picker                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Toggles 'picker' mode.  When picker is on, the bounding box, pivot and debugging text is displayed for whatever entity the player is looking at.
	Arguments:	full - enables all debug information
// cl_ent_pivot                                              FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the pivot for the given entity(ies).
	(y=up=green, z=forward=blue, x=left=red). 
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_remove                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Removes the given entity(s)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_remove_all                                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // Removes all entities of the specified type
	Arguments:   	{entity_name} / {class_name} 
// cl_ent_scale                                              FCVAR_CLIENTDLL|FCVAR_CHEAT  // Scales entities.	Arguments: <scale factor> <{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}>
// cl_ent_scenehierarchy                                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Prints the entity scenenode hierarchy tree rooted at the specified ent(s)
// cl_ent_script_dump                                        FCVAR_CLIENTDLL|FCVAR_CHEAT  // Dumps the names and values of this entity's script scope to the console
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_select                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Select or deselects the given entities(s) for later manipulation
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_setang                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Set entity angles
// cl_ent_setname                                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Sets the targetname of the given entity(s)
	Arguments:   	<new entity name> <{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}>
// cl_ent_setpos                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Move entity to position
// cl_ent_show_damage                                        FCVAR_CLIENTDLL|FCVAR_CHEAT  // Sets damage display mode.  When on, you will see the amount of damage dealt over the target's head.
// cl_ent_skeleton                                           FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the skeleton for the given entity(ies).
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_spew_derived_classes                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Prints out all entity classes which inherit from a specified base class
// cl_ent_text                                               FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Displays text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_text256                                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays text debugging information about the given entity(ies) [within 256 units of the player] on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_text_clear                                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // Hide text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_text_filter                                        FCVAR_CLIENTDLL|FCVAR_CHEAT  // Set which ent_text filters you want: 
// cl_ent_text_radius                                        FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays text debugging information about the given entity(ies) [near the player] on top of the entity (See Overlay Text)
	2 Arguments:   	<Radius> <{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}>
// cl_ent_text_sticky_add                                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Adds to list of names to display text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_text_sticky_clear                                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Clears the list of names to display text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_text_sticky_dump                                   FCVAR_CLIENTDLL|FCVAR_CHEAT  // Spews the list of names to display text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_text_sticky_remove                                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Removes from the list of names to display text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_ungrab                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // un-grabs all objects
// cl_ent_vcollide_wireframe                                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the interpolated vcollide wireframe pm am entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_viewoffset                                         FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the eye position for the given entity(ies) in red.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_ent_visibility_traces                                  FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Displays visibility traces for the given entity
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_entityreport                                           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Reports all extant entities. Optional 2nd arg is a substring of a classname that the list will be filtered by.
// cl_entitysummary                                          FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Summarizes (by class) all extant entities. Optional 2nd arg is a substring of a classname that the list will be filtered by.
// cl_ents                                                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // List client entities, sorted by spawn group
// cl_fullupdate                                             FCVAR_CHEAT  // Force uncompressed update
// cl_game_mode_convars                                      FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Display the values of the convars for the current game_mode.
// cl_groups                                                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Show status of all spawn groups.
// cl_imgui_debug_entity                                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Shows the entity browser, focused on the entity you specify.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cl_imgui_set_selection                                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Sets ImGui selection
// cl_imgui_set_status_text                                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Sets ImGui header status text
// cl_in_forcebuttonstate                                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_VCONSOLE_FUZZY_MATCHING  // Forces a button to be a particular state - WHEN PROCESSING USERCOMMANDS
// cl_interp                                                 FCVAR_CLIENTDLL|FCVAR_RELEASE  // Read the effective client simulation interpolation amount in terms of time.
// cl_mainmenu_hide_blog                                     FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Show the news panel and hide blog
// cl_mainmenu_show_blog                                     FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Show the blog and hide news panel
// cl_matchstats_print_own_data                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // cl_matchstats_print_own_data RANGENAME
// cl_net_printsummary                                       FCVAR_DONTRECORD|FCVAR_RELEASE  // Print a summary report of Source2 engine networking statistics.  (Ticks, netchan messages, etc.)
// cl_particles_dump_effects                                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// cl_particles_dumplist                                     FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump all new particles, optional name substring.
// cl_particles_dumpsimlist                                  FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump all simulating particles, optional name substring.
// cl_phys_create_test_character_proxy                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Create test character proxy
// cl_phys_dump_intersection_controller                      FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump intersection controller status
// cl_phys_dump_main_world                                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump physics main world to file
// cl_phys_dump_memory                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump memory usage
// cl_phys_list                                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // List all physics component contents of every entity in the game;
    -stream [1|0]         initiate|terminate streaming to physics debugger
    -allents              include non-physical entities
    -classes              print class names
    -sdk        
// cl_phys_sleep                                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Put all physics in all the worlds to sleep
// cl_phys_wakeup                                            FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Wake all physics objects in the Main physics up
// cl_physics_add_test                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // add test object
// cl_physics_highlight_active                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Turns on the absbox for all active physics objects.
  0 : un-highlight.

// cl_physics_remove_test                                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // remove test object
// cl_physics_report_active                                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Lists all active physics objects
  -more : extra info

// cl_precacheinfo                                           FCVAR_DEVELOPMENTONLY  // Show precache info (client).
// cl_pred_track                                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // <entindex> <fieldname>:  Track changes to entity index entindex, for field fieldname.
// cl_pred_track_off                                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // clear field track changes.
// cl_predictioncopy_describe                                FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Describe PredictionMap_t for entindex
// cl_predictioncopy_print                                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Print simple description of prediction copy fields for entindex
// cl_printfps                                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Print information from cl_showfps.
// cl_prop_debug                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Toggle prop debug mode. If on, props will show colorcoded bounding boxes. Red means ignore all damage. White means respond physically to damage but never break. Green maps health in the range of 100 down to 1.
// cl_querycache_stats                                       FCVAR_CLIENTDLL|FCVAR_CHEAT  // Display status of the query cache (client only)
// cl_reload_hud                                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Reloads the hud scale and resets scale and borders
// cl_removedecals                                           FCVAR_CLIENTDLL|FCVAR_CHEAT  // Remove the decals from the entity under the crosshair.
// cl_report_entities                                        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // Lists all entities
// cl_report_predcopy_overrides                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Report prediction copy overrides
// cl_report_simthinklist                                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Lists all simulating/thinking entities
// cl_report_soundpatch                                      FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // reports client-side sound patch count
// cl_resetfps                                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Reset information from cl_showfps.
// cl_rr_dump_rules                                          FCVAR_CLIENTDLL|FCVAR_CHEAT  // Print all response rules
// cl_script_add_debug_filter                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // Add a filter to the game debug overlay
// cl_script_add_watch                                       FCVAR_CLIENTDLL|FCVAR_CHEAT  // Add a watch to the game debug overlay
// cl_script_add_watch_pattern                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Add a watch to the game debug overlay
// cl_script_attach_debugger                                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Connect the vscript VM to the script debugger
// cl_script_clear_watches                                   FCVAR_CLIENTDLL|FCVAR_CHEAT  // Clear all watches from the game debug overlay
// cl_script_debug                                           FCVAR_CLIENTDLL|FCVAR_CHEAT  // Toggle the in-game script debug features
// cl_script_dump_all                                        FCVAR_CLIENTDLL|FCVAR_CHEAT  // Dump the state of the VM to the console
// cl_script_find                                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Find a key in the VM 
// cl_script_help                                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Output help for script functions
// cl_script_help2                                           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Output help for script functions suitable for auto-completion
// cl_script_reload                                          FCVAR_CLIENTDLL|FCVAR_CHEAT  // Reload scripts
// cl_script_reload_code                                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Execute a vscript file, replacing existing functions with the functions in the run script
// cl_script_reload_entity_code                              FCVAR_CLIENTDLL|FCVAR_CHEAT  // Execute all of this entity's VScripts, replacing existing functions with the functions in the run scripts
// cl_script_remove_debug_filter                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Remove a filter from the game debug overlay
// cl_script_remove_watch                                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Remove a watch from the game debug overlay
// cl_script_remove_watch_pattern                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Remove a watch from the game debug overlay
// cl_script_resurrect_unreachable                           FCVAR_CLIENTDLL|FCVAR_CHEAT  // Use the garbage collector to track down reference cycles
// cl_script_trace_disable                                   FCVAR_CLIENTDLL|FCVAR_CHEAT  // Turn off a particular trace output by file or function name
// cl_script_trace_disable_all                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Turn off all trace output
// cl_script_trace_disable_key                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Turn off a particular trace output by table/instance
// cl_script_trace_enable                                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // Turn on a particular trace output by file or function name
// cl_script_trace_enable_all                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // Turn on all trace output
// cl_script_trace_enable_key                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // Turn on a particular trace output by table/instance
// cl_showents                                               FCVAR_CLIENTDLL|FCVAR_CHEAT  // Dump entity list to console.
// cl_sim_grenade_trajectory                                 FCVAR_CLIENTDLL|FCVAR_CHEAT  // Draw trajectory of the deployed grenade if thrown from this position. Takes an optional parameter for how long the drawn trajectory will last.
// cl_sos_test_get_opvar                                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_sos_test_set_opvar                                     FCVAR_CLIENTDLL|FCVAR_CHEAT
// cl_soundscape_flush                                       FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_SERVER_CAN_EXECUTE  // Flushes the client side soundscapes
// cl_soundscape_printdebuginfo                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // print soundscapes
// cl_spewserializers                                        FCVAR_CHEAT  // Spew serializers
// cl_spewworldgroups                                        FCVAR_DEVELOPMENTONLY  // Spew world groups (client)
// cl_ss_origin                                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // print origin in script format
// cl_test_list_entities                                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // test-list entities
// cl_ticktiming                                             FCVAR_DONTRECORD|FCVAR_RELEASE  // {print|<interval>} [summary|detail]  Print timing stats now, or set report interval
// cl_updatevisibility                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Updates visibility bits.
// clear                                                     FCVAR_DONTRECORD|FCVAR_RELEASE  // Clear console output.
// clear_bombs                                               FCVAR_GAMEDLL|FCVAR_CHEAT
// clearall                                                  FCVAR_DONTRECORD|FCVAR_RELEASE  // Clear console output from all views.
// cli_ent_attachments                                       FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the interpolated attachment points on an entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cli_ent_hitbox                                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the skeleton for the given entity(ies).
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cli_ent_pivot                                             FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the interpolated pivot for the given entity(ies).
	(y=up=green, z=forward=blue, x=left=red). 
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cli_ent_skeleton                                          FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the skeleton for the given entity(ies).
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// cli_ent_vcollide_wireframe                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the interpolated vcollide wireframe pm am entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// clutch_mode_toggle                                        FCVAR_CLIENTDLL|FCVAR_RELEASE  // Toggle clutch mode convar
// cmd                                                       FCVAR_DEVELOPMENTONLY  // Forward command to server.
// collect_entity_model_name                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Collect model names of the entities you're pointing at
// commentary_cvarsnotchanging                               FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// commentary_finishnode                                     FCVAR_GAMEDLL
// condump                                                   FCVAR_RELEASE  // dump the text currently in the console to condumpXX.log
// confirm_abandon_match                                     FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Confirm that we wish to abandon match
// confirm_activate_itemid_now                               FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Confirm item activation by item id
// confirm_join_friend_session_exit_current                  FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Confirm that we wish to join a friend session, destroying a previous session
// confirm_join_new_session_exit_current                     FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Confirm that we wish to join a new session, destroying a previous session
// confirm_join_party_session_exit_current                   FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Confirm that we wish to join a party session, destroying a previous session
// confirm_watch_friend_session_exit_current                 FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Confirm that we wish to watch a friend session, destroying a previous session
// connect                                                   FCVAR_RELEASE  // Connect to a remote server.
// connect_hltv                                              FCVAR_RELEASE  // Connect to a remote HLTV server.
// console_test                                              FCVAR_DEVELOPMENTONLY  // Output text to test console
// consoletool                                               FCVAR_DONTRECORD|FCVAR_RELEASE  // Open a VConsole subtool.
// convert_steamid                                           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Convert SteamID into multiple formats
// cpuinfo                                                   FCVAR_DEVELOPMENTONLY  // Print CPU configuration information
// crash                                                     FCVAR_CHEAT  // Crash the client. Optional parameter -- type of crash:
 0: read from NULL
 1: write to NULL
 2: force an Assert
 3: infinite loop
 4: stack buffer overrun
 5: multiple asserts across multiple threads. Optional number of threads (default 5)
 6: looping memo
// crash_error                                               FCVAR_CHEAT  // Cause the engine to crash by Plat_FatalError on main thread (Debug!!)
// crash_error_job                                           FCVAR_CHEAT  // Cause the engine to crash by Plat_FatalError on job thread (Debug!!)
// crash_error_thread                                        FCVAR_CHEAT  // Cause the engine to crash by Plat_FatalError on non-main thread (Debug!!)
// crash_job                                                 FCVAR_CHEAT  // Cause the engine to crash in a job thread (Debug!!)
// crash_thread                                              FCVAR_CHEAT  // Cause the engine to crash in a brand new non-main thread (Debug!!)
// create_radius_damage                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Causes radius damage where you're looking, at the passed in radius.
// creditsdone                                               FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// cs_quit_prompt                                            FCVAR_CLIENTDLL|FCVAR_RELEASE  // Quit the game
// csgo_download_match                                       FCVAR_CLIENTDLL|FCVAR_DONTRECORD|FCVAR_CLIENTCMD_CAN_EXECUTE  // Downloads a match via serial code and starts playback
// csgo_econ_action_preview                                  FCVAR_CLIENTDLL|FCVAR_DONTRECORD|FCVAR_CLIENTCMD_CAN_EXECUTE  // Preview an economy item
// csgo_watch_friend_session_exit_current                    FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE
// cvarlist                                                  FCVAR_RELEASE  // Show the list of convars/concommands.
// cyclevar                                                  FCVAR_DONTRECORD|FCVAR_RELEASE  // Cycle through specified convar values.
// dbghist_addline                                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Add a line to the debug history. Format: <category id> <line>
// dbghist_dump                                              FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Dump the debug history to the console. Format: <category id>
    Categories:
     0: Entity I/O
     1: AI Decisions
     2: Scene Print
     3: Alyx Blind
     4: Log of damage done to player
	 5: Player Teleport
	 6: Blind Zombie Sounds
	 7: Player Conti
// debug_purchase_defidx                                     FCVAR_CLIENTDLL|FCVAR_RELEASE|FCVAR_CLIENTCMD_CAN_EXECUTE  // Purchase an item by defindex
// debugoverlay_cycle_domain                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggles visibility of the debug overlay system.
// debugoverlay_cycle_state                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggles visibility of the debug overlay system.
// debugoverlay_dashboard                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Makes the debug overlay dashboard visible.
// debugoverlay_hide_imgui                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Hides the overlay.
// debugoverlay_toggle                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggles visibility of the debug overlay system.
// demo_goto                                                 FCVAR_RELEASE  // Skips to location in demo.
// demo_gotomark                                             FCVAR_RELEASE  // Skips the current demo playback to the marked tick
// demo_gototick                                             FCVAR_RELEASE  // Skips to a tick in demo.
// demo_info                                                 FCVAR_RELEASE  // Print information about currently playing demo.
// demo_marktick                                             FCVAR_RELEASE  // Marks the current demo playback tick for later use
// demo_pause                                                FCVAR_RELEASE  // Pauses demo playback.
// demo_pauseatservertick                                    FCVAR_RELEASE  // Pauses when the 'render time' reaches the specified tick.
// demo_resume                                               FCVAR_RELEASE  // Resumes demo playback.
// demo_step_tick                                            FCVAR_RELEASE  // Play for N ticks (default=1) and then pause.
// demo_timescale                                            FCVAR_RELEASE  // Sets demo replay speed.
// demo_togglepause                                          FCVAR_RELEASE  // Toggles demo playback.
// demo_writemetafile                                        FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // save current meta file demo_<version>.meta file for use in demo upconversion.
// demolist                                                  FCVAR_RELEASE  // Print demo sequence list.
// demoui                                                    FCVAR_CLIENTDLL|FCVAR_RELEASE  // Show/hide demo playback ui
// dev_send_gc_message                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // <msgid> Send a blank body message with a given ID to gc for routing tests
// dev_send_gc_message_server                                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // <msgid> Send a blank body message with a given ID to gc for routing tests
// dev_simulate_gcdown                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // <state> Turn on/off simulated GC communications failure (GC is down in a way that we know it is down)
// differences                                               FCVAR_RELEASE  // Show all convars which are not at their default values (optional restricted to specific flags).
// disable_priority_boost                                    FCVAR_DEVELOPMENTONLY  // Disable focus based priority boost
// disconnect                                                FCVAR_RELEASE  // Disconnect from server
// dlight_debug                                              FCVAR_CLIENTDLL|FCVAR_CHEAT  // Creates a dlight in front of the player
// dm_reset_spawns                                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// dm_togglerandomweapons                                    FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE|FCVAR_CLIENTCMD_CAN_EXECUTE  // Turns random weapons in deathmatch on/off
// drawcross                                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Draws a cross at the given location
	Arguments: x y z
// drawline                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Draws line between two 3D Points.
	Green if no collision
	Red is collides with something
	Arguments: x1 y1 z1 x2 y2 z2
// drawoverviewmap                                           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Draws the overview map
// drawradar                                                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Draws HUD radar
// drop_hostage                                              FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // drop held hostage
// ds_workshop_changelevel                                   FCVAR_GAMEDLL|FCVAR_RELEASE  // Changelevel to an available workshop map by name
// ds_workshop_listmaps                                      FCVAR_GAMEDLL|FCVAR_RELEASE  // Dump workshop maps available on this server
// dump_entity_report                                        FCVAR_CLIENTDLL|FCVAR_CHEAT  // List all client-side entities in the scene
// dump_globals                                              FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Dump all global entities/states
// dump_loc_token                                            FCVAR_DEVELOPMENTONLY  // List information on the given token
// dump_localization_files                                   FCVAR_DEVELOPMENTONLY  // List all loaded localization files
// dump_panorama_css_properties                              FCVAR_RELEASE  // Prints out all valid panorama CSS properties and their documentation
// dump_panorama_events                                      FCVAR_RELEASE  // print panorama event types and their documentation
// dump_panorama_render_command_stats                        FCVAR_DEVELOPMENTONLY
// dump_portrait_world_info_with_debug_name_containing       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// dump_response_symbols                                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // print all response symbols to the console
// dump_secondary_scene_worlds                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Lists secondary scene worlds and ref counts
// dumpparticlelist                                          FCVAR_CHEAT  // Print out information on existing particle systems
// dumpstringtable                                           FCVAR_DEVELOPMENTONLY  // Usage:  dumpstringtable <tablename |all> <sv | cl> <verbose | simple> <element>      Print string tables to console, verbose to dump data, simple to show name and count only, can specifiy a single numeric element index to restrict spew.
// echo                                                      FCVAR_SERVER_CAN_EXECUTE  // Echo text to console.
// echoln                                                    FCVAR_RELEASE  // Echo the command arguments on the console
// econ_build_pinboard_images_from_collection_name           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Renders and saves images for all models in a collection.
// econ_clear_inventory_images                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // clear the local inventory images (they will regenerate)
// econ_show_items_with_tag                                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Lists the item definitions that have a specified tag.
// enable_priority_boost                                     FCVAR_DEVELOPMENTONLY  // Disable focus based priority boost
// endmatch_votenextmap                                      FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Votes for the next map at the end of the match
// endmovie                                                  FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Stop recording movie frames.
// endround                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // End the current round.
// engine_frametime_print_report                             FCVAR_DEVELOPMENTONLY  // Print a performance report from the current data in the vprof 'lite' profiler
// ent_absbox                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the total bounding box for the given entity(s) in green.  Some entites will also display entity specific overlays.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_actornames                                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the entity name for all entities that have ShouldDisplayInActorNames true in code
// ent_animgraph2_open_graph                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Opens the graph and starts live debugging the AG2 graph for a given entity
	Arguments: entityName
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_animgraph2_record                                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Starts live debugging & recording the AG2 graph for a given entity
	Arguments: entityName
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_animgraph_debug                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays debug draws about the given entity(ies) animgraph
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_attachments                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the attachment points on an entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_autoaim                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the entity's autoaim radius.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_bbox                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the movement bounding box for the given entity(ies) in orange.  Some entites will also display entity specific overlays.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_bonemergeplayer                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Bonemerge the player onto the entity under the crosshairs
// ent_call                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // ent_call <funcname> <option:entname> calls function on current look target or filtername, checks on ent, then root, then mode, then map scope
// ent_cancelpendingentfires                                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Cancels all ent_fire created outputs that are currently waiting for their delay to expire.
// ent_characterize                                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Spew PVS debug info for entity
// ent_clear_debug_overlays                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Clears all debug overlays
// ent_create                                                FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Creates an entity of the given designer or subclass name where the player is looking.
// ent_debug_anim                                            FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Use the specified entity for animation debugging.
// ent_debug_origin_changes                                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // turn on, off, or toggle origin changes on server for entity by index
// ent_find                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Find and list all entities with classnames or targetnames that contain the specified substrings.
Format: find_ent <substring>

// ent_find_index                                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Display data for entity matching specified index.
Format: find_ent_index <index>

// ent_fire                                                  FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Usage:
   ent_fire <target> [action] [value] [delay]

// ent_fire_output                                           FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Usage:
   ent_fire_output <target> [output name] [value] [delay]

// ent_gib                                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Gibs the given entity(s)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_grab                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // grabs the object in front of the player. Options: -loose -multiple -toggle
// ent_hierarchy                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Prints the entity hierarchy tree rooted at the specified ent(s)
// ent_hitbox                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the hitboxes for the given entity(ies).
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_info                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Usage:
   ent_info <class name>

// ent_joints                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the joint names + axes an entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_kill                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Kills the given entity(s)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_list_report                                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Reports all list of all entities in a map, one by one
// ent_messages                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggles input/output message display for the selected entity(ies).  The name of the entity will be displayed as well as any messages that it sends or receives.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player i
// ent_name                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the entity name
// ent_orient                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Orient the specified entity to match the player's angles. By default, only orients target entity's YAW. Use the 'allangles' option to orient on all axis.
	Format: ent_orient <entity name> <optional: allangles>
// ent_picker                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggles 'picker' mode.  When picker is on, the bounding box, pivot and debugging text is displayed for whatever entity the player is looking at.
	Arguments:	full - enables all debug information
// ent_pivot                                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the pivot for the given entity(ies).
	(y=up=green, z=forward=blue, x=left=red). 
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_rbox                                                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // Displays the total bounding box for the given entity(s) in green.  Some entites will also display entity specific overlays.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_remove                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Removes the given entity(s)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_remove_all                                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Removes all entities of the specified type
	Arguments:   	{entity_name} / {class_name} 
// ent_reveal_in_hammer                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Given a mapname and hammer uniqueid, reveal it in Hammer
// ent_rotate                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Rotates an entity by a specified # of degrees
// ent_scale                                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Scales entities.	Arguments: <scale factor> <{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}>
// ent_scenehierarchy                                        FCVAR_GAMEDLL|FCVAR_CHEAT  // Prints the entity scenenode hierarchy tree rooted at the specified ent(s)
// ent_script_dump                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // Dumps the names and values of this entity's script scope to the console
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_select                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Select or deselects the given entities(s) for later manipulation
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_setang                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Set entity angles
// ent_setname                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Sets the targetname of the given entity(s)
	Arguments:   	<new entity name> <{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}>
// ent_setpos                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Move entity to position
// ent_show_damage                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // Sets damage display mode.  When on, you will see the amount of damage dealt over the target's head.
// ent_show_response_criteria                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Print, to the console, an entity's current criteria set used to select responses.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_skeleton                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the skeleton for the given entity(ies).
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_spew_derived_classes                                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Prints out all entity classes which inherit from a specified base class
// ent_teleport                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Teleport the specified entity to where the player is looking.
	Format: ent_teleport <entity name>
// ent_text                                                  FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Displays text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_text256                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays text debugging information about the given entity(ies) [within 256 units of the player] on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_text_clear                                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Hide text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_text_filter                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // Set which ent_text filters you want: 
// ent_text_radius                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays text debugging information about the given entity(ies) [near the player] on top of the entity (See Overlay Text)
	2 Arguments:   	<Radius> <{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}>
// ent_text_sticky_add                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Adds to list of names to display text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_text_sticky_clear                                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Clears the list of names to display text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_text_sticky_dump                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Spews the list of names to display text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_text_sticky_remove                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Removes from the list of names to display text debugging information about the given entity(ies) on top of the entity (See Overlay Text)
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_ungrab                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // un-grabs all objects
// ent_vcollide_wireframe                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the interpolated vcollide wireframe pm am entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_viewentity                                            FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Selects the picked entity as the view entity
// ent_viewoffset                                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the eye position for the given entity(ies) in red.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// ent_viewpunch                                             FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Used to debug ViewPunch
// ent_visibility_traces                                     FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Displays visibility traces for the given entity
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// entity_lump_list                                          FCVAR_DEVELOPMENTONLY  // List all known entity lumps
// entity_lump_spew                                          FCVAR_DEVELOPMENTONLY  // Dump the contents of an entity lump
// entityreport                                              FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Reports all extant entities. Optional 2nd arg is a substring of a classname that the list will be filtered by.
// entitysummary                                             FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Summarizes (by class) all extant entities. Optional 2nd arg is a substring of a classname that the list will be filtered by.
// ents                                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // List server entities, sorted by spawn group
// error_message_explain_pure                                FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Take user to Steam support article
// error_message_explain_unsigned                            FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Take user to Steam support article
// error_message_explain_vac                                 FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Take user to Steam support article
// escape                                                    FCVAR_RELEASE|FCVAR_CLIENTCMD_CAN_EXECUTE  // Escape key pressed.
// exec                                                      FCVAR_DONTRECORD|FCVAR_RELEASE  // Execute a cfg file
// exec_async                                                FCVAR_CHEAT|FCVAR_DONTRECORD  // Execute a cfg file over time
// execifexists                                              FCVAR_DONTRECORD|FCVAR_RELEASE  // Execute a cfg file if file exists
// explode                                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Kills the player with explosive damage
// explodevector                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Kills a player applying an explosive force. Usage: explodevector <player> <x value> <y value> <z value>
// fadein                                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // fadein {time r g b}: Fades the screen in from black or from the specified color over the given number of seconds.
// fadeout                                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // fadeout {time r g b}: Fades the screen to black or to the specified color over the given number of seconds.
// find                                                      FCVAR_RELEASE  // Find concommands with the specified string in their name/help text.
// findflags                                                 FCVAR_RELEASE  // Find concommands by flags.
// firetarget                                                FCVAR_GAMEDLL|FCVAR_CHEAT
// firstperson                                               FCVAR_CLIENTDLL|FCVAR_RELEASE|FCVAR_EXECUTE_PER_TICK  // Switch to firstperson camera.
// fog_override_color                                        FCVAR_CHEAT  // Sets the fog color override
// force_assert                                              FCVAR_DEVELOPMENTONLY  // Fire an assertion failure
// force_fatal_error                                         FCVAR_DEVELOPMENTONLY  // Fire a fatal error
// force_floating_point_exceptions                           FCVAR_DEVELOPMENTONLY  // Enable floating point exceptions to find bugs
// force_hibernate                                           FCVAR_DEVELOPMENTONLY  // Force toggle hibernation state
// fp_trace                                                  FCVAR_DEVELOPMENTONLY  // Toggle field path tracing to file

// fs_clear_open_duplicate_times                             FCVAR_DEVELOPMENTONLY  // Clear the list of files that have been opened.
// fs_dump_open_duplicate_times                              FCVAR_DEVELOPMENTONLY  // Set fs_report_long_reads 1 before loading to use this. Prints a list of files that were opened more than once and ~how long was spent reading from them.
// fs_spew_readfieldlist                                     FCVAR_CHEAT  // index <threshold bytes>: spew changes to ent index, optionally only spewing if update is > than threshold bytes
// func_mover_count                                          FCVAR_GAMEDLL|FCVAR_CHEAT
// func_mover_enable_debug_all                               FCVAR_GAMEDLL|FCVAR_CHEAT
// game_alias                                                FCVAR_RELEASE  // Set the configuration of game type and mode based on game alias like "deathmatch".
// game_particle_manager_dump_requeue                        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump contents of particle manager requeue
// game_particle_manager_list_active                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Dump counts of active particles
// gameevents_analyze                                        FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // compare game events across all mods
// gameevents_dumptofile                                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // write gameevents keyvalues (sorted by name) to gameevents_<modname>.txt
// gameinstructor_dump_open_lessons                          FCVAR_CLIENTDLL|FCVAR_CHEAT  // Gives a list of all currently open lessons.
// gameinstructor_dump_run_lesson_counts                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Gives a list of lessons that been completed or shown
// gameinstructor_reload_lessons                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Shuts down all open lessons and reloads them from the script file.
// gameinstructor_reset_counts                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Resets all display and success counts to zero.
// gameinstructor_teach_lesson                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Force a specific lesson to be triggered
// gameui_activate                                           FCVAR_RELEASE  // Shows the game UI
// gameui_allowescape                                        FCVAR_RELEASE  // Escape key allowed to hide game UI
// gameui_allowescapetoshow                                  FCVAR_RELEASE  // Escape key allowed to show game UI
// gameui_hide                                               FCVAR_RELEASE  // Hides the game UI
// gameui_preventescape                                      FCVAR_RELEASE  // Escape key doesn't hide game UI
// gameui_preventescapetoshow                                FCVAR_RELEASE  // Escape key doesn't show game UI
// gcmd                                                      FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Generate a command
// generate_minidump_comment                                 FCVAR_DEVELOPMENTONLY  // Generate a minidump comment and spew the results to the console
// generate_null_container                                   FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Generated a nulled out container.
// generate_trash_synth                                      FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Args: [Asset directory Path]
// getpos                                                    FCVAR_CLIENTDLL|FCVAR_CHEAT  // dump position and angles to the console
// getpos_exact                                              FCVAR_CLIENTDLL|FCVAR_CHEAT  // dump origin and angles to the console
// getposcopy                                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // dump position and angles to the console and clipboard
// getposcopy_exact                                          FCVAR_CLIENTDLL|FCVAR_CHEAT  // dump origin and angles to the console and clipboard
// give                                                      FCVAR_GAMEDLL|FCVAR_VCONSOLE_FUZZY_MATCHING  // Give item to player.
	Arguments: <item_name>
// give_oriented                                             FCVAR_GAMEDLL|FCVAR_VCONSOLE_FUZZY_MATCHING  // Give item oriented to player angles.
	Arguments: <item_name>
// givecurrentammo                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // Give a supply of ammo for current weapon..

// global_set                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // global_set <globalname> <state>: Sets the state of the given env_global (0 = OFF, 1 = ON, 2 = DEAD).
// god                                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggle by default, or 0 to disable and 1 to enable. Player becomes invulnerable.
// graphcontroller_dumpparams                                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Print all anim graph parameters for the specified entity.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// grep                                                      FCVAR_RELEASE  // grep line for pattern, print out matching lines only
// groundlist                                                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // Display ground entity list <index>
// groups                                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Show status of all spawn groups.
// healme                                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Heals the player.
	Arguments: <health to gain>
// help                                                      FCVAR_RELEASE  // Find help about a convar/concommand.
// hideconsole                                               FCVAR_DONTRECORD|FCVAR_RELEASE  // Hide the console.
// hideoverviewmap                                           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Hides the overview map
// hideradar                                                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Hides HUD radar
// host_timescale_dec                                        FCVAR_CHEAT  // Decrement the timescale by one step
// host_timescale_inc                                        FCVAR_CHEAT  // Increment the timescale by one step
// host_workshop_collection                                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Host a workshop map collection as a mapgroup
// host_workshop_map                                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Get the latest version of the map and host it on this server.
// host_writeconfig                                          FCVAR_RELEASE  // Saves out the user config values.
// host_writeconfig_with_prompt                              FCVAR_CLIENTDLL|FCVAR_RELEASE|FCVAR_SERVER_CAN_EXECUTE  // Write settings if user agrees
// hud_reloadscheme                                          FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Reloads hud layout and animation scripts.
// hurtme                                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Hurts the player.
	Arguments: <health to lose>
// hurtthem                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Hurts the enemy in front of you.
	Arguments: <health to lose>
// ic                                                        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // interp entity count

// ik_debug_fabrik_backwards_iteration_toggle                FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY
// ik_debug_fabrik_forwards_iteration_toggle                 FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY
// ime_hkl_info                                              FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Spew IME HKL info.
// ime_info                                                  FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Spew IME info.
// ime_installed_names                                       FCVAR_DONTRECORD|FCVAR_RELEASE  // Spew list of installed IMEs.
// ime_supported_info                                        FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Spew IME Supported info.
// imgui_cycle_undocked_window_focus                         FCVAR_DEVELOPMENTONLY  // Cycles focus between the game window and undocked imgui windows
// imgui_debug_entity                                        FCVAR_GAMEDLL|FCVAR_CHEAT  // Shows the entity browser, focused on the entity you specify.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// imgui_set_selection                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Sets ImGui selection
// imgui_set_status_text                                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Sets ImGui header status text
// import_csgo_config                                        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Imports an existing CS:GO configuration file into CS2
// impulse                                                   FCVAR_CLIENTDLL|FCVAR_RELEASE  // Triggers impulse command
// in_forcebuttonstate                                       FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_VCONSOLE_FUZZY_MATCHING  // Forces a button to be a particular state - WHEN PROCESSING USERCOMMANDS
// in_forceinput                                             FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_VCONSOLE_FUZZY_MATCHING  // Forces a button to be a particular state -- WHEN SAMPLING INPUT
// incrementvar                                              FCVAR_DONTRECORD|FCVAR_RELEASE  // Increment specified convar value.
// input_state                                               FCVAR_DEVELOPMENTONLY  // input_state
// instant_replay_goto_tick                                  FCVAR_DEVELOPMENTONLY  // Goto a direct timestamp of the replay
// instant_replay_goto_tick_relative                         FCVAR_DEVELOPMENTONLY  // Goto a direct timestamp of the replay
// instant_replay_live                                       FCVAR_DEVELOPMENTONLY  // If in replay, jumps back to live
// instant_replay_pause                                      FCVAR_DEVELOPMENTONLY  // Pauses instant replay.
// instant_replay_resume                                     FCVAR_DEVELOPMENTONLY  // Resumes instant replay.
// instant_replay_skip                                       FCVAR_DEVELOPMENTONLY  // Number of seconds to skip back to instant replay from current position
// instant_replay_skip_live                                  FCVAR_DEVELOPMENTONLY  // Number of seconds to skip back to instant replay from live
// instant_replay_timescale                                  FCVAR_DEVELOPMENTONLY  // Sets instant replay speed.
// instant_replay_togglepause                                FCVAR_DEVELOPMENTONLY  // Toggles instant replay.
// invnext                                                   FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// invnextselect                                             FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// invprev                                                   FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// invprevselect                                             FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// iv_debug                                                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Spew interpolated var info for entity.
// iv_interp                                                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Spew interpolated var info for entity.
// iv_off                                                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Turn off all interpolation variable spew.
// iv_on                                                     FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Spew both interpolated var debug info and history for entity.
// jpeg_screenshot                                           FCVAR_DEVELOPMENTONLY  // Take a jpeg screenshot: jpeg_screenshot [filename] [quality 1-100].
// key_findbinding                                           FCVAR_RELEASE  // Find key bound to specified command string.
// key_listboundkeys                                         FCVAR_RELEASE  // List bound keys with bindings.
// key_updatelayout                                          FCVAR_DEVELOPMENTONLY  // Updates game keyboard layout to current windows keyboard setting.
// kick                                                      FCVAR_DONTRECORD|FCVAR_RELEASE  // Kick a player by name.
// kickid                                                    FCVAR_DONTRECORD|FCVAR_RELEASE  // Kick a player by userid or uniqueid, with a message.
// kickid_hltv                                               FCVAR_DONTRECORD|FCVAR_RELEASE  // Kick a player by userid or uniqueid, with a message.
// kill                                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Kills the player with generic damage
// killvector                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Kills a player applying force. Usage: killvector <player> <x value> <y value> <z value>
// lastinv                                                   FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// launch_training_map                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// launch_warmup_map                                         FCVAR_CLIENTDLL|FCVAR_DONTRECORD|FCVAR_CLIENTCMD_CAN_EXECUTE  // Launches warmup map
// lightbinner_precompute                                    FCVAR_DEVELOPMENTONLY
// lightbinner_test_computespheresilhouette                  FCVAR_DEVELOPMENTONLY
// lightbinner_test_computesumsilhouette                     FCVAR_DEVELOPMENTONLY
// listRecentNPCSpeech                                       FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_DONTRECORD  // Displays a list of the last 5 lines of speech from NPCs.
// listdemo                                                  FCVAR_RELEASE  // List demo file contents.
// listid                                                    FCVAR_DEVELOPMENTONLY  // Lists banned users.
// listip                                                    FCVAR_DEVELOPMENTONLY  // List IP addresses on the ban list.
// listissues                                                FCVAR_GAMEDLL  // List all the issues that can be voted on.
// load                                                      FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD|FCVAR_VCONSOLE_FUZZY_MATCHING  // Usage:
   load [save file name]

// localization_quest_item_string_printout                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // localization_quest_item_string_printout
// log                                                       FCVAR_RELEASE  // Enables logging to file, console, and udp < on | off >.
// log_color                                                 FCVAR_DONTRECORD|FCVAR_RELEASE  // Set the color of a logging channel.
// log_dumpchannels                                          FCVAR_DONTRECORD|FCVAR_RELEASE  // Dumps information about all logging channels.
// log_flags                                                 FCVAR_DONTRECORD|FCVAR_RELEASE  // Set the flags on a logging channel.
// log_level                                                 FCVAR_DONTRECORD|FCVAR_RELEASE  // Set the spew level of a logging channel.
// log_verbosity                                             FCVAR_DONTRECORD|FCVAR_RELEASE  // Set the verbosity of a logging channel.
// logaddress_add_http                                       FCVAR_GAMEDLL|FCVAR_UNLOGGED|FCVAR_RELEASE  // Set URI of a listener to receive logs via http post. Wrap URI in double quotes.
// logaddress_add_http_delayed                               FCVAR_GAMEDLL|FCVAR_UNLOGGED|FCVAR_RELEASE  // Set a delay and URI of a listener to receive logs via http post. Wrap URI in double quotes.
// logaddress_del_http                                       FCVAR_GAMEDLL|FCVAR_UNLOGGED|FCVAR_RELEASE  // Remove http listener by URI. Wrap URI in double quotes.
// logaddress_delall_http                                    FCVAR_GAMEDLL|FCVAR_UNLOGGED|FCVAR_RELEASE  // Remove all http listeners from the dispatch list.
// logaddress_list_http                                      FCVAR_GAMEDLL|FCVAR_UNLOGGED|FCVAR_RELEASE  // List all URIs currently receiving server logs
// loop_dump                                                 FCVAR_DEVELOPMENTONLY  // Print the listeners of the current loop mode
// lrucache_flush                                            FCVAR_DEVELOPMENTONLY  // Flushes the specified cache
// lrucache_reset_stats                                      FCVAR_DEVELOPMENTONLY  // Resets stats for the specified CUtlLRUCaches (or all if none specified)
// lrucache_set_size                                         FCVAR_DEVELOPMENTONLY  // Sets the specified cache to the specified size
// lrucache_stats                                            FCVAR_DEVELOPMENTONLY  // Spews information about all CUtlLRUCaches
// lua_report_memory                                         FCVAR_DEVELOPMENTONLY
// map                                                       FCVAR_RELEASE|FCVAR_VCONSOLE_FUZZY_MATCHING|FCVAR_VCONSOLE_SET_FOCUS  // map <mapname> :Load a new map.
// map_enable_portrait_worlds                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // Enables/disables portrait worlds
// map_setbombradius                                         FCVAR_GAMEDLL|FCVAR_CHEAT  // Sets the bomb radius for the map.
// map_showbombradius                                        FCVAR_GAMEDLL|FCVAR_CHEAT  // Shows bomb radius from the center of each bomb site and planted bomb.
// map_showspawnpoints                                       FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Shows player spawn points (red=invalid). Optionally pass in the duration.
// map_workshop                                              FCVAR_CLIENTDLL|FCVAR_RELEASE|FCVAR_VCONSOLE_FUZZY_MATCHING|FCVAR_VCONSOLE_SET_FOCUS  // Launch a workshop map
// mapgroup                                                  FCVAR_GAMEDLL|FCVAR_DONTRECORD|FCVAR_RELEASE  // Specify a map group
// maps                                                      FCVAR_RELEASE  // Displays list of maps.
// markup_group_ent_bbox                                     FCVAR_GAMEDLL|FCVAR_CHEAT  // markup_group_ent_bbox <markup_group name> -> toggle ent_bbox for all members of the named markup group
// markup_group_ent_text                                     FCVAR_GAMEDLL|FCVAR_CHEAT  // markup_group_ent_text <markup_group name> -> toggle ent_text for all members of the named markup group
// markup_group_spew                                         FCVAR_GAMEDLL|FCVAR_CHEAT  // Spew all current markup groups and their members
// mat_assert_on_shader_use                                  FCVAR_DEVELOPMENTONLY  // Assert on shader used based on substring of shader name
// mat_clearshadercache                                      FCVAR_DEVELOPMENTONLY  // Clears the shader cache used for dynamic shader compile.
// mat_debug                                                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Sets a mat_fullbright debug visualization mode
// mat_disable_dynamic_shader_compile                        FCVAR_DEVELOPMENTONLY  // Reloads all shaders from vcs files until the next time mat_reloadshaders is called
// mat_forcereloadshaders                                    FCVAR_DEVELOPMENTONLY  // Force reloads all shaders (skips MD5 check). Takes optional substrings of shader names to recompile as arguments.
// mat_print_dead_materials                                  FCVAR_DEVELOPMENTONLY  // Print loaded materials that have no valid layers due to not supporting any of the modes in gameinfo.gi.
// mat_print_error_materials                                 FCVAR_DEVELOPMENTONLY  // Print loaded materials that are using the error shader or material.
// mat_print_expensive_materials                             FCVAR_DEVELOPMENTONLY  // Print materials sorted by cost heuristic
// mat_print_material_info                                   FCVAR_DEVELOPMENTONLY  // Print info about a specific material
// mat_print_materials                                       FCVAR_DEVELOPMENTONLY  // Print loaded materials. Takes an optional substring as an argument.
// mat_print_materials_last_frame                            FCVAR_DEVELOPMENTONLY  // Print materials used last frame
// mat_print_materials_unused                                FCVAR_DEVELOPMENTONLY  // Print materials that have never been used
// mat_print_modes                                           FCVAR_DEVELOPMENTONLY  // Print supported rendering modes.
// mat_print_shader_info                                     FCVAR_DEVELOPMENTONLY  // Print detailed info about a single shader. Takes a shader name (hero.vfx) as an argument.
// mat_print_shader_quality                                  FCVAR_DEVELOPMENTONLY  // Print current shader quality setting
// mat_print_shaders                                         FCVAR_DEVELOPMENTONLY  // Print loaded shaders. Takes a substring as an argument.
// mat_print_textures                                        FCVAR_DEVELOPMENTONLY  // Print loaded textures in alphabetical order. Takes an optional substring as an argument.
// mat_print_textures_size                                   FCVAR_DEVELOPMENTONLY  // Print loaded textures in ascending size order. Takes an optional substring as an argument.
// mat_print_textures_size_in_memory                         FCVAR_DEVELOPMENTONLY  // Print loaded textures in ascending size order as they are in memory. Takes an optional substring as an argument.
// mat_reinitmaterials                                       FCVAR_DEVELOPMENTONLY  // Reinitializes all loaded materials, reloading their shaders.
// mat_reloadshaders                                         FCVAR_DEVELOPMENTONLY  // Reloads all shaders. Takes optional substrings of shader names to recompile as arguments.
// mat_reset_material_costs                                  FCVAR_DEVELOPMENTONLY  // Reset material cost heuristic
// mat_set_shader_quality                                    FCVAR_DEVELOPMENTONLY  // Force shader quality setting (valid values are 0 or 1)
// matchdraft_debug_sendlog                                  FCVAR_CLIENTDLL|FCVAR_DONTRECORD|FCVAR_RELEASE|FCVAR_CLIENTCMD_CAN_EXECUTE  // Print debug draft into HTTP log
// mem_compact                                               FCVAR_DEVELOPMENTONLY  // Compacts the heap
// mem_dump                                                  FCVAR_DEVELOPMENTONLY  // Dump memory stats to text file or <stdout>.
// mem_test                                                  FCVAR_DEVELOPMENTONLY
// memory                                                    FCVAR_DEVELOPMENTONLY  // Print memory stats.
// memory_check_limit                                        FCVAR_DEVELOPMENTONLY  // Assert if peak memory use is over the limit.
// menuselect                                                FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // menuselect
// minimap_create                                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Does a bunch of work to create a minimap
// mm_debugprint                                             FCVAR_DEVELOPMENTONLY  // Show debug information about current matchmaking session
// mm_ignored_sessions_reset                                 FCVAR_DEVELOPMENTONLY  // Reset ignored sessions
// mm_queue_draft_show                                       FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Display current draft
// mm_queue_show_stats                                       FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Display global server stats
// model_dump_convert_info                                   FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CLIENTDLL  // Print model load-time conversion info
// movie_fixwave                                             FCVAR_DEVELOPMENTONLY  // Fixup corrupted .wav file if engine crashed during startmovie/endmovie, etc.
// mp_backup_restore_list_files                              FCVAR_GAMEDLL|FCVAR_RELEASE  // Lists recent backup round files matching the prefix, most recent files first, accepts a numeric parameter to limit the number of files displayed
// mp_backup_restore_load_file                               FCVAR_GAMEDLL|FCVAR_RELEASE  // Loads player cash, KDA, scores and team scores; resets to the next round after the backup
// mp_bot_ai_bt_clear_cache                                  FCVAR_GAMEDLL|FCVAR_RELEASE  // Clears the cache for behavior tree files.
// mp_debug_timeouts                                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Prints time outs to the console for debugging
// mp_disable_autokick                                       FCVAR_GAMEDLL|FCVAR_RELEASE  // Prevents a userid from being auto-kicked
// mp_dump_timers                                            FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Prints round timers to the console for debugging
// mp_modify_timeouts                                        FCVAR_GAMEDLL|FCVAR_RELEASE  // mp_modify_timeouts <CT|T> <N>, e.g., mp_modify ct -1
// mp_pause_match                                            FCVAR_GAMEDLL|FCVAR_RELEASE  // Pause the match in the next freeze time
// mp_scrambleteams                                          FCVAR_GAMEDLL|FCVAR_RELEASE  // Scramble the teams and restart the game
// mp_swapteams                                              FCVAR_GAMEDLL|FCVAR_RELEASE  // Swap the teams and restart the game
// mp_unpause_match                                          FCVAR_GAMEDLL|FCVAR_RELEASE  // Resume the match
// mp_warmup_end                                             FCVAR_GAMEDLL|FCVAR_RELEASE  // End warmup immediately.
// mp_warmup_start                                           FCVAR_GAMEDLL|FCVAR_RELEASE  // Start warmup.
// multvar                                                   FCVAR_DONTRECORD|FCVAR_RELEASE  // Multiply specified convar value.
// nav_add_to_selected_set                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Add current area to the selected set.
// nav_add_to_selected_set_by_id                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Add specified area id to the selected set.
// nav_begin_deselecting                                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Start continuously removing from the selected set.
// nav_begin_drag_deselecting                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Start dragging a selection area.
// nav_begin_drag_selecting                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Start dragging a selection area.
// nav_begin_selecting                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Start continuously adding to the selected set.
// nav_check_connectivity                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Checks to be sure every (or just the marked) nav area can get to every goal area for the map (hostages or bomb site).
// nav_clear_attribute                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Remove given nav attribute from all areas in the selected set.
// nav_clear_attributes                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Clear all nav attributes of selected area.
// nav_clear_selected_set                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Clear the selected set.
// nav_create_indirect_connection                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Create a connection between the selected area and the area pointed at by the crosshair.
// nav_create_indirect_connection_from_to                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Create a connection between the current 'from' and 'to' locations.
// nav_create_indirect_connection_set_from_using_editpos     FCVAR_GAMEDLL|FCVAR_CHEAT  // Set the 'from' location of an indirect connection to be the current edit pos of nav_edit.
// nav_create_indirect_connection_set_to_using_editpos       FCVAR_GAMEDLL|FCVAR_CHEAT  // Set the 'to' location of an indirect connection to be the current edit pos of nav_edit.
// nav_delete                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Deletes the currently highlighted Area.
// nav_end_deselecting                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Stop continuously removing from the selected set.
// nav_end_drag_deselecting                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Stop dragging a selection area.
// nav_end_drag_selecting                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Stop dragging a selection area.
// nav_end_selecting                                         FCVAR_GAMEDLL|FCVAR_CHEAT  // Stop continuously adding to the selected set.
// nav_list_movable_meshes                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // List the movable meshes registered with the movable meshes manager.
// nav_lower_drag_volume_max                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Lower the top of the drag select volume.
// nav_lower_drag_volume_min                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Lower the bottom of the drag select volume.
// nav_mark                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Marks the Area or Ladder under the cursor for manipulation by subsequent editing commands.
// nav_mark_attribute                                        FCVAR_GAMEDLL|FCVAR_CHEAT  // Set nav attribute for all areas in the selected set.
// nav_raise_drag_volume_max                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Raise the top of the drag select volume.
// nav_raise_drag_volume_min                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Raise the bottom of the drag select volume.
// nav_recall_selected_set                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Re-selects the stored selected set.
// nav_remove_from_selected_set                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Remove current area from the selected set.
// nav_select_radius                                         FCVAR_GAMEDLL|FCVAR_CHEAT  // Adds all areas in a radius to the selection set
// nav_select_with_attribute                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Selects areas with the given attribute.
// nav_set_movable_mesh_dormant_flag                         FCVAR_GAMEDLL|FCVAR_CHEAT  // Set the movable mesh dormant flag (0=active, 1=dormant)
// nav_split                                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // To split an Area into two, align the split line using your cursor and invoke the split command.
// nav_store_selected_set                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Stores the current selected set for later retrieval.
// nav_switch                                                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Switches to navmesh for the specified spawngroup
// nav_test_level_hull                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Find entities that intrude into the nav mesh.  List those entities in console output, and display bounding boxes around them for a while.
// nav_test_level_hull_move                                  FCVAR_GAMEDLL|FCVAR_CHEAT
// nav_toggle_deselecting                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Start or stop continuously removing from the selected set.
// nav_toggle_in_selected_set                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Remove current area from the selected set.
// nav_toggle_selected_set                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggles all areas into/out of the selected set.
// nav_toggle_selecting                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Start or stop continuously adding to the selected set.
// nav_unmark                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Clears the marked Area or Ladder.
// net_captureculldata                                       FCVAR_DEVELOPMENTONLY  // Captures low-level data to replay path culling algorithm behavior in controlled unit test environment
// net_channels                                              FCVAR_RELEASE  // Shows net channel info
// net_connections_stats                                     FCVAR_RELEASE  // Print detailed network statistics for each network connection
// net_fakeclear                                             FCVAR_RELEASE  // Clear all simulated network conditions
// net_fakejitter                                            FCVAR_RELEASE  // Shortcut to set jitter net options.  Run with no arguments for usage.
// net_fakelag                                               FCVAR_RELEASE  // Shortcut to set both FakePacketLag_Recv and FakePacketLag_Send net options
// net_fakeloss                                              FCVAR_RELEASE  // Shortcut to set both FakePacketLoss_Recv and FakePacketLoss_Send net options
// net_fakestatus                                            FCVAR_RELEASE  // Print current simulated network condifions
// net_listallmessages                                       FCVAR_CHEAT  // List all registered net messages
// net_messageinfo                                           FCVAR_CHEAT  // Display info about a message (by classname or id)
// net_option                                                FCVAR_RELEASE  // Get or set SteamNetworkingSockets options such as fake packet lag and loss
// net_permutation_info                                      FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // show permutation information for serializer
// net_print_sdr_ping_times                                  FCVAR_RELEASE  // Print current ping times to SDR points of presence, and selected route
// net_reloadgameevents                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Reload the game events
// net_serializedentitymemory                                FCVAR_DEVELOPMENTONLY  // Spew CSerializedEntity memory
// net_serializedentitymetadatainfo                          FCVAR_DEVELOPMENTONLY  // Spew CSerializedEntity metadata information
// net_spewcounts                                            FCVAR_DEVELOPMENTONLY  // Spew serializer counts, client only by default, specify server to spew server counts

// net_spewserializer                                        FCVAR_DEVELOPMENTONLY  // Spew serializer info

// net_stats_json                                            FCVAR_DEVELOPMENTONLY  // Output server networking statistics in json format
// net_status                                                FCVAR_RELEASE  // Shows current network status
// net_validatemessages                                      FCVAR_CHEAT  // Activates/deactivates net message validation
// net_why_field_excluded                                    FCVAR_DEVELOPMENTONLY  // <classname> <fieldname>:  spew why field was excluded from networking for classname.
// nextdemo                                                  FCVAR_RELEASE  // Play next demo in sequence.
// noclip                                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggle. Player becomes non-solid and flies.  Optional argument of 0 or 1 to force enable/disable
// notarget                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggle. Player becomes hidden to NPCs.
// open_asset                                                FCVAR_DEVELOPMENTONLY  // Opens an asset in it's primary editor of choice. Specify the full path to the asset from the mod directory.
// open_current_map_in_hammer                                FCVAR_DEVELOPMENTONLY  // opens the current map in hammer.
// open_current_map_pulse_in_tool                            FCVAR_DEVELOPMENTONLY|FCVAR_VCONSOLE_FUZZY_MATCHING  // Opens the specified pulse used in this map in the editor.
	Arguments: <pulse_file>
// p2p_listpeers                                             FCVAR_DEVELOPMENTONLY  // List currently known peers.
// p2p_ping                                                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Ping a peer.
// panorama_console_reset_size_and_position                  FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Resets the panorama console to its default size and position
// panorama_dispatch_event                                   FCVAR_DEVELOPMENTONLY  // Dispatch the event defined by the argument string. No creating panel is specified.
// panorama_dump_symbols                                     FCVAR_DEVELOPMENTONLY  // <ESymbolType> Dump all of the symbols in the Panorama symbol table
// panorama_generate_layout_xsd                              FCVAR_DEVELOPMENTONLY  // Generate the Layout XML Schema Definition for the current run-time (types are dependent on which game DLL is running).
// panorama_print_cache_status                               FCVAR_DEVELOPMENTONLY  // Print internal panorama refcounts for every file
// panorama_print_svg_stats                                  FCVAR_DEVELOPMENTONLY
// particle_profile                                          FCVAR_DEVELOPMENTONLY  // Profile particle
// particle_profile_spike                                    FCVAR_DEVELOPMENTONLY  // Profile particle spike
// particle_reset_assertions                                 FCVAR_DEVELOPMENTONLY  // Causes all single-fire particle assertions to trigger once more.
// particle_stop_all                                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // Stops all particle systems currently playing
// particle_stop_specified                                   FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // Stops all particle systems that match specified name
// particle_stop_unspecified                                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // Stops all particle systems that don't match specified name
// particle_test_create                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Creates the named particle system where the player is looking.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// particle_test_destroy                                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Destroys all particle systems matching the specified name.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// particle_test_start                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Dispatches the test particle system with the parameters specified in particle_test_file,
 particle_test_attach_mode and particle_test_attach_param on the entity the player is looking at.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no ar
// particle_test_stop                                        FCVAR_GAMEDLL|FCVAR_CHEAT  // Stops all particle systems on the selected entities.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// path                                                      FCVAR_DEVELOPMENTONLY  // Show the filesystem path.
// pause                                                     FCVAR_RELEASE  // Toggle the server pause state.
// perfectworld_replenish_funds                              FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Opens Perfect World funds replenishment page for account.
// phys_create_test_character_proxy                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Create test character proxy
// phys_debug_draw                                           FCVAR_DEVELOPMENTONLY  // Set up debug-draw of physics internal state
// phys_dump_intersection_controller                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Dump intersection controller status
// phys_dump_main_world                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Dump physics main world to file
// phys_dump_memory                                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Dump memory usage
// phys_list                                                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // List all physics component contents of every entity in the game;
    -stream [1|0]         initiate|terminate streaming to physics debugger
    -allents              include non-physical entities
    -classes              print class names
    -sdk        
// phys_shoot                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Shoots a phys object.
// phys_sleep                                                FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Put all physics in all the worlds to sleep
// phys_wakeup                                               FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Wake all physics objects in the Main physics up
// physics_add_test                                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // add test object
// physics_debug_entity                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Dumps debug info for an entity
// physics_highlight_active                                  FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Turns on the absbox for all active physics objects.
  0 : un-highlight.

// physics_remove_test                                       FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // remove test object
// physics_report_active                                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Lists all active physics objects
  -more : extra info

// pixelvis_debug                                            FCVAR_CHEAT  // Dump debug info
// plant_bomb                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Plant a bomb where the player is looking.
// play                                                      FCVAR_SERVER_CAN_EXECUTE  // Play a sound.
// playcast                                                  FCVAR_RELEASE  // Play a broadcast
// playdemo                                                  FCVAR_RELEASE  // Play a recorded demo file (.dem ).
// player_ping                                               FCVAR_GAMEDLL  // Creates a ping notification where the player is looking.
// playsound                                                 FCVAR_DEVELOPMENTONLY  // playsound <soundname>
// playsoundscape                                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Forces a soundscape to play
// playvol                                                   FCVAR_DEVELOPMENTONLY  // Play a sound at a specified volume.
// png_screenshot                                            FCVAR_DEVELOPMENTONLY  // Take a .png screenshot: png_screenshot [filename]
// pop_var_values                                            FCVAR_DEVELOPMENTONLY  // Restore previously pushed convars and config values
// print_changed_convars                                     FCVAR_RELEASE  // Prints all convars that have changed from their default value
// print_mapgroup                                            FCVAR_CLIENTDLL|FCVAR_RELEASE  // Prints the current mapgroup and the contained maps
// print_mapgroup_sv                                         FCVAR_GAMEDLL|FCVAR_RELEASE  // Prints the current mapgroup and the contained maps
// print_model_bind_pose                                     FCVAR_DEVELOPMENTONLY  // Prints the bind pose of the specified model. Optionally limits to a particular bone and its parent chain, otherwise prints the entire skeleton.
// progress_enable                                           FCVAR_DEVELOPMENTONLY
// prop_debug                                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggle prop debug mode. If on, props will show colorcoded bounding boxes. Red means ignore all damage. White means respond physically to damage but never break. Green maps health in the range of 100 down to 1.
// prop_dynamic_create                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Creates a dynamic prop with a specific .vmdl aimed away from where the player is looking.
	Arguments: {.vmdl name}
// prop_physics_create                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Creates a physics prop with a specific .vmdl aimed away from where the player is looking.
	Arguments: {.vmdl name}
// pulse_debug_entity                                        FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Opens a graph referencing the selected entity. If it is referenced by more than 1 graph, list all the active pulse graph instances referring to that entity so you can pick which one you want.
// pulse_debug_print                                         FCVAR_DEVELOPMENTONLY  // Usage: pulse_debug_print <vpulse_resource>
// pulse_list_game_blackboards                               FCVAR_GAMEDLL|FCVAR_CHEAT  // List all the active pulse graph instances
// pulse_list_graphs                                         FCVAR_CHEAT  // List all the active pulse graph instances
// pulse_open_graph_id                                       FCVAR_CHEAT  // Open a specific graph instance by id
// pulse_print_graph_execution_history                       FCVAR_CHEAT  // Prints the execution history of a graph by filename or instanceid
// push_var_values                                           FCVAR_DEVELOPMENTONLY  // Save convars and config values
// quit                                                      FCVAR_RELEASE|FCVAR_VCONSOLE_SET_FOCUS  // Quit the game
// r_camerapos                                               FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Prints out the current camera position + orientation to the console
// r_dx11_report_live_objects                                FCVAR_DEVELOPMENTONLY  // Prints out live D3D11 objects (requires -dx11debug)
// r_entpos                                                  FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Moves the camera position + orientation to the named entity
// r_force_engine_render_frame                               FCVAR_DEVELOPMENTONLY  // Force a single render of the engine viewport.
// r_gpu_mem_stats                                           FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Display GPU memory usage.
// r_print_buffers                                           FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Print Vertex/Index/GPU buffers.
// r_print_texture_stats                                     FCVAR_DEVELOPMENTONLY  // Texture stats
// r_render_coordination_state                               FCVAR_DEVELOPMENTONLY  // Prints out the current render coordination state.
// r_renderdoc_capture_frame                                 FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Triggers a RenderDoc capture
// r_renderdoc_capture_window_dx11                           FCVAR_DEVELOPMENTONLY  // Triggers a RenderDoc capture of a specific Window
// r_setpos                                                  FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Moves the camera position + orientation to the specified position
// r_textures_evict_all                                      FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Evict all resident texture.
// r_toggleviewportsize                                      FCVAR_DEVELOPMENTONLY  // Toggles viewport size between small + full window.
// r_viewport                                                FCVAR_DEVELOPMENTONLY  // Slams viewport size to a specified value.
// radio                                                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Opens a radio menu
// radio1                                                    FCVAR_CLIENTDLL|FCVAR_RELEASE  // Opens a radio menu
// radio2                                                    FCVAR_CLIENTDLL|FCVAR_RELEASE  // Opens a radio menu
// radio3                                                    FCVAR_CLIENTDLL|FCVAR_RELEASE  // Opens a radio menu
// ragdoll_cleanup_all                                       FCVAR_LINKED_CONCOMMAND|FCVAR_GAMEDLL|FCVAR_CLIENTDLL|FCVAR_CHEAT  // Cleans up all ragdolls.
// rangefinder                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Measures distance along a ray
// rangefinder2d                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Measures distance along a ray, only measuring along XY plane.
// ray_bench                                                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Load the rays and run the benchmark
// rcon                                                      FCVAR_DONTRECORD|FCVAR_RELEASE  // Issue an rcon command.
// rebuy                                                     FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Attempt to repurchase items with the order listed in cl_rebuy
// record                                                    FCVAR_CHEAT|FCVAR_DONTRECORD|FCVAR_RELEASE  // Record a demo.
// redirectend                                               FCVAR_RELEASE  // Redirect server console output
// redirectstart                                             FCVAR_RELEASE  // Redirect server console output
// regenerate_weapon_skins                                   FCVAR_CLIENTDLL|FCVAR_CHEAT
// reload_model                                              FCVAR_DEVELOPMENTONLY  // Force a reload of a vmdl resource
// reload_store_config                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// reloadgame                                                FCVAR_CHEAT|FCVAR_VCONSOLE_SET_FOCUS  // Reload the most recent saved game.
// remove_weapon                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Remove a weapon held by the player.
	Arguments: <weapon subclass name>
// removeid                                                  FCVAR_DEVELOPMENTONLY  // Remove a user ID from the ban list.
// removeip                                                  FCVAR_DEVELOPMENTONLY  // Remove an IP address from the ban list.
// repeat_last_console_command                               FCVAR_RELEASE  // Repeat last console command.
// replant_bomb                                              FCVAR_GAMEDLL|FCVAR_CHEAT
// replay_death                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // start hltv replay of last death
// replay_start                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Start Source2 TV replay: replay_start <delay>|stash [<player name or index>]
// replay_stop                                               FCVAR_GAMEDLL  // stop hltv replay
// report_entities                                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // Lists all entities
// report_simthinklist                                       FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Lists all simulating/thinking entities
// report_soundpatch                                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // reports sound patch count
// reset_gameconvars                                         FCVAR_CHEAT  // Reset game convars to default values
// resource_leaks                                            FCVAR_DEVELOPMENTONLY  // resource_leaks <resource_name>: Show resource leaks for the named resource
// resource_list                                             FCVAR_DEVELOPMENTONLY  // List loaded resources matching a substring
// resource_log_allocate_timing                              FCVAR_DEVELOPMENTONLY  // Log time spent in Allocate for all resource types
// resource_manifest_validate_modules                        FCVAR_DEVELOPMENTONLY  // Scan all of the loaded modules and validate any resource manifests found
// resource_repeated_reload                                  FCVAR_DEVELOPMENTONLY  // resource_repeated_reload <count> <resource_name> (<resource name> ...): Load and unload the specified resource(s)
// resource_reset_allocate_timing                            FCVAR_DEVELOPMENTONLY  // Reset tracked time spent in Allocate (see resource_log_allocate_timing)
// respawn_player                                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Respawns the player from death!

// restart                                                   FCVAR_CHEAT|FCVAR_VCONSOLE_SET_FOCUS  // Poor man's restart: reload the current map from disk.
// restart_in_insecure                                       FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Restart in insecure mode
// restart_in_trusted                                        FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Restart in trusted mode
// restart_in_untrusted                                      FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Restart in untrusted mode
// restart_normal                                            FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Restart
// retake_barrier_clear                                      FCVAR_GAMEDLL|FCVAR_CHEAT
// retake_barrier_point                                      FCVAR_GAMEDLL|FCVAR_CHEAT
// retake_barrier_spawn                                      FCVAR_GAMEDLL|FCVAR_CHEAT
// rr_dump_rules                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Print all response rules
// rr_forceconcept                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // fire a response concept directly at a given character.
USAGE: rr_forceconcept <target name or index> <concept> "criteria1:value1,criteria2:value2,..."
criteria values are optional.

// rs_dump_stats                                             FCVAR_DEVELOPMENTONLY  // rs_dump_stats - Dump resourcesystem stats.
// run_perftest                                              FCVAR_CHEAT|FCVAR_DONTRECORD  // Execute perftest.cfg
// save                                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_DONTRECORD  // Save Game
// save_clear_subdirectory                                   FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// save_finish_async                                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL
// save_set_subdirectory                                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_REPLICATED
// save_showelapsedtime                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // display up-to-date elapsed play time
// save_watchclass                                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Restrict spew to entities with matching classname
// save_watchentity                                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Restrict spew to entity index
// say                                                       FCVAR_GAMEDLL  // Display player message
// say_team                                                  FCVAR_GAMEDLL  // Display player message to team
// sc_dumpworld                                              FCVAR_CHEAT  // Dump a list of the objects in a sceneworld (Usage: sc_dumpworld <world_index>)
// sc_dumpworld3d                                            FCVAR_CHEAT  // Dump the objects in a sceneworld into a 3d geoview buffer (Usage: sc_dumpworld3d <world_index>)
// sc_list_extradata_allocations                             FCVAR_DEVELOPMENTONLY  // Prints out the overall extra data allocation counts
// sc_listworlds                                             FCVAR_CHEAT  // List all the active sceneworlds
// sc_setclassflags                                          FCVAR_CHEAT  // Low level command to set the flags byte associated with an object class. sc_SetClassFlags <classname> <value>

// sc_showclasses                                            FCVAR_CHEAT  // List the object class names known by scenesystem

// scene_playvcd                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Play the given VCD as an instanced scripted scene.
// schema_all_list_bindings                                  FCVAR_DEVELOPMENTONLY  // schema_all_list_bindings <substring> - List all scopes registered schema bindings (classes & enums). If no substring, list them all.
// schema_detailed_class_layout                              FCVAR_DEVELOPMENTONLY  // schema_detailed_class_layout <class_name> - Print a detailed memory layout of the class (including inline structs).
// schema_dump_binding                                       FCVAR_DEVELOPMENTONLY  // schema_dump_binding <class_or_enum_name> - Print information about the named class or enum.
// schema_list_bindings                                      FCVAR_DEVELOPMENTONLY  // schema_list_bindings <substring> - List registered global-scope schema bindings (classes & enums). If no substring, list them all.
// schema_meta_stats                                         FCVAR_DEVELOPMENTONLY  // schema_meta_stats [<options>]- Print a summary of schemasystem metadata statistics.
// schema_stats                                              FCVAR_DEVELOPMENTONLY  // schema_stats - Print a summary of various schemasystem statistics.
// screenshot                                                FCVAR_DEVELOPMENTONLY  // Take a screenshot: screenshot [filename]
// script_add_debug_filter                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Add a filter to the game debug overlay
// script_add_watch                                          FCVAR_GAMEDLL|FCVAR_CHEAT  // Add a watch to the game debug overlay
// script_add_watch_pattern                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Add a watch to the game debug overlay
// script_attach_debugger                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Connect the vscript VM to the script debugger
// script_clear_watches                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Clear all watches from the game debug overlay
// script_debug                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Toggle the in-game script debug features
// script_dump_all                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // Dump the state of the VM to the console
// script_find                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Find a key in the VM 
// script_help                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Output help for script functions
// script_help2                                              FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Output help for script functions suitable for auto-completion
// script_reload                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Reload scripts
// script_reload_code                                        FCVAR_GAMEDLL|FCVAR_CHEAT  // Execute a vscript file, replacing existing functions with the functions in the run script
// script_reload_entity_code                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Execute all of this entity's VScripts, replacing existing functions with the functions in the run scripts
// script_remove_debug_filter                                FCVAR_GAMEDLL|FCVAR_CHEAT  // Remove a filter from the game debug overlay
// script_remove_watch                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Remove a watch from the game debug overlay
// script_remove_watch_pattern                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Remove a watch from the game debug overlay
// script_resurrect_unreachable                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Use the garbage collector to track down reference cycles
// script_trace_disable                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Turn off a particular trace output by file or function name
// script_trace_disable_all                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Turn off all trace output
// script_trace_disable_key                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Turn off a particular trace output by table/instance
// script_trace_enable                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Turn on a particular trace output by file or function name
// script_trace_enable_all                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Turn on all trace output
// script_trace_enable_key                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Turn on a particular trace output by table/instance
// scrubber                                                  FCVAR_DEVELOPMENTONLY  // Scrub system off - not a dev build
// sdr                                                       FCVAR_RELEASE  // An old command that has been renamed to 'net_option'
// sellbackall                                               FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Attempt to refund all equipment
// send_round_backup_file_list                               FCVAR_GAMEDLL|FCVAR_RELEASE
// server_game_time                                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Gives the game time in seconds (server's curtime)
// server_snd_cast                                           FCVAR_GAMEDLL|FCVAR_CHEAT  // Casts a ray and starts a sound event where the ray hits. The sound event will retrigger periodically. Usage: server_snd_cast <eventname> [<retrigger time>] [<max distance>]. Arguments that are specified will become defaults for the remainder of the session
// server_snd_pos                                            FCVAR_GAMEDLL|FCVAR_CHEAT  // Starts a sound event at a given position. The sound event will retrigger periodically. Usage: server_snd_pos <eventname> <retrigger time> <x> <y> <z>.
// servervoice_clear                                         FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // servervoice_clear
// servervoice_dump                                          FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // servervoice_dump
// setang                                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Snap player eyes to specified pitch yaw <roll:optional> (must have sv_cheats).
// setang_exact                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Snap player eyes and orientation to specified pitch yaw <roll:optional> (must have sv_cheats).
// setinfo                                                   FCVAR_CLIENTCMD_CAN_EXECUTE  // Adds a new user info value
// setmodel                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Changes's player's model
// setpause                                                  FCVAR_RELEASE  // Set the pause state of the server.
// setpos                                                    FCVAR_GAMEDLL|FCVAR_CHEAT  // Move player to specified origin (must have sv_cheats).
// setpos_exact                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Move player to an exact specified origin (must have sv_cheats).
// setpos_player                                             FCVAR_GAMEDLL|FCVAR_CHEAT  // Move specified player to specified origin (must have sv_cheats).
// shake                                                     FCVAR_GAMEDLL|FCVAR_CHEAT  // Shake the screen.
// shake_stop                                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // Stops all active screen shakes.

// shake_testpunch                                           FCVAR_CLIENTDLL|FCVAR_CHEAT  // Test a punch-style screen shake.

// shatterglass_break                                        FCVAR_GAMEDLL|FCVAR_CHEAT
// shatterglass_restore                                      FCVAR_GAMEDLL|FCVAR_CHEAT
// show_loadout_toggle                                       FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Toggles loadout display
// show_untrusted_warning_again                              FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Show untrusted warning again
// showconsole                                               FCVAR_DONTRECORD|FCVAR_RELEASE  // Show the console.
// showents                                                  FCVAR_GAMEDLL|FCVAR_CHEAT  // Dump entity list to console.
// showtriggers                                              FCVAR_GAMEDLL|FCVAR_CHEAT  // Enable or Disable showing trigger entities
// showtriggers_toggle                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Displays the movement bounding box for the triggers in orange.  Some entites will also display entity specific overlays.
	Arguments:   	{entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// slot0                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot1                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot10                                                    FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot11                                                    FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot12                                                    FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot13                                                    FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot2                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot3                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot4                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot5                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot6                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot7                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot8                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// slot9                                                     FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE
// snapto                                                    FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// snd__beatpattern_stop_track                               FCVAR_CHEAT  // Stops the specified track
// snd_async_flush                                           FCVAR_DEVELOPMENTONLY  // Flush all unlocked async audio data
// snd_async_showmem                                         FCVAR_DEVELOPMENTONLY  // Show async memory stats
// snd_async_showmem_music                                   FCVAR_DEVELOPMENTONLY  // Show async memory stats for just non-streamed music
// snd_async_showmem_summary                                 FCVAR_DEVELOPMENTONLY  // Show brief async memory stats
// snd_beatpattern_flush                                     FCVAR_CHEAT  // Purge and reload all beat pattern data and files.
// snd_beatpattern_print_activetracks                        FCVAR_CHEAT  // List all active tracks
// snd_beatpattern_print_patterns                            FCVAR_CHEAT  // List all available beat patterns
// snd_beatpattern_set_track_bpm                             FCVAR_CHEAT  // Sets the tempo of the specified track
// snd_beatpattern_set_track_transpose                       FCVAR_CHEAT  // Sets the transposition of the specified track
// snd_beatpattern_stop_all_tracks                           FCVAR_CHEAT  // Stops all currently playing patterns
// snd_cast                                                  FCVAR_CHEAT  // Casts a ray and starts a sound event where the ray hits. The sound event will retrigger periodically if cl_snd_cast_retrigger is set. The sound event will clear previous snd_cast events if cl_snd_cast_clear is set. Usage: snd_cast <eventname> [<retrigger t
// snd_compare_soundevents                                   FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Compare the compiled and loaded contents of 2 soundevents.
// snd_cs_duck_reverb                                        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // One shot trigger to duck reverb for a few seconds.
// snd_front_headphone_position                              FCVAR_DEVELOPMENTONLY  // Specifies the position (in degrees) of the virtual front left/right headphones.
// snd_front_stereo_speaker_position                         FCVAR_DEVELOPMENTONLY  // Specifies the position (in degrees) of the virtual front left/right speakers.
// snd_front_surround_speaker_position                       FCVAR_DEVELOPMENTONLY  // Specifies the position (in degrees) of the virtual front left/right speakers.
// snd_get_physics_surface_properties                        FCVAR_CHEAT  // Get physics surface properties for all the materials.
// snd_headphone_pan_exponent                                FCVAR_DEVELOPMENTONLY  // Specifies the exponent for the pan xfade from phone to phone if the "exp" pan law is being used.
// snd_headphone_pan_radial_weight                           FCVAR_DEVELOPMENTONLY  // Apply cos(angle) * weight before pan law
// snd_list_deferred_soundevents                             FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // List all current deferred load soundevents
// snd_list_soundevents                                      FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // List all available soundevents
// snd_list_soundevents_by_stack                             FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // List all available soundevents using specified stack name
// snd_print_current_mixer_mixgroup                          FCVAR_DEVELOPMENTONLY  // Get data related to mix group matching string
// snd_print_samplers                                        FCVAR_CHEAT  // List all available samplers
// snd_print_soundevent                                      FCVAR_DEVELOPMENTONLY|FCVAR_VCONSOLE_FUZZY_MATCHING|FCVAR_VCONSOLE_SET_FOCUS  // Print the data associated with the specified soundevent.
// snd_print_soundevent_default_public_properties            FCVAR_DEVELOPMENTONLY|FCVAR_VCONSOLE_FUZZY_MATCHING|FCVAR_VCONSOLE_SET_FOCUS  // Print the default public properties of a specified soundevent. Values do not reflect values set on the soundevent. For that see "snd_print_soundevent"
// snd_purge_vsnd_table                                      FCVAR_DEVELOPMENTONLY  // Purges the VSnd table
// snd_rear_headphone_position                               FCVAR_DEVELOPMENTONLY  // Specifies the position  (in degrees) of the virtual rear left/right headphones.
// snd_rear_stereo_speaker_position                          FCVAR_DEVELOPMENTONLY  // Specifies the position (in degrees) of the virtual rear left/right speakers.
// snd_rear_surround_speaker_position                        FCVAR_DEVELOPMENTONLY  // Specifies the position (in degrees) of the virtual rear left/right speakers.
// snd_refresh_ui_audio_state                                FCVAR_CLIENTDLL|FCVAR_CHEAT  // Restores audio DSP state for the UI.
// snd_remove_all_soundevents                                FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Remove all soundevents
// snd_remove_soundevent                                     FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Remove the specified soundevent
// snd_samplers_play_note                                    FCVAR_CHEAT  // Play a note from a specified sampler
// snd_samplers_stop_note                                    FCVAR_CHEAT  // Stop a note from a specified sampler
// snd_set_physics_surface_properties                        FCVAR_CHEAT  // Set physics surface properties for materials. Usage: <heuristic #> <commit>
// snd_setmixer                                              FCVAR_DEVELOPMENTONLY  // Set named Mixgroup of current mixer to mix vol, mute, solo.
// snd_setmixlayer                                           FCVAR_DEVELOPMENTONLY  // Set named Mixgroup of named mix layer to mix vol, mute, solo.
// snd_side_surround_speaker_position                        FCVAR_DEVELOPMENTONLY  // Specifies the position (in degrees) of the virtual rear left/right speakers.
// snd_sos_cl_soundevent_pause_last                          FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Test
// snd_sos_cl_soundevent_start                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Test
// snd_sos_cl_soundevent_stop_last                           FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Test
// snd_sos_cl_soundevent_unpause_last                        FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Test
// snd_sos_compare_operator_stacks                           FCVAR_CHEAT  // Compares 2 operator stacks and spews any errors
// snd_sos_flush_operators                                   FCVAR_CHEAT  // Flush and re-parse the sound operator system
// snd_sos_get_operator_field_info                           FCVAR_CHEAT  // Currently gets info for a single operator field
// snd_sos_pause_soundevent                                  FCVAR_CHEAT  // Pause the specified soundevent in the list
// snd_sos_print_class_sizes                                 FCVAR_CHEAT  // Prints the sizes of relevant sos classes.
// snd_sos_print_field_name_strings                          FCVAR_CHEAT  // Prints a list of currently cached field name strings
// snd_sos_print_groups                                      FCVAR_CHEAT  // Prints the current state of the groups system
// snd_sos_print_operator_stack                              FCVAR_CHEAT  // Prints a master list of currently exposed variables
// snd_sos_print_operator_stack_operator                     FCVAR_CHEAT  // Prints an operator from a stack
// snd_sos_print_operator_stacks                             FCVAR_CHEAT  // Prints a list of currently available stacks
// snd_sos_print_operators                                   FCVAR_CHEAT  // Prints a list of currently available operators
// snd_sos_print_stack_exec_list                             FCVAR_CHEAT  // Prints the current stack execution list
// snd_sos_print_strings                                     FCVAR_CHEAT  // Prints a list of currently cached strings
// snd_sos_print_tool_properties                             FCVAR_CHEAT  // Prints the current state of tool properties.
// snd_sos_resolve_execute_operator                          FCVAR_CHEAT  // Resolve the inputs and execute one specified operator from a specified stack
// snd_sos_set_operator_field                                FCVAR_CHEAT  // Currently sets a single float operator field
// snd_sos_set_operator_field_by_guid                        FCVAR_CHEAT  // Currently sets a single float operator field
// snd_sos_soundevent_profile                                FCVAR_CHEAT  // Dump a record of current soundevents and profile data
// snd_sos_start_soundevent                                  FCVAR_CHEAT  // Starts a specified soundevent
// snd_sos_start_soundevent_at_pos                           FCVAR_CHEAT  // Starts a specified soundevent at the given position
// snd_sos_start_stack                                       FCVAR_CHEAT  // Starts a specified stack via an empty soundevent
// snd_sos_stop_all_soundevents                              FCVAR_CHEAT  // Stops all soundevents currently on the execution list
// snd_sos_stop_soundevent_guid                              FCVAR_CHEAT  // Stops a specified soundevent
// snd_sos_stop_soundevent_index                             FCVAR_CHEAT  // Stops a specified soundevent
// snd_sos_stop_track                                        FCVAR_CHEAT  // Stop the specified track and it's queue.
// snd_sos_sv_soundevent_pause_last                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Test
// snd_sos_sv_soundevent_start                               FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Test
// snd_sos_sv_soundevent_stop_last                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Test
// snd_sos_sv_soundevent_unpause_last                        FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Test
// snd_sos_sv_test_gender                                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Test
// snd_sos_test_soundmessage                                 FCVAR_CHEAT  // test
// snd_sos_unpause_soundevent                                FCVAR_CHEAT  // UnPause the first soundevent in the list
// snd_soundevent_clear_deferred                             FCVAR_DEVELOPMENTONLY|FCVAR_CHEAT  // Clear the list of deferred soundevents for loading.
// snd_soundmixer_flush                                      FCVAR_DEVELOPMENTONLY  // Reload soundmixers.txt file.
// snd_soundmixer_list_mix_groups                            FCVAR_DEVELOPMENTONLY  // List all mix groups to dev console.
// snd_soundmixer_list_mix_layers                            FCVAR_DEVELOPMENTONLY  // List all mix layers to dev console.
// snd_soundmixer_list_mixers                                FCVAR_DEVELOPMENTONLY  // List all mixers to dev console.
// snd_soundmixer_set_trigger_factor                         FCVAR_DEVELOPMENTONLY  // Set named mix layer / mix group, trigger amount.
// snd_soundmixer_setmixlayer_amount                         FCVAR_DEVELOPMENTONLY  // Set named mix layer mix amount.
// snd_steamaudio_baked_data_stats                           FCVAR_CHEAT  // Display baked data stats for the current mod.
// snd_steamaudio_display_probes                             FCVAR_DEVELOPMENTONLY  // Load all the probes from a file and display probes based on the passed on arguments.
// snd_steamaudio_enable_spatial_blend_fix                   FCVAR_CHEAT  // Toggles the speculative fix for low-frequency issues when using spatial blend.
// snd_steamaudio_halton_sequence                            FCVAR_CHEAT  // Generate Halton Sequence for a given order and number of samples.
// snd_stereo_speaker_pan_exponent                           FCVAR_DEVELOPMENTONLY  // Specifies the exponent for the pan xfade from speaker to speaker if the "exp" pan law is being used.
// snd_stereo_speaker_pan_radial_weight                      FCVAR_DEVELOPMENTONLY  // Apply cos(angle) * weight before pan law
// snd_surround_speaker_pan_exponent                         FCVAR_DEVELOPMENTONLY  // Specifies the exponent for the pan xfade from speaker to speaker if the "exp" pan law is being used.
// snd_surround_speaker_pan_radial_weight                    FCVAR_DEVELOPMENTONLY  // Apply cos(angle) * weight before pan law
// sndplaydelay                                              FCVAR_DEVELOPMENTONLY
// soundinfo                                                 FCVAR_RELEASE  // Describe the current sound device with an active voice list.
// soundlist                                                 FCVAR_DEVELOPMENTONLY  // List all known sounds.
// soundscape_dumpclient                                     FCVAR_CLIENTDLL|FCVAR_CHEAT  // Dumps the client's soundscape data.

// soundscape_flush                                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Flushes the server & client side soundscapes
// soundsysteminfo                                           FCVAR_DEVELOPMENTONLY  // Describe the current sound device without an active voice list.
// spawnCashStack                                            FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT
// spawn_group_activate                                      FCVAR_GAMEDLL|FCVAR_CHEAT  // Activate specified spawngroup.
// spawn_group_load                                          FCVAR_GAMEDLL|FCVAR_CHEAT  // Load named spawn group.
// spawn_group_unload                                        FCVAR_GAMEDLL|FCVAR_CHEAT  // Unload named spawn group.
// spec_goto                                                 FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Move the spectator camera to a specific location. `spec_goto x y z pitch yaw`
// spec_lock_to_current_player                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // As an observer, lock the spectator target to the currently observed target
// spec_mode                                                 FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Set spectator mode
// spec_next                                                 FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Spectate next player
// spec_player                                               FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Spectate a player by name or slot
// spec_pos                                                  FCVAR_CLIENTDLL|FCVAR_CHEAT  // dump position and angles to the console
// spec_prev                                                 FCVAR_CLIENTDLL|FCVAR_CLIENTCMD_CAN_EXECUTE  // Spectate previous player
// spew_fonts                                                FCVAR_DEVELOPMENTONLY  // Spew information about font manager fonts
// splitscreen_testreadconfigconflict                        FCVAR_DEVELOPMENTONLY
// ss_add                                                    FCVAR_DEVELOPMENTONLY  // Adds a splitscreen user.
// ss_remove                                                 FCVAR_DEVELOPMENTONLY  // Removes a splitscreen user.
// ss_teleport                                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL|FCVAR_CHEAT  // Teleport other splitscreen player to my location.
// start_rec_mic                                             FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Start recording to a desired wav.
// startdemos                                                FCVAR_RELEASE  // Play demos in demo sequence.
// startmovie                                                FCVAR_DEVELOPMENTONLY|FCVAR_DONTRECORD  // Start recording movie frames.
// stats                                                     FCVAR_DEVELOPMENTONLY  // Prints server performance variables
// stats_print                                               FCVAR_DEVELOPMENTONLY  // Prints out perf statistics to the console, clears perf history
// stats_print_gpu                                           FCVAR_DEVELOPMENTONLY  // Prints out GPU perf statistics to the console.  Requires stats_display > 0, and stats_collect_gpu = true.  Optional argument of CSV filename
// status                                                    FCVAR_RELEASE  // Print connection status
// status_json                                               FCVAR_RELEASE  // Print status in JSON format
// steamvrevent_quit                                         FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // steamvrevent_quit
// stop                                                      FCVAR_RELEASE  // Finish recording demo.
// stop_rec_mic                                              FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Stop recording to a desired wav.
// stop_rec_mic_all                                          FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Stop recording all mic streams.
// stopdemos                                                 FCVAR_RELEASE  // Stop looping demos (current demo will complete).
// stopsound                                                 FCVAR_CHEAT
// stopsoundscape                                            FCVAR_CLIENTDLL|FCVAR_CHEAT  // Stops all soundscape processing and fades current looping sounds
// stopwatch                                                 FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // General purpose timer. use 'stopwatch' to toggle or explicitly call 'stopwatch start' and/or 'stopwatch stop'.
// subclass_change                                           FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Changes the subclass of the given entity.
	Arguments:   	<new_subclass> {entity_name} / {class_name} / {entity_index} / {no argument = pick what player is looking at}
// subclass_create                                           FCVAR_GAMEDLL|FCVAR_CHEAT|FCVAR_VCONSOLE_FUZZY_MATCHING  // Creates an entity of the given subclass where the player is looking.
// surfaceprop                                               FCVAR_GAMEDLL|FCVAR_CHEAT  // Reports the surface properties at the cursor
// sv_annotation_give_weapon                                 FCVAR_GAMEDLL|FCVAR_RELEASE  // Give weapon required by annotation
// sv_annotation_remove_weapon                               FCVAR_GAMEDLL|FCVAR_RELEASE  // Remove weapon given by annotation
// sv_clientrates                                            FCVAR_DEVELOPMENTONLY  // Show client rates.
// sv_cs_dump_econ_item_stringtable                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // sv_cs_dump_econ_item_stringtable
// sv_dev_simulate_gcdown                                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // <state> Turn on/off simulated GC communications failure (GC is down in a way that we know it is down)
// sv_explode_inferno_at_crosshair                           FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // explodes molotov(0) or Incendiary (1) at crosshair location with single param
// sv_explode_smokegrenade_at_crosshair                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // explodes smoke grenade at crosshair location
// sv_fullupdate                                             FCVAR_DEVELOPMENTONLY  // Force a full update for all clients.
// sv_game_mode_convars                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Display the values of the convars for the current game_mode.
// sv_histogram                                              FCVAR_DEVELOPMENTONLY  // var change info histogram

// sv_kill_smokegrenade                                      FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // kill all smoke grenades
// sv_load_forced_client_names_file                          FCVAR_GAMEDLL|FCVAR_RELEASE  // Loads a file containing SteamID64 names for clients
// sv_load_random_client_names_file                          FCVAR_GAMEDLL|FCVAR_RELEASE  // Loads a file containing random name words for clients
// sv_metaduplication                                        FCVAR_CHEAT  // Check serializer meta for duplication, add verbose to command for full spew
// sv_packstats                                              FCVAR_RELEASE  // Show entity packing stats, pass 'clear' as argument to reset counts.
// sv_querycache_stats                                       FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Display status of the query cache (client only)
// sv_rethrow_last_grenade                                   FCVAR_GAMEDLL|FCVAR_CHEAT  // Emit the last grenade thrown on the server.
// sv_setsteamaccount                                        FCVAR_RELEASE  // token
Set game server account token to use for logging in to a persistent game server account
// sv_showtags                                               FCVAR_DEVELOPMENTONLY  // Describe current gametags.
// sv_shutdown                                               FCVAR_RELEASE  // Sets the server to shutdown when all games have completed
// sv_soundscape_printdebuginfo                              FCVAR_GAMEDLL|FCVAR_CHEAT  // print soundscapes
// sv_spewmeta                                               FCVAR_CHEAT  // Spew serializer meta
// sv_spewworldgroups                                        FCVAR_DEVELOPMENTONLY  // Spew world groups (server)
// sv_throw_decoygrenade                                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // throw decoy grenade with parmas.
// sv_throw_flashgrenade                                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // throw flash grenade with parmas.
// sv_throw_hegrenade                                        FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // throw HEgrenade with parmas.
// sv_throw_molotov                                          FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // throw molotov grenade with parmas.
// sv_throw_smokegrenade                                     FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL|FCVAR_CHEAT  // throw smoke grenade with parmas.
// switchhands                                               FCVAR_CLIENTDLL|FCVAR_RELEASE
// switchhandsleft                                           FCVAR_CLIENTDLL|FCVAR_RELEASE
// switchhandsright                                          FCVAR_CLIENTDLL|FCVAR_RELEASE
// sys_info                                                  FCVAR_RELEASE  // Print system information to the console
// teammenu                                                  FCVAR_CLIENTDLL|FCVAR_SERVER_CAN_EXECUTE  // Show team selection window
// telemetry_message                                         FCVAR_GAMEDLL|FCVAR_CHEAT  // Place a message in the telemetry timeline
// telemetry_toggle_timespan                                 FCVAR_GAMEDLL|FCVAR_CHEAT  // Starts/stops a timespan with an ever increasing name.
// test_dispatcheffect                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Test a clientside dispatch effect.
	Usage: test_dispatcheffect <effect name> <distance away> <flags> <magnitude> <scale>
	Defaults are: <distance 1024> <flags 0> <magnitude 0> <scale 0>

// test_entity_blocker                                       FCVAR_GAMEDLL|FCVAR_CHEAT  // Test command that drops an entity blocker out in front of the player.
// test_list_entities                                        FCVAR_GAMEDLL|FCVAR_CHEAT  // test-list entities
// test_shipping_assert                                      FCVAR_RELEASE  // Generate an assert to test shipping assertion code
// test_voice_container_nesting                              FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Test nesting voice containers.
// test_voice_containers                                     FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Quick example for how we'd derive traits from voice containers.
// thirdperson                                               FCVAR_CLIENTDLL|FCVAR_CHEAT|FCVAR_EXECUTE_PER_TICK  // Switch to thirdperson camera.
// thirdperson_mayamode                                      FCVAR_CLIENTDLL|FCVAR_CHEAT  // Switch to thirdperson Maya-like camera controls.
// thirdpersonshoulder                                       FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Switch to thirdperson-shoulder camera.
// timedemo                                                  FCVAR_RELEASE  // Play a demo and report performance info.
// timedemoquit                                              FCVAR_RELEASE  // Play a demo, report performance info, and then exit
// timeleft                                                  FCVAR_GAMEDLL  // prints the time remaining in the match
// timeout_ct_start                                          FCVAR_GAMEDLL|FCVAR_RELEASE
// timeout_terrorist_start                                   FCVAR_GAMEDLL|FCVAR_RELEASE
// toggle                                                    FCVAR_DONTRECORD|FCVAR_RELEASE  // Toggles specified convar value on and off.
// toggleRdrOpt                                              FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL
// toggleconsole                                             FCVAR_DONTRECORD|FCVAR_RELEASE  // Show/hide the console.
// toggleradarscale                                          FCVAR_CLIENTDLL|FCVAR_RELEASE  // Toggles the radar scale
// traceattack                                               FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // traceattack damage hitgroup
// tv_broadcast_resend                                       FCVAR_DEVELOPMENTONLY  // resend broadcast data to broadcast relay
// tv_broadcast_status                                       FCVAR_RELEASE  // Print out broadcast status
// tv_clients                                                FCVAR_RELEASE  // Shows list of connected SourceTV clients.
// tv_mem                                                    FCVAR_RELEASE  // hltv memory statistics. Use with "ent 10" (dump entity 10 memory usage) or "top 8" (dump top 8 memory users) or "class" CWorld (dump CWorld class)
// tv_msg                                                    FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Send a screen message to all clients.
// tv_record                                                 FCVAR_RELEASE  // Starts SourceTV demo recording.
// tv_relay                                                  FCVAR_RELEASE  // Connect to SourceTV server and relay broadcast.
// tv_retry                                                  FCVAR_RELEASE  // Reconnects the SourceTV relay proxy.
// tv_status                                                 FCVAR_RELEASE  // Show SourceTV server status.
// tv_stop                                                   FCVAR_RELEASE  // Stops the SourceTV broadcast.
// tv_stoprecord                                             FCVAR_RELEASE  // Stops SourceTV demo recording.
// unbind                                                    FCVAR_RELEASE  // Unbind a key.
// unbindall                                                 FCVAR_RELEASE  // Unbind all keys.
// unpause                                                   FCVAR_RELEASE  // Clear the pause state of the server.
// url_execute                                               FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Executes url-based commands, used for incoming commands from url-based launches when the game's already running.
// users                                                     FCVAR_DEVELOPMENTONLY  // Show user info for players on server.
// v8_dump_heap_snapshot                                     FCVAR_DEVELOPMENTONLY
// v8_write_heap_stats                                       FCVAR_DEVELOPMENTONLY  // Dump output of v8::Isolate::GetHeapStatistics.
// vis_debug_currentcluster                                  FCVAR_DEVELOPMENTONLY  // Show the current cluster number
// vis_debug_drawcluster                                     FCVAR_DEVELOPMENTONLY  // Add cluster # to visualization, (-1) to clear
// vis_debug_dumpvisibleclusters                             FCVAR_DEVELOPMENTONLY  // Show the list of visible clusters
// vis_debug_find_los                                        FCVAR_DEVELOPMENTONLY  // Find or clear the vis LOS to here
// vis_debug_lock                                            FCVAR_DEVELOPMENTONLY  // Lock vis LOS origin to current
// vis_debug_record_start                                    FCVAR_DEVELOPMENTONLY  // Record a path to debug vis
// vis_debug_record_stop                                     FCVAR_DEVELOPMENTONLY  // Record a path to debug vis
// vis_debug_show                                            FCVAR_DEVELOPMENTONLY  // Show/hide the vis debug visualization
// vis_debug_sunclusters                                     FCVAR_DEVELOPMENTONLY  // Showing clusters for sun/csm rendering. Red (full sun csm & lighting), Orange (no viewmodel sun or csm), Green (no sun at all)
// vis_debug_tracelos                                        FCVAR_DEVELOPMENTONLY  // Trace rays and check vis from the current camera
// vmem_dump                                                 FCVAR_DEVELOPMENTONLY  // Dump memory stats to log.
// vmix_debug_list                                           FCVAR_DEVELOPMENTONLY  // Debug dump the list of available vmix graphs
// vmix_input                                                FCVAR_CHEAT  // Set an input mix value
// vmix_output                                               FCVAR_CHEAT  // Dump main graph control output values
// voice_containers_get_instance_args                        FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Args: [Voice Container Path]
// voice_containers_get_instance_params                      FCVAR_LINKED_CONCOMMAND|FCVAR_DEVELOPMENTONLY  // Args: [Voice Container Path]
// voice_modenable_toggle                                    FCVAR_CLIENTDLL|FCVAR_RELEASE  // Toggle the voice_modenable convar.
// voice_status_test_toggle                                  FCVAR_DEVELOPMENTONLY|FCVAR_CLIENTDLL  // Test voice and status notices
// voice_toggle_open_mic                                     FCVAR_CLIENTDLL|FCVAR_RELEASE  // Toggles between open-mic and push-to-talk
// vprof_dump_counters                                       FCVAR_DEVELOPMENTONLY  // Dump vprof counters to the console
// vprof_generate_report                                     FCVAR_DEVELOPMENTONLY  // Generate a report to the console.
// vprof_generate_report_budget                              FCVAR_DEVELOPMENTONLY  // Generate a report to the console based on budget group.
// vprof_generate_report_hierarchy                           FCVAR_DEVELOPMENTONLY  // Generate a report to the console.
// vprof_loadhitstore_scale                                  FCVAR_DEVELOPMENTONLY  // Scale used when displaying load-hit-stores (0 = use default)
// vprof_off                                                 FCVAR_DEVELOPMENTONLY  // Disable vprof
// vprof_on                                                  FCVAR_DEVELOPMENTONLY  // Enable vprof
// vprof_remote_start                                        FCVAR_DEVELOPMENTONLY  // Request a VProf data stream from the remote server (requires authentication)
// vprof_remote_stop                                         FCVAR_DEVELOPMENTONLY  // Stop an existing remote VProf data request
// vprof_reset                                               FCVAR_DEVELOPMENTONLY  // Reset the stats in VProf profiler
// vprof_reset_peaks                                         FCVAR_DEVELOPMENTONLY  // Reset just the peak time in VProf profiler
// vprof_time_scale                                          FCVAR_DEVELOPMENTONLY  // Scale used when displaying time (0 = use default)
// vtune                                                     FCVAR_DEVELOPMENTONLY  // Controls VTune's sampling.
// weapon_switch                                             FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Use a particular weapon	
Arguments: <weapon_name>
// workshop_annotation_submit                                FCVAR_CLIENTDLL|FCVAR_RELEASE  // Submit annotation to workshop. To update an existing submission add its ID number from the workshop URL as a second argument.
// workshop_clear_cloud_save                                 FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Remove a workshop save from steam cloud. Pass the published file id to delete or 0 to delete the non-workshop addon save.
// workshop_dump_cloud_contents                              FCVAR_DEVELOPMENTONLY|FCVAR_GAMEDLL  // Spew contents of steam cloud.
// workshop_item_submit                                      FCVAR_CLIENTDLL|FCVAR_RELEASE
// workshop_tournament_item_submit                           FCVAR_CLIENTDLL|FCVAR_RELEASE
// world_dump_loaded_worlds                                  FCVAR_DEVELOPMENTONLY  // Dump all of the worlds that we know about
// world_layer_list                                          FCVAR_DEVELOPMENTONLY  // List all world layers
// world_layer_set_visible                                   FCVAR_DEVELOPMENTONLY  // Show or hide the specified world layer
// writeid                                                   FCVAR_DEVELOPMENTONLY  // Writes a list of permanently-banned user IDs to file.
// writeip                                                   FCVAR_DEVELOPMENTONLY  // Save the ban list to file.
// writekeybindings                                          FCVAR_RELEASE  // Saves current key bindings to disk.
