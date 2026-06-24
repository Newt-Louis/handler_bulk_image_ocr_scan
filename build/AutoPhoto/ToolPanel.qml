import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: root
    radius: 8
    color: "#ffffff"
    border.color: "#d8dde3"

    property int imageCount: 0
    property string outputFolder: ""
    property bool blurFaces: true
    property string blurMode: "gaussian"
    property int strength: 100
    property int detectionSensitivity: 35
    property bool sizeFilterEnabled: true
    property bool skinColorFilterEnabled: true
    property bool cascadeCrossCheckEnabled: true
    property int compressionLevel: 0
    property string outputFormat: "jpg"
    property string renamePattern: "autophoto"
    property bool running: false
    property bool paused: false
    property int progress: 0
    property int processedImages: 0
    property int failedImages: 0
    property int totalImages: 0
    property int workerCount: 0
    property string statusText: "Idle"

    readonly property bool canStart: imageCount > 0 && outputFolder.length > 0 && !running

    signal chooseSource()
    signal chooseOutput()
    signal pauseBatch()
    signal resumeBatch()
    signal stopBatch()

    Material.theme: Material.Light
    Material.accent: Material.Teal

    Flickable {
        id: flickable
        anchors.fill: parent
        anchors.margins: 12
        contentHeight: contentColumn.implicitHeight
        clip: true
        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds

        Column {
            id: contentColumn
            width: flickable.width
            spacing: 10

            Label {
                width: parent.width
                text: "Batch Setup"
                color: "#111827"
                font.pixelSize: 20
                font.weight: Font.DemiBold
            }

            GroupBox {
                width: parent.width
                title: "Images"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    Button {
                        Layout.fillWidth: true
                        text: "Choose source folder"
                        enabled: !root.running
                        onClicked: root.chooseSource()
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.imageCount + " image(s) loaded"
                        color: "#4b5563"
                        elide: Text.ElideRight
                        font.pixelSize: 12
                    }
                }
            }

            GroupBox {
                width: parent.width
                title: "Output"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    Button {
                        Layout.fillWidth: true
                        text: "Choose output folder"
                        enabled: !root.running
                        onClicked: root.chooseOutput()
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.outputFolder.length > 0 ? root.outputFolder : "No output folder"
                        color: root.outputFolder.length > 0 ? "#4b5563" : "#b45309"
                        elide: Text.ElideMiddle
                        font.pixelSize: 11
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: root.renamePattern
                        enabled: !root.running
                        placeholderText: "Rename pattern"
                        selectByMouse: true
                        font.pixelSize: 12
                        onTextChanged: root.renamePattern = text
                    }
                }
            }

            GroupBox {
                width: parent.width
                title: "Face Blur"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    Switch {
                        text: "Blur faces"
                        checked: root.blurFaces
                        enabled: !root.running
                        font.pixelSize: 12
                        onToggled: root.blurFaces = checked
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        ButtonGroup { id: blurGroup }

                        RadioButton {
                            Layout.fillWidth: true
                            text: "Gaussian"
                            checked: root.blurMode !== "pixelate"
                            enabled: !root.running && root.blurFaces
                            font.pixelSize: 11
                            ButtonGroup.group: blurGroup
                            onClicked: root.blurMode = "gaussian"
                        }

                        RadioButton {
                            Layout.fillWidth: true
                            text: "Pixelate"
                            checked: root.blurMode === "pixelate"
                            enabled: !root.running && root.blurFaces
                            font.pixelSize: 11
                            ButtonGroup.group: blurGroup
                            onClicked: root.blurMode = "pixelate"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Label {
                            Layout.fillWidth: true
                            text: "Strength"
                            color: "#374151"
                            font.pixelSize: 12
                        }

                        Label {
                            text: root.strength
                            color: "#0f766e"
                            font.weight: Font.DemiBold
                            font.pixelSize: 12
                        }
                    }

                    Slider {
                        Layout.fillWidth: true
                        from: 1
                        to: 100
                        stepSize: 1
                        value: root.strength
                        enabled: !root.running && root.blurFaces
                        onMoved: root.strength = Math.round(value)
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Label {
                            Layout.fillWidth: true
                            text: "Detection sensitivity"
                            color: "#374151"
                            font.pixelSize: 12
                        }

                        Label {
                            text: root.detectionSensitivity + "%"
                            color: "#0f766e"
                            font.weight: Font.DemiBold
                            font.pixelSize: 12
                        }
                    }

                    Slider {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        stepSize: 1
                        value: root.detectionSensitivity
                        enabled: !root.running && root.blurFaces
                        onMoved: root.detectionSensitivity = Math.round(value)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: "False positive filters"
                        color: "#374151"
                        font.weight: Font.DemiBold
                        font.pixelSize: 12
                        topPadding: 2
                    }

                    Switch {
                        text: "Size filter"
                        checked: root.sizeFilterEnabled
                        enabled: !root.running && root.blurFaces
                        font.pixelSize: 12
                        onToggled: root.sizeFilterEnabled = checked
                    }

                    Switch {
                        text: "Skin-color filter"
                        checked: root.skinColorFilterEnabled
                        enabled: !root.running && root.blurFaces
                        font.pixelSize: 12
                        onToggled: root.skinColorFilterEnabled = checked
                    }

                    Switch {
                        text: "Cascade cross-check"
                        checked: root.cascadeCrossCheckEnabled
                        enabled: !root.running && root.blurFaces
                        font.pixelSize: 12
                        onToggled: root.cascadeCrossCheckEnabled = checked
                    }
                }
            }

            GroupBox {
                width: parent.width
                title: "Image Compression"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Label {
                            Layout.fillWidth: true
                            text: "Compression"
                            color: "#374151"
                            font.pixelSize: 12
                        }

                        Label {
                            text: root.compressionLevel + "%"
                            color: "#0f766e"
                            font.weight: Font.DemiBold
                            font.pixelSize: 12
                        }
                    }

                    Slider {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        stepSize: 1
                        value: root.compressionLevel
                        enabled: !root.running
                        onMoved: root.compressionLevel = Math.round(value)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: {
                            if (root.compressionLevel <= 0) return "No compression"
                            if (root.compressionLevel <= 25) return "~30% smaller"
                            if (root.compressionLevel <= 50) return "~55% smaller"
                            if (root.compressionLevel <= 75) return "~75% smaller"
                            return "~90% smaller"
                        }
                        color: "#6b7280"
                        font.pixelSize: 11
                    }

                    ComboBox {
                        Layout.fillWidth: true
                        id: formatCombo
                        model: ["JPG", "PNG", "WEBP"]
                        enabled: !root.running
                        font.pixelSize: 12
                        onCurrentIndexChanged: root.outputFormat = model[currentIndex].toLowerCase()
                    }
                }
            }
        }
    }

    ScrollBar {
        anchors.right: flickable.right
        anchors.top: flickable.top
        anchors.bottom: flickable.bottom
        anchors.margins: 2
        width: 6
        policy: ScrollBar.AsNeeded
        size: flickable.height / flickable.contentHeight
        position: flickable.originY / (flickable.contentHeight - flickable.height)
    }
}
