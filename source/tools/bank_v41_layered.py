# Driftfield 4.1 bank: LAYERED — two finished presets per patch, as layer 1 and layer 2.
# L(name, layer1, layer2, mode, split=, width=, morph=, balance=dB on layer 2, tags=)
# Modes: layer (both), split (key), velocity (soft -> 1, hard -> 2), pressure (per note, MPE / note
# expression), morph (lyMorph blends; automate it). Macros stay per layer (layer 1's macros on MAIN).

# ---- stacks: a bed and a texture together
L('Void and Grain Rain', 'Magnificent Void Drone', 'Aurora Granules', 'layer', balance=-4, tags=('roach', 'stack'))
L('Basalt Choir Under Ice', 'Basalt Choir', 'Frozen Lake', 'layer', balance=-3, tags=('choir', 'stack'))
L('Monolith with Monks', 'Pure Fifth Monolith', 'Monks in the Valley', 'layer', balance=-4, tags=('roach', 'ritual'))
L('Desert Saws and Wind', 'Desert Night Saws', 'Canyon at Dusk', 'layer', balance=-3, tags=('desert', 'roach'))
L('Cathedral Organ Breath', 'Overtone Cathedral', 'Breath Choir Pad', 'layer', balance=-4, tags=('sacred', 'organ'))
L('Gong Bath Over Earth Hum', 'Gong Bath', 'Earth Hum', 'layer', balance=-2, tags=('ritual', 'drone'))
L('Solina Over Rain', 'Solina Dusk Section', 'Rain on the Tin Roof', 'layer', balance=-5, tags=('strings', 'rain'))
L('Tanpura and Sympathy', 'Yaman Evening Tanpura', 'Sympathetic Viola', 'layer', balance=-5, tags=('raga', 'physical'))
L('Glass Horizon Shimmer Stack', 'Glass Horizon', 'Night Sky in Seven', 'layer', balance=-3, tags=('glass', 'corbacho'))
L('Structures and Cicadas', 'Structures of Saw', 'Cicada Temple Noon', 'layer', balance=-6, tags=('structures', 'roach'))
L('Analog Heaven with Bells', 'Analog Heaven', 'Monastery Chimes', 'layer', balance=-6, tags=('bells', 'pad'))
L('Throat Singers in the Cave', 'Throat Singers', 'Cave Mantra', 'layer', balance=-3, tags=('overtone', 'ritual'))

# ---- splits: bass or drone below, lead or pad above
L('Taurus Floor, Jupiter Sky', 'Taurus Pedal', 'Jupiter Silk', 'split', split=48, width=0, tags=('split', 'bass-and-pad'))
L('Moog Bass, OB Brass', 'Model D Thump', 'OB Poly Brass', 'split', split=48, width=0, tags=('split', 'bass-and-brass'))
L('Drone Below, Bells Above', 'Ancestor Hum', 'Celesta Dream', 'split', split=55, width=4, tags=('split', 'drone-and-bells'))
L('Sub Under Strings', 'Sub Weight Bass', 'CS Blade Strings', 'split', split=47, width=2, tags=('split', 'cinematic'))
L('Cello Below, Flute Above', 'Waveguide Cello', 'Waveguide Flute', 'split', split=60, width=0, tags=('split', 'duet', 'physical'))
L('Tanpura Drone, Sarangi Voice', 'Bhairav Dawn Tanpura', 'Bhairavi Bowed Sarangi', 'split', split=55, width=0, tags=('split', 'raga'))
L('Berlin Bass, Shimmer Top', 'Dark Berlin Bass Seq', 'Logan Shimmer Strings', 'split', split=52, width=0, tags=('split', 'berlin'))
L('Gong Floor, Kalimba Top', 'Bronze Gong Drone', 'Kalimba Box', 'split', split=50, width=6, tags=('split', 'ritual'))
L('Acid Below, Choir Above', 'Acid Ladder Squelch', 'Cathedral Choir', 'split', split=48, width=0, tags=('split',))
L('Crossfading Keyboard Horizon', 'Dark Analog Velvet', 'Ice Cathedral Harmonics', 'split', split=60, width=24, tags=('split', 'crossfade', 'wide-blend'))

# ---- velocity: soft plays one sound, hard the other
L('Soft Pad, Hard Brass', 'Juno Silver Strings', 'CS Brass Section', 'velocity', morph=0.6, width=8, tags=('velocity', 'expressive'))
L('Whisper to Choir', 'Whisper Choir', 'Cathedral Choir', 'velocity', morph=0.55, width=12, tags=('velocity', 'choir'))
L('Felt to Glass Keys', 'Analog Electric Piano', 'Glass Keys', 'velocity', morph=0.6, width=6, tags=('velocity', 'keys'))
L('Bowed or Struck Bowl', 'Glass Bowl Pair Bowed', 'Bowl Strike Shimmer', 'velocity', morph=0.65, width=4, tags=('velocity', 'physical'))
L('Pluck Then Pad', 'Plucked Gut String', 'Golden Hour Pad', 'velocity', morph=0.4, width=20, tags=('velocity', 'pluck-pad'))

# ---- pressure: each held note moves from layer 1 to layer 2 (MPE pressure / note-expression Expression)
L('Pressure Into Light', 'Dark Matter Pad', 'Glass Cathedral', 'pressure', morph=0.45, width=24, tags=('pressure', 'mpe-showcase'))
L('Pressure Choir Bloom', 'Monks Below', 'Angel Hair', 'pressure', morph=0.4, width=20, tags=('pressure', 'choir', 'mpe-showcase'))
L('Pressure Drone to Scream', 'Iron Tide', 'MS-20 Howl Drone', 'pressure', morph=0.55, width=18, tags=('pressure', 'drone', 'mpe-showcase'))
L('Pressure Bow to Tanpura', 'Bowed String Solo', 'Bowed Tanpura', 'pressure', morph=0.5, width=20, tags=('pressure', 'physical', 'mpe-showcase'))
L('Pressure Strings to Brass', 'Eminent Organ Strings', 'Blade Fanfare', 'pressure', morph=0.5, width=22, tags=('pressure', 'cinematic', 'mpe-showcase'))
L('Pressure Breath Into Sweep', 'Breathing Glacier', 'Feedback Phaser Storm', 'pressure', morph=0.5, width=24, tags=('pressure', 'sweep', 'mpe-showcase'))

# ---- morph: one knob between two worlds (automate Layer Morph)
L('Morph Void to Horizon', 'Magnificent Pure Void', 'Corbacho Pure Horizon', 'morph', morph=0.0, tags=('morph', 'automate-morph', 'just-intonation'))
L('Morph Earth to Sky', 'Deep Hollow Earth', 'Inner Sky', 'morph', morph=0.0, tags=('morph', 'automate-morph'))
L('Morph Analog to Spectral', 'OB Eight Voice Wash', 'Harmonic Searchlight', 'morph', morph=0.0, tags=('morph', 'automate-morph'))
L('Morph Rain to Fire', 'Rain on the Tin Roof', 'Distant Village Fire', 'morph', morph=0.0, tags=('morph', 'automate-morph', 'soundscape'))
L('Morph Equal to Just', 'Fifths of Eternity', 'Just Fifths Drone', 'morph', morph=0.0, tags=('morph', 'automate-morph', 'just-intonation'))
L('Morph Choir to Glass', 'Formant Choir Pad', 'Glass Choir Pad', 'morph', morph=0.0, tags=('morph', 'automate-morph'))
L('Morph Bowl to Gong', 'Singing Bowl Choir', 'Bronze Gong', 'morph', morph=0.0, tags=('morph', 'automate-morph'))
