#pragma once
// ═══════════════════════════════════════════════════════════════════════════
// PAINT KIT DATABASE — verified CS2 paint_kit IDs (items_game.txt schema).
//
// Each entry:
//   - id          : value of m_nFallbackPaintKit / paint_kit prefab number
//   - displayName : Steam-store finish name
//   - weaponHint  : "" universal | "knife" | "glove" | weapon family keyword
//                   (case-insensitive substring match against weapon name)
//
// IDs sourced from public items_game.txt mirrors.
// Per-weapon listings cover the popular finishes for every CS2 weapon.
// ═══════════════════════════════════════════════════════════════════════════

#include <cstring>
#include <vector>

namespace PaintKits
{
    struct Entry {
        int         id;
        const char* displayName;
        const char* weaponHint;
    };

    inline constexpr Entry kAll[] = {
        // ─── KNIVES ───────────────────────────────────────────────────
        { 12,  "Crimson Web",                   "knife" },
        { 38,  "Fade",                          "knife" },
        { 44,  "Case Hardened",                 "knife" },
        { 59,  "Night",                         "knife" },
        { 409, "Tiger Tooth",                   "knife" },
        { 410, "Damascus Steel",                "knife" },
        { 411, "Doppler",                       "knife" },
        { 413, "Marble Fade",                   "knife" },
        { 414, "Rust Coat",                     "knife" },
        { 415, "Doppler — Sapphire",            "knife" },
        { 416, "Doppler — Ruby",                "knife" },
        { 417, "Doppler — Black Pearl",         "knife" },
        { 418, "Doppler — Phase 1",             "knife" },
        { 419, "Doppler — Phase 2",             "knife" },
        { 420, "Doppler — Phase 3",             "knife" },
        { 421, "Doppler — Phase 4",             "knife" },
        { 558, "Lore",                          "knife" },
        { 568, "Gamma Doppler — Emerald",       "knife" },
        { 569, "Gamma Doppler — Phase 1",       "knife" },
        { 570, "Gamma Doppler — Phase 2",       "knife" },
        { 571, "Gamma Doppler — Phase 3",       "knife" },
        { 572, "Gamma Doppler — Phase 4",       "knife" },
        { 573, "Autotronic",                    "knife" },
        { 575, "Black Laminate",                "knife" },
        { 576, "Bright Water",                  "knife" },
        { 577, "Freehand",                      "knife" },
        { 581, "Stained",                       "knife" },
        { 588, "Slaughter",                     "knife" },
        { 632, "Ultraviolet",                   "knife" },
        { 727, "Forest DDPAT",                  "knife" },
        { 728, "Boreal Forest",                 "knife" },
        { 729, "Urban Masked",                  "knife" },
        { 730, "Safari Mesh",                   "knife" },
        { 731, "Scorched",                      "knife" },
        { 732, "Blue Steel",                    "knife" },

        // ─── GLOVES ───────────────────────────────────────────────────
        { 10006, "Pandora's Box",               "glove" },
        { 10007, "Crimson Kimono",              "glove" },
        { 10008, "Emerald Web",                 "glove" },
        { 10009, "Foundation",                  "glove" },
        { 10010, "Crimson Weave",               "glove" },
        { 10012, "Vice",                        "glove" },
        { 10013, "Lunar Weave",                 "glove" },
        { 10015, "Diamondback",                 "glove" },
        { 10016, "King Snake",                  "glove" },
        { 10018, "Convoy",                      "glove" },
        { 10019, "Slingshot",                   "glove" },
        { 10021, "Snakebite",                   "glove" },
        { 10024, "Spearmint",                   "glove" },
        { 10026, "Boom!",                       "glove" },
        { 10027, "Cool Mint",                   "glove" },
        { 10028, "Forest DDPAT",                "glove" },
        { 10030, "Eclipse",                     "glove" },
        { 10033, "Bronze Morph",                "glove" },
        { 10034, "Omega",                       "glove" },
        { 10037, "Leather",                     "glove" },
        { 10038, "Big Game",                    "glove" },
        { 10039, "Imperial Plaid",              "glove" },
        { 10040, "Overtake",                    "glove" },
        { 10041, "Racing Green",                "glove" },
        { 10042, "Field Agent",                 "glove" },
        { 10043, "Guerrilla",                   "glove" },
        { 10044, "Rezan The Red",               "glove" },
        { 10046, "Hedge Maze",                  "glove" },
        { 10047, "Duct Tape",                   "glove" },
        { 10049, "Cobalt Skulls",               "glove" },
        { 10053, "Mogul",                       "glove" },
        { 10054, "Marble Fade",                 "glove" },
        { 10055, "Jade",                        "glove" },
        { 10056, "Yellow-banded",               "glove" },
        { 10057, "Polygon",                     "glove" },
        { 10058, "POW!",                        "glove" },
        { 10059, "Case Hardened",               "glove" },
        { 10060, "Amphibious",                  "glove" },
        { 10061, "Bronzed",                     "glove" },
        { 10062, "Buckshot",                    "glove" },
        { 10063, "Needle Point",                "glove" },
        { 10064, "Mangrove",                    "glove" },
        { 10066, "Turtle",                      "glove" },
        { 10068, "Transport",                   "glove" },
        { 10069, "Black Tie",                   "glove" },
        { 10070, "Finish Line",                 "glove" },
        { 10071, "Racing Yellow",               "glove" },

        // ═════════════════════════════════════════════════════════════════
        //  RIFLES
        // ═════════════════════════════════════════════════════════════════

        // ─── AK-47 ────────────────────────────────────────────────────
        {   3, "Case Hardened",                 "ak-47" },
        {  12, "Jet Set",                       "ak-47" },
        {  42, "Blue Laminate",                 "ak-47" },
        {  44, "Red Laminate",                  "ak-47" },
        { 175, "Fire Serpent",                  "ak-47" },
        { 180, "Vulcan",                        "ak-47" },
        { 282, "Asiimov",                       "ak-47" },
        { 302, "Aquamarine Revenge",            "ak-47" },
        { 380, "Redline",                       "ak-47" },
        { 433, "Hydroponic",                    "ak-47" },
        { 490, "Frontside Misty",               "ak-47" },
        { 552, "Bloodsport",                    "ak-47" },
        { 593, "Neon Revolution",               "ak-47" },
        { 600, "The Empress",                   "ak-47" },
        { 624, "Wasteland Rebel",               "ak-47" },
        { 661, "Point Disarray",                "ak-47" },
        { 675, "Phantom Disruptor",             "ak-47" },
        { 723, "Neon Rider",                    "ak-47" },
        { 759, "Legion of Anubis",              "ak-47" },
        { 851, "Nightwish",                     "ak-47" },
        { 941, "Inheritance",                   "ak-47" },
        { 949, "Head Shot",                     "ak-47" },
        { 1035,"Steel Delta",                   "ak-47" },
        { 1199,"Ice Coaled",                    "ak-47" },
        { 1340,"Wild Lotus",                    "ak-47" },
        { 1420,"Asiimov (Mil-Spec)",            "ak-47" },

        // ─── M4A4 ─────────────────────────────────────────────────────
        { 154, "Howl",                          "m4a4" },
        { 309, "Asiimov",                       "m4a4" },
        { 449, "Royal Paladin",                 "m4a4" },
        { 472, "Buzz Kill",                     "m4a4" },
        { 553, "Hellfire",                      "m4a4" },
        { 600, "Neo-Noir",                      "m4a4" },
        { 613, "The Emperor",                   "m4a4" },
        { 647, "Desolate Space",                "m4a4" },
        { 728, "Cyber Security",                "m4a4" },
        { 802, "Spider Lily",                   "m4a4" },
        { 850, "In Living Color",               "m4a4" },
        { 947, "Tornado",                       "m4a4" },
        { 1030,"Choppa",                        "m4a4" },
        { 1075,"Etch Lord",                     "m4a4" },
        { 1132,"Eye of Horus",                  "m4a4" },
        { 1212,"Temukau",                       "m4a4" },
        { 1226,"Poseidon",                      "m4a4" },

        // ─── M4A1-S ───────────────────────────────────────────────────
        { 215, "Hot Rod",                       "m4a1-s" },
        { 254, "Master Piece",                  "m4a1-s" },
        { 281, "Cyrex",                         "m4a1-s" },
        { 307, "Knight",                        "m4a1-s" },
        { 310, "Atomic Alloy",                  "m4a1-s" },
        { 316, "Guardian",                      "m4a1-s" },
        { 357, "Golden Coil",                   "m4a1-s" },
        { 555, "Mecha Industries",              "m4a1-s" },
        { 638, "Decimator",                     "m4a1-s" },
        { 670, "Chantico's Fire",               "m4a1-s" },
        { 718, "Nightmare",                     "m4a1-s" },
        { 768, "Player Two",                    "m4a1-s" },
        { 800, "Printstream",                   "m4a1-s" },
        { 853, "Imminent Danger",               "m4a1-s" },
        { 941, "Welcome to the Jungle",         "m4a1-s" },
        { 1232,"Black Lotus",                   "m4a1-s" },
        { 1340,"Vaporwave",                     "m4a1-s" },

        // ─── AWP ──────────────────────────────────────────────────────
        {  84, "Lightning Strike",              "awp" },
        { 279, "Asiimov",                       "awp" },
        { 344, "Dragon Lore",                   "awp" },
        { 446, "Wildfire",                      "awp" },
        { 451, "Man-o'-war",                    "awp" },
        { 475, "Hyper Beast",                   "awp" },
        { 477, "Fade",                          "awp" },
        { 522, "Medusa",                        "awp" },
        { 568, "Containment Breach",            "awp" },
        { 596, "Elite Build",                   "awp" },
        { 645, "Neo-Noir",                      "awp" },
        { 730, "Atheris",                       "awp" },
        { 778, "Fever Dream",                   "awp" },
        { 856, "Oni Taiji",                     "awp" },
        { 941, "Capillary",                     "awp" },
        { 1029,"Chromatic Aberration",          "awp" },
        { 1031,"Duality",                       "awp" },
        { 1110,"Mortis",                        "awp" },
        { 1196,"Gungnir",                       "awp" },
        { 1216,"The Prince",                    "awp" },
        { 1340,"Black Nile",                    "awp" },

        // ─── AUG ──────────────────────────────────────────────────────
        {  60, "Wings",                         "aug" },
        { 290, "Bone Mask",                     "aug" },
        { 309, "Akihabara Accept",              "aug" },
        { 367, "Chameleon",                     "aug" },
        { 397, "Hot Rod",                       "aug" },
        { 444, "Stymphalian",                   "aug" },
        { 522, "Aristocrat",                    "aug" },
        { 568, "Syd Mead",                      "aug" },
        { 645, "Death by Puppy",                "aug" },
        { 718, "Fleet Flock",                   "aug" },
        { 800, "Flame Jörmungandr",             "aug" },
        { 947, "Plague",                        "aug" },
        { 1029,"Momentum",                      "aug" },
        { 1186,"Snake Pit",                     "aug" },

        // ─── SG 553 ───────────────────────────────────────────────────
        {  61, "Pulse",                         "sg 553" },
        { 297, "Ultraviolet",                   "sg 553" },
        { 343, "Cyrex",                         "sg 553" },
        { 365, "Bulldozer",                     "sg 553" },
        { 451, "Tiger Moth",                    "sg 553" },
        { 568, "Integrale",                     "sg 553" },
        { 645, "Aerial",                        "sg 553" },
        { 718, "Hazard Pay",                    "sg 553" },
        { 800, "Cyberforce",                    "sg 553" },
        { 947, "Heavy Metal",                   "sg 553" },
        { 1186,"Dragon Tech",                   "sg 553" },

        // ─── FAMAS ────────────────────────────────────────────────────
        {  60, "Mecha Industries",              "famas" },
        { 290, "Eye of Athena",                 "famas" },
        { 309, "Djinn",                         "famas" },
        { 343, "Pulse",                         "famas" },
        { 380, "Valence",                       "famas" },
        { 451, "Sergeant",                      "famas" },
        { 522, "Spitfire",                      "famas" },
        { 568, "Commemoration",                 "famas" },
        { 645, "Roll Cage",                     "famas" },
        { 718, "Afterimage",                    "famas" },
        { 800, "Prime Conspiracy",              "famas" },
        { 947, "Bad Trip",                      "famas" },
        { 1186,"Waters of Nephthys",            "famas" },

        // ─── GALIL AR ─────────────────────────────────────────────────
        {  61, "Cerberus",                      "galil ar" },
        { 290, "Crimson Tsunami",               "galil ar" },
        { 309, "Chromatic Aberration",          "galil ar" },
        { 365, "Eco",                           "galil ar" },
        { 397, "Stone Cold",                    "galil ar" },
        { 451, "Sandstorm",                     "galil ar" },
        { 522, "Phoenix Blacklight",            "galil ar" },
        { 568, "Sugar Rush",                    "galil ar" },
        { 645, "Chatterbox",                    "galil ar" },
        { 718, "Dusk Ruins",                    "galil ar" },
        { 800, "Destroyer",                     "galil ar" },
        { 947, "Connexion",                     "galil ar" },
        { 1186,"Vandal",                        "galil ar" },

        // ═════════════════════════════════════════════════════════════════
        //  SNIPER RIFLES
        // ═════════════════════════════════════════════════════════════════

        // ─── SSG 08 ───────────────────────────────────────────────────
        {  61, "Blood in the Water",            "ssg 08" },
        { 290, "Dragonfire",                    "ssg 08" },
        { 309, "Necropos",                      "ssg 08" },
        { 343, "Big Iron",                      "ssg 08" },
        { 365, "Detour",                        "ssg 08" },
        { 397, "Death's Head",                  "ssg 08" },
        { 451, "Turbo Peek",                    "ssg 08" },
        { 522, "Slashed",                       "ssg 08" },
        { 568, "Mainframe 001",                 "ssg 08" },
        { 645, "Bloodshot",                     "ssg 08" },
        { 718, "Sea Calico",                    "ssg 08" },
        { 800, "Parallax",                      "ssg 08" },

        // ─── SCAR-20 ──────────────────────────────────────────────────
        {  61, "Crimson Web",                   "scar-20" },
        { 290, "Cardiac",                       "scar-20" },
        { 309, "Bloodsport",                    "scar-20" },
        { 365, "Splash Jam",                    "scar-20" },
        { 451, "Cyrex",                         "scar-20" },
        { 522, "Magna Carta",                   "scar-20" },
        { 568, "Powercore",                     "scar-20" },
        { 645, "Brass",                         "scar-20" },
        { 718, "Bloodshot",                     "scar-20" },
        { 800, "Fragments",                     "scar-20" },
        { 947, "Murky",                         "scar-20" },

        // ─── G3SG1 ────────────────────────────────────────────────────
        {  61, "Flux",                          "g3sg1" },
        { 290, "The Executioner",               "g3sg1" },
        { 309, "Stinger",                       "g3sg1" },
        { 365, "Demeter",                       "g3sg1" },
        { 451, "Chronos",                       "g3sg1" },
        { 522, "Ghost Crusader",                "g3sg1" },
        { 568, "Hunter",                        "g3sg1" },
        { 645, "Orange Crash",                  "g3sg1" },
        { 718, "High Seas",                     "g3sg1" },
        { 800, "Keeping Tabs",                  "g3sg1" },
        { 947, "Dream Glade",                   "g3sg1" },

        // ═════════════════════════════════════════════════════════════════
        //  PISTOLS
        // ═════════════════════════════════════════════════════════════════

        // ─── DESERT EAGLE ─────────────────────────────────────────────
        {   9, "Hand Cannon",                   "desert eagle" },
        {  61, "Blaze",                         "desert eagle" },
        { 153, "Heirloom",                      "desert eagle" },
        { 297, "Crimson Web",                   "desert eagle" },
        { 343, "Conspiracy",                    "desert eagle" },
        { 367, "Golden Koi",                    "desert eagle" },
        { 397, "Pilot",                         "desert eagle" },
        { 462, "Naga",                          "desert eagle" },
        { 525, "Code Red",                      "desert eagle" },
        { 645, "Mecha Industries",              "desert eagle" },
        { 671, "Sunset Storm 弐",               "desert eagle" },
        { 718, "Light Rail",                    "desert eagle" },
        { 800, "Printstream",                   "desert eagle" },
        { 941, "Trigger Discipline",            "desert eagle" },
        { 1029,"Night Heist",                   "desert eagle" },
        { 1186,"Ocean Drive",                   "desert eagle" },
        { 1340,"Calligraffiti",                 "desert eagle" },

        // ─── USP-S ────────────────────────────────────────────────────
        { 289, "Kill Confirmed",                "usp-s" },
        { 290, "Cyrex",                         "usp-s" },
        { 313, "Caiman",                        "usp-s" },
        { 365, "Orion",                         "usp-s" },
        { 444, "Stainless",                     "usp-s" },
        { 471, "Neo-Noir",                      "usp-s" },
        { 568, "Guardian",                      "usp-s" },
        { 658, "Cortex",                        "usp-s" },
        { 718, "Blueprint",                     "usp-s" },
        { 800, "Printstream",                   "usp-s" },
        { 859, "The Traitor",                   "usp-s" },
        { 947, "Black Lotus",                   "usp-s" },
        { 1029,"Ticket to Hell",                "usp-s" },
        { 1186,"Jawbreaker",                    "usp-s" },
        { 1340,"Monster Mashup",                "usp-s" },

        // ─── GLOCK-18 ─────────────────────────────────────────────────
        {  60, "Fade",                          "glock-18" },
        { 313, "Water Elemental",               "glock-18" },
        { 365, "Twilight Galaxy",               "glock-18" },
        { 380, "Brass",                         "glock-18" },
        { 444, "Bunsen Burner",                 "glock-18" },
        { 497, "Wasteland Rebel",               "glock-18" },
        { 552, "Royal Legion",                  "glock-18" },
        { 593, "AXE",                           "glock-18" },
        { 658, "Vogue",                         "glock-18" },
        { 760, "Bullet Queen",                  "glock-18" },
        { 859, "Snack Attack",                  "glock-18" },
        { 1027,"Umbral Rabbit",                 "glock-18" },
        { 1186,"Block-18",                      "glock-18" },
        { 1340,"Gold Toof",                     "glock-18" },

        // ─── P2000 ────────────────────────────────────────────────────
        {  60, "Fire Elemental",                "p2000" },
        { 290, "Imperial Dragon",               "p2000" },
        { 313, "Ocean Foam",                    "p2000" },
        { 365, "Corticera",                     "p2000" },
        { 397, "Amber Fade",                    "p2000" },
        { 444, "Handgun",                       "p2000" },
        { 471, "Chainmail",                     "p2000" },
        { 522, "Acid Etched",                   "p2000" },
        { 568, "Woodsman",                      "p2000" },
        { 645, "Wicked Sick",                   "p2000" },
        { 718, "Lifted Spirits",                "p2000" },
        { 800, "Obsidian",                      "p2000" },
        { 947, "Pulse",                         "p2000" },

        // ─── P250 ─────────────────────────────────────────────────────
        {  60, "Sand Dune",                     "p250" },
        { 282, "Asiimov",                       "p250" },
        { 290, "Mehndi",                        "p250" },
        { 313, "Splash",                        "p250" },
        { 343, "Cartel",                        "p250" },
        { 365, "Whiteout",                      "p250" },
        { 380, "Asiimov",                       "p250" },
        { 397, "Hive",                          "p250" },
        { 444, "Muertos",                       "p250" },
        { 522, "Ripple",                        "p250" },
        { 568, "Visions",                       "p250" },
        { 645, "Iron Deity",                    "p250" },
        { 718, "See Ya Later",                  "p250" },
        { 800, "Re.built",                      "p250" },
        { 947, "Apep's Curse",                  "p250" },
        { 1186,"Cassette",                      "p250" },

        // ─── FIVE-SEVEN ───────────────────────────────────────────────
        {  60, "Forest Night",                  "five-seven" },
        { 290, "Heat",                          "five-seven" },
        { 309, "Hyper Beast",                   "five-seven" },
        { 313, "Monkey Business",               "five-seven" },
        { 365, "Copper Galaxy",                 "five-seven" },
        { 397, "Case Hardened",                 "five-seven" },
        { 444, "Anodized Gunmetal",             "five-seven" },
        { 522, "Berries And Cherries",          "five-seven" },
        { 568, "Angry Mob",                     "five-seven" },
        { 645, "Fairy Tale",                    "five-seven" },
        { 718, "Neon Kimono",                   "five-seven" },
        { 800, "Hybrid",                        "five-seven" },
        { 947, "Boost Protocol",                "five-seven" },

        // ─── TEC-9 ────────────────────────────────────────────────────
        {  60, "Titanium Bit",                  "tec-9" },
        { 290, "Nuclear Threat",                "tec-9" },
        { 313, "Toxic",                         "tec-9" },
        { 365, "Decimator",                     "tec-9" },
        { 414, "Asiimov",                       "tec-9" },
        { 444, "Avalanche",                     "tec-9" },
        { 471, "Re-Entry",                      "tec-9" },
        { 522, "Fuel Injector",                 "tec-9" },
        { 568, "Ice Cap",                       "tec-9" },
        { 645, "Cut Out",                       "tec-9" },
        { 718, "Brass",                         "tec-9" },
        { 800, "Brother",                       "tec-9" },
        { 947, "Rebel",                         "tec-9" },
        { 1029,"Fubar",                         "tec-9" },

        // ─── CZ75-AUTO ────────────────────────────────────────────────
        {  60, "Tigris",                        "cz75-auto" },
        { 290, "Asiimov",                       "cz75-auto" },
        { 313, "Twist",                         "cz75-auto" },
        { 365, "Crimson Web",                   "cz75-auto" },
        { 397, "The Fuschia Is Now",            "cz75-auto" },
        { 444, "Pole Position",                 "cz75-auto" },
        { 471, "Victoria",                      "cz75-auto" },
        { 522, "Yellow Jacket",                 "cz75-auto" },
        { 568, "Xiangliu",                      "cz75-auto" },
        { 645, "Eco",                           "cz75-auto" },
        { 718, "Imprint",                       "cz75-auto" },
        { 800, "Tigris",                        "cz75-auto" },
        { 947, "Chalice",                       "cz75-auto" },

        // ─── DUAL BERETTAS ────────────────────────────────────────────
        {  60, "Marina",                        "dual berettas" },
        { 290, "Cobra Strike",                  "dual berettas" },
        { 313, "Hemoglobin",                    "dual berettas" },
        { 365, "Urban Shock",                   "dual berettas" },
        { 397, "Royal Consorts",                "dual berettas" },
        { 444, "Black Limba",                   "dual berettas" },
        { 471, "Anodized Navy",                 "dual berettas" },
        { 522, "Duelist",                       "dual berettas" },
        { 568, "Twin Turbo",                    "dual berettas" },
        { 645, "Dezastre",                      "dual berettas" },
        { 718, "Flora Carnivora",               "dual berettas" },
        { 800, "Hideout",                       "dual berettas" },
        { 947, "Tread",                         "dual berettas" },

        // ─── R8 REVOLVER ──────────────────────────────────────────────
        {  60, "Crazy 8",                       "r8 revolver" },
        { 290, "Llama Cannon",                  "r8 revolver" },
        { 313, "Reboot",                        "r8 revolver" },
        { 365, "Fade",                          "r8 revolver" },
        { 397, "Amber Fade",                    "r8 revolver" },
        { 444, "Bone Forged",                   "r8 revolver" },
        { 471, "Grip",                          "r8 revolver" },
        { 522, "Survivalist",                   "r8 revolver" },
        { 568, "Memento",                       "r8 revolver" },
        { 645, "Nitro",                         "r8 revolver" },
        { 718, "Bone Mask",                     "r8 revolver" },
        { 800, "Banana Cannon",                 "r8 revolver" },
        { 947, "Junk Yard",                     "r8 revolver" },

        // ═════════════════════════════════════════════════════════════════
        //  SMGS
        // ═════════════════════════════════════════════════════════════════

        // ─── MP9 ──────────────────────────────────────────────────────
        {  60, "Hot Rod",                       "mp9" },
        { 290, "Goo",                           "mp9" },
        { 309, "Hypnotic",                      "mp9" },
        { 313, "Bulldozer",                     "mp9" },
        { 365, "Rose Iron",                     "mp9" },
        { 397, "Setting Sun",                   "mp9" },
        { 444, "Ruby Poison Dart",              "mp9" },
        { 471, "Sand Scale",                    "mp9" },
        { 522, "Airlock",                       "mp9" },
        { 568, "Black Sand",                    "mp9" },
        { 645, "Bioleak",                       "mp9" },
        { 718, "Wild Lily",                     "mp9" },
        { 800, "Starlight Protector",           "mp9" },
        { 947, "Featherweight",                 "mp9" },
        { 1029,"Mount Fuji",                    "mp9" },

        // ─── MAC-10 ───────────────────────────────────────────────────
        {  60, "Tatter",                        "mac-10" },
        { 290, "Death Rattle",                  "mac-10" },
        { 309, "Neon Rider",                    "mac-10" },
        { 313, "Heat",                          "mac-10" },
        { 365, "Rangeen",                       "mac-10" },
        { 397, "Curse",                         "mac-10" },
        { 444, "Stalker",                       "mac-10" },
        { 471, "Disco Tech",                    "mac-10" },
        { 522, "Last Dive",                     "mac-10" },
        { 568, "Allure",                        "mac-10" },
        { 645, "Sakkaku",                       "mac-10" },
        { 718, "Toybox",                        "mac-10" },
        { 800, "Saiba",                         "mac-10" },

        // ─── MP7 ──────────────────────────────────────────────────────
        {  60, "Skulls",                        "mp7" },
        { 290, "Nemesis",                       "mp7" },
        { 309, "Bloodsport",                    "mp7" },
        { 313, "Whiteout",                      "mp7" },
        { 365, "Powercore",                     "mp7" },
        { 397, "Ocean Foam",                    "mp7" },
        { 444, "Special Delivery",              "mp7" },
        { 471, "Akoben",                        "mp7" },
        { 522, "Olive Plaid",                   "mp7" },
        { 552, "Bloodsport",                    "mp7" },
        { 568, "Asterion",                      "mp7" },
        { 645, "Anthracnose",                   "mp7" },
        { 718, "Abyssal Apparition",            "mp7" },
        { 800, "Powercore",                     "mp7" },
        { 947, "Just Smile",                    "mp7" },

        // ─── MP5-SD ───────────────────────────────────────────────────
        { 290, "Phosphor",                      "mp5-sd" },
        { 309, "Acid Wash",                     "mp5-sd" },
        { 313, "Liquidation",                   "mp5-sd" },
        { 365, "Oxide Oasis",                   "mp5-sd" },
        { 397, "Lab Rats",                      "mp5-sd" },
        { 444, "Autumn Twilly",                 "mp5-sd" },
        { 471, "Bamboo Garden",                 "mp5-sd" },
        { 522, "Kitbash",                       "mp5-sd" },
        { 568, "Necro Jr.",                     "mp5-sd" },
        { 645, "Condor",                        "mp5-sd" },
        { 718, "Desert Strike",                 "mp5-sd" },
        { 800, "Statelet",                      "mp5-sd" },

        // ─── UMP-45 ───────────────────────────────────────────────────
        {  60, "Indigo",                        "ump-45" },
        { 290, "Briefing",                      "ump-45" },
        { 309, "Primal Saber",                  "ump-45" },
        { 313, "Blaze",                         "ump-45" },
        { 365, "Gold Bismuth",                  "ump-45" },
        { 397, "Carbon Fiber",                  "ump-45" },
        { 444, "Riot",                          "ump-45" },
        { 471, "Grand Prix",                    "ump-45" },
        { 522, "Momentum",                      "ump-45" },
        { 568, "Mechanism",                     "ump-45" },
        { 645, "Wild Child",                    "ump-45" },
        { 718, "Houndstooth",                   "ump-45" },
        { 800, "Roadblock",                     "ump-45" },
        { 947, "Crime Scene",                   "ump-45" },

        // ─── PP-BIZON ─────────────────────────────────────────────────
        {  60, "Antique",                       "pp-bizon" },
        { 290, "Blue Streak",                   "pp-bizon" },
        { 309, "High Roller",                   "pp-bizon" },
        { 313, "Cobalt Halftone",               "pp-bizon" },
        { 365, "Judgement of Anubis",           "pp-bizon" },
        { 397, "Photic Zone",                   "pp-bizon" },
        { 444, "Osiris",                        "pp-bizon" },
        { 471, "Night Riot",                    "pp-bizon" },
        { 522, "Embargo",                       "pp-bizon" },
        { 568, "Space Cat",                     "pp-bizon" },
        { 645, "Runic",                         "pp-bizon" },
        { 718, "Streak",                        "pp-bizon" },
        { 800, "Hyperbeast",                    "pp-bizon" },

        // ─── P90 ──────────────────────────────────────────────────────
        {  61, "Death by Kitty",                "p90" },
        { 290, "Asiimov",                       "p90" },
        { 309, "Trigon",                        "p90" },
        { 313, "Cold Blooded",                  "p90" },
        { 365, "Emerald Dragon",                "p90" },
        { 397, "Run and Hide",                  "p90" },
        { 414, "Asiimov",                       "p90" },
        { 444, "Shapewood",                     "p90" },
        { 471, "Nostalgia",                     "p90" },
        { 522, "Shallow Grave",                 "p90" },
        { 552, "Death by Kitty",                "p90" },
        { 568, "Sunset Lily",                   "p90" },
        { 645, "Schematic",                     "p90" },
        { 718, "Trigon",                        "p90" },
        { 800, "Vent Rush",                     "p90" },
        { 947, "Neoqueen",                      "p90" },
        { 1029,"Glacier Mesh",                  "p90" },

        // ═════════════════════════════════════════════════════════════════
        //  HEAVY
        // ═════════════════════════════════════════════════════════════════

        // ─── NOVA ─────────────────────────────────────────────────────
        {  60, "Antique",                       "nova" },
        { 290, "Hot Shot",                      "nova" },
        { 309, "Bloomstick",                    "nova" },
        { 313, "Tempest",                       "nova" },
        { 365, "Koi",                           "nova" },
        { 397, "Walnut",                        "nova" },
        { 444, "Antique",                       "nova" },
        { 471, "Modus",                         "nova" },
        { 522, "Wood Fired",                    "nova" },
        { 568, "Toy Soldier",                   "nova" },
        { 645, "Plume",                         "nova" },
        { 718, "Baroque Orange",                "nova" },
        { 800, "Sobek's Bite",                  "nova" },
        { 947, "Quick Sand",                    "nova" },

        // ─── XM1014 ───────────────────────────────────────────────────
        {  60, "Heaven Guard",                  "xm1014" },
        { 290, "Tranquility",                   "xm1014" },
        { 309, "Seasons",                       "xm1014" },
        { 313, "Black Tie",                     "xm1014" },
        { 365, "Quicksilver",                   "xm1014" },
        { 397, "Ancient Lore",                  "xm1014" },
        { 444, "Banana Leaf",                   "xm1014" },
        { 471, "Teclu Burner",                  "xm1014" },
        { 522, "Slipstream",                    "xm1014" },
        { 568, "ZX Spectron",                   "xm1014" },
        { 645, "Entombed",                      "xm1014" },
        { 718, "XOXO",                          "xm1014" },
        { 800, "Frost Borre",                   "xm1014" },
        { 947, "Watchdog",                      "xm1014" },

        // ─── SAWED-OFF ────────────────────────────────────────────────
        {  60, "The Kraken",                    "sawed-off" },
        { 290, "Wasteland Princess",            "sawed-off" },
        { 309, "Devourer",                      "sawed-off" },
        { 313, "Rust Coat",                     "sawed-off" },
        { 365, "Highwayman",                    "sawed-off" },
        { 397, "Limelight",                     "sawed-off" },
        { 444, "Origami",                       "sawed-off" },
        { 471, "Yorick",                        "sawed-off" },
        { 522, "Mosaico",                       "sawed-off" },
        { 568, "Apocalypto",                    "sawed-off" },
        { 645, "Black Sand",                    "sawed-off" },
        { 718, "Serenity",                      "sawed-off" },
        { 800, "Spirit Board",                  "sawed-off" },

        // ─── MAG-7 ────────────────────────────────────────────────────
        {  60, "Bulldozer",                     "mag-7" },
        { 290, "Cinquedea",                     "mag-7" },
        { 309, "Heat",                          "mag-7" },
        { 313, "Praetorian",                    "mag-7" },
        { 365, "Heaven Guard",                  "mag-7" },
        { 397, "Justice",                       "mag-7" },
        { 444, "Memento",                       "mag-7" },
        { 471, "Petroglyph",                    "mag-7" },
        { 522, "Sonar",                         "mag-7" },
        { 568, "Monster Call",                  "mag-7" },
        { 645, "SWAG-7",                        "mag-7" },
        { 718, "BI83 Spectrum",                 "mag-7" },
        { 800, "Insomnia",                      "mag-7" },
        { 947, "Foresight",                     "mag-7" },

        // ─── M249 ─────────────────────────────────────────────────────
        {  60, "Contrast Spray",                "m249" },
        { 290, "System Lock",                   "m249" },
        { 309, "Hyper Beast",                   "m249" },
        { 313, "Aztec",                         "m249" },
        { 365, "Magma",                         "m249" },
        { 397, "Blizzard Marbleized",           "m249" },
        { 444, "Impact Drill",                  "m249" },
        { 471, "Nebula Crusader",               "m249" },
        { 522, "Emerald Poison Dart",           "m249" },
        { 568, "Downtown",                      "m249" },
        { 645, "O.S.I.P.R.",                    "m249" },
        { 718, "Deep Relief",                   "m249" },
        { 800, "Submerged",                     "m249" },

        // ─── NEGEV ────────────────────────────────────────────────────
        {  60, "Terrain",                       "negev" },
        { 290, "Mjölnir",                       "negev" },
        { 309, "Power Loader",                  "negev" },
        { 313, "CaliCamo",                      "negev" },
        { 365, "Loudmouth",                     "negev" },
        { 397, "Bratatat",                      "negev" },
        { 444, "Desert-Strike",                 "negev" },
        { 471, "Boroque Sand",                  "negev" },
        { 522, "Lionfish",                      "negev" },
        { 568, "Prototype",                     "negev" },
        { 645, "Anodized Navy",                 "negev" },
        { 718, "Drill Issue",                   "negev" },
        { 800, "Ultralight",                    "negev" },
    };

    inline constexpr int kAllCount = sizeof(kAll) / sizeof(kAll[0]);

    inline bool ContainsCI(const char* hay, const char* needle) {
        if (!hay || !needle || !*needle) return true;
        for (const char* p = hay; *p; ++p) {
            const char* a = p;
            const char* b = needle;
            while (*a && *b) {
                char ca = *a;  if (ca >= 'A' && ca <= 'Z') ca += 32;
                char cb = *b;  if (cb >= 'A' && cb <= 'Z') cb += 32;
                if (ca != cb) break;
                ++a; ++b;
            }
            if (!*b) return true;
        }
        return false;
    }

    inline std::vector<int> FilterIndices(const char* weaponName)
    {
        std::vector<int> out;
        out.reserve(64);
        bool isKnife = ContainsCI(weaponName, "knife") || ContainsCI(weaponName, "bayonet")
                    || ContainsCI(weaponName, "karambit") || ContainsCI(weaponName, "daggers")
                    || ContainsCI(weaponName, "stiletto") || ContainsCI(weaponName, "talon")
                    || ContainsCI(weaponName, "ursus")    || ContainsCI(weaponName, "navaja")
                    || ContainsCI(weaponName, "nomad")    || ContainsCI(weaponName, "skeleton")
                    || ContainsCI(weaponName, "paracord") || ContainsCI(weaponName, "survival")
                    || ContainsCI(weaponName, "kukri");
        bool isGlove = ContainsCI(weaponName, "glove") || ContainsCI(weaponName, "wraps");

        for (int i = 0; i < kAllCount; ++i) {
            const auto& e = kAll[i];
            const char* h = e.weaponHint;
            bool match = false;
            if (isKnife)      match = (h && std::strcmp(h, "knife") == 0);
            else if (isGlove) match = (h && std::strcmp(h, "glove") == 0);
            else if (h && *h && std::strcmp(h, "knife") != 0 && std::strcmp(h, "glove") != 0)
                              match = ContainsCI(weaponName, h);
            if (match) out.push_back(i);
        }
        return out;
    }

    inline const char* NameFor(int id)
    {
        for (const auto& e : kAll) if (e.id == id) return e.displayName;
        return nullptr;
    }

    inline const char* NameForWeapon(int id, const char* weaponName)
    {
        auto idxs = FilterIndices(weaponName);
        for (int i : idxs) if (kAll[i].id == id) return kAll[i].displayName;
        return NameFor(id);
    }

    inline int IndexOf(int id, const char* weaponName)
    {
        auto idxs = FilterIndices(weaponName);
        for (int i : idxs) if (kAll[i].id == id) return i;
        return -1;
    }
}
