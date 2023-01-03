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
    return NULL;
}


static void* get_magic_method_implementation(char const* name_str) {
    std::string name {name_str};

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
