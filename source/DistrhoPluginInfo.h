#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_BRAND   "Sydalesis"
#define DISTRHO_PLUGIN_NAME    "Sydalesis"                // 5.1.2: renamed (new plug-in, new IDs)
#define DISTRHO_PLUGIN_URI     "urn:sydalesis:sydalesis"
#define DISTRHO_PLUGIN_CLAP_ID "com.sydalesis.sydalesis"

#define DISTRHO_PLUGIN_BRAND_ID  Sydl
#define DISTRHO_PLUGIN_UNIQUE_ID Syd5

#define DISTRHO_PLUGIN_HAS_UI           1
#define DISTRHO_UI_USE_NANOVG           1
#define DISTRHO_UI_USER_RESIZABLE       1
#define DISTRHO_UI_FILE_BROWSER         1
#define DISTRHO_UI_DEFAULT_WIDTH        1440
#define DISTRHO_UI_DEFAULT_HEIGHT       994   // 5.1.1: 1104 x 0.9 (keyboard strip)
#define DISTRHO_PLUGIN_IS_RT_SAFE       1
#define DISTRHO_PLUGIN_IS_SYNTH         1
#define DISTRHO_PLUGIN_WANT_MIDI_INPUT  1
#define DISTRHO_PLUGIN_WANT_MIDI_AS_MPE 1
#define DISTRHO_PLUGIN_NUM_INPUTS       0
#define DISTRHO_PLUGIN_NUM_OUTPUTS      2
#define DISTRHO_PLUGIN_WANT_PROGRAMS    1
#define DISTRHO_PLUGIN_WANT_STATE       1
#define DISTRHO_PLUGIN_WANT_FULL_STATE  1
#define DISTRHO_PLUGIN_WANT_TIMEPOS     1
#define DISTRHO_PLUGIN_WANT_DIRECT_ACCESS 1
#define DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST 1

#define DISTRHO_PLUGIN_VST3_CATEGORIES "Instrument|Synth"
#define DISTRHO_PLUGIN_CLAP_FEATURES   "instrument", "synthesizer", "stereo"

#endif
