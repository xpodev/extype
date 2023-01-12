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

#define UNARY_OP(name)                                                                    \
    static PyObject *impl##name(PyObject *self)                                           \
    {                                                                                     \
        if (!PyObject_HasAttrString(self, #name))                                         \
        {                                                                                 \
            Py_RETURN_NOTIMPLEMENTED;                                                     \
        }                                                                                 \
        return PyObject_Call(PyObject_GetAttrString(self, #name), PyTuple_Pack(0), NULL); \
    }

#define BINARY_OP(name)                                                                          \
    static PyObject *impl##name(PyObject *left, PyObject *right)                                 \
    {                                                                                            \
        if (!PyObject_HasAttrString(left, #name))                                                \
        {                                                                                        \
            Py_RETURN_NOTIMPLEMENTED;                                                            \
        }                                                                                        \
        return PyObject_Call(PyObject_GetAttrString(left, #name), PyTuple_Pack(1, right), NULL); \
    }

#define TERNARY_OP(name)                                                                               \
    static PyObject *impl##name(PyObject *self, PyObject *left, PyObject *right)                       \
    {                                                                                                  \
        if (!PyObject_HasAttrString(self, #name))                                                      \
        {                                                                                              \
            Py_RETURN_NOTIMPLEMENTED;                                                                  \
        }                                                                                              \
        return PyObject_Call(PyObject_GetAttrString(self, #name), PyTuple_Pack(2, left, right), NULL); \
    }

// Number protocol

BINARY_OP(__add__)
BINARY_OP(__sub__)
BINARY_OP(__mul__)
BINARY_OP(__mod__)
BINARY_OP(__divmod__)
TERNARY_OP(__pow__)
UNARY_OP(__neg__)
UNARY_OP(__pos__)
UNARY_OP(__abs__)

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

UNARY_OP(__invert__)
BINARY_OP(__shl__)
BINARY_OP(__shr__)
BINARY_OP(__and__)
BINARY_OP(__xor__)
BINARY_OP(__or__)
UNARY_OP(__int__)
UNARY_OP(__float__)

BINARY_OP(__iadd__)
BINARY_OP(__isub__)
BINARY_OP(__imul__)
BINARY_OP(__imod__)
TERNARY_OP(__ipow__)
BINARY_OP(__ishl__)
BINARY_OP(__ishr__)
BINARY_OP(__iand__)
BINARY_OP(__ixor__)
BINARY_OP(__ior__)

BINARY_OP(__floordiv__)
BINARY_OP(__truediv__)
BINARY_OP(__ifloordiv__)
BINARY_OP(__itruediv__)

BINARY_OP(__index__)

BINARY_OP(__matmul__)
BINARY_OP(__imatmul__)

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
    return PyNumber_AsSsize_t(PyObject_Call(PyObject_GetAttrString(self, "__setitem__"), PyTuple_Pack(0), NULL), NULL);
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

static Py_hash_t impl__hash__(PyObject* self) {
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

#define CASE_NUMBER(method_name, field)      \
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
    CASE_NUMBER(__sub__, nb_subtract)
    if (name == "__mul__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_multiply);
        return &type->tp_as_sequence->sq_repeat;
    }
    CASE_NUMBER(__mod__, nb_remainder)
    CASE_NUMBER(__divmod__, nb_divmod)
    CASE_NUMBER(__pow__, nb_power)
    CASE_NUMBER(__neg__, nb_negative)
    CASE_NUMBER(__pos__, nb_positive)
    CASE_NUMBER(__abs__, nb_absolute)

    CASE_NUMBER(__bool__, nb_bool)

    CASE_NUMBER(__invert__, nb_invert)
    CASE_NUMBER(__shl__, nb_lshift)
    CASE_NUMBER(__shr__, nb_rshift)
    CASE_NUMBER(__and__, nb_and)
    CASE_NUMBER(__xor__, nb_xor)
    CASE_NUMBER(__or__, nb_or)
    CASE_NUMBER(__int__, nb_int)
    CASE_NUMBER(__float__, nb_float)

    if (name == "__iadd__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_inplace_add);
        return &type->tp_as_sequence->sq_inplace_repeat;
    }
    CASE_NUMBER(__isub__, nb_inplace_subtract)
    if (name == "__imul__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return NOT_IMPLEMENTED;
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_inplace_multiply);
        return &type->tp_as_sequence->sq_inplace_repeat;
    }
    CASE_NUMBER(__imod__, nb_inplace_remainder)
    CASE_NUMBER(__ipow__, nb_inplace_power)
    CASE_NUMBER(__ishl__, nb_inplace_lshift)
    CASE_NUMBER(__ishr__, nb_inplace_rshift)
    CASE_NUMBER(__iand__, nb_inplace_and)
    CASE_NUMBER(__ixor__, nb_inplace_xor)
    CASE_NUMBER(__ior__, nb_inplace_or)

    CASE_NUMBER(__floordiv__, nb_floor_divide)
    CASE_NUMBER(__truediv__, nb_true_divide)
    CASE_NUMBER(__ifloordiv__, nb_inplace_floor_divide)
    CASE_NUMBER(__itruediv__, nb_inplace_true_divide)

    CASE_NUMBER(__index__, nb_index)

    CASE_NUMBER(__matmul__, nb_matrix_multiply)
    CASE_NUMBER(__imatmul__, nb_inplace_matrix_multiply)

#undef CASE_NUMBER

    return INVALID_MAGIC_METHOD;
}

static MagicMethodLookupResult get_magic_method_implementation(char const *name_str)
{
    std::string name{name_str};

#define IMPL(method_name)         \
    if (name == #method_name)     \
    {                             \
        return impl##method_name; \
    }

    IMPL(__repr__)
    IMPL(__call__)
    if (name == "__hash__")
    {
        return impl__hash__;
    }
    IMPL(__str__)
    if (name == "__getattr__")
    {
        return PyObject_GenericGetAttr;
    }
    if (name == "__setattr__")
    {
        return PyObject_GenericSetAttr;
    }
    IMPL(__iter__)
    IMPL(__next__)

    IMPL(__len__)
    IMPL(__contains__)
    IMPL(__getitem__)
    IMPL(__setitem__)

    IMPL(__add__)
    IMPL(__sub__)
    IMPL(__mul__)
    IMPL(__mod__)
    IMPL(__divmod__)
    IMPL(__pow__)
    IMPL(__neg__)
    IMPL(__pos__)
    IMPL(__abs__)

    IMPL(__bool__)

    IMPL(__invert__)
    IMPL(__shl__)
    IMPL(__shr__)
    IMPL(__and__)
    IMPL(__xor__)
    IMPL(__or__)
    IMPL(__int__)
    IMPL(__float__)

    IMPL(__iadd__)
    IMPL(__isub__)
    IMPL(__imul__)
    IMPL(__imod__)
    IMPL(__ipow__)
    IMPL(__ishl__)
    IMPL(__ishr__)
    IMPL(__iand__)
    IMPL(__ixor__)
    IMPL(__ior__)

    IMPL(__floordiv__)
    IMPL(__truediv__)
    IMPL(__ifloordiv__)
    IMPL(__itruediv__)

    IMPL(__index__)

    IMPL(__matmul__)
    IMPL(__imatmul__)

#undef IMPL

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
            PyErr_SetString(PyExc_ValueError, (std::string{"[ExTypes] Invalid magic method name: "} + method_name).c_str());
            return NULL;
        }
        if (slot_result.error_code == NOT_IMPLEMENTED)
        {
            PyErr_SetString(PyExc_TypeError, (std::string{"[ExTypes] Could not find a slot for method: "} + method_name).c_str());
            return NULL;
        }

        PyErr_SetString(PyExc_RuntimeError, "[ExTypes] Unknown error :: get_magic_method_slot()");
        return NULL;
    }

    void **slot = (void **)slot_result.result;

    auto impl_result = get_magic_method_implementation(method_name);

    if (impl_result.is_error())
    {
        if (impl_result.error_code == NOT_IMPLEMENTED)
        {
            PyErr_SetString(PyExc_NotImplementedError, (std::string{"[ExTypes] Method '"} + method_name + "' is not implemented").c_str());
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

static PyMethodDef ExTypesMethods[] = {
    {"get_builtin_type_dict", get_builtin_type_dict, METH_VARARGS, "get the builtin type's tp_dict slot"},
    {"implement_protocol_on_type", implement_protocol_on_type, METH_VARARGS, "implement protocols by the given flag"},
    {"enable_magic_method", enable_magic_method, METH_VARARGS, "enable magic method on"},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef ExTypesModule{
    PyModuleDef_HEAD_INIT,
    "extypes",                                          /* name of module */
    "ExTypes enables extending any type super easily.", /* module documentation, may be NULL */
    -1,                                                 /* size of per-interpreter state of the module,
                                                           or -1 if the module keeps state in global variables. */
    ExTypesMethods};

PyMODINIT_FUNC PyInit_core(void)
{
    return PyModule_Create(&ExTypesModule);
}
