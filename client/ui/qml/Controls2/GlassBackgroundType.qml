import QtQuick
import QtQuick.Shapes

import Style 1.0

Rectangle {
    id: root

    color: AmneziaStyle.color.bgBaseGlass
    clip: true

    Shape {
        anchors.fill: parent

        ShapePath {
            strokeWidth: -1
            strokeColor: AmneziaStyle.color.transparent
            fillGradient: RadialGradient {
                centerX: root.width * 0.1
                centerY: root.height * 0.05
                centerRadius: Math.max(root.width, root.height) * 0.55
                focalX: centerX
                focalY: centerY

                GradientStop { position: 0.0; color: AmneziaStyle.color.glowPrimary }
                GradientStop { position: 1.0; color: AmneziaStyle.color.transparent }
            }

            startX: 0
            startY: 0
            PathLine { x: root.width; y: 0 }
            PathLine { x: root.width; y: root.height }
            PathLine { x: 0; y: root.height }
            PathLine { x: 0; y: 0 }
        }

        ShapePath {
            strokeWidth: -1
            strokeColor: AmneziaStyle.color.transparent
            fillGradient: RadialGradient {
                centerX: root.width * 0.95
                centerY: root.height * 0.45
                centerRadius: Math.max(root.width, root.height) * 0.5
                focalX: centerX
                focalY: centerY

                GradientStop { position: 0.0; color: AmneziaStyle.color.glowSecondary }
                GradientStop { position: 1.0; color: AmneziaStyle.color.transparent }
            }

            startX: 0
            startY: 0
            PathLine { x: root.width; y: 0 }
            PathLine { x: root.width; y: root.height }
            PathLine { x: 0; y: root.height }
            PathLine { x: 0; y: 0 }
        }

        ShapePath {
            strokeWidth: -1
            strokeColor: AmneziaStyle.color.transparent
            fillGradient: RadialGradient {
                centerX: root.width * 0.3
                centerY: root.height * 1.0
                centerRadius: Math.max(root.width, root.height) * 0.45
                focalX: centerX
                focalY: centerY

                GradientStop { position: 0.0; color: AmneziaStyle.color.glowTertiary }
                GradientStop { position: 1.0; color: AmneziaStyle.color.transparent }
            }

            startX: 0
            startY: 0
            PathLine { x: root.width; y: 0 }
            PathLine { x: root.width; y: root.height }
            PathLine { x: 0; y: root.height }
            PathLine { x: 0; y: 0 }
        }
    }
}
