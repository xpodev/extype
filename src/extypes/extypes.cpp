#define PY_SSIZE_T_CLEAN

#include <string>

#include <Python.h>


enum class Protocols {
    None = 0,
    Number = 1 << 0,
    Sequence = 1 << 1,
    Mapping = 1 << 2,
};


static int operator&(uint8_t const x, Protocols const y) {
    return static_cast<int>(x) & static_cast<int>(y);
}


// Implementations

#define UNARY_OP(name) \
static PyObject* impl##name(PyObject* self) { \
    if (!PyObject_HasAttrString(self, #name)) { \
        Py_RETURN_NOTIMPLEMENTED; \
    } \
    return PyObject_Call(PyObject_GetAttrString(self, #name), PyTuple_Pack(0), NULL); \
}

#define BINARY_OP(name) \
static PyObject* impl##name(PyObject* left, PyObject* right) { \
    if (!PyObject_HasAttrString(left, #name)) { \
        Py_RETURN_NOTIMPLEMENTED; \
    } \
    return PyObject_Call(PyObject_GetAttrString(left, #name), PyTuple_Pack(1, right), NULL); \
}

#define TERNARY_OP(name) \
static PyObject* impl##name(PyObject* self, PyObject* left, PyObject* right) { \
    if (!PyObject_HasAttrString(self, #name)) { \
        Py_RETURN_NOTIMPLEMENTED; \
    } \
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

static int impl__bool__(PyObject* self) {
    if (!PyObject_HasAttrString(self, "__bool__")) {
        PyErr_SetString(
            PyExc_NotImplementedError, 
            (std::string{"Type "} + 
            _PyType_Name((PyTypeObject*)PyObject_Type(self)) + 
            " does not implement the '__bool__' method"
            ).c_str()
        );
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


// Mapping protocol

// End mapping protocol


// Sequence protocol

// End sequence protocol


// Special methods

static PyObject* impl__call__(PyObject* self, PyObject* args, PyObject* kwargs) {
    return PyObject_Call(PyObject_GetAttrString(self, "__call__"), args, kwargs);
}

// End special methods

// End


static PyObject* implement_protocol_on_type(PyObject* self, PyObject* args) {
    PyTypeObject* type;
    uint8_t protocols;

    PyArg_ParseTuple(args, "OB", &type, &protocols);

    if (protocols & Protocols::Number && type->tp_as_number == NULL) {
        PyNumberMethods* protocol = new PyNumberMethods();
        type->tp_as_number = protocol;
    }
    if (protocols & Protocols::Sequence && type->tp_as_sequence == NULL) {
        PySequenceMethods* protocol = new PySequenceMethods();
        type->tp_as_sequence = protocol;
    }   
    if (protocols & Protocols::Mapping && type->tp_as_mapping == NULL) {
        PyMappingMethods* protocol = new PyMappingMethods();
        type->tp_as_mapping = protocol;
    }

    Py_RETURN_NONE;
}


static PyObject* get_builtin_type_dict(PyObject* self, PyObject* args) {
    PyTypeObject* type;
    PyArg_ParseTuple(args, "O", &type);

    Py_INCREF(type->tp_dict);
    return type->tp_dict;
}


static void* get_magic_method_slot(PyTypeObject* type, char const* name_str) {
    std::string name {name_str};

    // special methods

    if (name == "__repr__") {
        return &(type->tp_repr);
    }
    if (name == "__call__") {
        return &(type->tp_call);
    }
    if (name == "__hash__") {
        return &(type->tp_hash);
    }
    if (name == "__str__") {
        return &(type->tp_str);
    }
    if (name == "__getattr__") {
        return &(type->tp_getattro);
    }
    if (name == "__setattr__") {
        return &(type->tp_setattro);
    }
    if (name == "__iter__") {
        return &(type->tp_iter);
    }
    if (name == "__next__") {
        return &(type->tp_iternext);
    }

    // protocol methods

#define CASE_NUMBER(method_name, field) \
    if (name == #method_name) { \
        return &(type->tp_as_number->field); \
    }

// #define CASE_MAPPING(method_name, field) \
// if (name == #method_name) { \
//     return &type->tp_as_mapping.field; \
// }
// #define CASE_SEQUENCE(method_name, field) \
// if (name == #method_name) { \
//     return &type->tp_as_sequence.field; \
// }

    CASE_NUMBER(__add__, nb_add)
    CASE_NUMBER(__sub__, nb_subtract)
    CASE_NUMBER(__mul__, nb_multiply)
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

    CASE_NUMBER(__iadd__, nb_inplace_add)
    CASE_NUMBER(__isub__, nb_inplace_subtract)
    CASE_NUMBER(__imul__, nb_inplace_multiply)
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

    return NULL;
}


static void* get_magic_method_implementation(char const* name_str) {
    std::string name {name_str};

    if (name == "__call__") {
        return impl__call__;
    }

#define CASE_NUMBER(method_name) \
    if (name == #method_name) { \
        return impl##method_name; \
    }

    CASE_NUMBER(__add__)
    CASE_NUMBER(__sub__)
    CASE_NUMBER(__mul__)
    CASE_NUMBER(__mod__)
    CASE_NUMBER(__divmod__)
    CASE_NUMBER(__pow__)
    CASE_NUMBER(__neg__)
    CASE_NUMBER(__pos__)
    CASE_NUMBER(__abs__)

    CASE_NUMBER(__bool__)

    CASE_NUMBER(__invert__)
    CASE_NUMBER(__shl__)
    CASE_NUMBER(__shr__)
    CASE_NUMBER(__and__)
    CASE_NUMBER(__xor__)
    CASE_NUMBER(__or__)
    CASE_NUMBER(__int__)
    CASE_NUMBER(__float__)

    CASE_NUMBER(__iadd__)
    CASE_NUMBER(__isub__)
    CASE_NUMBER(__imul__)
    CASE_NUMBER(__imod__)
    CASE_NUMBER(__ipow__)
    CASE_NUMBER(__ishl__)
    CASE_NUMBER(__ishr__)
    CASE_NUMBER(__iand__)
    CASE_NUMBER(__ixor__)
    CASE_NUMBER(__ior__)

    CASE_NUMBER(__floordiv__)
    CASE_NUMBER(__truediv__)
    CASE_NUMBER(__ifloordiv__)
    CASE_NUMBER(__itruediv__)

    CASE_NUMBER(__index__)

    CASE_NUMBER(__matmul__)
    CASE_NUMBER(__imatmul__)

#undef CASE_NUMBER

    return NULL;
}


static PyObject* enable_magic_method(PyObject* self, PyObject* args) {
    PyTypeObject* type;
    PyObject* method;
    if (!PyArg_ParseTuple(args, "OO", &type, &method)) {
        return NULL;
    }

    char const* method_name = PyUnicode_AsUTF8(method);
    void** slot = (void**)get_magic_method_slot(type, method_name);

    if (slot == NULL) {
        PyErr_SetString(PyExc_ValueError, (std::string{"Invalid magic method name: "} + method_name).c_str());
        return NULL;
    }

    void* impl = get_magic_method_implementation(method_name);

    if (impl == NULL) {
        PyErr_SetString(PyExc_NotImplementedError, (std::string{"Method '"} + method_name + "' is not implemented").c_str());
        return NULL;
    }

    *slot = impl;

    Py_RETURN_NONE;
}


static PyMethodDef ExTypesMethods[] = {
    { "get_builtin_type_dict", get_builtin_type_dict, METH_VARARGS, "get the builtin type's tp_dict slot" },
    { "implement_protocol_on_type", implement_protocol_on_type, METH_VARARGS, "implement protocols by the given flag" },
    { "enable_magic_method", enable_magic_method, METH_VARARGS, "enable magic method on" },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


static PyModuleDef ExTypesModule {
    PyModuleDef_HEAD_INIT,
    "extypes",   /* name of module */
    "ExTypes enables extending any type super easily.", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    ExTypesMethods
};


PyMODINIT_FUNC PyInit_core(void) {
    return PyModule_Create(&ExTypesModule);
}
