pragma Singleton

import QtQuick

QtObject {
    property QtObject color: QtObject {
        readonly property color transparent: 'transparent'

        readonly property color textPrimary: '#F7F3EE'
        readonly property color textSecondary: '#E6DFD7'
        readonly property color textTertiary: '#A8A097'
        readonly property color textQuaternary: '#8A827A'
        readonly property color textInverted: '#170B06'
        readonly property color textAccent: '#FFB08F'
        readonly property color textInfoInverse: '#0369A1'
        readonly property color textStaticWhite: '#FFFFFF'

        readonly property color bgBase: '#0D0B0A'
        readonly property color bgSurface: '#B8201C19'
        readonly property color bgSurfaceAlt: '#C72A2521'
        readonly property color bgSheet: '#F51C1916'
        readonly property color bgPanel: '#D9332D28'
        readonly property color bgPanelAlt: '#E0443C35'
        readonly property color bgOverlay: '#99060504'
        readonly property color bgStateLayer: '#14FFF3EA'
        readonly property color bgStateLayerPressed: '#1FFFF3EA'
        readonly property color bgStateLayerAccent: '#29FF7A4D'
        readonly property color bgSurfaceInverse: '#F2E9E1'
        readonly property color bgSurfaceInverseAlt: '#E2D6CB'
        readonly property color bgControlTrack: '#B32A2521'
        readonly property color bgField: '#99171412'
        readonly property color bgSelected: '#33FF7A4D'
        readonly property color bgAccentSubtle: '#4DFF7A4D'
        readonly property color bgStaticWhite: '#FFFFFF'

        readonly property color bgAction: '#FF7A4D'
        readonly property color bgActionHover: '#FF916A'
        readonly property color bgActionPress: '#E8623A'

        readonly property color borderFaint: '#1AFFF3EA'
        readonly property color borderSoft: '#2BFFF3EA'
        readonly property color borderMedium: '#45FFF3EA'
        readonly property color borderBold: '#A8A097'
        readonly property color borderEmphasis: '#F2E9E1'
        readonly property color borderAccent: '#FF8A5B'
        readonly property color borderField: '#3DFFF3EA'
        readonly property color borderFocus: '#FF9E7A'

        readonly property color accentLight: '#FFB08F'
        readonly property color accentDefault: '#FF7A4D'
        readonly property color accentDark: '#E0582E'
        readonly property color accentDeep: '#5A1F0E'

        readonly property color statusDestructive: '#FF6B7A'
        readonly property color statusWarning: '#FFC94D'
        readonly property color statusSuccess: '#4DF5A3'

        readonly property color promoRecommendation: '#B69CFF'
        readonly property color promoUpdate: '#4FD8FF'
        readonly property color promoOverlay: '#1FB69CFF'
        readonly property color promoPink: '#FF4FA3'
        readonly property color promoPinkDeep: '#A3125E'

        readonly property color iconDefault: '#EDE6DE'
        readonly property color iconSecondary: '#A8A097'
        readonly property color iconTertiary: '#4A423B'
        readonly property color iconInverted: '#170B06'

        readonly property color shadowLayer1: '#26000000'
        readonly property color shadowLayer2: '#4D000000'
        readonly property color shadowGlow: '#FF7A4D'

        readonly property color glowPrimary: '#59FF7A4D'
        readonly property color glowSecondary: '#40A26BFF'
        readonly property color glowTertiary: '#334FD8FF'
    }
}
