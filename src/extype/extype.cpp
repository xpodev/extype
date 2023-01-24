#define PY_SSIZE_T_CLEAN

#include <string>

#include <Python.h>

enum class Protocols
{
    None = 0,
    Number = 1 << 0,
    Sequence = 1 << 1,
    Mapping = 1 << 2,
};

static int operator&(uint8_t const x, Protocols const y)
{
    return static_cast<int>(x) & static_cast<int>(y);
}

struct MagicMethodLookupResult
{
private:
    MagicMethodLookupResult(uint32_t error_code, void *ptr)
        : error_code{error_code}, result{ptr}
    {
    }

public:
    uint32_t error_code;
    void *result;

    MagicMethodLookupResult(uint32_t ec)
        : MagicMethodLookupResult{ec, NULL}
    {
    }

    MagicMethodLookupResult(void *result)
        : MagicMethodLookupResult{0, result}
    {
    }

    bool is_error() { return error_code != 0; }
};

static constexpr uint32_t INVALID_MAGIC_METHOD = 1U;
static constexpr uint32_t NOT_IMPLEMENTED = 2U;

// Implementations

#define UNARY_OP_IMPL(name)                                                               \
    static PyObject *impl##name##(PyObject * self)                                        \
    {                                                                                     \
        if (!PyObject_HasAttrString(self, #name))                                         \
        {                                                                                 \
            Py_RETURN_NOTIMPLEMENTED;                                                     \
        }                                                                                 \
        return PyObject_Call(PyObject_GetAttrString(self, #name), PyTuple_Pack(0), NULL); \
    }

#define UNARY_OP(name) UNARY_OP_IMPL(__##name##__)

#define BINARY_OP_IMPL(name)                                                                     \
    static PyObject *impl##name(PyObject *left, PyObject *right)                                 \
    {                                                                                            \
        if (!PyObject_HasAttrString(left, #name))                                                \
        {                                                                                        \
            Py_RETURN_NOTIMPLEMENTED;                                                            \
        }                                                                                        \
        return PyObject_Call(PyObject_GetAttrString(left, #name), PyTuple_Pack(1, right), NULL); \
    }

#define BINARY_OP(name) BINARY_OP_IMPL(__##name##__)

#define _RBINARY_OP_IMPL(name, rname)                                                                        \
    static PyObject *impl##name##(PyObject * left, PyObject * right)                                         \
    {                                                                                                        \
        if (!PyObject_HasAttrString(left, #name))                                                            \
        {                                                                                                    \
            if (!PyObject_HasAttrString(right, #rname))                                                      \
            {                                                                                                \
                Py_RETURN_NOTIMPLEMENTED;                                                                    \
            }                                                                                                \
            else                                                                                             \
            {                                                                                                \
                return PyObject_Call(PyObject_GetAttrString(right, #rname), PyTuple_Pack(1, left), NULL);    \
            }                                                                                                \
        }                                                                                                    \
        PyObject *result = PyObject_Call(PyObject_GetAttrString(left, #name), PyTuple_Pack(1, right), NULL); \
        if (result == Py_NotImplemented)                                                                     \
        {                                                                                                    \
            if (!PyObject_HasAttrString(right, #rname))                                                      \
            {                                                                                                \
                Py_RETURN_NOTIMPLEMENTED;                                                                    \
            }                                                                                                \
            else                                                                                             \
            {                                                                                                \
                return PyObject_Call(PyObject_GetAttrString(right, #rname), PyTuple_Pack(1, left), NULL);    \
            }                                                                                                \
        }                                                                                                    \
        return result;                                                                                       \
    }

#define RBINARY_OP(name) _RBINARY_OP_IMPL(__##name##__, __r##name##__)

#define TERNARY_OP_IMPL(name)                                                                          \
    static PyObject *impl##name(PyObject *self, PyObject *left, PyObject *right)                       \
    {                                                                                                  \
        if (!PyObject_HasAttrString(self, #name))                                                      \
        {                                                                                              \
            Py_RETURN_NOTIMPLEMENTED;                                                                  \
        }                                                                                              \
        return PyObject_Call(PyObject_GetAttrString(self, #name), PyTuple_Pack(2, left, right), NULL); \
    }

#define TERNARY_OP(name) TERNARY_OP_IMPL(__##name##__)

// Number protocol

RBINARY_OP(add)
RBINARY_OP(sub)
RBINARY_OP(mul)
RBINARY_OP(mod)
RBINARY_OP(divmod)
TERNARY_OP(pow)
BINARY_OP(rpow)
UNARY_OP(neg)
UNARY_OP(pos)
UNARY_OP(abs)

static int impl__bool__(PyObject *self)
{
    if (!PyObject_HasAttrString(self, "__bool__"))
    {
        PyErr_SetString(
            PyExc_NotImplementedError,
            (std::string{"Type "} +
             _PyType_Name((PyTypeObject *)PyObject_Type(self)) +
             " does not implement the '__bool__' method")
                .c_str());
    }
    return PyObject_IsTrue(PyObject_Call(PyObject_GetAttrString(self, "__bool__"), PyTuple_Pack(0), NULL));
}

UNARY_OP(invert)
RBINARY_OP(lshift)
RBINARY_OP(rshift)
RBINARY_OP(and)
RBINARY_OP(xor)
RBINARY_OP(or)
UNARY_OP(int)
UNARY_OP(float)

BINARY_OP(iadd)
BINARY_OP(isub)
BINARY_OP(imul)
BINARY_OP(imod)
TERNARY_OP(ipow)
BINARY_OP(ilshift)
BINARY_OP(irshift)
BINARY_OP(iand)
BINARY_OP(ixor)
BINARY_OP(ior)

RBINARY_OP(floordiv)
RBINARY_OP(truediv)
BINARY_OP(ifloordiv)
BINARY_OP(itruediv)

BINARY_OP(index)

RBINARY_OP(matmul)
BINARY_OP(imatmul)

// End number protocol

static Py_ssize_t impl__len__(PyObject *self)
{
    return PyNumber_AsSsize_t(PyObject_Call(PyObject_GetAttrString(self, "__len__"), PyTuple_Pack(0), NULL), NULL);
}

static int impl__contains__(PyObject *self, PyObject *value)
{
    return PyNumber_AsSsize_t(PyObject_Call(PyObject_GetAttrString(self, "__contains__"), PyTuple_Pack(1, value), NULL), NULL);
}

static PyObject *impl__getitem__(PyObject *self, PyObject *item)
{
    return PyObject_Call(PyObject_GetAttrString(self, "__getitem__"), PyTuple_Pack(0), NULL);
}

static int impl__setitem__(PyObject *self, PyObject *item, PyObject *value)
{
    if (value == NULL)
    {
        return PyNumber_AsSsize_t(PyObject_Call(PyObject_GetAttrString(self, "__delitem__"), PyTuple_Pack(1, item), NULL), NULL);
    }
    return PyNumber_AsSsize_t(PyObject_Call(PyObject_GetAttrString(self, "__setitem__"), PyTuple_Pack(2, item, value), NULL), NULL);
}

// Special methods

static PyObject *impl__repr__(PyObject *self)
{
    return PyObject_Call(PyObject_GetAttrString(self, "__repr__"), PyTuple_Pack(0), NULL);
}

static PyObject *impl__call__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    return PyObject_Call(PyObject_GetAttrString(self, "__call__"), args, kwargs);
}

static Py_hash_t impl__hash__(PyObject *self)
{
    return PyNumber_AsSsize_t(PyObject_Call(PyObject_GetAttrString(self, "__hash__"), PyTuple_Pack(0), NULL), NULL);
}

static PyObject *impl__str__(PyObject *self)
{
    return PyObject_Call(PyObject_GetAttrString(self, "__str__"), PyTuple_Pack(0), NULL);
}

// PyObject_GenericGetAttr
// static PyObject* impl__getattr__(PyObject* self, PyObject* attr) {

// }

// PyObject_GenericSetAttr
// static PyObject* impl__setattr__(PyObject* self, PyObject* attr, PyObject* value) {

// }

static PyObject *impl__iter__(PyObject *self)
{
    return PyObject_Call(PyObject_GetAttrString(self, "__iter__"), PyTuple_Pack(0), NULL);
}

static PyObject *impl__next__(PyObject *self)
{
    return PyObject_Call(PyObject_GetAttrString(self, "__next__"), PyTuple_Pack(0), NULL);
}

// End special methods

// End

static PyObject *implement_protocol_on_type(PyObject *self, PyObject *args)
{
    PyTypeObject *type;
    uint8_t protocols;

    PyArg_ParseTuple(args, "OB", &type, &protocols);

    if (protocols & Protocols::Number && type->tp_as_number == NULL)
    {
        PyNumberMethods *protocol = new PyNumberMethods();
        type->tp_as_number = protocol;
    }
    if (protocols & Protocols::Sequence && type->tp_as_sequence == NULL)
    {
        PySequenceMethods *protocol = new PySequenceMethods();
        type->tp_as_sequence = protocol;
    }
    if (protocols & Protocols::Mapping && type->tp_as_mapping == NULL)
    {
        PyMappingMethods *protocol = new PyMappingMethods();
        type->tp_as_mapping = protocol;
    }

    Py_RETURN_NONE;
}

static PyObject *get_builtin_type_dict(PyObject *self, PyObject *args)
{
    PyTypeObject *type;
    PyArg_ParseTuple(args, "O", &type);

    Py_INCREF(type->tp_dict);
    return type->tp_dict;
}

static MagicMethodLookupResult get_magic_method_slot(PyTypeObject *type, char const *name_str)
{
    std::string name{name_str};

    // special methods

    if (name == "__repr__")
    {
        return &(type->tp_repr);
    }
    if (name == "__call__")
    {
        return &(type->tp_call);
    }
    if (name == "__hash__")
    {
        return &(type->tp_hash);
    }
    if (name == "__str__")
    {
        return &(type->tp_str);
    }
    if (name == "__getattr__")
    {
        return &(type->tp_getattro);
    }
    if (name == "__setattr__")
    {
        return &(type->tp_setattro);
    }
    if (name == "__iter__")
    {
        return &(type->tp_iter);
    }
    if (name == "__next__")
    {
        return &(type->tp_iternext);
    }

    if (name == "__len__")
    {
        if (type->tp_as_mapping == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_mapping != NULL)
            return &type->tp_as_mapping->mp_length;
        return &type->tp_as_sequence->sq_length;
    }
    if (name == "__getitem__")
    {
        if (type->tp_as_mapping == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_mapping != NULL)
            return &type->tp_as_mapping->mp_subscript;
        return &type->tp_as_sequence->sq_item;
    }
    if (name == "__setitem__")
    {
        if (type->tp_as_mapping == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_mapping != NULL)
            return &type->tp_as_mapping->mp_ass_subscript;
        return &type->tp_as_sequence->sq_ass_item;
    }
    if (name == "__contains__")
    {
        if (type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        return &type->tp_as_sequence->sq_contains;
    }

    // protocol methods

#define SLOT(method_name, field)      \
    if (name == #method_name)                \
    {                                        \
        if (type->tp_as_number == NULL)      \
            return NOT_IMPLEMENTED;          \
        return &(type->tp_as_number->field); \
    }

    if (name == "__add__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_add);
        return &(type->tp_as_sequence->sq_concat);
    }
    SLOT(__sub__, nb_subtract)
    if (name == "__mul__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_multiply);
        return &type->tp_as_sequence->sq_repeat;
    }
    SLOT(__mod__, nb_remainder)
    SLOT(__divmod__, nb_divmod)
    SLOT(__pow__, nb_power)
    SLOT(__neg__, nb_negative)
    SLOT(__pos__, nb_positive)
    SLOT(__abs__, nb_absolute)

    SLOT(__bool__, nb_bool)

    SLOT(__invert__, nb_invert)
    SLOT(__lshift__, nb_lshift)
    SLOT(__rshift__, nb_rshift)
    SLOT(__and__, nb_and)
    SLOT(__xor__, nb_xor)
    SLOT(__or__, nb_or)
    SLOT(__int__, nb_int)
    SLOT(__float__, nb_float)

    if (name == "__iadd__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_inplace_add);
        return &type->tp_as_sequence->sq_inplace_repeat;
    }
    SLOT(__isub__, nb_inplace_subtract)
    if (name == "__imul__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_inplace_multiply);
        return &type->tp_as_sequence->sq_inplace_repeat;
    }
    SLOT(__imod__, nb_inplace_remainder)
    SLOT(__ipow__, nb_inplace_power)
    SLOT(__ilshift__, nb_inplace_lshift)
    SLOT(__irshift__, nb_inplace_rshift)
    SLOT(__iand__, nb_inplace_and)
    SLOT(__ixor__, nb_inplace_xor)
    SLOT(__ior__, nb_inplace_or)

    SLOT(__floordiv__, nb_floor_divide)
    SLOT(__truediv__, nb_true_divide)
    SLOT(__ifloordiv__, nb_inplace_floor_divide)
    SLOT(__itruediv__, nb_inplace_true_divide)

    SLOT(__index__, nb_index)

    SLOT(__matmul__, nb_matrix_multiply)
    SLOT(__imatmul__, nb_inplace_matrix_multiply)

#undef SLOT

    return INVALID_MAGIC_METHOD;
}

static MagicMethodLookupResult get_magic_method_implementation(char const *name_str)
{
    std::string name{name_str};

#define _IMPL(method_name)        \
    if (name == #method_name)     \
    {                             \
        return impl##method_name; \
    }

#define IMPL(method_name) _IMPL(__##method_name##__)

    IMPL(repr)
    IMPL(call)
    IMPL(hash)
    IMPL(str)
    if (name == "__getattr__")
    {
        return PyObject_GenericGetAttr;
    }
    if (name == "__setattr__")
    {
        return PyObject_GenericSetAttr;
    }
    IMPL(iter)
    IMPL(next)

    IMPL(len)
    IMPL(contains)
    IMPL(getitem)
    IMPL(setitem)

    IMPL(add)
    IMPL(sub)
    IMPL(mul)
    IMPL(mod)
    IMPL(divmod)
    IMPL(pow)
    IMPL(neg)
    IMPL(pos)
    IMPL(abs)

    IMPL(bool)

    IMPL(invert)
    IMPL(lshift)
    IMPL(rshift)
    IMPL(and)
    IMPL(xor)
    IMPL(or)
    IMPL(int)
    IMPL(float)

    IMPL(iadd)
    IMPL(isub)
    IMPL(imul)
    IMPL(imod)
    IMPL(ipow)
    IMPL(ilshift)
    IMPL(irshift)
    IMPL(iand)
    IMPL(ixor)
    IMPL(ior)

    IMPL(floordiv)
    IMPL(truediv)
    IMPL(ifloordiv)
    IMPL(itruediv)

    IMPL(index)

    IMPL(matmul)
    IMPL(imatmul)

#undef IMPL
#undef _IMPL

    return INVALID_MAGIC_METHOD;
}

static PyObject *enable_magic_method(PyObject *self, PyObject *args)
{
    PyTypeObject *type;
    PyObject *method;
    if (!PyArg_ParseTuple(args, "OO", &type, &method))
    {
        return NULL;
    }

    char const *method_name = PyUnicode_AsUTF8(method);
    auto slot_result = get_magic_method_slot(type, method_name);
    if (slot_result.is_error())
    {
        if (slot_result.error_code == INVALID_MAGIC_METHOD)
        {
            PyErr_SetString(PyExc_ValueError, (std::string{"[ExType] Invalid magic method name: "} + method_name).c_str());
            return NULL;
        }
        if (slot_result.error_code == NOT_IMPLEMENTED)
        {
            PyErr_SetString(PyExc_TypeError, (std::string{"[ExType] Could not find a slot for method: "} + method_name).c_str());
            return NULL;
        }

        PyErr_SetString(PyExc_RuntimeError, "[ExType] Unknown error :: get_magic_method_slot()");
        return NULL;
    }

    void **slot = (void **)slot_result.result;

    auto impl_result = get_magic_method_implementation(method_name);

    if (impl_result.is_error())
    {
        if (impl_result.error_code == NOT_IMPLEMENTED)
        {
            PyErr_SetString(PyExc_NotImplementedError, (std::string{"[ExType] Method '"} + method_name + "' is not implemented").c_str());
            return NULL;
        }
        else
        {
            Py_RETURN_NONE;
        }
    }

    void *impl = impl_result.result;

    if (slot == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "slot was null");
        return NULL;
    }

    *slot = impl;

    Py_RETURN_NONE;
}

static PyMethodDef ExTypeMethods[] = {
    {"get_builtin_type_dict", get_builtin_type_dict, METH_VARARGS, "get the builtin type's tp_dict slot"},
    {"implement_protocol_on_type", implement_protocol_on_type, METH_VARARGS, "implement protocols by the given flag"},
    {"enable_magic_method", enable_magic_method, METH_VARARGS, "enable magic method on"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef ExTypeModule{
    PyModuleDef_HEAD_INIT,
    "extype",                                          /* name of module */
    "ExType enables extending any type super easily.", /* module documentation, may be NULL */
    -1,                                                /* size of per-interpreter state of the module,
                                                          or -1 if the module keeps state in global variables. */
    ExTypeMethods};

PyMODINIT_FUNC PyInit_core(void)
{
    return PyModule_Create(&ExTypeModule);
}
