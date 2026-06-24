#include "BatchProcessor.h"
#include "ImageListModel.h"
#include "ImageProcessor.h"
#include "PreviewController.h"
#include "SshTunnelController.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageReader>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QSet>
#include <QStringList>
#include <QtGlobal>
#include <QUrl>

namespace {

bool isSupportedImage(const QFileInfo &fileInfo)
{
    static const QSet<QByteArray> supportedFormats = [] {
        QSet<QByteArray> formats;
        const QList<QByteArray> readerFormats = QImageReader::supportedImageFormats();
        for (const QByteArray &format : readerFormats) {
            formats.insert(format.toLower());
        }
        return formats;
    }();

    return supportedFormats.contains(fileInfo.suffix().toLower().toUtf8());
}

QStringList collectImages(const QString &folderPath)
{
    QStringList images;
    QDirIterator iterator(folderPath, QDir::Files, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QFileInfo info(iterator.next());
        if (isSupportedImage(info)) {
            images.append(info.absoluteFilePath());
        }
    }
    images.sort(Qt::CaseInsensitive);
    return images;
}

QString outputPathFor(const QString &sourcePath, const QString &outputFolder, const QString &renamePattern, int index, const ProcessingOptions &options)
{
    const QFileInfo source(sourcePath);
    const QString baseName = renamePattern.isEmpty() ? source.completeBaseName() : renamePattern;
    const QString number = QString::number(index + 1).rightJustified(4, QLatin1Char('0'));
    const QString ext = (options.compressionLevel > 0 && options.outputFormat != QLatin1String("jpg"))
        ? options.outputFormat
        : (source.suffix().isEmpty() ? QStringLiteral("jpg") : source.suffix());
    return QDir(outputFolder).filePath(QStringLiteral("%1_%2.%3").arg(baseName, number, ext));
}

int runHeadless(QCoreApplication &app, QCommandLineParser &parser)
{
    const QString inputFolder = parser.value(QStringLiteral("input"));
    const QString outputFolder = parser.value(QStringLiteral("output"));
    if (inputFolder.isEmpty() || outputFolder.isEmpty()) {
        qCritical("Headless mode requires --input and --output.");
        return 2;
    }
    if (!QDir(inputFolder).exists()) {
        qCritical().noquote() << QStringLiteral("Input folder does not exist: %1").arg(inputFolder);
        return 2;
    }

    const QStringList images = collectImages(inputFolder);
    if (images.isEmpty()) {
        qCritical().noquote() << QStringLiteral("No supported images found in: %1").arg(inputFolder);
        return 2;
    }

    const ProcessingOptions options {
        !parser.isSet(QStringLiteral("no-blur-faces")),
        parser.value(QStringLiteral("blur-mode")) == QLatin1String("pixelate") ? QStringLiteral("pixelate") : QStringLiteral("gaussian"),
        qBound(1, parser.value(QStringLiteral("strength")).toInt(), 100),
        qBound(0.0f, parser.value(QStringLiteral("detection-sensitivity")).toFloat() / 100.0f, 1.0f),
        !parser.isSet(QStringLiteral("no-size-filter")),
        !parser.isSet(QStringLiteral("no-skin-filter")),
        parser.isSet(QStringLiteral("cascade-cross-check")),
        qBound(0, parser.value(QStringLiteral("compression")).toInt(), 100),
        parser.value(QStringLiteral("output-format")),
        QStringLiteral("yunet")
    };

    QDir().mkpath(outputFolder);
    ImageProcessor processor(options);
    const QString renamePattern = parser.value(QStringLiteral("rename-pattern"));
    for (int i = 0; i < images.size(); ++i) {
        const QString targetPath = outputPathFor(images.at(i), outputFolder, renamePattern, i, options);
        const ProcessingResult result = processor.processFile(images.at(i), targetPath);
        if (!result.success) {
            qCritical().noquote() << result.error;
            return 1;
        }
        qInfo().noquote() << QStringLiteral("[%1/%2] %3 (%4 face(s), %5)")
            .arg(i + 1)
            .arg(images.size())
            .arg(QFileInfo(targetPath).fileName())
            .arg(result.facesBlurred)
            .arg(result.detectorUsed.isEmpty() ? QStringLiteral("no detector") : result.detectorUsed);
    }

    qInfo().noquote() << QStringLiteral("Completed %1 image(s).").arg(images.size());
    Q_UNUSED(app)
    return 0;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName(QStringLiteral("AutoPhoto"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    QCoreApplication::setOrganizationName(QStringLiteral("AutoPhoto"));

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Batch image processing app"));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption modeOption(QStringLiteral("mode"), QStringLiteral("Run mode: gui or auto."), QStringLiteral("mode"), QStringLiteral("gui"));
    const QCommandLineOption inputOption(QStringLiteral("input"), QStringLiteral("Input image folder for headless mode."), QStringLiteral("folder"));
    const QCommandLineOption outputOption(QStringLiteral("output"), QStringLiteral("Output folder for headless mode."), QStringLiteral("folder"));
    const QCommandLineOption renamePatternOption(QStringLiteral("rename-pattern"), QStringLiteral("Output base filename pattern."), QStringLiteral("pattern"), QStringLiteral("autophoto"));
    const QCommandLineOption noBlurFacesOption(QStringLiteral("no-blur-faces"), QStringLiteral("Disable face blur."));
    const QCommandLineOption blurModeOption(QStringLiteral("blur-mode"), QStringLiteral("Blur mode: gaussian or pixelate."), QStringLiteral("mode"), QStringLiteral("gaussian"));
    const QCommandLineOption strengthOption(QStringLiteral("strength"), QStringLiteral("Blur strength from 1 to 100."), QStringLiteral("value"), QStringLiteral("100"));
    const QCommandLineOption detectionSensitivityOption(QStringLiteral("detection-sensitivity"), QStringLiteral("Detection sensitivity from 0 to 100."), QStringLiteral("value"), QStringLiteral("35"));
    const QCommandLineOption noSizeFilterOption(QStringLiteral("no-size-filter"), QStringLiteral("Disable box size filter."));
    const QCommandLineOption noSkinFilterOption(QStringLiteral("no-skin-filter"), QStringLiteral("Disable skin-color filter."));
    const QCommandLineOption cascadeCrossCheckOption(QStringLiteral("cascade-cross-check"), QStringLiteral("Enable cascade cross-check filter."));
    const QCommandLineOption compressionOption(QStringLiteral("compression"), QStringLiteral("Compression level from 0 to 100."), QStringLiteral("value"), QStringLiteral("0"));
    const QCommandLineOption outputFormatOption(QStringLiteral("output-format"), QStringLiteral("Output format: jpg, png, webp."), QStringLiteral("format"), QStringLiteral("jpg"));
    parser.addOption(modeOption);
    parser.addOption(inputOption);
    parser.addOption(outputOption);
    parser.addOption(renamePatternOption);
    parser.addOption(noBlurFacesOption);
    parser.addOption(blurModeOption);
    parser.addOption(strengthOption);
    parser.addOption(detectionSensitivityOption);
    parser.addOption(noSizeFilterOption);
    parser.addOption(noSkinFilterOption);
    parser.addOption(cascadeCrossCheckOption);
    parser.addOption(compressionOption);
    parser.addOption(outputFormatOption);

    bool requestedHeadless = false;
    bool requestedConsoleOnly = false;
    for (int i = 1; i < argc; ++i) {
        const QString argument = QString::fromLocal8Bit(argv[i]);
        if (argument == QLatin1String("--help") || argument == QLatin1String("-h")
            || argument == QLatin1String("--version") || argument == QLatin1String("-v")) {
            requestedConsoleOnly = true;
            break;
        }
        if (argument == QLatin1String("--mode=auto")) {
            requestedHeadless = true;
            break;
        }
        if (argument == QLatin1String("--mode") && i + 1 < argc) {
            requestedHeadless = QString::fromLocal8Bit(argv[i + 1]) == QLatin1String("auto");
            break;
        }
    }

    if (requestedHeadless || requestedConsoleOnly) {
        QCoreApplication coreApp(argc, argv);
        parser.process(coreApp);
        if (requestedConsoleOnly) {
            return 0;
        }
        return runHeadless(coreApp, parser);
    }

    QGuiApplication app(argc, argv);
    parser.process(app);

    QQuickStyle::setStyle(QStringLiteral("Material"));

    ImageListModel imageModel;
    PreviewController previewController;
    BatchProcessor batchProcessor;
    SshTunnelController sshTunnelController;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("imageModel"), &imageModel);
    engine.rootContext()->setContextProperty(QStringLiteral("previewController"), &previewController);
    engine.rootContext()->setContextProperty(QStringLiteral("batchProcessor"), &batchProcessor);
    engine.rootContext()->setContextProperty(QStringLiteral("sshTunnelController"), &sshTunnelController);

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] {
        QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);

    engine.load(QUrl(QStringLiteral("qrc:/AutoPhoto/Main.qml")));
    return app.exec();
}
