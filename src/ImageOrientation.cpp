#include "ImageOrientation.h"

#include <QByteArray>
#include <QFile>
#include <QImageIOHandler>
#include <QImageReader>
#include <QtGlobal>

namespace {

struct ExifInfo
{
    int orientation = 1;
    int pixelWidth = -1;
    int pixelHeight = -1;
};

quint16 read16(const QByteArray &data, qsizetype offset, bool littleEndian)
{
    if (offset < 0 || offset + 2 > data.size()) {
        return 0;
    }

    const auto b0 = static_cast<quint8>(data.at(offset));
    const auto b1 = static_cast<quint8>(data.at(offset + 1));
    if (littleEndian) {
        return static_cast<quint16>(b0 | (b1 << 8));
    }
    return static_cast<quint16>((b0 << 8) | b1);
}

quint32 read32(const QByteArray &data, qsizetype offset, bool littleEndian)
{
    if (offset < 0 || offset + 4 > data.size()) {
        return 0;
    }

    const auto b0 = static_cast<quint8>(data.at(offset));
    const auto b1 = static_cast<quint8>(data.at(offset + 1));
    const auto b2 = static_cast<quint8>(data.at(offset + 2));
    const auto b3 = static_cast<quint8>(data.at(offset + 3));
    if (littleEndian) {
        return static_cast<quint32>(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
    }
    return static_cast<quint32>((b0 << 24) | (b1 << 16) | (b2 << 8) | b3);
}

quint32 ifdValue(const QByteArray &tiff, qsizetype entryOffset, bool littleEndian)
{
    const quint16 type = read16(tiff, entryOffset + 2, littleEndian);
    const quint32 count = read32(tiff, entryOffset + 4, littleEndian);
    if (count == 0) {
        return 0;
    }

    if (type == 3) {
        return read16(tiff, entryOffset + 8, littleEndian);
    }
    if (type == 4) {
        return read32(tiff, entryOffset + 8, littleEndian);
    }
    return 0;
}

void parseIfd(const QByteArray &tiff, quint32 ifdOffset, bool littleEndian, ExifInfo &info, quint32 *exifIfdOffset)
{
    if (ifdOffset > static_cast<quint32>(tiff.size()) || ifdOffset + 2 > static_cast<quint32>(tiff.size())) {
        return;
    }

    const quint16 entryCount = read16(tiff, ifdOffset, littleEndian);
    for (quint16 i = 0; i < entryCount; ++i) {
        const qsizetype entryOffset = static_cast<qsizetype>(ifdOffset) + 2 + (i * 12);
        if (entryOffset + 12 > tiff.size()) {
            break;
        }

        const quint16 tag = read16(tiff, entryOffset, littleEndian);
        const quint32 value = ifdValue(tiff, entryOffset, littleEndian);
        if (tag == 0x0112) {
            info.orientation = static_cast<int>(value);
        } else if (tag == 0xA002 || tag == 0x0100) {
            info.pixelWidth = static_cast<int>(value);
        } else if (tag == 0xA003 || tag == 0x0101) {
            info.pixelHeight = static_cast<int>(value);
        } else if (tag == 0x8769 && exifIfdOffset) {
            *exifIfdOffset = value;
        }
    }
}

ExifInfo readExifInfo(const QString &sourcePath)
{
    QFile file(sourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    const QByteArray jpegHeader = file.read(2);
    if (jpegHeader.size() != 2
        || static_cast<quint8>(jpegHeader.at(0)) != 0xFF
        || static_cast<quint8>(jpegHeader.at(1)) != 0xD8) {
        return {};
    }

    while (!file.atEnd()) {
        QByteArray markerPrefix = file.read(1);
        while (!file.atEnd() && !markerPrefix.isEmpty() && static_cast<quint8>(markerPrefix.at(0)) != 0xFF) {
            markerPrefix = file.read(1);
        }
        if (file.atEnd()) {
            break;
        }

        QByteArray markerByte = file.read(1);
        while (!file.atEnd() && !markerByte.isEmpty() && static_cast<quint8>(markerByte.at(0)) == 0xFF) {
            markerByte = file.read(1);
        }
        if (markerByte.isEmpty()) {
            break;
        }

        const quint8 marker = static_cast<quint8>(markerByte.at(0));
        if (marker == 0xD9 || marker == 0xDA) {
            break;
        }

        const QByteArray lengthBytes = file.read(2);
        if (lengthBytes.size() != 2) {
            break;
        }
        const quint16 segmentLength = static_cast<quint16>(
            (static_cast<quint8>(lengthBytes.at(0)) << 8) | static_cast<quint8>(lengthBytes.at(1))
        );
        if (segmentLength < 2) {
            break;
        }

        const int payloadLength = segmentLength - 2;
        if (marker != 0xE1) {
            file.skip(payloadLength);
            continue;
        }

        const QByteArray payload = file.read(payloadLength);
        if (payload.size() < 6 || payload.left(6) != QByteArray::fromRawData("Exif\0\0", 6)) {
            continue;
        }

        const QByteArray tiff = payload.mid(6);
        if (tiff.size() < 8) {
            return {};
        }

        const bool littleEndian = tiff.size() >= 2 && tiff.left(2) == QByteArrayLiteral("II");
        if (!littleEndian && (tiff.size() < 2 || tiff.left(2) != QByteArrayLiteral("MM"))) {
            return {};
        }
        if (read16(tiff, 2, littleEndian) != 42) {
            return {};
        }

        ExifInfo info;
        quint32 exifIfdOffset = 0;
        parseIfd(tiff, read32(tiff, 4, littleEndian), littleEndian, info, &exifIfdOffset);
        if (exifIfdOffset > 0) {
            parseIfd(tiff, exifIfdOffset, littleEndian, info, nullptr);
        }
        return info;
    }

    return {};
}

bool transformationRotates90(QImageIOHandler::Transformations transformation)
{
    return transformation.testFlag(QImageIOHandler::TransformationRotate90)
        || transformation.testFlag(QImageIOHandler::TransformationRotate270);
}

bool shouldIgnoreExifRotation(const QString &sourcePath, QImageIOHandler::Transformations transformation, const QSize &rawSize)
{
    if (!transformationRotates90(transformation)) {
        return false;
    }

    if (rawSize.isValid() && rawSize.width() > rawSize.height()) {
        return true;
    }

    const ExifInfo info = readExifInfo(sourcePath);
    return info.orientation != 1 && (info.pixelWidth == 0 || info.pixelHeight == 0);
}

} // namespace

QImage readImageWithResolvedOrientation(const QString &sourcePath)
{
    QImageReader probe(sourcePath);
    const QImageIOHandler::Transformations transformation = probe.transformation();
    const QSize rawSize = probe.size();

    QImageReader reader(sourcePath);
    reader.setAutoTransform(!shouldIgnoreExifRotation(sourcePath, transformation, rawSize));
    return reader.read();
}
