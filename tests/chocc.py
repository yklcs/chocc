import pytest
import os
from ctypes import *
from distutils.sysconfig import parse_makefile


class LOC(Structure):
    _fields_ = [("ln", c_int), ("col", c_int)]


class LINE(Structure):
    _fields_ = [
        ("num", c_int),
        ("src", c_char_p),
        ("len", c_int),
        ("splice", c_int),
        ("cpp", c_int),
    ]


class FILE(Structure):
    _fields_ = [
        ("lines", POINTER(LINE)),
        ("lines_len", c_int),
        ("lines_cap", c_int),
        ("cur", LOC),
    ]


class TOKEN(Structure):
    _fields_ = [("kind", c_int), ("line", c_int), ("column", c_int), ("text", c_char_p)]


class UNIT(Structure):
    _fields_ = [
        ("toks", POINTER(TOKEN)),
        ("len", c_int),
        ("cap", c_int),
    ]


@pytest.fixture
def chocc():
    makefile = parse_makefile("Makefile")
    dll = os.path.join(os.getcwd(), makefile["LIB"])
    chocc = CDLL(dll)
    return chocc


@pytest.fixture
def load_file(chocc):
    chocc.load_file.restype = POINTER(FILE)
    chocc.load_file.argtypes = [c_char_p]
    return chocc.load_file


@pytest.fixture
def src_to_file(chocc):
    chocc.src_to_file.restype = POINTER(FILE)
    chocc.src_to_file.argtypes = [c_char_p]
    return chocc.src_to_file


@pytest.fixture
def next_char(chocc):
    chocc.next_char.restype = c_char
    chocc.next_char.argtypes = [POINTER(FILE), POINTER(LOC)]
    return chocc.src_to_file


@pytest.fixture
def lex_file(chocc):
    chocc.lex_file.restype = POINTER(UNIT)
    chocc.lex_file.argtypes = [POINTER(FILE)]
    return chocc.lex_file


@pytest.fixture
def cpp(chocc):
    chocc.cpp.restype = POINTER(UNIT)
    chocc.cpp.argtypes = [POINTER(UNIT)]
    return chocc.cpp
