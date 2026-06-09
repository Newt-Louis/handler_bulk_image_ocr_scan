/****************************************************************************
** Meta object code from reading C++ file 'ImageListModel.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/ImageListModel.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ImageListModel.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_ImageListModel_t {
    uint offsetsAndSizes[36];
    char stringdata0[15];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[17];
    char stringdata4[8];
    char stringdata5[11];
    char stringdata6[10];
    char stringdata7[6];
    char stringdata8[9];
    char stringdata9[6];
    char stringdata10[8];
    char stringdata11[6];
    char stringdata12[6];
    char stringdata13[5];
    char stringdata14[13];
    char stringdata15[12];
    char stringdata16[13];
    char stringdata17[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_ImageListModel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_ImageListModel_t qt_meta_stringdata_ImageListModel = {
    {
        QT_MOC_LITERAL(0, 14),  // "ImageListModel"
        QT_MOC_LITERAL(15, 12),  // "countChanged"
        QT_MOC_LITERAL(28, 0),  // ""
        QT_MOC_LITERAL(29, 16),  // "folderLoadFailed"
        QT_MOC_LITERAL(46, 7),  // "message"
        QT_MOC_LITERAL(54, 10),  // "loadFolder"
        QT_MOC_LITERAL(65, 9),  // "folderUrl"
        QT_MOC_LITERAL(75, 5),  // "clear"
        QT_MOC_LITERAL(81, 8),  // "itemPath"
        QT_MOC_LITERAL(90, 5),  // "index"
        QT_MOC_LITERAL(96, 7),  // "itemUrl"
        QT_MOC_LITERAL(104, 5),  // "paths"
        QT_MOC_LITERAL(110, 5),  // "count"
        QT_MOC_LITERAL(116, 4),  // "Role"
        QT_MOC_LITERAL(121, 12),  // "FilePathRole"
        QT_MOC_LITERAL(134, 11),  // "FileUrlRole"
        QT_MOC_LITERAL(146, 12),  // "FileNameRole"
        QT_MOC_LITERAL(159, 10)   // "StatusRole"
    },
    "ImageListModel",
    "countChanged",
    "",
    "folderLoadFailed",
    "message",
    "loadFolder",
    "folderUrl",
    "clear",
    "itemPath",
    "index",
    "itemUrl",
    "paths",
    "count",
    "Role",
    "FilePathRole",
    "FileUrlRole",
    "FileNameRole",
    "StatusRole"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_ImageListModel[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       1,   71, // properties
       1,   76, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x06,    2 /* Public */,
       3,    1,   57,    2, 0x06,    3 /* Public */,

 // methods: name, argc, parameters, tag, flags, initial metatype offsets
       5,    1,   60,    2, 0x02,    5 /* Public */,
       7,    0,   63,    2, 0x02,    7 /* Public */,
       8,    1,   64,    2, 0x102,    8 /* Public | MethodIsConst  */,
      10,    1,   67,    2, 0x102,   10 /* Public | MethodIsConst  */,
      11,    0,   70,    2, 0x102,   12 /* Public | MethodIsConst  */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    4,

 // methods: parameters
    QMetaType::Bool, QMetaType::QUrl,    6,
    QMetaType::Void,
    QMetaType::QString, QMetaType::Int,    9,
    QMetaType::QUrl, QMetaType::Int,    9,
    QMetaType::QStringList,

 // properties: name, type, flags
      12, QMetaType::Int, 0x00015001, uint(0), 0,

 // enums: name, alias, flags, count, data
      13,   13, 0x0,    4,   81,

 // enum data: key, value
      14, uint(ImageListModel::FilePathRole),
      15, uint(ImageListModel::FileUrlRole),
      16, uint(ImageListModel::FileNameRole),
      17, uint(ImageListModel::StatusRole),

       0        // eod
};

Q_CONSTINIT const QMetaObject ImageListModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractListModel::staticMetaObject>(),
    qt_meta_stringdata_ImageListModel.offsetsAndSizes,
    qt_meta_data_ImageListModel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_ImageListModel_t,
        // property 'count'
        QtPrivate::TypeAndForceComplete<int, std::true_type>,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<ImageListModel, std::true_type>,
        // method 'countChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'folderLoadFailed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'loadFolder'
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QUrl &, std::false_type>,
        // method 'clear'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'itemPath'
        QtPrivate::TypeAndForceComplete<QString, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'itemUrl'
        QtPrivate::TypeAndForceComplete<QUrl, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'paths'
        QtPrivate::TypeAndForceComplete<QStringList, std::false_type>
    >,
    nullptr
} };

void ImageListModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ImageListModel *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->countChanged(); break;
        case 1: _t->folderLoadFailed((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 2: { bool _r = _t->loadFolder((*reinterpret_cast< std::add_pointer_t<QUrl>>(_a[1])));
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 3: _t->clear(); break;
        case 4: { QString _r = _t->itemPath((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = std::move(_r); }  break;
        case 5: { QUrl _r = _t->itemUrl((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])));
            if (_a[0]) *reinterpret_cast< QUrl*>(_a[0]) = std::move(_r); }  break;
        case 6: { QStringList _r = _t->paths();
            if (_a[0]) *reinterpret_cast< QStringList*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ImageListModel::*)();
            if (_t _q_method = &ImageListModel::countChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ImageListModel::*)(const QString & );
            if (_t _q_method = &ImageListModel::folderLoadFailed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<ImageListModel *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->count(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
    } else if (_c == QMetaObject::ResetProperty) {
    } else if (_c == QMetaObject::BindableProperty) {
    }
}

const QMetaObject *ImageListModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ImageListModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ImageListModel.stringdata0))
        return static_cast<void*>(this);
    return QAbstractListModel::qt_metacast(_clname);
}

int ImageListModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractListModel::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void ImageListModel::countChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void ImageListModel::folderLoadFailed(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
