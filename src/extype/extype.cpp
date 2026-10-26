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

    template <typename T>
    MagicMethodLookupResult(T result)
        : MagicMethodLookupResult{0, reinterpret_cast<void *>(result)}
    {
    }

    bool is_error() { return error_code != 0; }

    static MagicMethodLookupResult error(uint32_t ec)
    {
        return MagicMethodLookupResult(ec, NULL);
    }
};

static constexpr uint32_t INVALID_MAGIC_METHOD = 1U;
static constexpr uint32_t NOT_IMPLEMENTED = 2U;

// Free-threading support
//
// On free-threaded builds, changes to type slots are serialized with a mutex, and slot
// pointers are published atomically since other threads read them without any lock.
// On regular builds the GIL already provides both guarantees, so the macros do nothing.

#ifdef Py_GIL_DISABLED

static PyMutex type_mutation_mutex = {0};

#define TYPE_MUTATION_LOCK() PyMutex_Lock(&type_mutation_mutex)
#define TYPE_MUTATION_UNLOCK() PyMutex_Unlock(&type_mutation_mutex)
#define STORE_SLOT(slot, value) _Py_atomic_store_ptr_release((slot), (value))

#else

#define TYPE_MUTATION_LOCK()
#define TYPE_MUTATION_UNLOCK()
#define STORE_SLOT(slot, value) (*(slot) = (value))

#endif

// Helpers

// Calls `self.<name>(*args)` and returns a new reference to the result.
// Steals the reference to `args`, which may be NULL if building it failed.
static PyObject *call_method(PyObject *self, char const *name, PyObject *args)
{
    if (args == NULL)
    {
        return NULL;
    }

    PyObject *method = PyObject_GetAttrString(self, name);
    if (method == NULL)
    {
        Py_DECREF(args);
        return NULL;
    }

    PyObject *result = PyObject_Call(method, args, NULL);
    Py_DECREF(method);
    Py_DECREF(args);
    return result;
}

// Like `call_method`, for slots that return 0 on success and -1 on error.
static int call_method_status(PyObject *self, char const *name, PyObject *args)
{
    PyObject *result = call_method(self, name, args);
    if (result == NULL)
    {
        return -1;
    }
    Py_DECREF(result);
    return 0;
}

// Like `call_method`, for slots that return the truth value of the result.
static int call_method_truth(PyObject *self, char const *name, PyObject *args)
{
    PyObject *result = call_method(self, name, args);
    if (result == NULL)
    {
        return -1;
    }
    int truth = PyObject_IsTrue(result);
    Py_DECREF(result);
    return truth;
}

// Like `call_method`, for slots that return the result as a Py_ssize_t.
static Py_ssize_t call_method_ssize(PyObject *self, char const *name, PyObject *args)
{
    PyObject *result = call_method(self, name, args);
    if (result == NULL)
    {
        return -1;
    }
    Py_ssize_t value = PyNumber_AsSsize_t(result, NULL);
    Py_DECREF(result);
    return value;
}

// Implementations

#define UNARY_OP_IMPL(name)                                     \
    static PyObject *impl##name(PyObject *self)                 \
    {                                                           \
        if (!PyObject_HasAttrString(self, #name))               \
        {                                                       \
            Py_RETURN_NOTIMPLEMENTED;                           \
        }                                                       \
        return call_method(self, #name, PyTuple_Pack(0));       \
    }

#define UNARY_OP(name) UNARY_OP_IMPL(__##name##__)

#define BINARY_OP_IMPL(name)                                         \
    static PyObject *impl##name(PyObject *left, PyObject *right)     \
    {                                                                \
        if (!PyObject_HasAttrString(left, #name))                    \
        {                                                            \
            Py_RETURN_NOTIMPLEMENTED;                                \
        }                                                            \
        return call_method(left, #name, PyTuple_Pack(1, right));     \
    }

#define BINARY_OP(name) BINARY_OP_IMPL(__##name##__)

#define _RBINARY_OP_IMPL(name, rname)                                          \
    static PyObject *impl##name(PyObject *left, PyObject *right)               \
    {                                                                          \
        if (PyObject_HasAttrString(left, #name))                               \
        {                                                                      \
            PyObject *result = call_method(left, #name, PyTuple_Pack(1, right)); \
            if (result != Py_NotImplemented)                                   \
            {                                                                  \
                return result;                                                 \
            }                                                                  \
            Py_DECREF(result);                                                 \
        }                                                                      \
        if (!PyObject_HasAttrString(right, #rname))                            \
        {                                                                      \
            Py_RETURN_NOTIMPLEMENTED;                                          \
        }                                                                      \
        return call_method(right, #rname, PyTuple_Pack(1, left));              \
    }

#define RBINARY_OP(name) _RBINARY_OP_IMPL(__##name##__, __r##name##__)

#define TERNARY_OP_IMPL(name)                                                    \
    static PyObject *impl##name(PyObject *self, PyObject *left, PyObject *right) \
    {                                                                            \
        if (!PyObject_HasAttrString(self, #name))                                \
        {                                                                        \
            Py_RETURN_NOTIMPLEMENTED;                                            \
        }                                                                        \
        return call_method(self, #name, PyTuple_Pack(2, left, right));           \
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
             _PyType_Name(Py_TYPE(self)) +
             " does not implement the '__bool__' method")
                .c_str());
        return -1;
    }
    return call_method_truth(self, "__bool__", PyTuple_Pack(0));
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

UNARY_OP(index)

RBINARY_OP(matmul)
BINARY_OP(imatmul)

// End number protocol

static Py_ssize_t impl__len__(PyObject *self)
{
    return call_method_ssize(self, "__len__", PyTuple_Pack(0));
}

static int impl__contains__(PyObject *self, PyObject *value)
{
    return call_method_truth(self, "__contains__", PyTuple_Pack(1, value));
}

static PyObject *impl__getitem__(PyObject *self, PyObject *item)
{
    return call_method(self, "__getitem__", PyTuple_Pack(1, item));
}

static int impl__setitem__(PyObject *self, PyObject *item, PyObject *value)
{
    if (value == NULL)
    {
        return call_method_status(self, "__delitem__", PyTuple_Pack(1, item));
    }
    return call_method_status(self, "__setitem__", PyTuple_Pack(2, item, value));
}

// Sequence slots that take a Py_ssize_t instead of an object

static PyObject *impl_sq_item(PyObject *self, Py_ssize_t index)
{
    return call_method(self, "__getitem__", Py_BuildValue("(n)", index));
}

static int impl_sq_ass_item(PyObject *self, Py_ssize_t index, PyObject *value)
{
    if (value == NULL)
    {
        return call_method_status(self, "__delitem__", Py_BuildValue("(n)", index));
    }
    return call_method_status(self, "__setitem__", Py_BuildValue("(nO)", index, value));
}

static PyObject *impl_sq_repeat(PyObject *self, Py_ssize_t count)
{
    return call_method(self, "__mul__", Py_BuildValue("(n)", count));
}

static PyObject *impl_sq_inplace_repeat(PyObject *self, Py_ssize_t count)
{
    return call_method(self, "__imul__", Py_BuildValue("(n)", count));
}

// Special methods

static PyObject *impl__repr__(PyObject *self)
{
    return call_method(self, "__repr__", PyTuple_Pack(0));
}

static PyObject *impl__call__(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *method = PyObject_GetAttrString(self, "__call__");
    if (method == NULL)
    {
        return NULL;
    }
    PyObject *result = PyObject_Call(method, args, kwargs);
    Py_DECREF(method);
    return result;
}

static Py_hash_t impl__hash__(PyObject *self)
{
    Py_hash_t hash = call_method_ssize(self, "__hash__", PyTuple_Pack(0));
    // -1 is reserved for errors, so a successful hash of -1 becomes -2 (like Python does)
    if (hash == -1 && !PyErr_Occurred())
    {
        hash = -2;
    }
    return hash;
}

static PyObject *impl__str__(PyObject *self)
{
    return call_method(self, "__str__", PyTuple_Pack(0));
}

// PyObject_GenericGetAttr
// static PyObject* impl__getattr__(PyObject* self, PyObject* attr) {

// }

// PyObject_GenericSetAttr
// static PyObject* impl__setattr__(PyObject* self, PyObject* attr, PyObject* value) {

// }

static PyObject *impl__iter__(PyObject *self)
{
    return call_method(self, "__iter__", PyTuple_Pack(0));
}

static PyObject *impl__next__(PyObject *self)
{
    return call_method(self, "__next__", PyTuple_Pack(0));
}

// End special methods

// End

static PyObject *implement_protocol_on_type(PyObject *self, PyObject *args)
{
    PyTypeObject *type;
    uint8_t protocols;

    if (!PyArg_ParseTuple(args, "O!B", &PyType_Type, &type, &protocols))
    {
        return NULL;
    }

    TYPE_MUTATION_LOCK();

    if (protocols & Protocols::Number && type->tp_as_number == NULL)
    {
        STORE_SLOT(&type->tp_as_number, new PyNumberMethods());
    }
    if (protocols & Protocols::Sequence && type->tp_as_sequence == NULL)
    {
        STORE_SLOT(&type->tp_as_sequence, new PySequenceMethods());
    }
    if (protocols & Protocols::Mapping && type->tp_as_mapping == NULL)
    {
        STORE_SLOT(&type->tp_as_mapping, new PyMappingMethods());
    }

    TYPE_MUTATION_UNLOCK();

    Py_RETURN_NONE;
}

// Returns a new reference to the type's dict
static PyObject *type_dict_of(PyTypeObject *type)
{
#if PY_VERSION_HEX >= 0x030C0000

    PyObject *dict = PyType_GetDict(type);

#else

    PyObject *dict = type->tp_dict;
    Py_XINCREF(dict);

#endif

    if (dict == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "type dict was null");
    }
    return dict;
}

static PyObject *get_type_dict(PyObject *self, PyObject *args)
{
    PyTypeObject *type;
    if (!PyArg_ParseTuple(args, "O!", &PyType_Type, &type))
    {
        return NULL;
    }

    return type_dict_of(type);
}

static PyObject *set_type_attribute(PyObject *self, PyObject *args)
{
    PyTypeObject *type;
    PyObject *name;
    PyObject *value;
    if (!PyArg_ParseTuple(args, "O!UO", &PyType_Type, &type, &name, &value))
    {
        return NULL;
    }

    PyObject *dict = type_dict_of(type);
    if (dict == NULL)
    {
        return NULL;
    }

    int status = PyDict_SetItem(dict, name, value);
    Py_DECREF(dict);
    if (status < 0)
    {
        return NULL;
    }

    // Writing to the dict directly bypasses type.__setattr__, so the attribute
    // cache has to be invalidated by hand or lookups may return the old value.
    PyType_Modified(type);

    Py_RETURN_NONE;
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
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED);
        if (type->tp_as_mapping != NULL)
            return &type->tp_as_mapping->mp_length;
        return &type->tp_as_sequence->sq_length;
    }
    if (name == "__getitem__")
    {
        if (type->tp_as_mapping == NULL && type->tp_as_sequence == NULL)
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED);
        if (type->tp_as_mapping != NULL)
            return &type->tp_as_mapping->mp_subscript;
        return &type->tp_as_sequence->sq_item;
    }
    if (name == "__setitem__")
    {
        if (type->tp_as_mapping == NULL && type->tp_as_sequence == NULL)
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED);
        if (type->tp_as_mapping != NULL)
            return &type->tp_as_mapping->mp_ass_subscript;
        return &type->tp_as_sequence->sq_ass_item;
    }
    if (name == "__contains__")
    {
        if (type->tp_as_sequence == NULL)
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED);
        return &type->tp_as_sequence->sq_contains;
    }

    // protocol methods

#define SLOT(method_name, field)                                    \
    if (name == #method_name)                                       \
    {                                                               \
        if (type->tp_as_number == NULL)                             \
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED); \
        return &(type->tp_as_number->field);                        \
    }

    if (name == "__add__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED);
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_add);
        return &(type->tp_as_sequence->sq_concat);
    }
    SLOT(__sub__, nb_subtract)
    if (name == "__mul__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED);
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
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED);
        if (type->tp_as_number != NULL)
            return &(type->tp_as_number->nb_inplace_add);
        return &type->tp_as_sequence->sq_inplace_concat;
    }
    SLOT(__isub__, nb_inplace_subtract)
    if (name == "__imul__")
    {
        if (type->tp_as_number == NULL && type->tp_as_sequence == NULL)
            return MagicMethodLookupResult::error(NOT_IMPLEMENTED);
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

    return MagicMethodLookupResult::error(INVALID_MAGIC_METHOD);
}

static MagicMethodLookupResult get_magic_method_implementation(char const *name_str)
{
    std::string name{name_str};

#define _IMPL(method_name)                \
    if (name == #method_name)             \
    {                                     \
        return reinterpret_cast<void*>(impl##method_name); \
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

    return MagicMethodLookupResult::error(INVALID_MAGIC_METHOD);
}

// The sequence slots that take a Py_ssize_t have a different signature than the
// matching number/mapping slots, so they need implementations of their own.
static void *get_sequence_slot_implementation(PyTypeObject *type, void **slot)
{
    PySequenceMethods *sequence = type->tp_as_sequence;
    if (sequence == NULL)
    {
        return NULL;
    }

    if (slot == reinterpret_cast<void **>(&sequence->sq_item))
    {
        return reinterpret_cast<void *>(impl_sq_item);
    }
    if (slot == reinterpret_cast<void **>(&sequence->sq_ass_item))
    {
        return reinterpret_cast<void *>(impl_sq_ass_item);
    }
    if (slot == reinterpret_cast<void **>(&sequence->sq_repeat))
    {
        return reinterpret_cast<void *>(impl_sq_repeat);
    }
    if (slot == reinterpret_cast<void **>(&sequence->sq_inplace_repeat))
    {
        return reinterpret_cast<void *>(impl_sq_inplace_repeat);
    }
    return NULL;
}

static PyObject *enable_magic_method_locked(PyTypeObject *type, char const *method_name)
{
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
            PyErr_SetString(PyExc_TypeError, (std::string{"[ExType] Could not find a slot for method (protocol not implemented): "} + method_name).c_str());
            return NULL;
        }

        PyErr_SetString(PyExc_RuntimeError, "[ExType] Unknown error :: get_magic_method_slot()");
        return NULL;
    }

    void **slot = (void **)slot_result.result;

    if (slot == NULL)
    {
        PyErr_SetString(PyExc_RuntimeError, "slot was null");
        return NULL;
    }

    void *impl = get_sequence_slot_implementation(type, slot);

    if (impl == NULL)
    {
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

        impl = impl_result.result;
    }

    STORE_SLOT(slot, impl);
    PyType_Modified(type);

    Py_RETURN_NONE;
}

static PyObject *enable_magic_method(PyObject *self, PyObject *args)
{
    PyTypeObject *type;
    PyObject *method;
    if (!PyArg_ParseTuple(args, "O!U", &PyType_Type, &type, &method))
    {
        return NULL;
    }

    char const *method_name = PyUnicode_AsUTF8(method);
    if (method_name == NULL)
    {
        return NULL;
    }

    TYPE_MUTATION_LOCK();
    PyObject *result = enable_magic_method_locked(type, method_name);
    TYPE_MUTATION_UNLOCK();

    return result;
}

static PyMethodDef ExTypeMethods[] = {
    {"get_type_dict", get_type_dict, METH_VARARGS, "get the given type's tp_dict slot"},
    {"set_type_attribute", set_type_attribute, METH_VARARGS, "set an attribute directly in the given type's dict"},
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
    PyObject *module = PyModule_Create(&ExTypeModule);

#ifdef Py_GIL_DISABLED
    // Tell the interpreter not to re-enable the GIL when this module is imported
    if (module != NULL && PyUnstable_Module_SetGIL(module, Py_MOD_GIL_NOT_USED) < 0)
    {
        Py_DECREF(module);
        return NULL;
    }
#endif

    return module;
}
