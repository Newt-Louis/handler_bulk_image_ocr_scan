import QtQuick
import QtQuick.Controls

Item {
    id: root
    property alias model: pathView.model
    property alias currentIndex: pathView.currentIndex
    property url previewUrl: ""
    property bool previewBusy: false

    PathView {
        id: pathView
        anchors.fill: parent
        preferredHighlightBegin: 0.5
        preferredHighlightEnd: 0.5
        highlightRangeMode: PathView.StrictlyEnforceRange
        snapMode: PathView.SnapToItem
        pathItemCount: 7
        interactive: count > 1
        focus: true
        cacheItemCount: 8

        path: Path {
            startX: 70
            startY: root.height * 0.58

            PathAttribute { name: "itemScale"; value: 0.56 }
            PathAttribute { name: "itemOpacity"; value: 0.32 }
            PathAttribute { name: "itemAngle"; value: -38 }
            PathAttribute { name: "itemZ"; value: 0 }

            PathCubic {
                x: root.width * 0.5
                y: root.height * 0.37
                control1X: root.width * 0.18
                control1Y: root.height * 0.72
                control2X: root.width * 0.33
                control2Y: root.height * 0.35
            }

            PathAttribute { name: "itemScale"; value: 1.0 }
            PathAttribute { name: "itemOpacity"; value: 1.0 }
            PathAttribute { name: "itemAngle"; value: 0 }
            PathAttribute { name: "itemZ"; value: 10 }

            PathCubic {
                x: root.width - 70
                y: root.height * 0.58
                control1X: root.width * 0.67
                control1Y: root.height * 0.35
                control2X: root.width * 0.82
                control2Y: root.height * 0.72
            }

            PathAttribute { name: "itemScale"; value: 0.56 }
            PathAttribute { name: "itemOpacity"; value: 0.32 }
            PathAttribute { name: "itemAngle"; value: 38 }
            PathAttribute { name: "itemZ"; value: 0 }
        }

        delegate: Item {
            id: tile
            width: Math.min(root.width * 0.44, 500)
            height: Math.min(root.height * 0.72, 440)
            scale: PathView.itemScale
            opacity: PathView.itemOpacity
            z: PathView.itemZ

            transform: Rotation {
                origin.x: tile.width / 2
                origin.y: tile.height / 2
                axis { x: 0; y: 1; z: 0 }
                angle: PathView.itemAngle
            }

            Behavior on scale {
                NumberAnimation { duration: 220; easing.type: Easing.OutCubic }
            }

            Behavior on opacity {
                NumberAnimation { duration: 180; easing.type: Easing.OutCubic }
            }

            Rectangle {
                anchors.fill: parent
                radius: 8
                color: "#ffffff"
                border.color: index === pathView.currentIndex ? "#2563eb" : "#cfd6dd"
                border.width: index === pathView.currentIndex ? 2 : 1

                Image {
                    id: image
                    anchors.fill: parent
                    anchors.margins: 10
                    source: index === pathView.currentIndex && root.previewUrl.toString().length > 0
                        ? root.previewUrl
                        : fileUrl
                    fillMode: Image.PreserveAspectFit
                    asynchronous: true
                    autoTransform: false
                    smooth: true
                    mipmap: true
                    cache: index !== pathView.currentIndex
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: root.previewBusy && index === pathView.currentIndex
                    visible: running
                }

                Rectangle {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.bottom: parent.bottom
                    height: 44
                    radius: 8
                    color: "#e6ffffff"

                    Label {
                        anchors.fill: parent
                        anchors.leftMargin: 12
                        anchors.rightMargin: 12
                        verticalAlignment: Text.AlignVCenter
                        text: fileName
                        color: "#111827"
                        elide: Text.ElideMiddle
                        font.pixelSize: 13
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: pathView.currentIndex = index
                }
            }
        }

        Keys.onLeftPressed: decrementCurrentIndex()
        Keys.onRightPressed: incrementCurrentIndex()
        Keys.onSpacePressed: forceActiveFocus()
    }

    WheelHandler {
        target: pathView
        onWheel: function(event) {
            if (event.angleDelta.y < 0) {
                pathView.incrementCurrentIndex()
            } else if (event.angleDelta.y > 0) {
                pathView.decrementCurrentIndex()
            }
            event.accepted = true
        }
    }
}
