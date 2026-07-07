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
    property int detectionSensitivity: 50
    property bool sizeFilterEnabled: true
    property bool skinColorFilterEnabled: true
    property bool cascadeCrossCheckEnabled: true
    property int compressionLevel: 0
    property string outputFormat: "jpg"
    property bool rotateEnabled: true
    property bool compressionEnabled: true
    property string renamePattern: "autophoto"
    property bool timestampEnabled: false
    property string timestampFormat: "yyyy-MM-dd HH:mm:ss"
    property string timestampPosition: "BottomRight"
    property string timestampColor: "#FFFFFF"
    property int timestampSize: 24
    property int timestampX: 10
    property int timestampY: 10
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

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
            policy: ScrollBar.AsNeeded
            interactive: true
        }

        Column {
            id: contentColumn
            width: flickable.width - 12
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
                title: "Rotate"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    Switch {
                        text: "Auto-rotate (EXIF)"
                        checked: root.rotateEnabled
                        enabled: !root.running
                        font.pixelSize: 12
                        onToggled: root.rotateEnabled = checked
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.rotateEnabled ? "Based on EXIF orientation metadata" : "Disabled - images keep original orientation"
                        color: root.rotateEnabled ? "#4b5563" : "#9ca3af"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
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
                        id: blurToggle
                        text: "Enable face blur"
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
                            color: root.blurFaces ? "#374151" : "#9ca3af"
                            font.pixelSize: 12
                        }

                        Label {
                            text: root.strength
                            color: root.blurFaces ? "#0f766e" : "#9ca3af"
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
                            color: root.blurFaces ? "#374151" : "#9ca3af"
                            font.pixelSize: 12
                        }

                        Label {
                            text: root.detectionSensitivity + "%"
                            color: root.blurFaces ? "#0f766e" : "#9ca3af"
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
                        color: root.blurFaces ? "#374151" : "#9ca3af"
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

                    Switch {
                        text: "Enable compression"
                        checked: root.compressionEnabled
                        enabled: !root.running
                        font.pixelSize: 12
                        onToggled: root.compressionEnabled = checked
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Label {
                            Layout.fillWidth: true
                            text: "Compression"
                            color: root.compressionEnabled ? "#374151" : "#9ca3af"
                            font.pixelSize: 12
                        }

                        Label {
                            text: root.compressionLevel + "%"
                            color: root.compressionEnabled ? "#0f766e" : "#9ca3af"
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
                        enabled: !root.running && root.compressionEnabled
                        onMoved: root.compressionLevel = Math.round(value)
                    }

                    Label {
                        Layout.fillWidth: true
                        text: {
                            if (!root.compressionEnabled) return "Compression disabled"
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
                        currentIndex: {
                            var fmt = root.outputFormat.toLowerCase();
                            if (fmt === "png") return 1;
                            if (fmt === "webp") return 2;
                            return 0;
                        }
                        enabled: !root.running && root.compressionEnabled
                        font.pixelSize: 12
                        onActivated: function(index) {
                            root.outputFormat = model[index].toLowerCase();
                        }
                    }
                }
            }
        }
    }}
