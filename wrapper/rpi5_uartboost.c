#include <Python.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

static int uart_fd = -1;
static FILE* uart_filestream = NULL;

static PyObject* uart_init(PyObject* self, PyObject* args) {
    // Open a file handle to file-mapped UART
    // TODO: swap current "dummy handle" with real valid handle for file-mapped
    // UART.
    uart_filestream = fopen("./sample.txt", "a+");
    Py_RETURN_NONE;
}

static PyObject* uart_sendpacket(PyObject* self, PyObject* args) {
    uint32_t packet;

    if (!(PyArg_ParseTuple(args, "I", &packet))) {
        return NULL;
    }

    // Ensure that we are appending to UART, not overwriting data at an 
    // intermediate position
    fseek(uart_filestream, 0, SEEK_END);

    uint8_t next_byte = 0;

    for (int i = 0; i < 4; i += 1) {
        next_byte = (uint8_t) ((packet >> (i*8)) & 0xFF);
        fwrite(&next_byte, sizeof(char), 1, uart_filestream);
    }

    Py_RETURN_NONE;
}

static PyObject* uart_recvpacket(PyObject* self, PyObject* args) {
    uint32_t full_packet = 0;
    uint8_t next_byte = 0;

    // TODO: verify fread() behavior (specifically offset) when dealing with 
    // "actual" file-mapped UART on Raspberry Pi OS
    fseek(uart_filestream, -4, SEEK_END);

    for (int i = 0; i < 4; i += 1) {
        fread(&next_byte, sizeof(char), 1, uart_filestream);
        full_packet |= ( ((uint32_t) next_byte) & 0xFF ) << (i * 8);
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
