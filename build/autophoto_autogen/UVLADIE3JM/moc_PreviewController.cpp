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
    uint offsetsAndSizes[44];
    char stringdata0[18];
    char stringdata1[18];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[17];
    char stringdata5[16];
    char stringdata6[16];
    char stringdata7[14];
    char stringdata8[8];
    char stringdata9[13];
    char stringdata10[8];
    char stringdata11[12];
    char stringdata12[5];
    char stringdata13[12];
    char stringdata14[9];
    char stringdata15[15];
    char stringdata16[9];
    char stringdata17[6];
    char stringdata18[11];
    char stringdata19[5];
    char stringdata20[10];
    char stringdata21[9];
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
        QT_MOC_LITERAL(98, 13),  // "previewFailed"
        QT_MOC_LITERAL(112, 7),  // "message"
        QT_MOC_LITERAL(120, 12),  // "setBlurFaces"
        QT_MOC_LITERAL(133, 7),  // "enabled"
        QT_MOC_LITERAL(141, 11),  // "setBlurMode"
        QT_MOC_LITERAL(153, 4),  // "mode"
        QT_MOC_LITERAL(158, 11),  // "setStrength"
        QT_MOC_LITERAL(170, 8),  // "strength"
        QT_MOC_LITERAL(179, 14),  // "requestPreview"
        QT_MOC_LITERAL(194, 8),  // "filePath"
        QT_MOC_LITERAL(203, 5),  // "clear"
        QT_MOC_LITERAL(209, 10),  // "previewUrl"
        QT_MOC_LITERAL(220, 4),  // "busy"
        QT_MOC_LITERAL(225, 9),  // "blurFaces"
        QT_MOC_LITERAL(235, 8)   // "blurMode"
    },
    "PreviewController",
    "previewUrlChanged",
    "",
    "busyChanged",
    "blurFacesChanged",
    "blurModeChanged",
    "strengthChanged",
    "previewFailed",
    "message",
    "setBlurFaces",
    "enabled",
    "setBlurMode",
    "mode",
    "setStrength",
    "strength",
    "requestPreview",
    "filePath",
    "clear",
    "previewUrl",
    "busy",
    "blurFaces",
    "blurMode"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_PreviewController[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       5,  101, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   80,    2, 0x06,    6 /* Public */,
       3,    0,   81,    2, 0x06,    7 /* Public */,
       4,    0,   82,    2, 0x06,    8 /* Public */,
       5,    0,   83,    2, 0x06,    9 /* Public */,
       6,    0,   84,    2, 0x06,   10 /* Public */,
       7,    1,   85,    2, 0x06,   11 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       9,    1,   88,    2, 0x0a,   13 /* Public */,
      11,    1,   91,    2, 0x0a,   15 /* Public */,
      13,    1,   94,    2, 0x0a,   17 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
      15,    1,   97,    2, 0x02,   19 /* Public */,
      17,    0,  100,    2, 0x02,   21 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void, QMetaType::Int,   14,

 // methods: parameters
    QMetaType::Void, QMetaType::QString,   16,
    QMetaType::Void,

 // properties: name, type, flags
      18, QMetaType::QUrl, 0x00015001, uint(0), 0,
      19, QMetaType::Bool, 0x00015001, uint(1), 0,
      20, QMetaType::Bool, 0x00015103, uint(2), 0,
      21, QMetaType::QString, 0x00015103, uint(3), 0,
      14, QMetaType::Int, 0x00015103, uint(4), 0,

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
        case 5: _t->previewFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->setBlurFaces((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 7: _t->setBlurMode((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->setStrength((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 9: _t->requestPreview((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 10: _t->clear(); break;
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
            using _t = void (PreviewController::*)(const QString & );
            if (_t _q_method = &PreviewController::previewFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
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
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
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
void PreviewController::previewFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 5, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
