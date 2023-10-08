from chocc import *
from ctypes import *


def test_file(src_to_file):
    src = b"int x;"
    file = src_to_file(c_char_p(src))
    assert file.contents.lines[0].src == src
