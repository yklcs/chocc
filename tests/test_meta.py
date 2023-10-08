import pytest
from chocc import *
from ctypes import *


@pytest.mark.parametrize(
    "path",
    ["test.c", "chocc.h"],
)
def test_meta(load_file, lex_file, cpp, path):
    print(path)
    file = load_file(c_char_p(path.encode()))
    unit = lex_file(file)
    unit = cpp(unit)
    for i in range(unit.contents.len):
        print(unit.contents.toks[i])
