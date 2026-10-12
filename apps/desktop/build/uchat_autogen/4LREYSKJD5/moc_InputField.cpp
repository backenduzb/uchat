/****************************************************************************
** Meta object code from reading C++ file 'InputField.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/components/InputField.hpp"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'InputField.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
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
struct qt_meta_tag_ZN10InputFieldE_t {};
} // unnamed namespace

template <> constexpr inline auto InputField::qt_create_metaobjectdata<qt_meta_tag_ZN10InputFieldE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "InputField",
        "normalBorderColor",
        "QColor",
        "focusedBorderColor",
        "disabledBorderColor",
        "placeholderColor",
        "underlineHeight",
        "focusedUnderlineHeight",
        "animationDuration"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
        // property 'normalBorderColor'
        QtMocHelpers::PropertyData<QColor>(1, 0x80000000 | 2, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet),
        // property 'focusedBorderColor'
        QtMocHelpers::PropertyData<QColor>(3, 0x80000000 | 2, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet),
        // property 'disabledBorderColor'
        QtMocHelpers::PropertyData<QColor>(4, 0x80000000 | 2, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet),
        // property 'placeholderColor'
        QtMocHelpers::PropertyData<QColor>(5, 0x80000000 | 2, QMC::DefaultPropertyFlags | QMC::Writable | QMC::EnumOrFlag | QMC::StdCppSet),
        // property 'underlineHeight'
        QtMocHelpers::PropertyData<int>(6, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'focusedUnderlineHeight'
        QtMocHelpers::PropertyData<int>(7, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
        // property 'animationDuration'
        QtMocHelpers::PropertyData<int>(8, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<InputField, qt_meta_tag_ZN10InputFieldE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject InputField::staticMetaObject = { {
    QMetaObject::SuperData::link<QLineEdit::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10InputFieldE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10InputFieldE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN10InputFieldE_t>.metaTypes,
    nullptr
} };

void InputField::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<InputField *>(_o);
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<QColor*>(_v) = _t->normalBorderColor(); break;
        case 1: *reinterpret_cast<QColor*>(_v) = _t->focusedBorderColor(); break;
        case 2: *reinterpret_cast<QColor*>(_v) = _t->disabledBorderColor(); break;
        case 3: *reinterpret_cast<QColor*>(_v) = _t->placeholderColor(); break;
        case 4: *reinterpret_cast<int*>(_v) = _t->underlineHeight(); break;
        case 5: *reinterpret_cast<int*>(_v) = _t->focusedUnderlineHeight(); break;
        case 6: *reinterpret_cast<int*>(_v) = _t->animationDuration(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setNormalBorderColor(*reinterpret_cast<QColor*>(_v)); break;
        case 1: _t->setFocusedBorderColor(*reinterpret_cast<QColor*>(_v)); break;
        case 2: _t->setDisabledBorderColor(*reinterpret_cast<QColor*>(_v)); break;
        case 3: _t->setPlaceholderColor(*reinterpret_cast<QColor*>(_v)); break;
        case 4: _t->setUnderlineHeight(*reinterpret_cast<int*>(_v)); break;
        case 5: _t->setFocusedUnderlineHeight(*reinterpret_cast<int*>(_v)); break;
        case 6: _t->setAnimationDuration(*reinterpret_cast<int*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *InputField::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *InputField::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN10InputFieldE_t>.strings))
        return static_cast<void*>(this);
    return QLineEdit::qt_metacast(_clname);
}

int InputField::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
