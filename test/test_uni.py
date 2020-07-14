import sys
sys.path.append('../lib')
import test_unipy

skel = test_unipy.get_skeleton()

assert(skel.name == "Sample name.")
assert(len(skel.bones) == 2)

bne0 = skel.bones[0]
boneTMType = test_unipy.uniBoneTMType()

assert(bne0.name == "bone0")
assert(bne0.index == 0)
assert(bne0.parent == None)
assert(bne0.tm_type == boneTMType.TMTYPE_RTS)
assert(bne0.transform == ((5, 6, 7, 8), (1, 2, 3, 4), (9, 10, 11, 12)))

bne1 = skel.bones[1]

assert(bne1.name == "bone1")
assert(bne1.index == 1)
assert(bne1.parent == bne0)
assert(bne1.tm_type == boneTMType.TMTYPE_MATRIX)
assert(bne1.transform == ((1, 2, 3, 4), (5, 6, 7, 8), (9, 10, 11, 12), (13, 14, 15, 16)))

