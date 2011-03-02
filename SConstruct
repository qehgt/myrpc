import sys

RootPath = '../'
sys.path.append(RootPath + 'Utilities')
import sconslib

b = sconslib.builder()

Includes = [
    b.install_inc_path,
    'myrpc',
    '../Boost/include',
    ]

Sources = Glob('myrpc/src/*.cpp', strings=True)

out_lib = b.do_lib('myrpc', Sources, Includes)

headers = Glob('myrpc/inc/*.h')

b.env.Install(b.install_lib_path, out_lib)
b.env.Install(b.install_inc_path + '/myrpc', headers)


