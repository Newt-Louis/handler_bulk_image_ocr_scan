/****************************************************************************
** Meta object code from reading C++ file 'PreviewController.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/PreviewController.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PreviewController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_PreviewController_t {
    uint offsetsAndSizes[98];
    char stringdata0[18];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[17];
    char stringdata5[16];
    char stringdata6[16];
    char stringdata7[28];
    char stringdata8[25];
    char stringdata9[30];
    char stringdata10[32];
    char stringdata11[24];
    char stringdata12[20];
    char stringdata13[21];
    char stringdata14[26];
    char stringdata15[14];
    char stringdata16[8];
    char stringdata17[13];
    char stringdata18[8];
    char stringdata19[12];
    char stringdata20[5];
    char stringdata21[12];
    char stringdata22[9];
    char stringdata23[24];
    char stringdata24[12];
    char stringdata25[21];
    char stringdata26[26];
    char stringdata27[28];
    char stringdata28[20];
    char stringdata29[6];
    char stringdata30[16];
    char stringdata31[7];
    char stringdata32[17];
    char stringdata33[22];
    char stringdata34[15];
    char stringdata35[9];
    char stringdata36[6];
    char stringdata37[11];
    char stringdata38[5];
    char stringdata39[10];
    char stringdata40[9];
    char stringdata41[21];
    char stringdata42[18];
    char stringdata43[23];
    char stringdata44[25];
    char stringdata45[17];
    char stringdata46[13];
    char stringdata47[14];
    char stringdata48[19];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_PreviewController_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_PreviewController_t qt_meta_stringdata_PreviewController = {
    {
        QT_MOC_LITERAL(0, 17),  // "PreviewController"
        QT_MOC_LITERAL(18, 17),  // "previewUrlChanged"
        QT_MOC_LITERAL(36, 0),  // ""
        QT_MOC_LITERAL(37, 11),  // "busyChanged"
        QT_MOC_LITERAL(49, 16),  // "blurFacesChanged"
        QT_MOC_LITERAL(66, 15),  // "blurModeChanged"
        QT_MOC_LITERAL(82, 15),  // "strengthChanged"
        QT_MOC_LITERAL(98, 27),  // "detectionSensitivityChanged"
        QT_MOC_LITERAL(126, 24),  // "sizeFilterEnabledChanged"
        QT_MOC_LITERAL(151, 29),  // "skinColorFilterEnabledChanged"
        QT_MOC_LITERAL(181, 31),  // "cascadeCrossCheckEnabledChanged"
        QT_MOC_LITERAL(213, 23),  // "compressionLevelChanged"
        QT_MOC_LITERAL(237, 19),  // "outputFormatChanged"
        QT_MOC_LITERAL(257, 20),  // "rotateEnabledChanged"
        QT_MOC_LITERAL(278, 25),  // "compressionEnabledChanged"
        QT_MOC_LITERAL(304, 13),  // "previewFailed"
        QT_MOC_LITERAL(318, 7),  // "message"
        QT_MOC_LITERAL(326, 12),  // "setBlurFaces"
        QT_MOC_LITERAL(339, 7),  // "enabled"
        QT_MOC_LITERAL(347, 11),  // "setBlurMode"
        QT_MOC_LITERAL(359, 4),  // "mode"
        QT_MOC_LITERAL(364, 11),  // "setStrength"
        QT_MOC_LITERAL(376, 8),  // "strength"
        QT_MOC_LITERAL(385, 23),  // "setDetectionSensitivity"
        QT_MOC_LITERAL(409, 11),  // "sensitivity"
        QT_MOC_LITERAL(421, 20),  // "setSizeFilterEnabled"
        QT_MOC_LITERAL(442, 25),  // "setSkinColorFilterEnabled"
        QT_MOC_LITERAL(468, 27),  // "setCascadeCrossCheckEnabled"
        QT_MOC_LITERAL(496, 19),  // "setCompressionLevel"
        QT_MOC_LITERAL(516, 5),  // "level"
        QT_MOC_LITERAL(522, 15),  // "setOutputFormat"
        QT_MOC_LITERAL(538, 6),  // "format"
        QT_MOC_LITERAL(545, 16),  // "setRotateEnabled"
        QT_MOC_LITERAL(562, 21),  // "setCompressionEnabled"
        QT_MOC_LITERAL(584, 14),  // "requestPreview"
        QT_MOC_LITERAL(599, 8),  // "filePath"
        QT_MOC_LITERAL(608, 5),  // "clear"
        QT_MOC_LITERAL(614, 10),  // "previewUrl"
        QT_MOC_LITERAL(625, 4),  // "busy"
        QT_MOC_LITERAL(630, 9),  // "blurFaces"
        QT_MOC_LITERAL(640, 8),  // "blurMode"
        QT_MOC_LITERAL(649, 20),  // "detectionSensitivity"
        QT_MOC_LITERAL(670, 17),  // "sizeFilterEnabled"
        QT_MOC_LITERAL(688, 22),  // "skinColorFilterEnabled"
        QT_MOC_LITERAL(711, 24),  // "cascadeCrossCheckEnabled"
        QT_MOC_LITERAL(736, 16),  // "compressionLevel"
        QT_MOC_LITERAL(753, 12),  // "outputFormat"
        QT_MOC_LITERAL(766, 13),  // "rotateEnabled"
        QT_MOC_LITERAL(780, 18)   // "compressionEnabled"
    },
    "PreviewController",
    "previewUrlChanged",
    "",
    "busyChanged",
    "blurFacesChanged",
    "blurModeChanged",
    "strengthChanged",
    "detectionSensitivityChanged",
    "sizeFilterEnabledChanged",
    "skinColorFilterEnabledChanged",
    "cascadeCrossCheckEnabledChanged",
    "compressionLevelChanged",
    "outputFormatChanged",
    "rotateEnabledChanged",
    "compressionEnabledChanged",
    "previewFailed",
    "message",
    "setBlurFaces",
    "enabled",
    "setBlurMode",
    "mode",
    "setStrength",
    "strength",
    "setDetectionSensitivity",
    "sensitivity",
    "setSizeFilterEnabled",
    "setSkinColorFilterEnabled",
    "setCascadeCrossCheckEnabled",
    "setCompressionLevel",
    "level",
    "setOutputFormat",
    "format",
    "setRotateEnabled",
    "setCompressionEnabled",
    "requestPreview",
    "filePath",
    "clear",
    "previewUrl",
    "busy",
    "blurFaces",
    "blurMode",
    "detectionSensitivity",
    "sizeFilterEnabled",
    "skinColorFilterEnabled",
    "cascadeCrossCheckEnabled",
    "compressionLevel",
    "outputFormat",
    "rotateEnabled",
    "compressionEnabled"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_PreviewController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      27,   14, // methods
      13,  229, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      14,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,  176,    2, 0x06,   14 /* Public */,
       3,    0,  177,    2, 0x06,   15 /* Public */,
       4,    0,  178,    2, 0x06,   16 /* Public */,
       5,    0,  179,    2, 0x06,   17 /* Public */,
       6,    0,  180,    2, 0x06,   18 /* Public */,
       7,    0,  181,    2, 0x06,   19 /* Public */,
       8,    0,  182,    2, 0x06,   20 /* Public */,
       9,    0,  183,    2, 0x06,   21 /* Public */,
      10,    0,  184,    2, 0x06,   22 /* Public */,
      11,    0,  185,    2, 0x06,   23 /* Public */,
      12,    0,  186,    2, 0x06,   24 /* Public */,
      13,    0,  187,    2, 0x06,   25 /* Public */,
      14,    0,  188,    2, 0x06,   26 /* Public */,
      15,    1,  189,    2, 0x06,   27 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      17,    1,  192,    2, 0x0a,   29 /* Public */,
      19,    1,  195,    2, 0x0a,   31 /* Public */,
      21,    1,  198,    2, 0x0a,   33 /* Public */,
      23,    1,  201,    2, 0x0a,   35 /* Public */,
      25,    1,  204,    2, 0x0a,   37 /* Public */,
      26,    1,  207,    2, 0x0a,   39 /* Public */,
      27,    1,  210,    2, 0x0a,   41 /* Public */,
      28,    1,  213,    2, 0x0a,   43 /* Public */,
      30,    1,  216,    2, 0x0a,   45 /* Public */,
      32,    1,  219,    2, 0x0a,   47 /* Public */,
      33,    1,  222,    2, 0x0a,   49 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      34,    1,  225,    2, 0x02,   51 /* Public */,
      36,    0,  228,    2, 0x02,   53 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   16,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, QMetaType::QString,   20,
    QMetaType::Void, QMetaType::Int,   22,
    QMetaType::Void, QMetaType::Float,   24,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, QMetaType::Int,   29,
    QMetaType::Void, QMetaType::QString,   31,
    QMetaType::Void, QMetaType::Bool,   18,
    QMetaType::Void, QMetaType::Bool,   18,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,   35,
    QMetaType::Void,

 // properties: name, type, flags
      37, QMetaType::QUrl, 0x00015001, uint(0), 0,
      38, QMetaType::Bool, 0x00015001, uint(1), 0,
      39, QMetaType::Bool, 0x00015103, uint(2), 0,
      40, QMetaType::QString, 0x00015103, uint(3), 0,
      22, QMetaType::Int, 0x00015103, uint(4), 0,
      41, QMetaType::Float, 0x00015103, uint(5), 0,
      42, QMetaType::Bool, 0x00015103, uint(6), 0,
      43, QMetaType::Bool, 0x00015103, uint(7), 0,
      44, QMetaType::Bool, 0x00015103, uint(8), 0,
      45, QMetaType::Int, 0x00015103, uint(9), 0,
      46, QMetaType::QString, 0x00015103, uint(10), 0,
      47, QMetaType::Bool, 0x00015103, uint(11), 0,
      48, QMetaType::Bool, 0x00015103, uint(12), 0,

       0        // eod
};

Q_CONSTINIT const QMetaObject PreviewController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_PreviewController.offsetsAndSizes,
    qt_meta_data_PreviewController,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_PreviewController_t,
        // property 'previewUrl'
        QtPrivate::TypeAndForceComplete<QUrl, std::true_type>,
        // property 'busy'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'blurFaces'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'blurMode'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'strength'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'detectionSensitivity'
        QtPrivate::TypeAndForceComplete<float, std::true_type>,
        // property 'sizeFilterEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'skinColorFilterEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'cascadeCrossCheckEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'compressionLevel'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // property 'outputFormat'
        QtPrivate::TypeAndForceComplete<QString, std::true_type>,
        // property 'rotateEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // property 'compressionEnabled'
        QtPrivate::TypeAndForceComplete<bool, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<PreviewController, std::true_type>,
        // method 'previewUrlChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'busyChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'blurFacesChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'blurModeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'strengthChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'detectionSensitivityChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sizeFilterEnabledChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'skinColorFilterEnabledChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'cascadeCrossCheckEnabledChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'compressionLevelChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'outputFormatChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'rotateEnabledChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'compressionEnabledChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'previewFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setBlurFaces'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setBlurMode'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setStrength'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setDetectionSensitivity'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<float, std::false_type>,
        // method 'setSizeFilterEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setSkinColorFilterEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setCascadeCrossCheckEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setCompressionLevel'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setOutputFormat'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setRotateEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'setCompressionEnabled'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'requestPreview'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'clear'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void PreviewController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PreviewController *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->previewUrlChanged(); break;
        case 1: _t->busyChanged(); break;
        case 2: _t->blurFacesChanged(); break;
        case 3: _t->blurModeChanged(); break;
        case 4: _t->strengthChanged(); break;
        case 5: _t->detectionSensitivityChanged(); break;
        case 6: _t->sizeFilterEnabledChanged(); break;
        case 7: _t->skinColorFilterEnabledChanged(); break;
        case 8: _t->cascadeCrossCheckEnabledChanged(); break;
        case 9: _t->compressionLevelChanged(); break;
        case 10: _t->outputFormatChanged(); break;
        case 11: _t->rotateEnabledChanged(); break;
        case 12: _t->compressionEnabledChanged(); break;
        case 13: _t->previewFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->setBlurFaces((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 15: _t->setBlurMode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 16: _t->setStrength((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 17: _t->setDetectionSensitivity((*reinterpret_cast< std::add_pointer_t<float>>(_a[1]))); break;
        case 18: _t->setSizeFilterEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 19: _t->setSkinColorFilterEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 20: _t->setCascadeCrossCheckEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 21: _t->setCompressionLevel((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 22: _t->setOutputFormat((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 23: _t->setRotateEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 24: _t->setCompressionEnabled((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 25: _t->requestPreview((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 26: _t->clear(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::previewUrlChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::busyChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::blurFacesChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::blurModeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::strengthChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::detectionSensitivityChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::sizeFilterEnabledChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::skinColorFilterEnabledChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::cascadeCrossCheckEnabledChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::compressionLevelChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::outputFormatChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::rotateEnabledChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 11;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)();
            if (_t _q_method = &PreviewController::compressionEnabledChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 12;
                return;
            }
        }
        {
            using _t = void (PreviewController::*)(const QString & );
            if (_t _q_method = &PreviewController::previewFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 13;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<PreviewController *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QUrl*>(_v) = _t->previewUrl(); break;
        case 1: *reinterpret_cast< bool*>(_v) = _t->busy(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->blurFaces(); break;
        case 3: *reinterpret_cast< QString*>(_v) = _t->blurMode(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->strength(); break;
        case 5: *reinterpret_cast< float*>(_v) = _t->detectionSensitivity(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->sizeFilterEnabled(); break;
        case 7: *reinterpret_cast< bool*>(_v) = _t->skinColorFilterEnabled(); break;
        case 8: *reinterpret_cast< bool*>(_v) = _t->cascadeCrossCheckEnabled(); break;
        case 9: *reinterpret_cast< int*>(_v) = _t->compressionLevel(); break;
        case 10: *reinterpret_cast< QString*>(_v) = _t->outputFormat(); break;
        case 11: *reinterpret_cast< bool*>(_v) = _t->rotateEnabled(); break;
        case 12: *reinterpret_cast< bool*>(_v) = _t->compressionEnabled(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<PreviewController *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 2: _t->setBlurFaces(*reinterpret_cast< bool*>(_v)); break;
        case 3: _t->setBlurMode(*reinterpret_cast< QString*>(_v)); break;
        case 4: _t->setStrength(*reinterpret_cast< int*>(_v)); break;
        case 5: _t->setDetectionSensitivity(*reinterpret_cast< float*>(_v)); break;
        case 6: _t->setSizeFilterEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 7: _t->setSkinColorFilterEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 8: _t->setCascadeCrossCheckEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 9: _t->setCompressionLevel(*reinterpret_cast< int*>(_v)); break;
        case 10: _t->setOutputFormat(*reinterpret_cast< QString*>(_v)); break;
        case 11: _t->setRotateEnabled(*reinterpret_cast< bool*>(_v)); break;
        case 12: _t->setCompressionEnabled(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *PreviewController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PreviewController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PreviewController.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int PreviewController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 27)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 27;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 27)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 27;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void PreviewController::previewUrlChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void PreviewController::busyChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void PreviewController::blurFacesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void PreviewController::blurModeChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void PreviewController::strengthChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void PreviewController::detectionSensitivityChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void PreviewController::sizeFilterEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void PreviewController::skinColorFilterEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 7, nullptr);
}

// SIGNAL 8
void PreviewController::cascadeCrossCheckEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 8, nullptr);
}

// SIGNAL 9
void PreviewController::compressionLevelChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void PreviewController::outputFormatChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void PreviewController::rotateEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}

// SIGNAL 12
void PreviewController::compressionEnabledChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 12, nullptr);
}

// SIGNAL 13
void PreviewController::previewFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 13, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
