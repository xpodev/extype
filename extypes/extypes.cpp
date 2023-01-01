#define PY_SSIZE_T_CLEAN

#include <Python.h>


static PyObject* get_builtin_type_dict(PyObject* self, PyObject* args) {
    PyTypeObject* type;
    PyArg_ParseTuple(args, "O", &type);

    Py_INCREF(type->tp_dict);
    return type->tp_dict;
}


static PyMethodDef BuiltinsPlusMethods[] = {
    { "get_builtin_type_dict", get_builtin_type_dict, METH_VARARGS, "get the builtin type's tp_dict slot" },
    {NULL, NULL, 0, NULL}        /* Sentinel */
};


static PyModuleDef BuiltinsPlusModule {
    PyModuleDef_HEAD_INIT,
    "extypes",   /* name of module */
    "BuiltinsPlus allows you to extend the builtin types.", /* module documentation, may be NULL */
    -1,       /* size of per-interpreter state of the module,
                 or -1 if the module keeps state in global variables. */
    BuiltinsPlusMethods
};


PyMODINIT_FUNC PyInit_core(void) {

    return PyModule_Create(&BuiltinsPlusModule);
}
