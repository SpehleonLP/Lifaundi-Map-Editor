/****************************************************************************
** Meta object code from reading C++ file 'widget_rangeslider.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.7.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/widget_rangeslider.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'widget_rangeslider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.7.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSRangeSliderENDCLASS_t {};
constexpr auto qt_meta_stringdata_CLASSRangeSliderENDCLASS = QtMocHelpers::stringData(
    "RangeSlider",
    "lowerValueChanged",
    "",
    "aLowerValue",
    "upperValueChanged",
    "aUpperValue",
    "rangeChanged",
    "aMin",
    "aMax",
    "valueChanged",
    "setLowerValue",
    "setUpperValue",
    "setMinimum",
    "aMinimum",
    "setMaximum",
    "aMaximum"
);
#else  // !QT_MOC_HAS_STRINGDATA
#error "qtmochelpers.h not found or too old."
#endif // !QT_MOC_HAS_STRINGDATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSRangeSliderENDCLASS[] = {

 // content:
      12,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   62,    2, 0x06,    1 /* Public */,
       4,    1,   65,    2, 0x06,    3 /* Public */,
       6,    2,   68,    2, 0x06,    5 /* Public */,
       9,    0,   73,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
      10,    1,   74,    2, 0x0a,    9 /* Public */,
      11,    1,   77,    2, 0x0a,   11 /* Public */,
      12,    1,   80,    2, 0x0a,   13 /* Public */,
      14,    1,   83,    2, 0x0a,   15 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    7,    8,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void, QMetaType::Int,   13,
    QMetaType::Void, QMetaType::Int,   15,

 // enums: name, alias, flags, count, data

 // enum data: key, value

       0        // eod
};

Q_CONSTINIT const QMetaObject RangeSlider::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSRangeSliderENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSRangeSliderENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSRangeSliderENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<RangeSlider, std::true_type>,
        // method 'lowerValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'upperValueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'rangeChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'valueChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'setLowerValue'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setUpperValue'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setMinimum'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'setMaximum'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void RangeSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<RangeSlider *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->lowerValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 1: _t->upperValueChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->rangeChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<int>>(_a[2]))); break;
        case 3: _t->valueChanged(); break;
        case 4: _t->setLowerValue((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->setUpperValue((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->setMinimum((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 7: _t->setMaximum((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (RangeSlider::*)(int );
            if (_t _q_method = &RangeSlider::lowerValueChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (RangeSlider::*)(int );
            if (_t _q_method = &RangeSlider::upperValueChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (RangeSlider::*)(int , int );
            if (_t _q_method = &RangeSlider::rangeChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (RangeSlider::*)();
            if (_t _q_method = &RangeSlider::valueChanged; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *RangeSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RangeSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSRangeSliderENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int RangeSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void RangeSlider::lowerValueChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RangeSlider::upperValueChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void RangeSlider::rangeChanged(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void RangeSlider::valueChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
