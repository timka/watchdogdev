#include <Python.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <linux/watchdog.h>


PyDoc_STRVAR(module_doc,
("This module implements watchdog API.\n"
 "Currently it can use only Linux watchdog device API.\n"
 "\n"
 "Objects:\n"
 "watchdog -- type object for watchdog objects"));


/*
 * Module functions (nothing is exported)
 */
static PyMethodDef module_functions[] = {
    {NULL, NULL, 0, NULL} /* Sentinel */
};

/*
 * Watchdog object
 */
PyDoc_STRVAR(watchdog_doc, (
"Watchdog object type.\n"
"\n"
"The object can be simply used as file object using the write()\n"
"method to keep the timer ticking.\n"
"\n"
"Also, it defines several IOCTL wrapper methods which can be used to send\n"
"commands to watchdog device. Not all watchdog drivers support all commands.\n"
"The caller is responsible to check if particular command if supported using\n"
"the options attribute of the watchdog object and WDIOF_* constants exported\n"
"by the module."));

/*
 *   Helper macros for version compatibility
 */
#if PY_MAJOR_VERSION >= 3
    #define PyInt_FromLong PyLong_FromLong
    #define PyString_FromString PyUnicode_FromString
#endif

/*
 *   Helper macros for methods
 */
#define METH(meth) watchdog_ ## meth
#define METH_ENTRY(meth)  #meth, (PyCFunction)watchdog_ ## meth
#define METH_ENTRY_NA(meth) METH_ENTRY(meth), METH_NOARGS
#define METH_ENTRY_VA(meth) METH_ENTRY(meth), METH_VARARGS

/* method prototype */
#define METH_PROTO(meth) \
static PyObject *METH(meth)(watchdogobject *self, PyObject *args)

/* getter method prototype */
#define GETTER_PROTO(meth) \
static PyObject *METH(meth)(watchdogobject *self, void *closure)

/* IOCTL wrapper with no args returning int */
#define IOCTL_GET_INT(cmd, meth) \
METH_PROTO(meth) { \
    int value; \
    if (ioctl(self->fd, WDIOC_ ## cmd, &value) == -1) { \
        PyErr_SetFromErrno(PyExc_IOError); \
        return NULL; \
    } \
    return PyInt_FromLong(value); \
}

/* IOCTL wrapper with int arg returning None */
#define IOCTL_SET_INT(cmd, meth) \
METH_PROTO(meth) { \
    int value; \
    if (!PyArg_ParseTuple(args, "i:" #meth, &value)) { \
        return NULL; \
    } \
    if (ioctl(self->fd, WDIOC_ ## cmd, &value) == -1) { \
        PyErr_SetFromErrno(PyExc_IOError); \
        return NULL; \
    } \
    Py_RETURN_NONE; \
}

typedef struct {
    PyObject_HEAD

    char *filename;
    int fd;
    struct watchdog_info *device_info;
} watchdogobject;

static PyTypeObject WatchdogType; 

static void watchdog_dealloc(watchdogobject *self);

static PyObject *watchdog_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

/* File methods */
METH_PROTO(write);
METH_PROTO(fileno);
METH_PROTO(close);

/* IOCTL warappers */
METH_PROTO(get_support);
METH_PROTO(keep_alive);
METH_PROTO(magic_close);
METH_PROTO(get_status);
METH_PROTO(get_boot_status);
METH_PROTO(get_temp);
METH_PROTO(set_options);
METH_PROTO(get_timeout);
METH_PROTO(set_timeout);
METH_PROTO(set_pretimeout);
METH_PROTO(get_pretimeout);
METH_PROTO(get_time_left);

/* getseters */
GETTER_PROTO(get_name);
GETTER_PROTO(get_closed);
GETTER_PROTO(get_options);
GETTER_PROTO(get_firmware_version);
GETTER_PROTO(get_identity);

/* Properties */
static PyGetSetDef watchdog_getseters[] = {
    {"name",
     (getter)METH(get_name), NULL,
     NULL,
     NULL},
    {"closed",
     (getter)METH(get_closed), NULL,
     NULL,
     NULL},

    {"options",
     (getter)METH(get_options), NULL,
     NULL,
     NULL},
    {"firmware_version",
     (getter)METH(get_firmware_version), NULL,
     NULL,
     NULL},
    {"identity",
     (getter)METH(get_identity), NULL,
     NULL,
     NULL},

    {NULL}  /* Sentinel */
};

/* Method table */
static PyMethodDef watchdog_methods[] = {
    /* File methods */
    { METH_ENTRY_VA(write) },
    { METH_ENTRY_NA(close) },
    { METH_ENTRY_NA(fileno) },

    /* IOCTL wrappers */
    { METH_ENTRY_NA(get_support),
      ("Send GETSUPPORT command."
       " This will set the following attributes:\n"
       "        options\n"
       "        firmware_version\n"
       "        identity")
    },

    { METH_ENTRY_NA(keep_alive),
      "Send KEEPALIVE command."},

    { METH_ENTRY_NA(magic_close),
      "Write special character to device and close it immediately.\n"
      "This will force timer stop."}, 

    { METH_ENTRY_NA(get_status),
      ("Get the current status (options bit flags).\n"
       "\n"
       "\n"
       "        WDIOF_OVERHEAT		Reset due to CPU overheat\n"
       "\n"
       "The machine was last rebooted by the watchdog because the thermal limit was\n"
       "exceeded\n"
       "\n"
       "        WDIOF_FANFAULT		Fan failed\n"
       "\n"
       "A system fan monitored by the watchdog card has failed\n"
       "\n"
       "        WDIOF_EXTERN1		External relay 1\n"
       "\n"
       "External monitoring relay/source 1 was triggered. Controllers intended for\n"
       "real world applications include external monitoring pins that will trigger\n"
       "a reset.\n"
       "\n"
       "        WDIOF_EXTERN2		External relay 2\n"
       "\n"
       "External monitoring relay/source 2 was triggered\n"
       "\n"
       "        WDIOF_POWERUNDER	Power bad/power fault\n"
       "\n"
       "The machine is showing an undervoltage status\n"
       "\n"
       "        WDIOF_CARDRESET		Card previously reset the CPU\n"
       "\n"
       "The last reboot was caused by the watchdog card\n"
       "\n"
       "        WDIOF_POWEROVER		Power over voltage\n"
       "\n"
       "The machine is showing an overvoltage status. Note that if one level is\n"
       "under and one over both bits will be set - this may seem odd but makes\n"
       "sense.\n"

       "        WDIOF_KEEPALIVEPING	Keep alive ping reply\n"
       "\n"
       "The watchdog saw a keepalive ping since it was last queried.\n"
       "\n"
       "        WDIOF_SETTIMEOUT	Can set/get the timeout\n"
       "\n"
       "The watchdog can do pretimeouts.\n"
       "\n"
       "        WDIOF_PRETIMEOUT	Pretimeout (in seconds), get/set\n"
       "\n"
       "\n"
       "For those drivers that return any bits set in the option field, the\n"
       "GETSTATUS and GETBOOTSTATUS ioctls can be used to ask for the current\n"
       "status, and the status at the last reboot, respectively.")},

    { METH_ENTRY_NA(get_boot_status),
      "Get the status at the last reboot."},

    { METH_ENTRY_NA(get_temp),
      "Get tempertature in fahrenheit dgrees."},

    { METH_ENTRY_VA(set_options),
      ("Set set watchdog options bits.\n"
       "The SETOPTIONS ioctl can be used to control some aspects of\n"
       "the cards operation; right now the pcwd driver is the only one\n"
       "supporting this ioctl. [XXX is this correct?]\n"
       "\n"
       "The following options are available:\n"
       "        WDIOS_DISABLECARD	Turn off the watchdog timer\n"
       "        WDIOS_ENABLECARD	Turn on the watchdog timer\n"
       "        WDIOS_TEMPPANIC		Kernel panic on temperature trip")}, 

    { METH_ENTRY_NA(get_timeout),
      "Get watchdog timer timeout in seconds."}, 

    { METH_ENTRY_VA(set_timeout),
      "Set watchdog timer timeout in seconds."}, 

    { METH_ENTRY_VA(set_pretimeout),
      ("Set timer pretimeout to n seconds.\n"
       "Pretimeout will go off n seconds before the timeout will.\n"
       "For instance, if timeout is 60 seconds and pretimeout is 10 seconds,\n"
       "pretimeout will go off in 50 seconds.")}, 

    { METH_ENTRY_NA(get_pretimeout),
      "Get current pretimeout."},

    { METH_ENTRY_NA(get_time_left),
      "Get time left."},

    { NULL, NULL} /* Sentinel */
};

/* Watchodg type defintion */
static PyTypeObject WatchdogType = {
        PyVarObject_HEAD_INIT(NULL, 0) /*ob_size*/
        "watchdogdev.watchdog",        /*tp_name*/
        sizeof(watchdogobject),        /*tp_basicsize*/
        0,                             /*tp_itemsize*/
        /* methods */
        (destructor)watchdog_dealloc,  /*tp_dealloc*/
        0,                             /*tp_print*/
        0,                             /*tp_getattr*/
        0,                             /*tp_setattr*/
        0,                             /*tp_compare*/
        0,                             /*tp_repr*/
        0,                             /*tp_as_number*/
        0,                             /*tp_as_sequence*/
        0,                             /*tp_as_mapping*/
        0,                             /*tp_hash*/
        0,                             /*tp_call*/
        0,                             /*tp_str*/
        0,                             /*tp_getattro*/
        0,                             /*tp_setattro*/
        0,                             /*tp_as_buffer*/
        Py_TPFLAGS_DEFAULT,            /*tp_flags*/
        watchdog_doc,                  /*tp_doc*/
        0,                             /*tp_traverse*/
        0,                             /*tp_clear*/
        0,                             /*tp_richcompare*/
        0,                             /*tp_weaklistoffset*/
        0,                             /*tp_iter*/
        0,                             /*tp_iternext*/
        watchdog_methods,              /*tp_methods*/
        0,                             /*tp_members*/
        watchdog_getseters,            /*tp_getset*/
        0,                             /* tp_base */
        0,                             /* tp_dict */
        0,                             /* tp_descr_get */
        0,                             /* tp_descr_set */
        0,                             /* tp_dictoffset */
        0,                             /* tp_init */
        0,                             /* tp_alloc */
        watchdog_new,                  /* tp_new */
};


static PyObject *watchdog_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    char *filename = NULL;
    int fd;
    watchdogobject *self = NULL;

    if (!PyArg_ParseTuple(args, "s:__new__", &filename)) {
        return NULL;
    }

    fd = open(filename, O_WRONLY);
    if (fd == -1) {
        PyErr_SetFromErrnoWithFilename(PyExc_IOError, filename);
        return NULL;
    }

    if ((self = PyObject_New(watchdogobject, &WatchdogType)) == NULL) {
        close(fd);
        return NULL;
    }

    self->filename = filename;
    self->fd = fd;
    self->device_info = NULL;
    return (PyObject *) self;
}


static void watchdog_dealloc(watchdogobject *self) {
    if (self->device_info != NULL) {
        free(self->device_info);
    }
    if (self->fd != -1) {
        close(self->fd);
    }
    PyObject_Del(self);
}

IOCTL_GET_INT(GETSTATUS, get_status);
IOCTL_GET_INT(GETBOOTSTATUS, get_boot_status);
IOCTL_GET_INT(GETTEMP, get_temp);
IOCTL_SET_INT(SETOPTIONS, set_options);
IOCTL_GET_INT(GETTIMEOUT, get_timeout);
IOCTL_SET_INT(SETTIMEOUT, set_timeout);
IOCTL_SET_INT(SETPRETIMEOUT, set_pretimeout);
IOCTL_GET_INT(GETPRETIMEOUT, get_pretimeout);
IOCTL_GET_INT(GETTIMELEFT, get_time_left);

METH_PROTO(write) {
    char *buffer;
    int nbytes, size;

    if (!PyArg_ParseTuple(args, "s#:write", &buffer, &size)) {
        return NULL;
    }

    Py_BEGIN_ALLOW_THREADS
    nbytes = write(self->fd, buffer, size);
    Py_END_ALLOW_THREADS

    if (nbytes == -1) {
        return PyErr_SetFromErrno(PyExc_IOError);
    }

    return PyInt_FromLong(nbytes);
}


METH_PROTO(close) {
    if (self->fd >= 0) {
        Py_BEGIN_ALLOW_THREADS
        close(self->fd);
        Py_END_ALLOW_THREADS
        self->fd = -1;
    }

    Py_RETURN_NONE;
}


METH_PROTO(fileno) {
    return PyInt_FromLong(self->fd);
}


METH_PROTO(get_support) {
    if (self->device_info == NULL) {
        self->device_info = malloc(sizeof(struct watchdog_info));
        if (self->device_info == NULL) {
            return PyErr_NoMemory();
        }
    }
    if (ioctl(self->fd, WDIOC_GETSUPPORT, self->device_info) == -1) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }

    Py_RETURN_NONE;
}


METH_PROTO(keep_alive) {
    int dummy;
    if (ioctl(self->fd, WDIOC_KEEPALIVE, &dummy) == -1) {
        PyErr_SetFromErrno(PyExc_IOError);
        return NULL;
    }

    Py_RETURN_NONE;
};


METH_PROTO(magic_close) {
    if (self->fd != -1) {
        int nbytes, size = 1;

        Py_BEGIN_ALLOW_THREADS
        nbytes = write(self->fd, "V", size);
        Py_END_ALLOW_THREADS

        if (nbytes < 0) {
            PyErr_SetFromErrno(PyExc_IOError);
            return NULL;
        }

        Py_BEGIN_ALLOW_THREADS
        close(self->fd);
        Py_END_ALLOW_THREADS
        self->fd = -1;
    }

    Py_RETURN_NONE;
}


GETTER_PROTO(get_name) {
    return PyString_FromString(self->filename);
}

GETTER_PROTO(get_closed) {
    PyObject *result = (self->fd == -1) ? Py_True : Py_False;

    Py_INCREF(result);
    return result;
}

GETTER_PROTO(get_options) {
    if (self->device_info != NULL) {
        return PyInt_FromLong(self->device_info->options);
    }

    Py_RETURN_NONE;
}

GETTER_PROTO(get_firmware_version) {
    if (self->device_info != NULL) {
        return PyInt_FromLong(self->device_info->firmware_version);
    }

    Py_RETURN_NONE;
}

GETTER_PROTO(get_identity) {
    if (self->device_info != NULL) {
        return PyString_FromString((char *)(self->device_info->identity));
    }

    Py_RETURN_NONE;
}

#define ADD_INT_CONSTANT(m, name) PyModule_AddIntConstant(m, #name, name)

#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "watchdogdev",       /* m_name */
        module_doc,          /* m_doc */
        -1,                  /* m_size */
        module_functions,    /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
#endif

static PyObject* moduleinit(void) {
    #if PY_MAJOR_VERSION >= 3
        PyObject *m = PyModule_Create(&moduledef);
    #else
        PyObject *m = Py_InitModule3("watchdogdev", module_functions, module_doc);
    #endif

    if (m == NULL) {
        return NULL;
    }

    if (PyType_Ready(&WatchdogType) < 0) {
        return NULL;
    }

    Py_INCREF(&WatchdogType);
    PyModule_AddObject(m, "watchdog", (PyObject *) &WatchdogType);

    ADD_INT_CONSTANT(m, WDIOC_GETSUPPORT);
    ADD_INT_CONSTANT(m, WDIOC_GETSTATUS);
    ADD_INT_CONSTANT(m, WDIOC_GETBOOTSTATUS);
    ADD_INT_CONSTANT(m, WDIOC_GETTEMP);
    ADD_INT_CONSTANT(m, WDIOC_SETOPTIONS);
    ADD_INT_CONSTANT(m, WDIOC_KEEPALIVE);
    ADD_INT_CONSTANT(m, WDIOC_SETTIMEOUT);
    ADD_INT_CONSTANT(m, WDIOC_GETTIMEOUT);
    ADD_INT_CONSTANT(m, WDIOC_SETPRETIMEOUT);
    ADD_INT_CONSTANT(m, WDIOC_GETPRETIMEOUT);
    ADD_INT_CONSTANT(m, WDIOC_GETTIMELEFT);

    ADD_INT_CONSTANT(m, WDIOF_UNKNOWN);
    ADD_INT_CONSTANT(m, WDIOS_UNKNOWN);

    ADD_INT_CONSTANT(m, WDIOF_OVERHEAT);
    ADD_INT_CONSTANT(m, WDIOF_FANFAULT);
    ADD_INT_CONSTANT(m, WDIOF_EXTERN1);
    ADD_INT_CONSTANT(m, WDIOF_EXTERN2);
    ADD_INT_CONSTANT(m, WDIOF_POWERUNDER);
    ADD_INT_CONSTANT(m, WDIOF_CARDRESET);
    ADD_INT_CONSTANT(m, WDIOF_POWEROVER);
    ADD_INT_CONSTANT(m, WDIOF_SETTIMEOUT);
    ADD_INT_CONSTANT(m, WDIOF_MAGICCLOSE);
    ADD_INT_CONSTANT(m, WDIOF_PRETIMEOUT);
    ADD_INT_CONSTANT(m, WDIOF_KEEPALIVEPING);

    ADD_INT_CONSTANT(m, WDIOS_DISABLECARD);
    ADD_INT_CONSTANT(m, WDIOS_ENABLECARD);
    ADD_INT_CONSTANT(m, WDIOS_TEMPPANIC);

    return m;
};

#if PY_MAJOR_VERSION < 3
    PyMODINIT_FUNC initwatchdogdev(void)
    {
        moduleinit();
    }
#else
    PyMODINIT_FUNC PyInit_watchdogdev(void)
    {
        return moduleinit();
    }
#endif
