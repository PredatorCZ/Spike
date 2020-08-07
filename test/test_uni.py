import sys
sys.path.append('../lib')
import test_unipy

skel = test_unipy.get_skeleton()

valid_names = ['Sample name.', 'skeleton_1']

assert(skel.name == valid_names[0])
assert(len(skel.bones) == 2)

boneTMType = test_unipy.uniBoneTMType()

assert(len(boneTMType) == 2)
assert(boneTMType[0] == 'TMTYPE_RTS')
assert(boneTMType[1] == 'TMTYPE_MATRIX')

throwed = False

try:
    boneTMType[10]
except IndexError:
    throwed = True

assert(throwed == True)

throwed = False

try:
    boneTMType.invalid
except SystemError:
    throwed = True

assert(throwed == True)

bne0 = skel.bones[0]

assert(bne0.name == 'bone0')
assert(bne0.index == 0)
assert(bne0.parent == None)
assert(bne0.tm_type == boneTMType.TMTYPE_RTS)
assert(bne0.transform == ((5, 6, 7, 8), (1, 2, 3, 4), (9, 10, 11, 12)))

bne1 = skel.bones[1]

assert(bne1.name == 'bone1')
assert(bne1.index == 1)
assert(bne1.parent == bne0)
assert(bne1.tm_type == boneTMType.TMTYPE_MATRIX)
assert(bne1.transform == ((1, 2, 3, 4), (5, 6, 7, 8), (9, 10, 11, 12), (13, 14, 15, 16)))

skels = test_unipy.get_skeletons()

assert(len(skels) == 2)

for sk in skels:
    valid_names.index(sk.name)

motionTrackType = test_unipy.uniMotionTrackType()

assert(len(motionTrackType) == 6)
assert(motionTrackType[0] == 'Position')
assert(motionTrackType[1] == 'Rotation')
assert(motionTrackType[2] == 'Scale')
assert(motionTrackType[3] == 'Matrix')
assert(motionTrackType[4] == 'PositionRotationScale')
assert(motionTrackType[5] == 'SingleFloat')

motionType = test_unipy.uniMotionType()

assert(len(motionType) == 4)
assert(motionType[0] == 'Absolute')
assert(motionType[1] == 'Relative')
assert(motionType[2] == 'Additive')
assert(motionType[3] == 'Delta')

motion = test_unipy.get_motion()

assert(motion.name == 'Sample motion.')
assert(motion.framerate == 30)
assert(motion.duration == 1.5)
assert(motion.motion_type == motionType.Delta)

motion.framerate = 60

assert(motion.framerate == 60)

tck0 = motion.tracks[0]
assert(tck0.track_type == motionTrackType.PositionRotationScale)
assert(tck0.bone_index == 1)
cvalues = tck0.get_values([0, 1])

assert(cvalues[0] == ((1.0, 2.0, 3.0, 4.0), (5.0, 6.0, 7.0, 8.0), (9.0, 10.0, 11.0, 12.0)))
assert(cvalues[1] == ((13.0, 14.0, 15.0, 16.0), (17.0, 18.0, 19.0, 20.0), (21.0, 22.0, 23.0, 24.0)))

tck1 = motion.tracks[1]
assert(tck1.track_type == motionTrackType.Matrix)
assert(tck1.bone_index == 2)
cvalues = tck1.get_values([0, 1])

assert(cvalues[0] == ((1.0, 2.0, 3.0, 4.0), (5.0, 6.0, 7.0, 8.0), (9.0, 10.0, 11.0, 12.0), (13.0, 14.0, 15.0, 16.0)))
assert(cvalues[1] == ((17.0, 18.0, 19.0, 20.0), (21.0, 22.0, 23.0, 24.0), (25.0, 26.0, 27.0, 28.0), (29.0, 30.0, 31.0, 32.0)))

tck2 = motion.tracks[2]
assert(tck2.track_type == motionTrackType.Position)
assert(tck2.bone_index == 3)
cvalues = tck2.get_values([0, 1])

assert(cvalues[0] == (1.0, 2.0, 3.0, 1.0))
assert(cvalues[1] == (3.0, 2.0, 1.0, 1.0))

tck3 = motion.tracks[3]
assert(tck3.track_type == motionTrackType.Rotation)
assert(tck3.bone_index == 4)
cvalues = tck3.get_values([0, 1])

assert(cvalues[0] == (0.25, 0.25, 0.25, 0.25))
assert(cvalues[1] == (0.0, 0.0, 1.0, 0.0))

tck4 = motion.tracks[4]
assert(tck4.track_type == motionTrackType.Scale)
assert(tck4.bone_index == 5)
cvalues = tck4.get_values([0, 1])

assert(cvalues[0] == (0.5, 0.5, 0.5, 0.0))
assert(cvalues[1] == (1.0, 1.0, 1.0, 0.0))

tck5 = motion.tracks[5]
assert(tck5.track_type == motionTrackType.SingleFloat)
assert(tck5.bone_index == 6)
cvalues = tck5.get_values([0, 1])

assert(cvalues[0] == 20)
assert(cvalues[1] == 50)
