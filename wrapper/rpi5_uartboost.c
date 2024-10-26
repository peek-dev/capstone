#include <Python.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static int uart_fd = -1;
static FILE* uart_fileptr = NULL;

static PyObject* uart_init(PyObject* self, PyObject* args) {
    /* TODO: add code to open UART (potentially as FILE*) and initialize this module's state */
    Py_RETURN_NONE;
}

static PyObject* uart_sendpacket(PyObject* self, PyObject* args) {
    uint32_t packet;

    if (!(PyArg_ParseTuple(args, "I", &packet))) {
        return NULL;
    }

    Py_RETURN_NONE;
}

static PyObject* uart_recvpacket(PyObject* self, PyObject* args) {
    char received_packet[5] = {0};
    /* TODO: add code to actually read from UART; */
    /* TODO: add low-level C library call(s) to parse char data as 32-bit unsigned int */
    return PyLong_FromString((const char*) received_packet, NULL, 10); // TODO: potentially modify to PyLong_FromUnsignedLong(unsigned long v)
}

static PyMethodDef uart_methods[] = {
    {"uart_init", uart_init, METH_VARARGS, "Initialize UART communication state, including file descriptor/file handle."},
    {"uart_sendpacket", uart_sendpacket, METH_VARARGS, "Send 32-bit packet to file-mapped UART."},
    {"uart_recvpacket", uart_recvpacket, METH_VARARGS, "Receive 32-bit packet from file-mapped UART."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef rpi5_uartboost = {
    PyModuleDef_HEAD_INIT,
    "rpi5_uartboost",
    NULL,
    -1,
    uart_methods
};

PyMODINIT_FUNC PyInit_rpi5_uartboost(void) {
    return PyModule_Create(&rpi5_uartboost);
}
