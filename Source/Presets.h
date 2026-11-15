#pragma once
#include "VocalDSP.h"
#include <string_view>

namespace vocal
{
struct Preset
{
    const char* category;
    const char* name;
    std::array<float, 6> amount;
    std::array<int, 6> mode;
    float output;
    Settings settings() const { Settings s; s.amount = amount; s.mode = mode; s.output = output; return s; }
};
// Original starting points, never extracted from Waves factory presets.
// Columns: Bass, Treble, Compress, Reverb, Delay, Pitch. 0=off; 1/2/3=bottom/middle/top.
inline constexpr Preset presets[] {
    {"Female Vocals", "Female / Natural", {-2,2,-3,-6,-10,-10}, {2,2,1,1,0,0}, -2},
    {"Female Vocals", "Female / Pop Air", {-3,4,1,-4,-6,-6}, {2,3,2,2,2,2}, -3},
    {"Female Vocals", "Female / Intimate", {1,-1,-4,-7,-10,-10}, {2,2,1,1,0,0}, -2},
    {"Female Vocals", "Female / Rock Edge", {-1,3,4,-5,-7,-8}, {2,1,2,2,1,1}, -4},
    {"Female Vocals", "Female / Dream Pop", {-2,3,0,2,0,1}, {2,3,1,3,2,3}, -5},
    {"Female Vocals", "Female / Dance Wide", {-3,4,4,-5,-3,-1}, {2,2,3,1,2,2}, -5},
    {"Male Vocals", "Male / Natural", {1,1,-3,-7,-10,-10}, {2,2,1,1,0,0}, -2},
    {"Male Vocals", "Male / Pop Forward", {-1,3,2,-4,-5,-7}, {2,2,2,2,2,2}, -3},
    {"Male Vocals", "Male / Warm Baritone", {3,1,-2,-6,-8,-10}, {2,2,1,1,1,0}, -3},
    {"Male Vocals", "Male / Rock Wall", {1,3,6,-4,-6,-7}, {3,1,3,2,1,1}, -5},
    {"Male Vocals", "Male / Rap Dry", {2,3,3,-10,-9,-10}, {2,1,2,0,1,0}, -4},
    {"Male Vocals", "Male / Ballad", {1,2,0,-1,-3,-4}, {2,2,1,3,3,2}, -4},
    {"Lead Vocals", "Lead / Studio Presence", {0,2,1,-4,-6,-7}, {2,2,2,2,2,1}, -3},
    {"Lead Vocals", "Lead / Vintage Slap", {2,-1,2,-6,-1,-10}, {2,1,1,1,1,0}, -4},
    {"Lead Vocals", "Lead / Big Chorus", {-1,3,3,0,-2,0}, {2,3,2,2,3,3}, -5},
    {"Backing & Choir", "Backing / Female Wide", {-4,-2,1,0,-6,3}, {2,2,1,3,2,3}, -5},
    {"Backing & Choir", "Backing / Male Warm", {-1,-2,2,-2,-7,1}, {2,2,1,2,2,2}, -5},
    {"Backing & Choir", "Backing / Tight Pop", {-3,1,3,-5,-8,0}, {2,2,2,1,1,2}, -4},
    {"Backing & Choir", "Choir / Chamber", {-2,-1,-2,3,-7,2}, {2,2,1,3,3,3}, -5},
    {"Backing & Choir", "Choir / Distant Atmosphere", {-4,-4,0,6,1,5}, {2,2,1,3,3,3}, -6},
    {"Speech", "Speech / Podcast Clean", {1,1,-1,-10,-10,-10}, {2,2,1,0,0,0}, -3},
    {"Speech", "Speech / Radio Body", {3,2,3,-10,-10,-10}, {2,1,1,0,0,0}, -4},
    {"Speech", "Speech / Narration Soft", {1,-1,-3,-8,-10,-10}, {2,2,1,1,0,0}, -2},
    {"Drums", "Snare / Rock Punch", {-1,4,4,-1,-10,-10}, {3,1,2,2,0,0}, -4},
    {"Drums", "Snare / Tight Crack", {-2,5,5,-5,-10,-10}, {3,1,3,1,0,0}, -5},
    {"Drums", "Snare / Large Tail", {1,3,3,4,-10,-8}, {3,1,2,2,0,1}, -5},
    {"Drums", "Kick / Deep Sub", {5,2,2,-10,-10,-10}, {1,1,2,0,0,0}, -5},
    {"Drums", "Kick / Click Definition", {3,5,4,-10,-10,-10}, {2,2,3,0,0,0}, -5},
    {"Drums", "Toms / Tight Stereo", {3,3,2,-4,-10,-8}, {2,1,2,1,0,2}, -4},
    {"Drums", "Toms / Arena", {3,2,4,3,-10,-4}, {2,2,2,2,0,2}, -5},
    {"Drums", "Overheads / Open", {-3,3,-2,-6,-10,-7}, {2,3,1,1,0,2}, -3},
    {"Drums", "Room / Crushed Wide", {1,2,8,0,-10,3}, {3,1,3,2,0,2}, -7},
    {"Drums", "Parallel Bus / Explosive", {4,4,10,-4,-10,-10}, {2,1,3,1,0,0}, -8},
    {"Percussion", "Tambourine / Smooth", {-4,-2,1,-5,-10,-6}, {3,2,1,1,0,2}, -4},
    {"Percussion", "Shaker / Air", {-4,3,-1,-6,-7,-5}, {2,3,1,1,2,2}, -3},
    {"Percussion", "Congas / Natural Space", {2,2,0,-3,-7,-10}, {2,1,1,1,1,0}, -3},
    {"Percussion", "Cajon / Body And Snap", {4,3,2,-5,-10,-10}, {1,1,2,1,0,0}, -4},
    {"Guitars & Bass", "Acoustic Steel / Shimmer", {-1,4,-1,-4,-8,-3}, {2,2,1,1,2,2}, -4},
    {"Guitars & Bass", "Acoustic Nylon / Warm", {2,2,-3,-3,-10,-6}, {2,2,1,3,0,2}, -3},
    {"Guitars & Bass", "Rhythm Guitar / Bite", {3,4,1,-6,-10,-9}, {3,1,2,1,0,1}, -4},
    {"Guitars & Bass", "Lead Guitar / Slap Sustain", {1,3,5,-3,-1,-6}, {3,1,2,2,1,2}, -5},
    {"Guitars & Bass", "Lead Guitar / Eighth Echo", {1,2,4,-1,2,-4}, {3,2,2,3,2,2}, -5},
    {"Guitars & Bass", "Bass / Solid Wall", {4,2,5,-10,-10,-10}, {1,1,3,0,0,0}, -5},
    {"Guitars & Bass", "Bass / Pick Definition", {3,4,4,-10,-10,-10}, {2,1,3,0,0,0}, -5},
    {"Strings", "Violin / Silk", {-1,2,-2,1,-10,-6}, {2,2,1,3,0,2}, -3},
    {"Strings", "Cello / Warm Chamber", {3,1,-2,0,-10,-8}, {2,2,1,3,0,1}, -3},
    {"Strings", "String Section / Cinema", {2,3,0,3,-6,1}, {2,2,1,3,3,2}, -5},
    {"Keys & Synths", "Piano / Pop Glue", {1,3,1,-3,-8,-8}, {2,2,1,2,2,1}, -3},
    {"Keys & Synths", "Piano / Concert Room", {2,2,-2,2,-10,-6}, {2,3,1,3,0,2}, -4},
    {"Keys & Synths", "Rhodes / Wide Chorus", {2,1,0,-2,-7,4}, {2,2,1,2,2,2}, -5},
    {"Keys & Synths", "Wurlitzer / Bite", {2,4,2,-4,-6,-2}, {3,1,1,1,1,2}, -4},
    {"Keys & Synths", "Synth Lead / Aggressive", {1,4,4,-4,1,-3}, {3,1,2,2,2,2}, -5},
    {"Keys & Synths", "Arpeggio / Tempo Eighth", {-2,3,1,-3,4,-1}, {2,2,1,2,2,2}, -5},
    {"Keys & Synths", "Arpeggio / Quarter Trails", {-2,2,0,0,5,1}, {2,3,1,3,3,3}, -5},
    {"Keys & Synths", "Pad / Deep Atmosphere", {-3,-3,-1,6,2,6}, {2,2,1,3,3,3}, -6},
    {"Keys & Synths", "Hammond / Growl", {4,5,3,-2,-7,-4}, {2,1,2,2,1,2}, -5},
    {"Brass & Winds", "Sax / Velvet Slap", {1,2,1,-3,-4,-8}, {2,2,1,2,1,1}, -4},
    {"Brass & Winds", "Trumpet / Controlled Shine", {-2,2,3,-3,-5,-8}, {3,2,2,2,1,1}, -4},
    {"Brass & Winds", "Trombone / Warm Room", {3,2,2,-2,-6,-10}, {2,1,1,2,1,0}, -4},
    {"Brass & Winds", "Brass Section / Wide", {-1,3,3,0,-6,1}, {2,2,2,3,1,2}, -5},
    {"Brass & Winds", "Harmonica / In Your Face", {1,3,6,-5,-5,-10}, {3,1,2,1,1,0}, -5},
    {"Brass & Winds", "Accordion / Short Ambience", {1,2,3,-4,-7,-6}, {2,1,1,1,1,2}, -4}
};
}
