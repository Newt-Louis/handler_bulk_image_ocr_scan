import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1280
    height: 820
    minimumWidth: 1040
    minimumHeight: 680
    visible: true
    title: "AutoPhoto"
    color: "#f7f8fa"
    Material.theme: Material.Light
    Material.accent: Material.Teal
    Material.primary: Material.BlueGrey

    palette.window: "#f7f8fa"
    palette.windowText: "#111827"
    palette.base: "#ffffff"
    palette.alternateBase: "#f1f3f5"
    palette.text: "#111827"
    palette.button: "#ffffff"
    palette.buttonText: "#111827"
    palette.highlight: "#2563eb"
    palette.highlightedText: "#ffffff"

    property int selectedIndex: 0
    property string selectedPath: imageModel.count > 0 ? imageModel.itemPath(Math.min(selectedIndex, imageModel.count - 1)) : ""
    property url selectedUrl: imageModel.count > 0 ? imageModel.itemUrl(Math.min(selectedIndex, imageModel.count - 1)) : ""
    property string outputFolder: ""

    function localFilePath(url) {
        var text = String(url)
        if (text.indexOf("file://") === 0) {
            var path = decodeURIComponent(text.slice(7))
            // On Windows: file:///F:/path -> slice(7) = /F:/path
            if (path.length > 2 && path.charAt(0) === "/" && path.charAt(2) === ":") {
                path = path.slice(1)
            }
            return path
        }
        return text
    }

    onSelectedPathChanged: {
        if (selectedPath.length > 0) {
            previewController.requestPreview(selectedPath)
        } else {
            previewController.clear()
        }
    }

    FolderDialog {
        id: sourceDialog
        title: "Choose source folder"
        onAccepted: {
            imageModel.loadFolder(selectedFolder)
            selectedIndex = 0
        }
    }

    FolderDialog {
        id: outputDialog
        title: "Choose output folder"
        onAccepted: outputFolder = window.localFilePath(selectedFolder)
    }

    Connections {
        target: imageModel
        function onFolderLoadFailed(message) {
            messageDialog.text = message
            messageDialog.open()
        }
        function onCountChanged() {
            selectedIndex = 0
        }
    }

    Connections {
        target: previewController
        function onPreviewFailed(message) {
            messageDialog.text = message
            messageDialog.open()
        }
    }

    Connections {
        target: batchProcessor
        function onFailed(message) {
            messageDialog.text = message
            messageDialog.open()
        }
    }

    MessageDialog {
        id: messageDialog
        title: "AutoPhoto"
    }

    Rectangle {
        anchors.fill: parent
        color: "#f7f8fa"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 18
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            Layout.preferredHeight: 48
            spacing: 14

            Label {
                Layout.fillWidth: true
                text: "AutoPhoto"
                color: "#111827"
                font.pixelSize: 26
                font.weight: Font.DemiBold
            }

            Label {
                text: imageModel.count + " images"
                color: "#4b5563"
                font.pixelSize: 14
            }

        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 16

            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 14

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 8
                    color: "#ffffff"
                    border.color: "#d8dde3"
                    clip: true

                    CoverFlowView {
                        id: coverFlow
                        anchors.fill: parent
                        anchors.margins: 18
                        model: imageModel
                        currentIndex: window.selectedIndex
                        previewUrl: previewController.previewUrl
                        previewBusy: previewController.busy
                        onCurrentIndexChanged: window.selectedIndex = currentIndex
                    }

                    Column {
                        anchors.centerIn: parent
                        width: Math.min(parent.width - 48, 360)
                        spacing: 14
                        visible: imageModel.count === 0

                        Label {
                            width: parent.width
                            text: "Choose a folder"
                            color: "#111827"
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: 28
                            font.weight: Font.DemiBold
                        }

                        Label {
                            width: parent.width
                            text: "JPG, PNG, TIFF, WEBP and other Qt-supported image formats"
                            color: "#4b5563"
                            horizontalAlignment: Text.AlignHCenter
                            wrapMode: Text.WordWrap
                            font.pixelSize: 14
                        }

                        Button {
                            text: "Choose source"
                            anchors.horizontalCenter: parent.horizontalCenter
                            onClicked: sourceDialog.open()
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 176
                    radius: 8
                    color: "#ffffff"
                    border.color: "#d8dde3"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 14

                        Rectangle {
                            Layout.preferredWidth: 250
                            Layout.fillHeight: true
                            radius: 6
                            color: "#f3f4f6"
                            border.color: "#d8dde3"
                            clip: true

                            Image {
                                anchors.fill: parent
                                anchors.margins: 10
                                source: selectedUrl
                                fillMode: Image.PreserveAspectFit
                                asynchronous: true
                                autoTransform: false
                                cache: false
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            spacing: 9

                            Label {
                                Layout.fillWidth: true
                                text: imageModel.count > 0 ? imageModel.itemPath(selectedIndex) : "No image selected"
                                color: "#111827"
                                elide: Text.ElideMiddle
                                font.pixelSize: 14
                            }

                            ProgressBar {
                                Layout.fillWidth: true
                                value: batchProcessor.progress / 100.0
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 12

                                Label {
                                    Layout.fillWidth: true
                                    text: batchProcessor.statusText
                                    color: batchProcessor.failedImages > 0 ? "#b45309" : "#4b5563"
                                    elide: Text.ElideRight
                                    font.pixelSize: 13
                                }

                                Label {
                                    text: batchProcessor.processedImages + "/" + batchProcessor.totalImages
                                    color: "#374151"
                                    font.pixelSize: 13
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10

                                Button {
                                    Layout.preferredWidth: 104
                                    text: "Start"
                                    enabled: imageModel.count > 0 && outputFolder.length > 0 && !batchProcessor.running
                                    onClicked: {
                                        batchProcessor.start(
                                            imageModel.paths(),
                                            outputFolder,
                                            toolPanel.renamePattern,
                                            previewController.rotateEnabled,
                                            previewController.blurFaces,
                                            previewController.blurMode,
                                            previewController.strength,
                                            previewController.detectionSensitivity,
                                            previewController.sizeFilterEnabled,
                                            previewController.skinColorFilterEnabled,
                                            previewController.cascadeCrossCheckEnabled,
                                            previewController.compressionEnabled,
                                            previewController.compressionLevel,
                                            previewController.outputFormat,
                                            previewController.timestampEnabled,
                                            previewController.timestampFormat,
                                            previewController.timestampPosition,
                                            previewController.timestampColor,
                                            previewController.timestampSize,
                                            previewController.timestampX,
                                            previewController.timestampY
                                        )
                                    }
                                }

                                Button {
                                    Layout.preferredWidth: 104
                                    text: batchProcessor.paused ? "Resume" : "Pause"
                                    enabled: batchProcessor.running
                                    onClicked: batchProcessor.paused ? batchProcessor.resume() : batchProcessor.pause()
                                }

                                Button {
                                    Layout.preferredWidth: 104
                                    text: "Stop"
                                    enabled: batchProcessor.running
                                    onClicked: batchProcessor.stop()
                                }

                                Item { Layout.fillWidth: true }

                                Label {
                                    text: batchProcessor.workerCount > 0 ? batchProcessor.workerCount + " workers" : "Ready"
                                    color: "#0f766e"
                                    font.pixelSize: 13
                                }
                            }
                        }
                    }
                }
            }

            ToolPanel {
                id: toolPanel
                Layout.preferredWidth: 344
                Layout.fillHeight: true
                imageCount: imageModel.count
                outputFolder: window.outputFolder
                blurFaces: previewController.blurFaces
                blurMode: previewController.blurMode
                strength: previewController.strength
                detectionSensitivity: Math.round(previewController.detectionSensitivity * 100)
                sizeFilterEnabled: previewController.sizeFilterEnabled
                skinColorFilterEnabled: previewController.skinColorFilterEnabled
                cascadeCrossCheckEnabled: previewController.cascadeCrossCheckEnabled
                compressionLevel: previewController.compressionLevel
                outputFormat: previewController.outputFormat
                rotateEnabled: previewController.rotateEnabled
                compressionEnabled: previewController.compressionEnabled
                timestampEnabled: previewController.timestampEnabled
                timestampFormat: previewController.timestampFormat
                timestampPosition: previewController.timestampPosition
                timestampColor: previewController.timestampColor
                timestampSize: previewController.timestampSize
                timestampX: previewController.timestampX
                timestampY: previewController.timestampY
                running: batchProcessor.running
                paused: batchProcessor.paused
                progress: batchProcessor.progress
                processedImages: batchProcessor.processedImages
                failedImages: batchProcessor.failedImages
                totalImages: batchProcessor.totalImages
                workerCount: batchProcessor.workerCount
                statusText: batchProcessor.statusText
                onChooseSource: sourceDialog.open()
                onChooseOutput: outputDialog.open()
                onBlurFacesChanged: previewController.blurFaces = blurFaces
                onBlurModeChanged: previewController.blurMode = blurMode
                onStrengthChanged: previewController.strength = strength
                onDetectionSensitivityChanged: previewController.detectionSensitivity = detectionSensitivity / 100.0
                onSizeFilterEnabledChanged: previewController.sizeFilterEnabled = sizeFilterEnabled
                onSkinColorFilterEnabledChanged: previewController.skinColorFilterEnabled = skinColorFilterEnabled
                onCascadeCrossCheckEnabledChanged: previewController.cascadeCrossCheckEnabled = cascadeCrossCheckEnabled
                onCompressionLevelChanged: previewController.compressionLevel = compressionLevel
                onOutputFormatChanged: previewController.outputFormat = outputFormat
                onRotateEnabledChanged: previewController.rotateEnabled = rotateEnabled
                onCompressionEnabledChanged: previewController.compressionEnabled = compressionEnabled
                onTimestampEnabledChanged: previewController.timestampEnabled = timestampEnabled
                onTimestampFormatChanged: previewController.timestampFormat = timestampFormat
                onTimestampPositionChanged: previewController.timestampPosition = timestampPosition
                onTimestampColorChanged: previewController.timestampColor = timestampColor
                onTimestampSizeChanged: previewController.timestampSize = timestampSize
                onTimestampXChanged: previewController.timestampX = timestampX
                onTimestampYChanged: previewController.timestampY = timestampY
                onPauseBatch: batchProcessor.pause()
                onResumeBatch: batchProcessor.resume()
                onStopBatch: batchProcessor.stop()
            }
        }
    }
}
