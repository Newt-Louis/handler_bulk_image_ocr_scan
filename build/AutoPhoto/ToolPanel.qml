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
    property bool cascadeCrossCheckEnabled: false
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

    ScrollView {
        anchors.fill: parent
        anchors.margins: 16
        clip: true

        ColumnLayout {
            width: Math.max(root.width - 32, 280)
            spacing: 16

            Label {
                Layout.fillWidth: true
                text: "Batch Setup"
                color: "#111827"
                font.pixelSize: 22
                font.weight: Font.DemiBold
            }

            GroupBox {
                Layout.fillWidth: true
                title: "Images"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

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
                    }
                }
            }

            GroupBox {
                Layout.fillWidth: true
                title: "Output"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

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
                    }

                    TextField {
                        Layout.fillWidth: true
                        text: root.renamePattern
                        enabled: !root.running
                        placeholderText: "Rename pattern"
                        selectByMouse: true
                        onTextChanged: root.renamePattern = text
                    }
                }
            }

            GroupBox {
                Layout.fillWidth: true
                title: "Face Blur"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 12

                    Switch {
                        text: "Blur faces"
                        checked: root.blurFaces
                        enabled: !root.running
                        onToggled: root.blurFaces = checked
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        ButtonGroup { id: blurGroup }

                        RadioButton {
                            Layout.fillWidth: true
                            text: "Gaussian"
                            checked: root.blurMode !== "pixelate"
                            enabled: !root.running && root.blurFaces
                            ButtonGroup.group: blurGroup
                            onClicked: root.blurMode = "gaussian"
                        }

                        RadioButton {
                            Layout.fillWidth: true
                            text: "Pixelate"
                            checked: root.blurMode === "pixelate"
                            enabled: !root.running && root.blurFaces
                            ButtonGroup.group: blurGroup
                            onClicked: root.blurMode = "pixelate"
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            text: "Strength"
                            color: "#374151"
                        }

                        Label {
                            text: root.strength
                            color: "#0f766e"
                            font.weight: Font.DemiBold
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
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            text: "Detection sensitivity"
                            color: "#374151"
                        }

                        Label {
                            text: root.detectionSensitivity + "%"
                            color: "#0f766e"
                            font.weight: Font.DemiBold
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
                        font.pixelSize: 13
                        topPadding: 4
                    }

                    Switch {
                        text: "Size filter"
                        checked: root.sizeFilterEnabled
                        enabled: !root.running && root.blurFaces
                        onToggled: root.sizeFilterEnabled = checked
                    }

                    Switch {
                        text: "Skin-color filter"
                        checked: root.skinColorFilterEnabled
                        enabled: !root.running && root.blurFaces
                        onToggled: root.skinColorFilterEnabled = checked
                    }

                    Switch {
                        text: "Cascade cross-check"
                        checked: root.cascadeCrossCheckEnabled
                        enabled: !root.running && root.blurFaces
                        onToggled: root.cascadeCrossCheckEnabled = checked
                    }
                }
            }

            GroupBox {
                Layout.fillWidth: true
                title: "Image Compression"

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 10

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Label {
                            Layout.fillWidth: true
                            text: "Compression"
                            color: "#374151"
                        }

                        Label {
                            text: root.compressionLevel + "%"
                            color: "#0f766e"
                            font.weight: Font.DemiBold
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
                        font.pixelSize: 12
                    }

                    ComboBox {
                        Layout.fillWidth: true
                        id: formatCombo
                        model: ["JPG", "PNG", "WEBP"]
                        enabled: !root.running
                        onCurrentIndexChanged: root.outputFormat = model[currentIndex].toLowerCase()
                    }
                }
            }
        }
    }
}
