"resource/ui/SettingsPanel_ControlsSettings.res"
{
    //This is the base panel that the scroll panel encapsulates.
    //Wide can be whatever, but tall should be as close to what you need
    // as possible.
    "ControlsSettings"
    {
        "ControlName" "SettingsPanel"
        "fieldName" "ControlsSettings"
        "tall" "255"
        "wide" "1000"
    }
    
    
    //Individual controls are below
    
    "YawSpeed"
    {
        "ControlName" "CCvarSlider"
        "fieldName" "YawSpeed"
        "xpos" "4"
        "ypos" "16"
        "wide"		"200"
		"tall"		"40"
		"autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"leftText"		"#GameUI_Low"
		"rightText"		"#GameUI_High"
        "minvalue" "0"
        "maxvalue" "300"
        "cvar_name" "cl_yawspeed"
        "allowoutofrange" "0"
    }
    
    "YawSpeedEntry"
    {
		"ControlName"		"TextEntry"
		"fieldName"		"YawSpeedEntry"
		"xpos"		"206"
		"ypos"		"16"
		"wide"		"35"
		"tall"		"15"
        "font" "DefaultSmall"
		"autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"3"
		"textHidden"		"0"
		"editable"		"1"
		"maxchars"		"-1"
		"NumericInputOnly"		"1"
		"unicode"		"0"
	}
    
    "YawSpeedLabel"
    {
        "ControlName" "Label"
        "fieldName" "YawSpeedLabel"
        "xpos" "4"
        "ypos" "4"
        "autoResize"		"0"
		"pinCorner"		"0"
		"RoundedCorners"		"15"
		"pin_corner_to_sibling"		"0"
		"pin_to_sibling_corner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"tabPosition"		"0"
		"labelText"		"#MOM_Settings_Yaw_Speed"
		"textAlignment"		"west"
		"dulltext"		"0"
		"brighttext"		"0"
		"wrap"		"0"
		"centerwrap"		"0"
		"textinsetx"		"0"
		"textinsety"		"0"
		"auto_wide_tocontents" "1"
		"use_proportional_insets"		"0"
    }
}