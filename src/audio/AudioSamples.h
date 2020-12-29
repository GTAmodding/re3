﻿#pragma once

enum eSfxSample
{
	SFX_CAR_HORN_JEEP = 0,
	SFX_CAR_HORN_BMW328,
	SFX_CAR_HORN_BUS,
	SFX_CAR_HORN_BUS2,
	SFX_CAR_HORN_56CHEV,
	SFX_CAR_HORN_PICKUP,
	SFX_CAR_HORN_PORSCHE,
	SFX_CAR_HORN_TRUCK,

	SFX_CAR_HELI_MAI,  // 8
	SFX_CAR_HELI_MAI2, // 9
	SFX_CAR_HELI_REA,  // 10
	SFX_CAR_HELI_STA,  // 11
	SFX_CAR_HELI_ROT,  // 12
	SFX_CAR_HELI_FAR,  // 13
	SFX_CAR_HELI_ROL,  // 14

	SFX_OLD_CAR_DOOR_OPEN,
	SFX_OLD_CAR_DOOR_CLOSE,
	SFX_NEW_CAR_DOOR_OPEN,
	SFX_NEW_CAR_DOOR_CLOSE,
	SFX_TRUCK_DOOR_OPEN,
	SFX_TRUCK_DOOR_CLOSE,
	SFX_REVERSE_GEAR,
	SFX_REVERSE_GEAR_2,
	SFX_CAR_STARTER, // 23
	SFX_ROAD_NOISE,  // 24
	SFX_SKID,        // 25
	SFX_GRAVEL_SKID, // 26
	SFX_POLICE_SIREN_SLOW,
	SFX_SIREN_FAST, // 28
	SFX_AMBULANCE_SIREN_SLOW,
	SFX_REVERSE_WARNING,
	SFX_ICE_CREAM_TUNE,
	SFX_AIR_BRAKES,    // 32
	SFX_TYRE_BUMP,     // 33
	SFX_TYRE_BURST_B,  // 34
	SFX_TYRE_BURST,    // 35
	SFX_TYRE_BURST_L,  // 36
	SFX_PALM_TREE_LO,  // 37
	SFX_BULLET_PASS_1, // 38
	SFX_BULLET_PASS_2, // 39
	SFX_SKATE_1,       // 40
	SFX_SKATE_2,       // 41
	SFX_FOOTSTEP_CONCRETE_1,
	SFX_FOOTSTEP_CONCRETE_2,
	SFX_FOOTSTEP_CONCRETE_3,
	SFX_FOOTSTEP_CONCRETE_4,
	SFX_FOOTSTEP_CONCRETE_5,
	SFX_EXPLOSION_1,              // 47
	SFX_EXPLOSION_2,              // 48
	SFX_EXPLOSION_3,              // 49
	SFX_COLT45_LEFT,              // 50
	SFX_COLT45_RIGHT,             // 51
	SFX_AK47_LEFT,                // 52
	SFX_AK47_RIGHT,               // 53
	SFX_UZI_LEFT,                 // 54
	SFX_UZI_RIGHT,                // 55
	SFX_UZI_END_LEFT,             // 56
	SFX_SNIPER_LEFT,              // 57
	SFX_SNIPER_RIGHT,             // 58
	SFX_ROCKET_LEFT,              // 59
	SFX_ROCKET_RIGHT,             // 60
	SFX_ROCKET_FLY,               // 61
	SFX_FLAMETHROWER_LEFT,        // 62
	SFX_FLAMETHROWER_RIGHT,       // 63
	SFX_FLAMETHROWER_START_LEFT,  // 64
	SFX_FLAMETHROWER_START_RIGHT, // 65
	SFX_SHOTGUN_LEFT,             // 66
	SFX_SHOTGUN_RIGH,             // 67
	SFX_M60_LEFT,                 // 68
	SFX_M60_RIGHT,                // 69
	SFX_M60_TAIL_LEFT,            // 70
	SFX_TEC_LEFT,                 // 71
	SFX_TEC_RIGHT,                // 72
	SFX_TEC_TAIL,                 // 73
	SFX_RUGER_LEFT,               // 74
	SFX_RUGER_RIGHT,              // 75
	SFX_RUGER_TAIL,               // 76
	SFX_PISTOL_RELOAD,            // 77
	SFX_AK47_RELOAD,              // 78
	SFX_ROCKET_RELOAD,            // 79
	SFX_RIFLE_RELOAD,             // 80
	SFX_GOLF_CLUB_SWING,          // 81
	SFX_MINIGUN_FIRE_LEFT,        // 82
	SFX_MINIGUN_FIRE_RIGHT,       // 83
	SFX_MINIGUN_STOP,             // 84
	SFX_SPAS12_LEFT,              // 85
	SFX_SPAS12_RIGHT,             // 86
	SFX_SPAS12_TAIL_LEFT,         // 87
	SFX_PYTHON_LEFT,              // 88
	SFX_PYTHON_RIGHT,             // 89
	SFX_MP5_LEFT,                 // 90
	SFX_MP5_RIGHT,                // 91
	SFX_COL_TARMAC_1,             // 92
	SFX_COL_TARMAC_2,             // 93
	SFX_COL_TARMAC_3,             // 94
	SFX_COL_TARMAC_4,             // 95
	SFX_COL_TARMAC_5,             // 96
	SFX_COL_GRASS_1,
	SFX_COL_GRAVEL_1,
	SFX_COL_MUD_1,
	SFX_COL_GARAGE_DOOR_1,
	SFX_COL_CAR_PANEL_1,
	SFX_COL_CAR_PANEL_2,
	SFX_COL_CAR_PANEL_3,
	SFX_COL_CAR_PANEL_4,
	SFX_COL_CAR_PANEL_5,
	SFX_COL_CAR_PANEL_6,
	SFX_COL_THICK_METAL_PLATE_1,
	SFX_COL_SCAFFOLD_POLE_1,
	SFX_COL_LAMP_POST_1,
	SFX_COL_HYDRANT_1,
	SFX_COL_METAL_CHAIN_FENCE_1,
	SFX_COL_METAL_CHAIN_FENCE_2,
	SFX_COL_METAL_CHAIN_FENCE_3,
	SFX_COL_METAL_CHAIN_FENCE_4,
	SFX_COL_PED_1, // 115
	SFX_COL_PED_2, // 116
	SFX_COL_SAND_1,
	SFX_COL_WOOD_CRATES_1,
	SFX_COL_WOOD_CRATES_2,
	SFX_COL_WOOD_CRATES_3,
	SFX_COL_WOOD_CRATES_4,
	SFX_COL_WOOD_BENCH_1,
	SFX_COL_WOOD_BENCH_2,
	SFX_COL_WOOD_BENCH_3,
	SFX_COL_WOOD_BENCH_4,
	SFX_COL_WOOD_SOLID_1,
	SFX_COL_VEG_1, // 127
	SFX_COL_VEG_2, // 128
	SFX_COL_VEG_3, // 129
	SFX_COL_VEG_4, // 130
	SFX_COL_VEG_5, // 131
	SFX_COL_CONTAINER_1,
	SFX_COL_NEWS_VENDOR_1,
	SFX_COL_NEWS_VENDOR_2,
	SFX_COL_NEWS_VENDOR_3,
	SFX_COL_CAR_1, // 136
	SFX_COL_CAR_2, // 137
	SFX_COL_CAR_3, // 138
	SFX_COL_CAR_4, // 139
	SFX_COL_CAR_5, // 140
	SFX_COL_CARDBOARD_1,
	SFX_COL_CARDBOARD_2,
	SFX_COL_GATE,     // 143
	SFX_SCRAPE_CAR_1, // 144
	SFX_CRATE_SMASH,
	SFX_GLASS_CRACK, // 146
	SFX_GLASS_SMASH, // 147
	SFX_GLASS_SHARD_1,
	SFX_GLASS_SHARD_2,
	SFX_GLASS_SHARD_3,
	SFX_GLASS_SHARD_4,
	SFX_PED_ON_FIRE,  // 152
	SFX_CAR_ON_FIRE,  // 153
	SFX_RAIN,         // 154
	SFX_HURRICANE_MA, // 155
	SFX_BULLET_SHELL_HIT_GROUND_1,
	SFX_BULLET_SHELL_HIT_GROUND_2,
	SFX_BULLET_PED,           // 158
	SFX_BULLET_CAR_1,         // 159
	SFX_BULLET_CAR_2,         // 160
	SFX_BULLET_CAR_3,         // 161
	SFX_BULLET_WALL_1,        // 162
	SFX_BULLET_WALL_2,        // 163
	SFX_BULLET_WALL_3,        // 164
	SFX_BAT_HIT_LEFT,         // 165
	SFX_BAT_HIT_RIGH,         // 166
	SFX_FIGHT_1,              // 167
	SFX_FIGHT_2,              // 168
	SFX_FIGHT_4,              // 169
	SFX_FIGHT_5,              // 170
	SFX_KNIFE_SWING,          // 171
	SFX_KNIFE_SLASH,          // 172
	SFX_KNIFE_STAB,           // 173
	SFX_HAMMER_HIT_1,         // 174
	SFX_HAMMER_HIT_2,         // 175
	SFX_GARAGE_DOOR_LOOP,     // 176
	SFX_COUNTDOWN,            // 177
	SFX_ARM_BOMB,             // 178
	SFX_POLICE_RADIO_CRACKLE, // 179

	SFX_WEVE_GOT,
	SFX_THERES,
	SFX_RESPOND_TO,
	SFX_A_10,
	SFX_IN,
	SFX_NORTH,
	SFX_EAST,
	SFX_SOUTH,
	SFX_WEST,
	SFX_CENTRAL,
	SFX_POLICE_RADIO_MESSAGE_NOISE_1,
	SFX_POLICE_RADIO_SUSPECT,
	SFX_POLICE_RADIO_LAST_SEEN,
	SFX_POLICE_RADIO_ON_FOOT,
	SFX_POLICE_RADIO_IN_A,
	SFX_POLICE_RADIO_DARK,
	SFX_POLICE_RADIO_LIGHT,
	SFX_POLICE_RADIO_BRIGHT,

	SFX_CRIME_1,
	SFX_CRIME_2,
	SFX_CRIME_3,
	SFX_CRIME_4,
	SFX_CRIME_5,
	SFX_CRIME_6,
	SFX_CRIME_7,
	SFX_CRIME_8,
	SFX_CRIME_9,
	SFX_CRIME_10,
	SFX_CRIME_11,
	SFX_CRIME_12,
	SFX_POLICE_RADIO_VICE_CITY,
	SFX_POLICE_RADIO_VICE_CITY_BEACH,
	SFX_POLICE_RADIO_VICE_CITY_MAINLAND,
	SFX_POLICE_RADIO_OCEAN_BEACH, //???
	SFX_POLICE_RADIO_WASHINGTON_BEACH,
	SFX_POLICE_RADIO_VICE_POINT,
	SFX_POLICE_RADIO_LEAF_LINKS,
	SFX_POLICE_RADIO_STRAFISH_ISLAND, //???????????
	SFX_POLICE_RADIO_VICE_PORT,
	SFX_POLICE_RADIO_LITTLE_HAVANA,
	SFX_POLICE_RADIO_LITTLE_HAITI,
	SFX_POLICE_RADIO_PRAWN_ISLAND, //??????????? IS THAT HOW SHE PRONOUNCES ISLAND?
	SFX_POLICE_RADIO_DOWNTOWN,
	SFX_POLICE_RADIO_ESCOBAR_INTERNATIONAL,
	SFX_POLICE_RADIO_BLACK,
	SFX_POLICE_RADIO_WHITE,
	SFX_POLICE_RADIO_BLUE,
	SFX_POLICE_RADIO_RED,
	SFX_POLICE_RADIO_PURPLE,
	SFX_POLICE_RADIO_YELLOW,
	SFX_POLICE_RADIO_GREY,
	SFX_POLICE_RADIO_ORANGE,
	SFX_POLICE_RADIO_GREEN,
	SFX_POLICE_RADIO_SILVER,
	SFX_POLICE_RADIO_AMBULANCE,
	SFX_POLICE_RADIO_2_DOOR,
	SFX_POLICE_RADIO_TRUCK,
	SFX_POLICE_RADIO_FIRE_TRUCK,
	SFX_POLICE_RADIO_PICKUP,
	SFX_POLICE_RADIO_POLICE_CAR,
	SFX_POLICE_RADIO_BOAT,
	SFX_POLICE_RADIO_BUGGY,
	SFX_POLICE_RADIO_BUS,
	SFX_POLICE_RADIO_COACH,
	SFX_POLICE_RADIO_CRUISER,
	SFX_POLICE_RADIO_DINGHY,
	SFX_POLICE_RADIO_GARBAGE_TRUCK,
	SFX_POLICE_RADIO_GOLF_CART,
	SFX_POLICE_RADIO_HEARSE,
	SFX_POLICE_RADIO_HELICOPTER,
	SFX_POLICE_RADIO_ICE_CREAM_VAN,
	SFX_POLICE_RADIO_LOWRIDER,
	SFX_POLICE_RADIO_MOPED,
	SFX_POLICE_RADIO_MOTOBIKE,
	SFX_POLICE_RADIO_OFFROAD,
	SFX_POLICE_RADIO_PLANE,
	SFX_POLICE_RADIO_RIG,
	SFX_POLICE_RADIO_SEDAN,
	SFX_POLICE_RADIO_SPEEDBOAT,
	SFX_POLICE_RADIO_SPORTS_CAR,
	SFX_POLICE_RADIO_STATION_WAGON,
	SFX_POLICE_RADIO_STRETCH,
	SFX_POLICE_RADIO_SWAT_VAN,
	SFX_POLICE_RADIO_TANK,
	SFX_POLICE_RADIO_TAXI,
	SFX_POLICE_RADIO_VAN,

	SFX_HELI_1,      // 198
	SFX_PHONE_RING,  // 199
	SFX_CAR_REV_1,   // PONT
	SFX_CAR_REV_2,   // PORSHE
	SFX_CAR_REV_3,   // SPIDER
	SFX_CAR_REV_4,   // MERC
	SFX_CAR_REV_5,   // TRUC
	SFX_CAR_REV_6,   // HOTROD
	SFX_CAR_REV_7,   // COBRA
	SFX_CAR_REV_8,   // PONT2
	SFX_CAR_REV_9,   // CADI
	SFX_CAR_REV_10,  // PATHFINDER
	SFX_CAR_REV_11,  // PACARD
	SFX_CAR_REV_12,  // GOLFCART
	SFX_CAR_REV_13,  // SFX_CAR_IDLE_GOL
	SFX_CAR_REV_14,  // SFX_CAR_IDLE_GOL
	SFX_CAR_REV_15,  // SFX_CAR_IDLE_GOL
	SFX_CAR_REV_16,  // SFX_CAR_IDLE_GOL
	SFX_CAR_REV_17,  // VTWI
	SFX_MOPED_REV,   // just moped
	SFX_CAR_REV_19,  // HOND(A)
	SFX_CAR_REV_20,  // SPOR(TCAR)
	SFX_CAR_IDLE_1,  // PONT
	SFX_CAR_IDLE_2,  // PORSHE
	SFX_CAR_IDLE_3,  // SPIDER
	SFX_CAR_IDLE_4,  // MERC
	SFX_CAR_IDLE_5,  // TRUC
	SFX_CAR_IDLE_6,  // HOTROD
	SFX_CAR_IDLE_7,  // COBRA
	SFX_CAR_IDLE_8,  // PONT2
	SFX_CAR_IDLE_9,  // CADI
	SFX_CAR_IDLE_10, // PATHFINDER
	SFX_CAR_IDLE_11, // PACARD
	SFX_CAR_IDLE_12, // GOLFCART
	SFX_CAR_IDLE_13, // SFX_CAR_IDLE_GOL
	SFX_CAR_IDLE_14, // SFX_CAR_IDLE_GOL
	SFX_CAR_IDLE_15, // SFX_CAR_IDLE_GOL
	SFX_CAR_IDLE_16, // SFX_CAR_IDLE_GOL
	SFX_CAR_IDLE_17, // VTWI
	SFX_MOPED_IDLE,  // 237
	SFX_CAR_IDLE_19, // HOND(A)
	SFX_CAR_IDLE_20, // SPOR(TCAR)
	SFX_JUMBO_DIST_FLY,
	SFX_JUMBO_TAXI,   // 241
	SFX_JUMBO_WHINE,  // 242
	SFX_JUMBO_ENGINE, // 243
	SFX_JUMBO_RUMBLE, // 244
	SFX_JUMBO_LAND_WHEELS,
	SFX_BOAT_CRUISER_LOOP, // 246
	SFX_BOAT_V12_LOOP,     // 247
	SFX_BOAT_WATER_LOOP,
	SFX_BOAT_SPLASH_1,
	SFX_BOAT_SPLASH_2,
	SFX_FISHING_BOAT_IDLE,
	SFX_CAR_RAIN_1,   // 252
	SFX_CAR_RAIN_2,   // 253
	SFX_CAR_RAIN_3,   // 254
	SFX_CAR_RAIN_4,   // 255
	SFX_SPLASH_1,     // 256
	SFX_PED_CRUNCH_1, // 257
	SFX_PED_CRUNCH_2, // 258
	SFX_WOODEN_BOX_SMASH,
	SFX_CARDBOARD_BOX_SMASH,
	SFX_ERROR_FIRE_ROCKET_LAUNCHER,
	SFX_ERROR_FIRE_RIFLE,
	SFX_TANK_TURRET, // 263
	SFX_BODY_LAND_AND_FALL,
	SFX_BODY_LAND,  // 265
	SFX_BOMB_BEEP,  // 266
	SFX_TIMER_BEEP, // 267
	SFX_SUSPENSION_FAST_MOVE,
	SFX_SUSPENSION_SLOW_MOVE_LOOP,
	SFX_SHAG_SUSPENSION,
	SFX_HIT_BALL,     // 271
	SFX_ARCADE,       // 272
	SFX_CESNA_IDLE,   // 273
	SFX_CESNA_REV,    // 274
	SFX_RADIO_CLICK,  // 275
	SFX_RADIO_DIAL_1, // 276
	SFX_RADIO_DIAL_2, // 277
	SFX_RADIO_DIAL_3, // 278

	// pc only
	SFX_RADIO_DIAL_4,
	SFX_RADIO_DIAL_5,
	SFX_RADIO_DIAL_6,
	SFX_RADIO_DIAL_7,
	SFX_RADIO_DIAL_8,
	SFX_RADIO_DIAL_9,
	SFX_RADIO_DIAL_10,
	SFX_RADIO_DIAL_11,
	SFX_RADIO_DIAL_12,

	SFX_INFO_LEFT,                    // 279
	SFX_INFO_RIGHT,                   // 280
	SFX_INFO_CENTRE,                  // 281
	SFX_MONEY_LEFT,                   // 282
	SFX_MONEY_RIGHT,                  // 283
	SFX_WEAPON_LEFT,                  // 284
	SFX_WEAPON_RIGHT,                 // 285
	SFX_WEAPON_CENTRE,                // 286
	SFX_PART_MISSION_COMPLETE_LEFT,   // 287
	SFX_PART_MISSION_COMPLETE_RIGHT,  // 288
	SFX_PART_MISSION_COMPLETE_CENTRE, // 289
	SFX_GO_LEFT,                      // 290
	SFX_GO_RIGHT,                     // 291
	SFX_GO_CENTRE,                    // 292
	SFX_TIMER,                        // 293
	SFX_EMPTY,                        // 294

	SFX_FE_HIGHLIGHT_LEFT,  //
	SFX_FE_HIGHLIGHT_RIGHT, //
	SFX_FE_SELECT_LEFT,     //
	SFX_FE_SELECT_RIGHT,    //
	SFX_FE_BACK_LEFT,       //
	SFX_FE_BACK_RIGHT,      //
	SFX_FE_ERROR_LEFT,      //
	SFX_FE_ERROR_RIGHT,     //
	SFX_FE_NOISE_BURST_1,
	SFX_FE_NOISE_BURST_2,
	SFX_FE_NOISE_BURST_3,

	SFX_CAR_ACCEL_1,
	SFX_CAR_AFTER_ACCEL_1,
	SFX_CAR_FINGER_OFF_ACCEL_1,

	SFX_CAR_ACCEL_2,
	SFX_CAR_AFTER_ACCEL_2,
	SFX_CAR_FINGER_OFF_ACCEL_2,

	SFX_CAR_ACCEL_3,
	SFX_CAR_AFTER_ACCEL_3,
	SFX_CAR_FINGER_OFF_ACCEL_3,

	SFX_CAR_ACCEL_4,
	SFX_CAR_AFTER_ACCEL_4,
	SFX_CAR_FINGER_OFF_ACCEL_4,

	SFX_CAR_ACCEL_5,
	SFX_CAR_AFTER_ACCEL_5,
	SFX_CAR_FINGER_OFF_ACCEL_5,

	SFX_CAR_ACCEL_6,
	SFX_CAR_AFTER_ACCEL_6,
	SFX_CAR_FINGER_OFF_ACCEL_6,

	SFX_CAR_ACCEL_7,
	SFX_CAR_AFTER_ACCEL_7,
	SFX_CAR_FINGER_OFF_ACCEL_7,

	SFX_CAR_ACCEL_8,
	SFX_CAR_AFTER_ACCEL_8,
	SFX_CAR_FINGER_OFF_ACCEL_8,

	SFX_CAR_ACCEL_9,
	SFX_CAR_AFTER_ACCEL_9,
	SFX_CAR_FINGER_OFF_ACCEL_9,

	SFX_CAR_ACCEL_10,
	SFX_CAR_AFTER_ACCEL_10,
	SFX_CAR_FINGER_OFF_ACCEL_10,

	SFX_CAR_ACCEL_11,
	SFX_CAR_AFTER_ACCEL_11,
	SFX_CAR_FINGER_OFF_ACCEL_11,

	SFX_CAR_ACCEL_12,
	SFX_CAR_AFTER_ACCEL_12,
	SFX_CAR_FINGER_OFF_ACCEL_12,

	// some CHAINSAW STUFF
	// SFX_CAR_CHAINSAW, //10973
	// SFX_CAR_CHAINSAW, //10974
	// SFX_CAR_CHAINSAW, //10975
	SFX_CAR_ACCEL_13,
	SFX_CAR_AFTER_ACCEL_13,
	SFX_CAR_FINGER_OFF_ACCEL_13,

	SFX_RC_IDLE,  // 10976
	SFX_RC_REV,   // 10977
	SFX_RC_EMPTY, // 10978

	SFX_CAR_RC_HELI,             // 10979
	SFX_CAR_AFTER_ACCEL_15,      // empty
	SFX_CAR_FINGER_OFF_ACCEL_15, // empty

	SFX_CAR_ACCEL_16,            // empty
	SFX_CAR_AFTER_ACCEL_16,      // empty
	SFX_CAR_FINGER_OFF_ACCEL_16, // empty

	// bike stuff apparently
	SFX_CAR_ACCEL_17,
	SFX_CAR_AFTER_ACCEL_17,
	SFX_CAR_FINGER_OFF_ACCEL_17,
	SFX_CAR_WIND_17,

	SFX_CAR_ACCEL_18,
	SFX_CAR_AFTER_ACCEL_18,
	SFX_CAR_FINGER_OFF_ACCEL_18,
	SFX_CAR_WIND_18,

	SFX_CAR_ACCEL_19,
	SFX_CAR_AFTER_ACCEL_19,
	SFX_CAR_FINGER_OFF_ACCEL_19,
	SFX_CAR_WIND_19,

	SFX_CAR_ACCEL_20,
	SFX_CAR_AFTER_ACCEL_20,
	SFX_CAR_FINGER_OFF_ACCEL_20,
	SFX_CAR_WIND_20,

	// some emptinnes here
	SFX_CAR_ACCEL_21,
	SFX_CAR_AFTER_ACCEL_21,
	SFX_CAR_FINGER_OFF_ACCEL_21,
	SFX_CAR_ACCEL_22,
	SFX_CAR_AFTER_ACCEL_22,
	SFX_CAR_FINGER_OFF_ACCEL_22,

	SFX_HELI_APACHE_1,
	SFX_HELI_APACHE_2,
	SFX_HELI_APACHE_3,
	SFX_HELI_APACHE_4,

	// something padded for more heli?
	SFX_HELI_UNUSED_1,
	SFX_HELI_UNUSED_2,
	SFX_HELI_UNUSED_3,
	SFX_HELI_UNUSED_4,

	SFX_SEAPLANE_PRO1, // 11018
	SFX_SEAPLANE_PRO2, // 11019
	SFX_SEAPLANE_PRO3, // 11020
	SFX_SEAPLANE_PRO4, // 11021
	// low fuel
	SFX_SEAPLANE_LOW, // 11022

	// something padded for more plane?
	SFX_PLANE_UNUSED_1,
	SFX_PLANE_UNUSED_2,
	SFX_PLANE_UNUSED_3,
	SFX_PLANE_UNUSED_4,

	// script objects
	SFX_BUILDINGS_BANK_ALARM, // 11027
	SFX_BUILDING_SNORE,       // 11028
	SFX_BUILDING_BAR_1,       // 11029
	SFX_BUILDING_BAR_2,       // 11030
	SFX_BUILDING_BAR_3,       // 11031
	SFX_BUILDING_BAR_4,       // 11032
	SFX_BUILDING_MAL1,        // 11033
	SFX_BUILDING_MAL2,        // 11034
	SFX_BUILDING_MAL3,        // 11035
	SFX_BUILDING_STR1,        // 11036
	SFX_BUILDING_STR2,        // 11037
	SFX_BUILDING_STR3,        // 11038
	SFX_BUILDING_CHURCH,      // 11039
	SFX_BUILDING_FAN_1,       // 11040
	SFX_BUILDING_FAN_2,       // 11041
	SFX_BUILDING_FAN_3,       // 11042
	SFX_BUILDING_FAN_4,       // 11043
	SFX_BUILDING_INSECTS_1,   // 11044
	SFX_BUILDING_INSECTS_2,   // 11045
	SFX_BUILDING_INSECTS_3,   // 11046
	SFX_BUILDING_INSECTS_4,   // 11047
	SFX_BUILDING_INSECTS_5,   // 11048
	SFX_CLUB_1,               // 11049
	SFX_CLUB_2,               // 11050
	SFX_CLUB_3,               // 11051
	SFX_CLUB_4,               // 11052

	SFX_FOOTSTEP_GRASS_1,
	SFX_FOOTSTEP_GRASS_2,
	SFX_FOOTSTEP_GRASS_3,
	SFX_FOOTSTEP_GRASS_4,
	SFX_FOOTSTEP_GRASS_5,
	SFX_FOOTSTEP_GRAVEL_1,
	SFX_FOOTSTEP_GRAVEL_2,
	SFX_FOOTSTEP_GRAVEL_3,
	SFX_FOOTSTEP_GRAVEL_4,
	SFX_FOOTSTEP_GRAVEL_5,
	SFX_FOOTSTEP_WOOD_1,
	SFX_FOOTSTEP_WOOD_2,
	SFX_FOOTSTEP_WOOD_3,
	SFX_FOOTSTEP_WOOD_4,
	SFX_FOOTSTEP_WOOD_5,
	SFX_FOOTSTEP_METAL_1,
	SFX_FOOTSTEP_METAL_2,
	SFX_FOOTSTEP_METAL_3,
	SFX_FOOTSTEP_METAL_4,
	SFX_FOOTSTEP_METAL_5,
	SFX_FOOTSTEP_WATER_1,
	SFX_FOOTSTEP_WATER_2,
	SFX_FOOTSTEP_WATER_3,
	SFX_FOOTSTEP_WATER_4,
	SFX_FOOTSTEP_SAND_1,
	SFX_FOOTSTEP_SAND_2,
	SFX_FOOTSTEP_SAND_3,
	SFX_FOOTSTEP_SAND_4,

	// TODO: miami ped comments... THERE'S OVER 9000
	SFX_POLICE_BOAT_1 = 714,
	SFX_POLICE_BOAT_2,
	SFX_POLICE_BOAT_3,
	SFX_POLICE_BOAT_4,
	SFX_POLICE_BOAT_5,
	SFX_POLICE_BOAT_6,
	SFX_POLICE_BOAT_7,
	SFX_POLICE_BOAT_8,
	SFX_POLICE_BOAT_9,
	SFX_POLICE_BOAT_10,
	SFX_POLICE_BOAT_11,
	SFX_POLICE_BOAT_12,
	SFX_POLICE_BOAT_13,
	SFX_POLICE_BOAT_14,
	SFX_POLICE_BOAT_15,
	SFX_POLICE_BOAT_16,
	SFX_POLICE_BOAT_17,
	SFX_POLICE_BOAT_18,
	SFX_POLICE_BOAT_19,
	SFX_POLICE_BOAT_20,
	SFX_POLICE_BOAT_21,
	SFX_POLICE_BOAT_22,
	SFX_POLICE_BOAT_23,

	SFX_POLICE_HELI_1,
	SFX_POLICE_HELI_2,
	SFX_POLICE_HELI_3,
	SFX_POLICE_HELI_4,
	SFX_POLICE_HELI_5,
	SFX_POLICE_HELI_6,
	SFX_POLICE_HELI_7,
	SFX_POLICE_HELI_8,
	SFX_POLICE_HELI_9,
	SFX_POLICE_HELI_10,
	SFX_POLICE_HELI_11,
	SFX_POLICE_HELI_12,
	SFX_POLICE_HELI_13,
	SFX_POLICE_HELI_14,
	SFX_POLICE_HELI_15,
	SFX_POLICE_HELI_16,
	SFX_POLICE_HELI_17,
	SFX_POLICE_HELI_18,
	SFX_POLICE_HELI_19,
	SFX_POLICE_HELI_20,

	SFX_GENERIC_FEMALE_GRUNT_1 = 2953,
	SFX_GENERIC_FEMALE_GRUNT_2 = 2954,
	SFX_GENERIC_FEMALE_GRUNT_3 = 2955,
	SFX_GENERIC_FEMALE_GRUNT_4 = 2956,
	SFX_GENERIC_FEMALE_GRUNT_5 = 2957,
	SFX_GENERIC_FEMALE_GRUNT_6 = 2958,
	SFX_GENERIC_FEMALE_GRUNT_7 = 2959,
	SFX_GENERIC_FEMALE_GRUNT_8 = 2960,
	SFX_GENERIC_FEMALE_GRUNT_9 = 2961,
	SFX_GENERIC_FEMALE_GRUNT_10 = 2962,
	SFX_GENERIC_FEMALE_GRUNT_11 = 2963,
	SFX_GENERIC_FEMALE_GRUNT_12 = 2964,
	SFX_GENERIC_FEMALE_GRUNT_13 = 2965,
	SFX_GENERIC_FEMALE_GRUNT_14 = 2966,
	SFX_GENERIC_FEMALE_GRUNT_15 = 2967,
	SFX_GENERIC_FEMALE_GRUNT_16 = 2968,
	SFX_GENERIC_FEMALE_GRUNT_17 = 2969,
	SFX_GENERIC_FEMALE_GRUNT_18 = 2970,
	SFX_GENERIC_FEMALE_GRUNT_19 = 2971,
	SFX_GENERIC_FEMALE_GRUNT_20 = 2972,
	SFX_GENERIC_FEMALE_GRUNT_21 = 2973,
	SFX_GENERIC_FEMALE_GRUNT_22 = 2974,
	SFX_GENERIC_FEMALE_GRUNT_23 = 2975,
	SFX_GENERIC_FEMALE_GRUNT_24 = 2976,
	SFX_GENERIC_FEMALE_GRUNT_25 = 2977,
	SFX_GENERIC_FEMALE_GRUNT_26 = 2978,
	SFX_GENERIC_FEMALE_GRUNT_27 = 2979,
	SFX_GENERIC_FEMALE_GRUNT_28 = 2980,
	SFX_GENERIC_FEMALE_GRUNT_29 = 2981,
	SFX_GENERIC_FEMALE_GRUNT_30 = 2982,
	SFX_GENERIC_FEMALE_GRUNT_31 = 2983,
	SFX_GENERIC_FEMALE_GRUNT_32 = 2984,
	SFX_GENERIC_FEMALE_GRUNT_33 = 2985,
	SFX_GENERIC_MALE_FIRE_1 = 3013,
	SFX_GENERIC_MALE_FIRE_2 = 3014,
	SFX_GENERIC_MALE_FIRE_3 = 3015,
	SFX_GENERIC_MALE_FIRE_4 = 3016,
	SFX_GENERIC_MALE_FIRE_5 = 3017,
	SFX_GENERIC_MALE_FIRE_6 = 3018,
	SFX_GENERIC_MALE_FIRE_7 = 3019,
	SFX_GENERIC_MALE_FIRE_8 = 3020,
	SFX_GENERIC_MALE_FIRE_9 = 3021,
	SFX_GENERIC_MALE_FIRE_10 = 3022,
	SFX_GENERIC_MALE_FIRE_11 = 3023,
	SFX_GENERIC_MALE_FIRE_12 = 3024,
	SFX_GENERIC_MALE_FIRE_13 = 3025,
	SFX_GENERIC_MALE_FIRE_14 = 3026,
	SFX_GENERIC_MALE_FIRE_15 = 3027,
	SFX_GENERIC_MALE_FIRE_16 = 3028,
	SFX_GENERIC_MALE_FIRE_17 = 3029,
	SFX_GENERIC_MALE_FIRE_18 = 3030,
	SFX_GENERIC_MALE_FIRE_19 = 3031,
	SFX_GENERIC_MALE_FIRE_20 = 3032,
	SFX_GENERIC_MALE_FIRE_21 = 3033,
	SFX_GENERIC_MALE_FIRE_22 = 3034,
	SFX_GENERIC_MALE_FIRE_23 = 3035,
	SFX_GENERIC_MALE_FIRE_24 = 3036,
	SFX_GENERIC_MALE_FIRE_25 = 3037,
	SFX_GENERIC_MALE_FIRE_26 = 3038,
	SFX_GENERIC_MALE_FIRE_27 = 3039,
	SFX_GENERIC_MALE_FIRE_28 = 3040,
	SFX_GENERIC_MALE_FIRE_29 = 3041,
	SFX_GENERIC_MALE_FIRE_30 = 3042,
	SFX_GENERIC_MALE_FIRE_31 = 3043,
	SFX_GENERIC_MALE_FIRE_32 = 3044,
	SFX_GENERIC_MALE_DEATH_1 = 3045,
	SFX_GENERIC_MALE_DEATH_2 = 3046,
	SFX_GENERIC_MALE_DEATH_3 = 3047,
	SFX_GENERIC_MALE_DEATH_4 = 3048,
	SFX_GENERIC_MALE_DEATH_5 = 3049,
	SFX_GENERIC_MALE_DEATH_6 = 3050,
	SFX_GENERIC_MALE_DEATH_7 = 3051,
	SFX_GENERIC_MALE_DEATH_8 = 3052,
	SFX_GENERIC_MALE_DEATH_9 = 3053,
	SFX_GENERIC_MALE_DEATH_10 = 3054,
	SFX_GENERIC_MALE_DEATH_11 = 3055,
	SFX_GENERIC_MALE_DEATH_12 = 3056,
	SFX_GENERIC_MALE_DEATH_13 = 3057,
	SFX_GENERIC_MALE_DEATH_14 = 3058,
	SFX_GENERIC_MALE_DEATH_15 = 3059,
	SFX_GENERIC_MALE_DEATH_16 = 3060,
	SFX_GENERIC_MALE_DEATH_17 = 3061,
	SFX_GENERIC_MALE_DEATH_18 = 3062,
	SFX_GENERIC_MALE_DEATH_19 = 3063,
	SFX_GENERIC_MALE_DEATH_20 = 3064,
	SFX_GENERIC_MALE_DEATH_21 = 3065,
	SFX_GENERIC_MALE_DEATH_22 = 3066,
	SFX_GENERIC_MALE_DEATH_23 = 3067,
	SFX_GENERIC_MALE_DEATH_24 = 3068,
	SFX_GENERIC_MALE_DEATH_25 = 3069,
	SFX_GENERIC_MALE_DEATH_26 = 3070,
	SFX_GENERIC_MALE_DEATH_27 = 3071,
	SFX_GENERIC_MALE_DEATH_28 = 3072,
	SFX_GENERIC_MALE_DEATH_29 = 3073,
	SFX_GENERIC_MALE_DEATH_30 = 3074,
	SFX_GENERIC_MALE_DEATH_31 = 3075,
	SFX_GENERIC_MALE_DEATH_32 = 3076,
	SFX_GENERIC_MALE_DEATH_33 = 3077,
	SFX_GENERIC_MALE_DEATH_34 = 3078,
	SFX_GENERIC_MALE_DEATH_35 = 3079,
	SFX_GENERIC_MALE_DEATH_36 = 3080,
	SFX_GENERIC_MALE_DEATH_37 = 3081,
	SFX_GENERIC_MALE_DEATH_38 = 3082,
	SFX_GENERIC_MALE_DEATH_39 = 3083,
	SFX_GENERIC_MALE_DEATH_40 = 3084,
	SFX_GENERIC_MALE_DEATH_41 = 3085,
	SFX_GENERIC_MALE_GRUNT_1 = 3086,
	SFX_GENERIC_MALE_GRUNT_2 = 3087,
	SFX_GENERIC_MALE_GRUNT_3 = 3088,
	SFX_GENERIC_MALE_GRUNT_4 = 3089,
	SFX_GENERIC_MALE_GRUNT_5 = 3090,
	SFX_GENERIC_MALE_GRUNT_6 = 3091,
	SFX_GENERIC_MALE_GRUNT_7 = 3092,
	SFX_GENERIC_MALE_GRUNT_8 = 3093,
	SFX_GENERIC_MALE_GRUNT_9 = 3094,
	SFX_GENERIC_MALE_GRUNT_10 = 3095,
	SFX_GENERIC_MALE_GRUNT_11 = 3096,
	SFX_GENERIC_MALE_GRUNT_12 = 3097,
	SFX_GENERIC_MALE_GRUNT_13 = 3098,
	SFX_GENERIC_MALE_GRUNT_14 = 3099,
	SFX_GENERIC_MALE_GRUNT_15 = 3100,
	SFX_GENERIC_MALE_GRUNT_16 = 3101,
	SFX_GENERIC_MALE_GRUNT_17 = 3102,
	SFX_GENERIC_MALE_GRUNT_18 = 3103,
	SFX_GENERIC_MALE_GRUNT_19 = 3104,
	SFX_GENERIC_MALE_GRUNT_20 = 3105,
	SFX_GENERIC_MALE_GRUNT_21 = 3106,
	SFX_GENERIC_MALE_GRUNT_22 = 3107,
	SFX_GENERIC_MALE_GRUNT_23 = 3108,
	SFX_GENERIC_MALE_GRUNT_24 = 3109,
	SFX_GENERIC_MALE_GRUNT_25 = 3110,
	SFX_GENERIC_MALE_GRUNT_26 = 3111,
	SFX_GENERIC_MALE_GRUNT_27 = 3112,
	SFX_GENERIC_MALE_GRUNT_28 = 3113,
	SFX_GENERIC_MALE_GRUNT_29 = 3114,
	SFX_GENERIC_MALE_GRUNT_30 = 3115,
	SFX_GENERIC_MALE_GRUNT_31 = 3116,
	SFX_GENERIC_MALE_GRUNT_32 = 3117,
	SFX_GENERIC_MALE_GRUNT_33 = 3118,
	SFX_GENERIC_MALE_GRUNT_34 = 3119,
	SFX_GENERIC_MALE_GRUNT_35 = 3120,
	SFX_GENERIC_MALE_GRUNT_36 = 3121,
	SFX_GENERIC_MALE_GRUNT_37 = 3122,
	SFX_GENERIC_MALE_GRUNT_38 = 3123,
	SFX_GENERIC_MALE_GRUNT_39 = 3124,
	SFX_GENERIC_MALE_GRUNT_40 = 3125,
	SFX_GENERIC_MALE_GRUNT_41 = 3126,
	SFX_GENERIC_MALE_PANIC_1 = 3127,
	SFX_GENERIC_MALE_PANIC_2 = 3128,
	SFX_GENERIC_MALE_PANIC_3 = 3129,
	SFX_GENERIC_MALE_PANIC_4 = 3130,
	SFX_GENERIC_MALE_PANIC_5 = 3131,
	SFX_GENERIC_MALE_PANIC_6 = 3132,
	SFX_GENERIC_MALE_PANIC_7 = 3133,
	SFX_GENERIC_MALE_PANIC_8 = 3134,
	SFX_GENERIC_MALE_PANIC_9 = 3135,
	SFX_GENERIC_MALE_PANIC_10 = 3136,
	SFX_GENERIC_MALE_PANIC_11 = 3137,
	SFX_GENERIC_MALE_PANIC_12 = 3138,
	SFX_GENERIC_MALE_PANIC_13 = 3139,
	SFX_GENERIC_MALE_PANIC_14 = 3140,
	SFX_GENERIC_MALE_PANIC_15 = 3141,
	SFX_GENERIC_MALE_PANIC_16 = 3142,
	SFX_GENERIC_MALE_PANIC_17 = 3143,
	SFX_GENERIC_MALE_PANIC_18 = 3144,
	SFX_GENERIC_MALE_PANIC_19 = 3145,
	SFX_GENERIC_MALE_PANIC_20 = 3146,
	SFX_GENERIC_MALE_PANIC_21 = 3147,
	SFX_GENERIC_MALE_PANIC_22 = 3148,
	SFX_GENERIC_MALE_PANIC_23 = 3149,
	SFX_GENERIC_MALE_PANIC_24 = 3150,
	SFX_GENERIC_MALE_PANIC_25 = 3151,
	SFX_GENERIC_MALE_PANIC_26 = 3152,
	SFX_GENERIC_MALE_PANIC_27 = 3153,
	SFX_GENERIC_MALE_PANIC_28 = 3154,
	SFX_GENERIC_MALE_PANIC_29 = 3155,
	SFX_GENERIC_MALE_PANIC_30 = 3156,
	SFX_GENERIC_MALE_PANIC_31 = 3157,
	SFX_GENERIC_MALE_PANIC_32 = 3158,
	SFX_GENERIC_MALE_PANIC_33 = 3159,
	SFX_GENERIC_MALE_PANIC_34 = 3160,
	SFX_GENERIC_MALE_PANIC_35 = 3161,

	TOTAL_AUDIO_SAMPLES = 9941,
	NO_SAMPLE,

	// shorthands
	SAMPLEBANK_START = SFX_CAR_HORN_JEEP,
	SAMPLEBANK_END = SFX_FOOTSTEP_SAND_4,
	SAMPLEBANK_MAX = SFX_FOOTSTEP_SAND_4 + 1,
	SAMPLEBANK_PED_START = SFX_FOOTSTEP_SAND_4 + 1,
	SAMPLEBANK_PED_END = 9940,
	SAMPLEBANK_PED_MAX = SAMPLEBANK_PED_END + 1,
};
