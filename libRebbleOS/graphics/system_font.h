#pragma once
/*
027 | gothic: 09
028 | gothic: 14
029 | gothic: 14 bold
030 | gothic: 18
007 | gothic: 18 bold
008 | gothic: 24
032 | gothic: 24 bold
034 | gothic: 28
035 | gothic: 28 bold

031 | emoji (partial)
033 | emoji (partial)
036 | emoji (partial)

037 | bitham: 30 black
038 | bitham: 42 bold
039 | bitham: 42 light
040 | bitham: 42 medium numbers
041 | bitham: 34 medium numbers
042 | bitham: 34 light subset
043 | bitham: 18 light subset

044 | roboto: 21 condensed
045 | roboto: 49 bold subset

046 | droid: 28 bold

047 | leco: 20 bold
048 | leco: 26 bold + am/pm
049 | leco: 32 bold
050 | leco: 36 bold
051 | leco: 38 bold
052 | leco: 42
053 | leco: 28 light
*/

// whats with this font key string crap?
// I know it lets pebble move stuff around, but still

#define FONT_KEY_FONT_FALLBACK              "RESOURCE_ID_FONT_FALLBACK"
#define FONT_KEY_GOTHIC_09                  "RESOURCE_ID_GOTHIC_09"
#define FONT_KEY_GOTHIC_14                  "RESOURCE_ID_GOTHIC_14"
#define FONT_KEY_GOTHIC_14_BOLD             "RESOURCE_ID_GOTHIC_14_BOLD"
#define FONT_KEY_GOTHIC_18                  "RESOURCE_ID_GOTHIC_18"
#define FONT_KEY_GOTHIC_18_BOLD             "RESOURCE_ID_GOTHIC_18_BOLD"
#define FONT_KEY_GOTHIC_24                  "RESOURCE_ID_GOTHIC_24"
#define FONT_KEY_GOTHIC_24_BOLD             "RESOURCE_ID_GOTHIC_24_BOLD"
#define FONT_KEY_GOTHIC_28                  "RESOURCE_ID_GOTHIC_28"
#define FONT_KEY_GOTHIC_28_BOLD             "RESOURCE_ID_GOTHIC_28_BOLD"
#define FONT_KEY_GOTHIC_36                  "RESOURCE_ID_GOTHIC_36"

#define FONT_KEY_BITHAM_30_BLACK            "RESOURCE_ID_BITHAM_30_BLACK"
#define FONT_KEY_BITHAM_42_BOLD             "RESOURCE_ID_BITHAM_42_BOLD"
#define FONT_KEY_BITHAM_42_LIGHT            "RESOURCE_ID_BITHAM_42_LIGHT"
#define FONT_KEY_BITHAM_42_MEDIUM_NUMBERS   "RESOURCE_ID_BITHAM_42_MEDIUM_NUMBERS"
#define FONT_KEY_BITHAM_34_MEDIUM_NUMBERS   "RESOURCE_ID_BITHAM_34_MEDIUM_NUMBERS"
#define FONT_KEY_BITHAM_34_LIGHT_SUBSET     "RESOURCE_ID_BITHAM_34_LIGHT_SUBSET"
#define FONT_KEY_BITHAM_18_LIGHT_SUBSET     "RESOURCE_ID_BITHAM_18_LIGHT_SUBSET"

#define FONT_KEY_ROBOTO_CONDENSED_21        "RESOURCE_ID_ROBOTO_CONDENSED_21"
#define FONT_KEY_ROBOTO_BOLD_SUBSET_49      "RESOURCE_ID_ROBOTO_BOLD_SUBSET_49"
#define FONT_KEY_DROID_SERIF_28_BOLD        "RESOURCE_ID_DROID_SERIF_28_BOLD"

#define FONT_KEY_LECO_20_BOLD_NUMBERS       "RESOURCE_ID_LECO_20_BOLD_NUMBERS"
#define FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM "RESOURCE_ID_LECO_26_BOLD_NUMBERS_AM_PM"
#define FONT_KEY_LECO_28_LIGHT_NUMBERS      "RESOURCE_ID_LECO_28_LIGHT_NUMBERS"
#define FONT_KEY_LECO_32_BOLD_NUMBERS       "RESOURCE_ID_LECO_32_BOLD_NUMBERS"
#define FONT_KEY_LECO_36_BOLD_NUMBERS       "RESOURCE_ID_LECO_36_BOLD_NUMBERS"
#define FONT_KEY_LECO_38_BOLD_NUMBERS       "RESOURCE_ID_LECO_38_BOLD_NUMBERS"
#define FONT_KEY_LECO_42_NUMBERS            "RESOURCE_ID_LECO_42_NUMBERS"

#define FONT_KEY_AGENCY_FB_60_THIN_NUMBERS_AM_PM "RESOURCE_ID_AGENCY_FB_60_THIN_NUMBERS_AM_PM"
#define FONT_KEY_AGENCY_FB_60_NUMBERS_AM_PM "RESOURCE_ID_AGENCY_FB_60_NUMBERS_AM_PM"
#define FONT_KEY_AGENCY_FB_36_NUMBERS_AM_PM "RESOURCE_ID_AGENCY_FB_36_NUMBERS_AM_PM"


// NOTE: These fonts are not exposed on current pebble platform
// they are internal. As such the licensing on these is sketchy at best
#define FONT_KEY_AGENCY_FB_60_THIN_NUMBERS_AM_PM_ID 468
#define FONT_KEY_AGENCY_FB_60_NUMBERS_AM_PM_ID      469
#define FONT_KEY_AGENCY_FB_36_NUMBERS_AM_PM_ID      470

// These fonts are exposed normally and have permissive use
#define FONT_KEY_GOTHIC_09_ID                     32
#define FONT_KEY_GOTHIC_14_ID                     33
#define FONT_KEY_GOTHIC_14_BOLD_ID                35
#define FONT_KEY_GOTHIC_18_ID                     36
#define FONT_KEY_GOTHIC_18_BOLD_ID                7
#define FONT_KEY_GOTHIC_24_ID                     38
#define FONT_KEY_GOTHIC_24_BOLD_ID                39
#define FONT_KEY_GOTHIC_28_ID                     41
#define FONT_KEY_GOTHIC_28_BOLD_ID                42
#define FONT_KEY_GOTHIC_36_ID                     44

#define FONT_KEY_BITHAM_18_LIGHT_SUBSET_ID        51
#define FONT_KEY_BITHAM_34_LIGHT_SUBSET_ID        52
#define FONT_KEY_BITHAM_30_BLACK_ID               46
#define FONT_KEY_BITHAM_42_BOLD_ID                47
#define FONT_KEY_BITHAM_42_LIGHT_ID               48

#define FONT_KEY_BITHAM_34_MEDIUM_NUMBERS_ID      50
#define FONT_KEY_BITHAM_42_MEDIUM_NUMBERS_ID      49

#define FONT_KEY_ROBOTO_CONDENSED_21_ID           53
#define FONT_KEY_ROBOTO_BOLD_SUBSET_49_ID         54
#define FONT_KEY_DROID_SERIF_28_BOLD_ID           55

#define FONT_KEY_LECO_20_BOLD_NUMBERS_ID          56
#define FONT_KEY_LECO_26_BOLD_NUMBERS_AM_PM_ID    57
#define FONT_KEY_LECO_32_BOLD_NUMBERS_ID          58
#define FONT_KEY_LECO_36_BOLD_NUMBERS_ID          59
#define FONT_KEY_LECO_38_BOLD_NUMBERS_ID          60
#define FONT_KEY_LECO_28_LIGHT_NUMBERS_ID         62
#define FONT_KEY_LECO_42_NUMBERS_ID               61

#define FONT_KEY_FONT_FALLBACK_ID                 55

// 45 is a large blocky font. like a console font. I like it.
// Where are the emojis?


// 473 has timezones
