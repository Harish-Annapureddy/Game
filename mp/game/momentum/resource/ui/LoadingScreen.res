"resource/ui/LoadingScreen.res"
{
    "LoadingScreen"
    {
        "ControlName"		"EditablePanel"
        "fieldName"		"LoadingScreen"
        "xpos"		"0"
        "ypos"		"0"
        "proportionalToParent" "1"
        "wide" "f0"
        "tall" "f0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"0"
        "enabled"		"1"
        "tabPosition"		"0"
        "BgColor_override" "Blank"
        "MapImageOpacityMax" "0.5"
    }

    "MapNameLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"MapNameLabel"
        "xpos"		"0"
        "ypos"		"4"
        "wide"		"f0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "BgColor"		"0 0 0 0"
        "allowColorOverrides"		"0"
        "font" "LoadingLarge"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"10"
        "textinsety"		"0"
        "auto_tall_tocontents" "1"
    }

    "AuthorsLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"AuthorsLabel"
        "xpos"		"0"
        "ypos"		"0"
        "wide"		"f0"
        "pin_to_sibling" "MapNameLabel"
        "pin_corner_to_sibling" "0"
        "pin_to_sibling_corner" "2"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "font" "LoadingSmall"
        "tabPosition"		"0"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"10"
        "textinsety"		"0"
        "auto_tall_tocontents" "1"
    }

    "DifficultyLayoutLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"DifficultyLayoutLabel"
        "xpos"		"0"
        "ypos"		"4"
        "wide"		"f0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "BgColor"		"0 0 0 0"
        "allowColorOverrides"		"0"
        "font" "LoadingLarge"
        "textAlignment"		"east"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"10"
        "textinsety"		"0"
        "auto_tall_tocontents" "1"
    }

    "TrackZonesLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"TrackZonesLabel"
        "xpos"		"0"
        "ypos"		"0"
        "wide"		"f0"
        "pin_to_sibling" "DifficultyLayoutLabel"
        "pin_corner_to_sibling" "0"
        "pin_to_sibling_corner" "2"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "font" "LoadingSmall"
        "tabPosition"		"0"
        "textAlignment"		"east"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"10"
        "textinsety"		"0"
        "auto_tall_tocontents" "1"
    }

    "TipLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"TipLabel"
        "xpos"		"0"
        "ypos"		"10"
        "wide"		"f0"
        "pin_to_sibling" "LoadingPercentLabel"
        "pin_corner_to_sibling" "2"
        "pin_to_sibling_corner" "0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "font" "LoadingTip"
        "tabPosition"		"0"
        "textAlignment"		"center"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"1"
        "centerwrap"		"1"
        "textinsetx"		"18"
        "textinsety"		"0"
        "auto_tall_tocontents" "1"
    }

    "LoadingPercentLabel"
    {
        "ControlName"		"Label"
        "fieldName"		"LoadingPercentLabel"
        "xpos"		"0"
        "ypos"		"0"  
        "wide"		"f0"
        "pin_to_sibling" "ProgressBar"
        "pin_corner_to_sibling" "2"
        "pin_to_sibling_corner" "0"
        "autoResize"		"0"
        "font" "LoadingLarge"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "textAlignment"		"west"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"10"
        "textinsety"		"0"
        "auto_tall_tocontents" "1"
    }

    "ProgressBar"
    {
        "ControlName"		"ContinuousProgressBar"
        "fieldName"		"ProgressBar"
        "xpos"		"0"
        "ypos"		"rs1.0"
        "wide"		"f0"
        "tall"		"10"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "textAlignment"		"east"
        "dulltext"		"0"
        "brighttext"		"0"
        "wrap"		"0"
        "centerwrap"		"0"
        "textinsetx"		"0"
        "textinsety"		"0"
    }

    "MapImagePanel"
    {
        "ControlName"		"ImagePanel"
        "fieldName"		"MapImagePanel"
        "xpos"		"0"
        "ypos"		"0"
        "zpos"      "-50"
        "wide"		"f0"
        "tall"		"f0"
        "autoResize"		"0"
        "pinCorner"		"0"
        "visible"		"1"
        "enabled"		"1"
        "tabPosition"		"0"
        "scaleImage" "1"
        "fillcolor" "MomGreydientStep1"
    }
}