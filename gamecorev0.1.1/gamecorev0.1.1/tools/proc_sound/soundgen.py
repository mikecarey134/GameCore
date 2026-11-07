
#!/usr/bin/env python3
"""
Procedural Game Audio Generator
================================

Procedurally generate game sounds and export mono 16-bit PCM WAV files.

Requires:
    python3 -m pip install numpy

Examples:
    python3 soundgen.py --list
    python3 soundgen.py pistol_shot -o pistol.wav
    python3 soundgen.py zombie_growl -o growl.wav --duration 3
    python3 soundgen.py forest_ambience -o forest.wav --duration 30
    python3 soundgen.py horror_loop -o horror.wav --duration 20
    python3 soundgen.py --batch ambience --output-dir assets/sounds
    python3 soundgen.py --batch weapons --output-dir assets/sounds
    python3 soundgen.py --random 10 --output-dir assets/variants
"""

import argparse
import os
import re
import wave

import numpy as np


# ---------------------------------------------------------------------
# COMPLETE ASSET CATALOG
# ---------------------------------------------------------------------

ASSET_CATALOG = {
    "weapons": """
    pistol_shot revolver_shot rifle_shot shotgun_blast smg_burst sniper_shot
    machinegun_burst silenced_shot dry_fire bullet_chamber magazine_insert
    magazine_remove weapon_reload bolt_action slide_rack shell_eject
    shell_casing_drop bullet_ricochet bullet_whiz bullet_impact_wood
    bullet_impact_metal bullet_impact_concrete muzzle_blast distant_gunshot
    gunshot_echo weapon_draw weapon_holster weapon_inspect weapon_jam
    weapon_clear_jam grenade_pin grenade_throw grenade_bounce grenade_explosion
    pipe_bomb_fuse pipe_bomb_explosion molotov_ignite molotov_throw molotov_impact
    fuel_explosion gas_can_explosion landmine_trigger blast_wave
    """,
    "melee": """
    punch_swing punch_impact kick_impact fistfight_combo crowbar_swing
    crowbar_hit machete_swing machete_hit axe_swing axe_chop bat_swing
    bat_hit pipe_swing pipe_hit knife_swing knife_stab blade_scrape shield_block
    parry armor_impact bone_crack body_fall body_drag heavy_body_drop
    combat_grunt pain_groan critical_hit finisher_impact
    """,
    "destruction": """
    debris_fall rubble_collapse wall_break concrete_crumble metal_debris
    wood_splinter glass_shatter_large glass_smash door_smash door_break
    door_kick door_breach barricade_hit wall_punch window_break roof_collapse
    """,
    "zombies": """
    zombie_idle zombie_breathing zombie_growl zombie_groan zombie_snarl
    zombie_scream zombie_roar zombie_whisper zombie_chuckle zombie_cough
    zombie_gasp zombie_howl zombie_bite zombie_bite_miss zombie_chewing
    zombie_feeding zombie_death zombie_wakeup zombie_alert zombie_searching
    zombie_lost_target zombie_attack_windup zombie_horde zombie_wall_scrape
    zombie_walk zombie_shuffle zombie_run zombie_crawl zombie_limp zombie_climb
    zombie_drop zombie_lunge zombie_grab zombie_miss zombie_impact
    zombie_knockdown zombie_rise zombie_dismember zombie_head_crush
    zombie_body_hit zombie_body_drag zombie_flesh_tear zombie_bone_break
    zombie_horde_movement mutant_idle mutant_roar mutant_attack mutant_footstep
    crawler_hiss crawler_scrape stalker_whisper brute_roar brute_charge
    swarm_chitter parasite_screech creature_death monster_heartbeat
    monster_proximity_warning
    """,
    "movement": """
    footstep footstep_concrete footstep_wood footstep_gravel footstep_dirt
    footstep_mud footstep_grass footstep_sand footstep_snow footstep_metal
    footstep_glass footstep_water footstep_puddle footstep_stairs footstep_roof
    footstep_debris footstep_crouch footstep_sprint footstep_sneak
    footstep_heavy_boot footstep_barefoot jump_takeoff jump_land fall_short
    fall_long slide_ground roll_land ladder_climb ladder_dismount
    """,
    "player": """
    player_idle_breath player_heavy_breath player_sprint_breath player_exhausted
    player_pain player_grunt player_cough player_sneeze player_choke player_gasp
    player_heartbeat_calm player_heartbeat_stressed player_injury player_heal
    player_bandage player_drink player_eat player_swallow player_vomit
    player_drown player_death player_revive cloth_rustle backpack_rustle
    gear_rattle armor_rattle keys_jingle buckle_click zipper_open zipper_close
    velcro_open velcro_close gloves_adjust equipment_drop backpack_open
    backpack_close inventory_item_pickup
    """,
    "doors": """
    door_open door_close door_slam wood_door_open wood_door_close wood_door_slam
    metal_door_open metal_door_close metal_door_slam creaky_door_open
    creaky_door_close heavy_door_open heavy_door_close garage_door_open
    garage_door_close sliding_door_open sliding_door_close car_door_open
    car_door_close hatch_open hatch_close cell_door_open cell_door_close
    elevator_door_open elevator_door_close door_barricade door_knock
    door_knock_single door_knock_urgent door_rattle door_creak_loop lock_click
    lock_jam lock_pick_start lock_pick_progress lock_pick_success lock_pick_fail
    key_insert key_turn key_remove padlock_unlock padlock_lock deadbolt_lock
    deadbolt_unlock combination_lock keypad_beep keypad_denied keypad_accepted
    electronic_lock_open electronic_lock_denied security_gate_open
    security_gate_close window_open window_close window_crawl shutter_open
    shutter_close board_nail board_remove ceiling_creak floor_creak pipe_rattle
    vent_rattle
    """,
    "fire": """
    fire fire_loop fire_crackle fire_roar fire_ignite fire_extinguish fire_burst
    wood_burning building_fire flame_whoosh torch_loop lighter_flick match_strike
    match_burn gas_leak gas_ignition
    """,
    "electricity": """
    electric_hum transformer_buzz power_surge electric_arc electric_spark
    short_circuit electrical_pop fuse_blow generator_fail power_restore
    power_down electric_fence electric_shock neon_flicker light_flicker
    """,
    "weather": """
    wind wind_gust wind_howl wind_tunnel rain rain_light rain_heavy rain_roof
    rain_window rain_gutter thunder thunder_distant thunder_crack thunder_roll
    hail snowstorm blizzard tornado_wind flood_rush dripping_water
    water_drip_cave leaking_pipe water_splash underwater_ambience toxic_gas
    radiation_geiger chemical_bubbling steam_release pressure_valve
    industrial_leak alarm_siren warning_beacon oxygen_warning contamination_warning
    """,
    "ambience": """
    city_ambience abandoned_city empty_street suburban_ambience forest_ambience
    dead_forest swamp_ambience desert_ambience mountain_wind coastal_wind
    graveyard_ambience farm_ambience wasteland_ambience night_ambience
    dawn_ambience dusk_ambience storm_ambience distant_horde distant_fire
    distant_explosions empty_room_tone abandoned_house abandoned_hospital
    hospital_hallway school_ambience office_ambience warehouse_ambience
    factory_ambience sewer_ambience underground_tunnel bunker_ambience
    military_base prison_ambience basement_ambience attic_ambience
    subway_station parking_garage server_room power_station laboratory_ambience
    ventilation_loop fluorescent_hum distant_pipe_bang building_settling
    room_reverb_tail horror_drone dark_ambient_loop tension_loop
    low_frequency_rumble ghost_whisper ghost_wail distant_scream unsettling_wind
    metallic_groan reversed_whoosh rising_tension horror_stinger suspense_pulse
    heartbeat_loop subsonic_rumble dissonant_chime eerie_chorus
    abandoned_radio_static paranormal_noise creaking_structure ambient_transition
    """,
    "vehicles": """
    car_engine_start car_engine_idle car_engine_rev car_engine_stall car_engine_stop
    car_driveby car_tire_screech car_crash car_door_lock truck_engine truck_idle
    truck_driveby motorcycle_start motorcycle_rev motorcycle_pass bus_engine
    diesel_engine electric_vehicle_hum vehicle_alarm vehicle_horn vehicle_collision
    tire_burst engine_overheat generator_start generator_idle generator_stop
    compressor_loop pump_loop industrial_fan conveyor_belt hydraulic_press
    winch_start winch_loop crane_motor motor_start motor_stop gear_grind
    metal_scrape machine_clunk machine_failure rotating_bearing steam_engine
    mechanical_drone helicopter_loop helicopter_pass aircraft_pass distant_jet
    radio_beacon distant_impact distant_artillery emergency_siren
    civil_defense_siren
    """,
    "inventory": """
    item_pickup item_drop item_place item_rotate inventory_open inventory_close
    inventory_move inventory_error inventory_equip inventory_unequip item_stack
    item_split item_combine craft_start craft_progress craft_complete craft_fail
    toolbox_open toolbox_close hammer_hit saw_wood wrench_turn screwdriver_turn
    metal_filing rope_pull rope_snap chain_pull chain_drop can_open bottle_open
    bottle_break water_bottle_drink food_wrapper can_rattle empty_container
    fuel_pour matchbox_rattle battery_insert battery_remove flashlight_click
    flashlight_toggle radio_tune radio_static radio_transmission
    """,
    "ui": """
    ui_confirm ui_cancel ui_hover ui_select ui_back ui_error ui_warning ui_success
    ui_notification ui_menu_open ui_menu_close ui_tab_change ui_slider
    ui_toggle_on ui_toggle_off ui_text_tick ui_loading ui_save ui_load ui_new_game
    ui_game_over ui_pause ui_unpause quest_start quest_complete quest_failed
    objective_updated achievement_unlock level_up checkpoint_reached secret_found
    locked_message low_health_warning stamina_depleted low_ammo_warning
    out_of_ammo reload_prompt interaction_prompt damage_taken critical_health
    death_transition respawn safe_zone_enter safe_zone_exit stealth_detected
    enemy_alerted enemy_lost wave_start wave_complete boss_intro boss_defeated
    survival_day_start survival_night_start
    """,
    "scifi": """
    teleport_in teleport_out energy_charge energy_release energy_blast plasma_shot
    laser_charge laser_beam force_field shield_activate shield_break scanner_sweep
    radar_ping sonar_ping computer_boot computer_shutdown data_transfer glitch_burst
    digital_corruption signal_loss signal_restore portal_open portal_loop
    portal_close time_warp gravity_shift magnetic_pulse power_core reactor_hum
    reactor_failure alien_chatter alien_screech psychic_pulse spectral_rise
    spectral_fade
    """,
    "water": """
    water_drop water_drip_loop puddle_splash shallow_water_step deep_water_step
    water_slosh water_swim water_surface waterfall_loop river_loop stream_loop
    drain_flow flood_water underwater_bubbles underwater_breathing underwater_muffle
    underwater_impact submerged_explosion pipe_water_flow water_tank_rumble
    """,
    "wildlife": """
    dog_bark dog_growl dog_whine dog_whimper dog_footsteps cat_hiss cat_meow
    bird_chirp crow_call owl_call raven_call wolf_howl wolf_growl insect_chirp
    cricket_loop frog_call bat_flutter rat_squeak snake_hiss animal_scurry
    animal_warning animal_attack animal_death
    """,
    "music": """
    horror_loop dark_ambient_loop tension_loop combat_loop exploration_loop
    safe_zone_loop menu_theme game_over_theme victory_sting danger_sting
    industrial_loop radio_music_loop chiptune_loop synth_bass_loop percussion_loop
    ambient_pad_loop dissonant_chord rising_tone descending_tone music_transition
    music_hit low_drone sci_fi_loop alarm_rhythm_loop pulse_loop drone
    """,
    "procedural": """
    footstep_random gunshot_random zombie_voice_random door_creak_random
    debris_random wind_gust_random thunder_random impact_random explosion_random
    ambient_event_random interior_reverb distance_filter radio_filter underwater_filter
    pitch_randomizer loop_crossfade sound_layer_mix batch_generate material_impact
    """,
}

PRESETS = tuple(dict.fromkeys(
    name
    for names in ASSET_CATALOG.values()
    for name in names.split()
))


def safe_name(name):
    """Convert an asset name into a safe filename component."""
    return re.sub(r"[^a-zA-Z0-9_.-]+", "_", name.strip().replace(" ", "_"))


# ---------------------------------------------------------------------
# DSP UTILITIES
# ---------------------------------------------------------------------

def normalize(x):
    x = np.asarray(x, dtype=np.float64)
    peak = np.max(np.abs(x)) if x.size else 0.0
    return x / peak if peak > 1e-12 else x


def lowpass(x, alpha=0.08):
    x = np.asarray(x, dtype=np.float64)
    if not len(x):
        return x.copy()
    y = np.empty_like(x)
    y[0] = x[0]
    for i in range(1, len(x)):
        y[i] = y[i - 1] + alpha * (x[i] - y[i - 1])
    return y


def highpass(x, alpha=0.96):
    x = np.asarray(x, dtype=np.float64)
    if not len(x):
        return x.copy()
    y = np.empty_like(x)
    y[0] = x[0]
    for i in range(1, len(x)):
        y[i] = alpha * (y[i - 1] + x[i] - x[i - 1])
    return y


def oscillator(freq, t, kind="sine", phase=0.0, sr=44100):
    freq = np.asarray(freq, dtype=np.float64)
    if freq.ndim == 0:
        angle = 2 * np.pi * float(freq) * t + phase
    else:
        angle = 2 * np.pi * np.cumsum(freq) / sr + phase

    if kind == "square":
        return np.sign(np.sin(angle))
    if kind == "saw":
        cycles = angle / (2 * np.pi)
        return 2 * (cycles - np.floor(cycles + 0.5))
    if kind == "triangle":
        cycles = angle / (2 * np.pi)
        return 2 * np.abs(2 * (cycles - np.floor(cycles + 0.5))) - 1
    return np.sin(angle)


def adsr(t, duration, attack=0.02, decay=0.1, sustain=0.65,
         release=0.15):
    attack = max(1e-5, attack)
    decay = max(0.0, decay)
    release = max(1e-5, min(release, duration))
    env = np.ones_like(t)

    env *= np.clip(t / attack, 0.0, 1.0)

    mask = (t >= attack) & (t < attack + decay)
    if decay > 0:
        env[mask] *= 1.0 - (1.0 - sustain) * (
            (t[mask] - attack) / decay
        )

    start = max(0.0, duration - release)
    mask = t >= start
    env[mask] *= np.clip((duration - t[mask]) / release, 0.0, 1.0)
    return env


def write_wav(path, samples, sr):
    os.makedirs(os.path.dirname(os.path.abspath(path)), exist_ok=True)
    samples = np.nan_to_num(samples, nan=0.0, posinf=0.0, neginf=0.0)
    pcm = (np.clip(samples, -1, 1) * 32767).astype("<i2")

    with wave.open(path, "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(sr)
        f.writeframes(pcm.tobytes())


# ---------------------------------------------------------------------
# SYNTHESIS ENGINE
# ---------------------------------------------------------------------

def synthesize(name, duration=1.0, sr=44100, frequency=220.0,
               volume=1.0, seed=None):
    if name not in PRESETS:
        raise ValueError(f"Unknown preset: {name}")

    if duration <= 0 or sr < 8000 or frequency <= 0:
        raise ValueError("Duration, sample rate, and frequency must be positive.")
    if not 0 <= volume <= 1:
        raise ValueError("Volume must be between 0 and 1.")

    rng = np.random.default_rng(seed)
    n = max(1, int(sr * duration))
    t = np.arange(n) / sr
    w = rng.uniform(-1, 1, n)
    fade = adsr(t, duration)
    f = frequency

    def osc(freq, kind="sine"):
        return oscillator(freq, t, kind, sr=sr)

    # Short, sharp impact with a low-frequency body.
    def impact(low=0.3, tail=18.0, brightness=0.8):
        return (
            lowpass(w, 0.05) * np.exp(-tail * t) * low
            + highpass(w, 0.85) * np.exp(-tail * 1.4 * t) * brightness
            + osc(max(25, f * 0.45)) * np.exp(-tail * 0.7 * t) * 0.6
        )

    # Broad category classifiers for catalog entries.
    is_ambience = any(x in name for x in (
        "ambience", "ambient", "loop", "wind", "rain", "river",
        "stream", "waterfall", "cricket", "insect", "forest",
        "swamp", "desert", "city", "street", "room_tone",
        "ventilation", "fluorescent", "industrial_fan",
    ))
    is_voice = any(x in name for x in (
        "zombie", "mutant", "crawler", "stalker", "brute", "swarm",
        "parasite", "creature", "monster", "ghost", "scream", "whisper",
        "growl", "roar", "groan", "snarl", "howl", "chitter", "alien",
        "wolf", "dog_", "cat_", "owl_", "crow_", "raven_", "frog_",
        "snake_", "player_", "grunting", "grunt", "gasp", "cough",
        "sneeze", "choke", "breath", "pain", "scream",
    ))
    is_door = any(x in name for x in (
        "door", "lock", "deadbolt", "padlock", "key_", "keypad",
        "shutter", "hatch", "gate", "window", "barricade", "board_",
    ))
    is_machine = any(x in name for x in (
        "engine", "generator", "motor", "machine", "pump", "compressor",
        "conveyor", "hydraulic", "winch", "crane", "bearing", "gear",
        "transformer", "reactor", "industrial", "helicopter", "aircraft",
        "truck_", "car_", "motorcycle", "bus_", "diesel",
    ))
    is_ui = name.startswith("ui_") or any(x in name for x in (
        "quest_", "achievement_", "level_up", "checkpoint_", "objective_",
        "notification", "prompt", "wave_start", "wave_complete",
        "boss_intro", "boss_defeated", "safe_zone_", "secret_found",
    ))
    is_music = any(x in name for x in (
        "music", "theme", "tension_loop", "combat_loop",
        "exploration_loop", "safe_zone_loop", "horror_loop",
        "dark_ambient", "chiptune", "synth_bass", "percussion_loop",
        "ambient_pad", "pulse_loop", "drone", "low_drone",
        "dissonant_chord", "rising_tone", "descending_tone",
        "victory_sting", "danger_sting", "stinger",
    ))

    # Special signature effects.
    if name == "tone":
        sound = osc(f) * fade

    elif name in ("laser", "laser_beam", "laser_charge", "plasma_shot",
                  "energy_blast", "energy_release"):
        sweep = f * (1.7 - 1.4 * t / duration)
        sound = (
            oscillator(sweep, t, "saw", sr=sr) * 0.5
            + osc(np.maximum(25, sweep * 0.5)) * 0.5
        ) * np.exp(-t * 2.5)

    elif name in ("pickup", "achievement_unlock", "quest_complete",
                  "craft_complete", "ui_success", "level_up",
                  "checkpoint_reached"):
        notes = np.select(
            [t < duration / 3, t < 2 * duration / 3],
            [f, f * 1.25],
            default=f * 1.5,
        )
        sound = oscillator(notes, t, "sine", sr=sr) * fade

    elif name in ("gunshot", "pistol_shot", "revolver_shot", "rifle_shot",
                  "shotgun_blast", "sniper_shot", "smg_burst",
                  "machinegun_burst", "silenced_shot", "distant_gunshot",
                  "muzzle_blast"):
        sound = (
            highpass(w, 0.85) * np.exp(-t * 48) * 0.9
            + lowpass(w, 0.07) * np.exp(-t * 18) * 0.7
            + osc(max(25, f * 0.3)) * np.exp(-t * 12) * 0.5
        )
        if "silenced" in name:
            sound = lowpass(sound, 0.13) * np.exp(-t * 3)

    elif name in ("explosion", "grenade_explosion", "pipe_bomb_explosion",
                  "fuel_explosion", "gas_can_explosion", "landmine_trigger",
                  "submerged_explosion", "blast_wave"):
        sound = (
            lowpass(w, 0.025) * np.exp(-t * 3.8)
            + highpass(w, 0.8) * np.exp(-t * 12) * 0.5
            + osc(max(25, f * 0.2)) * np.exp(-t * 2.5) * 0.8
        )

    elif name in ("fire", "fire_loop", "fire_crackle", "wood_burning",
                  "building_fire", "torch_loop"):
        crackle = w * (rng.random(n) < 0.035)
        flicker = 0.65 + 0.35 * np.sin(2 * np.pi * 0.8 * t)
        sound = lowpass(w, 0.018) * flicker + lowpass(crackle, 0.3) * 2

    elif name in ("fire_burst", "flame_whoosh", "molotov_impact"):
        sound = (
            lowpass(w, 0.04) * np.exp(-t * 3)
            + highpass(w, 0.45) * np.exp(-t * 8) * 0.6
        )

    elif name in ("thunder", "thunder_crack", "thunder_roll",
                  "thunder_distant", "distant_explosions",
                  "distant_artillery"):
        sound = (
            lowpass(w, 0.012) * np.exp(-t * 1.8)
            + highpass(w, 0.7) * np.exp(-t * 4) * 0.5
            + osc(max(25, f * 0.2)) * np.exp(-t * 1.5)
        )

    elif name in ("drone", "low_drone", "horror_drone",
                  "horror_loop", "dark_ambient_loop",
                  "subsonic_rumble", "low_frequency_rumble"):
        mod = np.sin(2 * np.pi * 0.12 * t)
        sound = (
            osc(np.maximum(25, f * (1 + 0.035 * mod))) * 0.7
            + osc(max(25, f * 0.5) * (1 - 0.025 * mod)) * 0.6
            + lowpass(w, 0.025) * 0.25
        ) * (0.7 + 0.3 * np.sin(2 * np.pi * 0.18 * t))

    elif name in ("chiptune_loop", "menu_theme", "game_over_theme",
                  "combat_loop", "exploration_loop", "safe_zone_loop",
                  "synth_bass_loop", "industrial_loop", "sci_fi_loop",
                  "radio_music_loop"):
        scale = np.array([1.0, 1.25, 1.5, 2.0, 1.5, 1.25, 1.125, 1.5])
        idx = np.floor(t / 0.25).astype(int) % len(scale)
        freq_seq = f * scale[idx]
        sound = (
            oscillator(freq_seq, t, "square", sr=sr) * 0.65
            + oscillator(freq_seq * 0.5, t, "square", sr=sr) * 0.25
        )
        sound *= 0.55 + 0.45 * np.sin(2 * np.pi * 2 * t) ** 2

    elif name in ("door_knock", "door_knock_single", "door_knock_urgent"):
        sound = np.zeros(n)
        interval = 0.18 if "urgent" in name else 0.3
        count = 5 if "urgent" in name else (1 if "single" in name else 3)

        for k in range(count):
            start = int(k * interval * sr)
            if start >= n:
                break
            size = min(int(0.12 * sr), n - start)
            local = np.arange(size) / sr
            hit = (
                lowpass(w[start:start + size], 0.12)
                + np.sin(2 * np.pi * f * 0.45 * local)
            ) * np.exp(-local * 28)
            sound[start:start + size] += hit

    elif name in ("heartbeat", "heartbeat_loop", "player_heartbeat_calm",
                  "player_heartbeat_stressed", "monster_heartbeat"):
        sound = np.zeros(n)
        interval = 0.42 if "stressed" in name else 0.8

        for start_time in np.arange(0, duration, interval):
            start = int(start_time * sr)
            size = min(int(0.2 * sr), n - start)
            if size <= 0:
                break
            local = np.arange(size) / sr
            sound[start:start + size] += (
                osc(max(25, f * 0.35))[:size]
                if len(osc(max(25, f * 0.35))) >= size
                else np.sin(2 * np.pi * f * 0.35 * local)
            ) * np.exp(-local * 18)

    elif name in ("pulse_loop", "suspense_pulse", "alarm_rhythm_loop",
                  "tension_loop", "percussion_loop"):
        pulse = np.maximum(0, np.sin(2 * np.pi * 2 * t)) ** 5
        sound = (osc(f) + 0.5 * osc(f * 1.5)) * pulse

    elif name in ("door_open", "wood_door_open", "metal_door_open",
                  "creaky_door_open", "heavy_door_open", "garage_door_open",
                  "sliding_door_open", "hatch_open", "cell_door_open",
                  "elevator_door_open"):
        sweep = f * (0.55 + 0.7 * t / duration)
        sound = (
            oscillator(sweep, t, "saw", sr=sr) * 0.45
            + lowpass(w, 0.04) * 0.45
        ) * (0.5 + 0.5 * np.sin(2 * np.pi * 4 * t))

    elif name in ("door_close", "door_slam", "wood_door_close",
                  "wood_door_slam", "metal_door_close", "metal_door_slam",
                  "heavy_door_close", "garage_door_close", "cell_door_close",
                  "hatch_close", "elevator_door_close"):
        sound = impact(0.8, 17, 0.55)

    elif name in ("door_lock", "lock_click", "deadbolt_lock",
                  "deadbolt_unlock", "padlock_lock", "padlock_unlock",
                  "key_turn", "key_insert", "key_remove"):
        sound = (
            highpass(w, 0.82) * np.exp(-t * 45)
            + osc(f * 0.65) * np.exp(-t * 24) * 0.6
        )

    elif name in ("glass_smash", "glass_shatter_large", "window_break",
                  "bottle_break"):
        sound = (
            highpass(w, 0.92) * np.exp(-t * 9)
            + (osc(f * 2.2) + 0.5 * osc(f * 3.1)) * np.exp(-t * 6) * 0.4
        )

    elif name in ("metal_hit", "hammer_hit", "pipe_hit", "crowbar_hit",
                  "axe_chop", "bat_hit", "bullet_impact_metal"):
        sound = (
            (osc(f) + 0.6 * osc(f * 1.47) + 0.3 * osc(f * 2.4))
            * np.exp(-t * 4)
            + highpass(w, 0.85) * np.exp(-t * 35) * 0.5
        )

    elif name in ("wood_hit", "punch_impact", "kick_impact",
                  "bullet_impact_wood", "bullet_impact_concrete",
                  "body_fall", "heavy_body_drop", "wood_splinter"):
        sound = impact(0.8, 22, 0.45)

    elif name in ("rain", "rain_light", "rain_heavy", "rain_roof",
                  "rain_window", "rain_gutter"):
        density = 0.07 if "light" in name else 0.2
        drops = w * (rng.random(n) < density)
        sound = lowpass(drops, 0.45) * 1.5 + highpass(w, 0.15) * 0.25

    elif name in ("forest_ambience", "dead_forest", "night_ambience",
                  "cricket_loop", "insect_chirp", "bird_chirp",
                  "crow_call", "owl_call", "raven_call"):
        insects = osc(f * 2.4) * (0.5 + 0.5 * np.sin(2 * np.pi * 0.25 * t))
        sound = (
            lowpass(w, 0.012) * 0.45
            + insects * 0.25
            + highpass(w, 0.2) * 0.05
        )

    elif name in ("wind", "wind_gust", "wind_howl", "wind_tunnel",
                  "mountain_wind", "coastal_wind", "unsettling_wind"):
        swell = 0.4 + 0.6 * np.sin(2 * np.pi * 0.12 * t) ** 2
        sound = (lowpass(w, 0.012) * 1.5 + highpass(w, 0.2) * 0.15) * swell

    elif name in ("zombie", "zombie_growl", "zombie_groan", "zombie_roar",
                  "zombie_scream", "mutant_roar", "crawler_hiss",
                  "brute_roar", "ghost_wail", "alien_screech"):
        mod = np.sin(2 * np.pi * 3 * t)
        pitch = np.maximum(25, f * (0.5 + 0.25 * mod))
        sound = (
            osc(pitch) * 0.7
            + lowpass(w, 0.12) * 0.65
            + highpass(w, 0.7) * 0.12
        ) * fade

    elif name in ("footstep", "footstep_concrete", "footstep_wood",
                  "footstep_gravel", "footstep_dirt", "footstep_mud",
                  "footstep_grass", "footstep_sand", "footstep_snow",
                  "footstep_metal", "footstep_glass", "footstep_water",
                  "footstep_puddle", "footstep_stairs", "footstep_roof",
                  "footstep_debris", "footstep_sprint", "footstep_sneak",
                  "footstep_crouch", "footstep_heavy_boot", "footstep_barefoot"):
        brightness = 0.9 if "gravel" in name or "glass" in name else 0.4
        sound = impact(0.8, 28, brightness)

    elif name in ("engine", "car_engine_idle", "truck_idle", "diesel_engine",
                  "generator_idle", "motor_start", "motor_stop", "reactor_hum",
                  "electric_hum", "transformer_buzz", "fluorescent_hum"):
        wobble = 1 + 0.025 * np.sin(2 * np.pi * 0.6 * t)
        sound = (
            oscillator(f * wobble, t, "saw", sr=sr) * 0.3
            + osc(f * 0.5) * 0.45
            + lowpass(w, 0.03) * 0.2
        )

    elif is_ui:
        pitch = f * (1.0 + 0.5 * t / duration)
        sound = (
            oscillator(pitch, t, "sine", sr=sr) * 0.75
            + oscillator(pitch * 2, t, "sine", sr=sr) * 0.15
        ) * adsr(t, duration, attack=0.003, decay=0.04, sustain=0.35, release=min(0.12, duration))

    elif is_voice:
        wobble = np.sin(2 * np.pi * 4 * t)
        pitch = np.maximum(25, f * (0.75 + 0.12 * wobble))
        sound = (
            osc(pitch) * 0.65
            + lowpass(w, 0.12) * 0.55
            + highpass(w, 0.65) * 0.12
        ) * adsr(t, duration, attack=0.05, decay=0.08, sustain=0.5, release=min(0.2, duration))

    elif is_door:
        sound = (
            lowpass(w, 0.08) * np.exp(-t * 15) * 0.75
            + highpass(w, 0.8) * np.exp(-t * 35) * 0.35
            + osc(f * (0.7 + 0.1 * np.sin(2 * np.pi * 3 * t)))
            * np.exp(-t * 10) * 0.3
        )

    elif is_machine:
        wobble = 1 + 0.035 * np.sin(2 * np.pi * 0.4 * t)
        sound = (
            oscillator(f * wobble, t, "saw", sr=sr) * 0.35
            + osc(f * 0.5) * 0.4
            + lowpass(w, 0.04) * 0.25
        )

    elif is_music:
        notes = np.array([1.0, 1.25, 1.5, 1.75, 2.0, 1.5, 1.25, 1.125])
        idx = np.floor(t / 0.3).astype(int) % len(notes)
        seq = f * notes[idx]
        sound = (
            oscillator(seq, t, "triangle", sr=sr) * 0.6
            + oscillator(seq * 0.5, t, "sine", sr=sr) * 0.35
        )

    elif is_ambience:
        swell = 0.65 + 0.35 * np.sin(2 * np.pi * 0.1 * t)
        sound = (
            lowpass(w, 0.018) * 0.85
            + highpass(w, 0.18) * 0.12
            + osc(f * 0.5) * 0.15
        ) * swell

    elif any(x in name for x in (
        "impact", "hit", "crash", "smash", "break", "fall", "drop",
        "explosion", "blast", "collision", "slam", "chop", "attack",
        "damage", "crack", "bang", "burst", "snap", "break",
    )):
        sound = impact(0.75, 18, 0.75)

    else:
        # Generic procedural effect for any remaining catalog asset.
        # The name is hashed into a repeatable spectral character.
        code = sum(ord(c) for c in name)
        kind = ("sine", "triangle", "saw", "square")[code % 4]
        pitch = f * (0.65 + (code % 11) / 10)
        mod = 0.5 + 0.5 * np.sin(2 * np.pi * (0.2 + code % 7 * 0.08) * t)

        sound = (
            oscillator(pitch * (1 + 0.025 * mod), t, kind, sr=sr) * 0.55
            + lowpass(w, 0.04) * 0.35
            + highpass(w, 0.8) * 0.1
        ) * adsr(t, duration, attack=0.01, decay=0.08, sustain=0.5, release=min(0.2, duration))

    sound = normalize(np.nan_to_num(sound))
    return np.clip(sound * volume, -1.0, 1.0)


# ---------------------------------------------------------------------
# COMMAND LINE
# ---------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Procedural game sound generator and WAV exporter."
    )
    parser.add_argument("preset", nargs="?", choices=PRESETS,
                        help="Name of one asset to generate.")
    parser.add_argument("-o", "--output", help="Output WAV filename for one preset.")
    parser.add_argument("-d", "--duration", type=float, default=1.0)
    parser.add_argument("-s", "--sample-rate", type=int, default=44100)
    parser.add_argument("-f", "--frequency", type=float, default=220.0)
    parser.add_argument("-v", "--volume", type=float, default=1.0)
    parser.add_argument("--seed", type=int, default=None,
                        help="Base random seed. Same seed/settings reproduce the same WAVs.")
    parser.add_argument("--list", action="store_true", help="List categories and presets.")
    parser.add_argument("--all", action="store_true", help="Generate every unique catalog asset.")
    parser.add_argument("--batch", choices=tuple(ASSET_CATALOG),
                        help="Generate every asset in one category.")
    parser.add_argument("--random", type=int, metavar="COUNT",
                        help="Choose and generate COUNT random catalog assets.")
    parser.add_argument("--output-dir", default="generated_sounds",
                        help="Output directory for --all, --batch, and --random.")
    args = parser.parse_args()

    if args.list:
        for category, raw_names in ASSET_CATALOG.items():
            names = raw_names.split()
            print(f"\n[{category}] ({len(names)} assets)")
            print(" ".join(names))
        print(f"\nTotal unique assets: {len(PRESETS)}")
        return 0

    if args.duration <= 0:
        parser.error("--duration must be positive.")
    if args.sample_rate < 8000:
        parser.error("--sample-rate must be at least 8000.")
    if args.frequency <= 0:
        parser.error("--frequency must be positive.")
    if not 0 <= args.volume <= 1:
        parser.error("--volume must be between 0 and 1.")

    modes = sum(bool(x) for x in (args.all, args.batch, args.random is not None, args.preset))
    if modes != 1:
        parser.error("Choose exactly one of a preset, --all, --batch CATEGORY, or --random COUNT.")
    if args.random is not None and args.random < 1:
        parser.error("--random must be at least 1.")

    def generate(name, output, seed):
        audio = synthesize(
            name,
            duration=args.duration,
            sr=args.sample_rate,
            frequency=args.frequency,
            volume=args.volume,
            seed=seed,
        )
        write_wav(output, audio, args.sample_rate)
        print(f"Created {output} (seed={seed})")

    if args.preset:
        output = args.output or (safe_name(args.preset) + ".wav")
        generate(args.preset, output, args.seed)
        return 0

    if args.all:
        names = sorted(set(PRESETS))
    elif args.batch:
        names = sorted(set(ASSET_CATALOG[args.batch].split()))
    else:
        catalog = sorted(set(PRESETS))
        selection_rng = np.random.default_rng(args.seed)
        count = min(args.random, len(catalog))
        indices = selection_rng.choice(len(catalog), size=count, replace=False)
        names = [catalog[int(i)] for i in indices]

    os.makedirs(args.output_dir, exist_ok=True)
    failures = 0
    for index, name in enumerate(names):
        # Each asset gets a distinct, deterministic seed derived from the base seed.
        asset_seed = None if args.seed is None else args.seed + index
        output = os.path.join(args.output_dir, safe_name(name) + ".wav")
        try:
            generate(name, output, asset_seed)
        except Exception as exc:
            failures += 1
            print(f"ERROR generating {name}: {exc}")

    print(f"\nFinished: {len(names) - failures}/{len(names)} assets generated in {args.output_dir}")
    if args.seed is not None:
        print(f"Base seed: {args.seed}; per-asset seeds are base seed + sorted/selected index.")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
