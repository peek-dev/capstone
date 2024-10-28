#include <Python.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

static int uart_fd = -1;

static PyObject* uart_init(PyObject* self, PyObject* args) {
    // Open a file handle to file-mapped UART
    uart_fd = open("/dev/ttyAMA0", O_RDWR | O_APPEND);
    Py_RETURN_NONE;
}

static PyObject* uart_sendpacket(PyObject* self, PyObject* args) {
    uint32_t packet;
    char next_byte;

    if (!(PyArg_ParseTuple(args, "I", &packet))) {
        return NULL;
    } 

    // No lseek() call *should* be needed here since opening with O_RDWR and 
    // O_APPEND allows write() calls to always append

    for (int i = 0; i < 4; i += 1) {
        next_byte = (char) (packet & 0xFF);
        write(uart_fd, &next_byte, 1);
        packet = packet >> 8;
    }

    Py_RETURN_NONE;
}

static PyObject* uart_recvpacket(PyObject* self, PyObject* args) {
    uint32_t full_packet = 0;
    unsigned char next_byte;
    uint32_t shifted_byte;

    for (int i = 0; i < 4; i += 1) {
        read(uart_fd, &next_byte, 1);
        shifted_byte = ((uint32_t) next_byte) << (i * 8); 
        full_packet |= shifted_byte;
    }

    return PyLong_FromUnsignedLong(full_packet); 
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
