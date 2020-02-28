/*  a header file for compile time loop
    more info in README for PreCore Project

    Copyright 2018-2020 Lukas Cone

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

// clang-format off
#pragma once
#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64,_65,_66,_67,_68,_69,_70,_71,_72,_73,_74,_75,_76,_77,_78,_79,_80,_81,_82,_83,_84,_85,_86,_87,_88,_89, N, ...) N
#define VA_NARGS_EVAL(x) x
#define VA_NARGS(...) VA_NARGS_EVAL(VA_NARGS_IMPL(__VA_ARGS__,89,87,88,86,85,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,59,58,57,56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

#define _LOOPER_CAT1(c1,c2) c1##c2
#define _LOOPER_CAT2(c1,c2) _LOOPER_CAT1(c1, c2)
#define _LOOPER_CAT(c1,...) _LOOPER_CAT2(c1, VA_NARGS(__VA_ARGS__))

#define _StaticFor_ADD0(eval, first,...)
#define _StaticFor_ADD1(eval, first,...) eval(first)
#define _StaticFor_ADD2(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD1(eval, __VA_ARGS__))
#define _StaticFor_ADD3(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD2(eval, __VA_ARGS__))
#define _StaticFor_ADD4(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD3(eval, __VA_ARGS__))
#define _StaticFor_ADD5(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD4(eval, __VA_ARGS__))
#define _StaticFor_ADD6(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD5(eval, __VA_ARGS__))
#define _StaticFor_ADD7(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD6(eval, __VA_ARGS__))
#define _StaticFor_ADD8(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD7(eval, __VA_ARGS__))
#define _StaticFor_ADD9(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD8(eval, __VA_ARGS__))
#define _StaticFor_ADD10(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD9(eval, __VA_ARGS__))
#define _StaticFor_ADD11(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD10(eval, __VA_ARGS__))
#define _StaticFor_ADD12(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD11(eval, __VA_ARGS__))
#define _StaticFor_ADD13(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD12(eval, __VA_ARGS__))
#define _StaticFor_ADD14(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD13(eval, __VA_ARGS__))
#define _StaticFor_ADD15(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD14(eval, __VA_ARGS__))
#define _StaticFor_ADD16(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD15(eval, __VA_ARGS__))
#define _StaticFor_ADD17(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD16(eval, __VA_ARGS__))
#define _StaticFor_ADD18(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD17(eval, __VA_ARGS__))
#define _StaticFor_ADD19(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD18(eval, __VA_ARGS__))
#define _StaticFor_ADD20(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD19(eval, __VA_ARGS__))
#define _StaticFor_ADD21(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD20(eval, __VA_ARGS__))
#define _StaticFor_ADD22(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD21(eval, __VA_ARGS__))
#define _StaticFor_ADD23(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD22(eval, __VA_ARGS__))
#define _StaticFor_ADD24(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD23(eval, __VA_ARGS__))
#define _StaticFor_ADD25(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD24(eval, __VA_ARGS__))
#define _StaticFor_ADD26(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD25(eval, __VA_ARGS__))
#define _StaticFor_ADD27(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD26(eval, __VA_ARGS__))
#define _StaticFor_ADD28(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD27(eval, __VA_ARGS__))
#define _StaticFor_ADD29(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD28(eval, __VA_ARGS__))
#define _StaticFor_ADD30(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD29(eval, __VA_ARGS__))
#define _StaticFor_ADD30(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD29(eval, __VA_ARGS__))
#define _StaticFor_ADD31(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD30(eval, __VA_ARGS__))
#define _StaticFor_ADD32(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD31(eval, __VA_ARGS__))
#define _StaticFor_ADD33(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD32(eval, __VA_ARGS__))
#define _StaticFor_ADD34(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD33(eval, __VA_ARGS__))
#define _StaticFor_ADD35(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD34(eval, __VA_ARGS__))
#define _StaticFor_ADD36(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD35(eval, __VA_ARGS__))
#define _StaticFor_ADD37(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD36(eval, __VA_ARGS__))
#define _StaticFor_ADD38(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD37(eval, __VA_ARGS__))
#define _StaticFor_ADD39(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD38(eval, __VA_ARGS__))
#define _StaticFor_ADD40(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD39(eval, __VA_ARGS__))
#define _StaticFor_ADD41(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD40(eval, __VA_ARGS__))
#define _StaticFor_ADD42(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD41(eval, __VA_ARGS__))
#define _StaticFor_ADD43(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD42(eval, __VA_ARGS__))
#define _StaticFor_ADD44(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD43(eval, __VA_ARGS__))
#define _StaticFor_ADD45(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD44(eval, __VA_ARGS__))
#define _StaticFor_ADD46(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD45(eval, __VA_ARGS__))
#define _StaticFor_ADD47(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD46(eval, __VA_ARGS__))
#define _StaticFor_ADD48(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD47(eval, __VA_ARGS__))
#define _StaticFor_ADD49(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD48(eval, __VA_ARGS__))
#define _StaticFor_ADD50(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD49(eval, __VA_ARGS__))
#define _StaticFor_ADD51(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD50(eval, __VA_ARGS__))
#define _StaticFor_ADD52(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD51(eval, __VA_ARGS__))
#define _StaticFor_ADD53(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD52(eval, __VA_ARGS__))
#define _StaticFor_ADD54(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD53(eval, __VA_ARGS__))
#define _StaticFor_ADD55(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD54(eval, __VA_ARGS__))
#define _StaticFor_ADD56(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD55(eval, __VA_ARGS__))
#define _StaticFor_ADD57(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD56(eval, __VA_ARGS__))
#define _StaticFor_ADD58(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD57(eval, __VA_ARGS__))
#define _StaticFor_ADD59(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD58(eval, __VA_ARGS__))
#define _StaticFor_ADD60(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD59(eval, __VA_ARGS__))
#define _StaticFor_ADD61(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD60(eval, __VA_ARGS__))
#define _StaticFor_ADD62(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD61(eval, __VA_ARGS__))
#define _StaticFor_ADD63(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD62(eval, __VA_ARGS__))
#define _StaticFor_ADD64(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD63(eval, __VA_ARGS__))
#define _StaticFor_ADD65(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD64(eval, __VA_ARGS__))
#define _StaticFor_ADD66(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD65(eval, __VA_ARGS__))
#define _StaticFor_ADD67(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD66(eval, __VA_ARGS__))
#define _StaticFor_ADD68(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD67(eval, __VA_ARGS__))
#define _StaticFor_ADD69(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD68(eval, __VA_ARGS__))
#define _StaticFor_ADD70(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD69(eval, __VA_ARGS__))
#define _StaticFor_ADD71(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD70(eval, __VA_ARGS__))
#define _StaticFor_ADD72(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD71(eval, __VA_ARGS__))
#define _StaticFor_ADD73(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD72(eval, __VA_ARGS__))
#define _StaticFor_ADD74(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD73(eval, __VA_ARGS__))
#define _StaticFor_ADD75(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD74(eval, __VA_ARGS__))
#define _StaticFor_ADD76(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD75(eval, __VA_ARGS__))
#define _StaticFor_ADD77(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD76(eval, __VA_ARGS__))
#define _StaticFor_ADD78(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD77(eval, __VA_ARGS__))
#define _StaticFor_ADD79(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD78(eval, __VA_ARGS__))
#define _StaticFor_ADD80(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD79(eval, __VA_ARGS__))
#define _StaticFor_ADD81(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD80(eval, __VA_ARGS__))
#define _StaticFor_ADD82(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD81(eval, __VA_ARGS__))
#define _StaticFor_ADD83(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD82(eval, __VA_ARGS__))
#define _StaticFor_ADD84(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD83(eval, __VA_ARGS__))
#define _StaticFor_ADD85(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD84(eval, __VA_ARGS__))
#define _StaticFor_ADD86(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD85(eval, __VA_ARGS__))
#define _StaticFor_ADD87(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD86(eval, __VA_ARGS__))
#define _StaticFor_ADD88(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD87(eval, __VA_ARGS__))
#define _StaticFor_ADD89(eval, first,...) eval(first) VA_NARGS_EVAL(_StaticFor_ADD88(eval, __VA_ARGS__))


#define _StaticForArgID_ADD1(eval, classname, _id, first,...) eval(classname, _id, first)
#define _StaticForArgID_ADD2(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD1(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD3(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD2(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD4(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD3(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD5(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD4(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD6(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD5(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD7(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD6(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD8(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD7(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD9(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD8(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD10(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD9(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD11(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD10(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD12(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD11(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD13(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD12(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD14(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD13(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD15(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD14(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD16(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD15(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD17(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD16(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD18(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD17(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD19(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD18(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD20(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD19(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD21(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD20(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD22(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD21(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD23(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD22(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD24(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD23(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD25(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD24(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD26(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD25(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD27(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD26(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD28(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD27(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD29(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD28(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD30(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD29(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD30(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD29(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD31(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD30(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD32(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD31(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD33(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD32(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD34(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD33(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD35(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD34(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD36(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD35(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD37(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD36(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD38(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD37(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD39(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD38(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD40(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD39(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD41(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD40(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD42(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD41(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD43(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD42(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD44(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD43(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD45(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD44(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD46(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD45(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD47(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD46(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD48(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD47(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD49(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD48(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD50(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD49(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD51(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD50(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD52(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD51(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD53(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD52(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD54(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD53(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD55(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD54(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD56(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD55(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD57(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD56(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD58(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD57(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD59(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD58(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD60(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD59(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD61(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD60(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD62(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD61(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD63(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD62(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD64(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD63(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD65(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD64(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD66(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD65(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD67(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD66(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD68(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD67(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD69(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD68(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD70(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD69(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD71(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD70(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD72(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD71(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD73(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD72(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD74(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD73(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD75(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD74(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD76(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD75(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD77(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD76(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD78(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD77(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD79(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD78(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD80(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD79(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD81(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD80(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD82(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD81(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD83(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD82(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD84(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD83(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD85(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD84(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD86(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD85(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD87(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD86(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD88(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD87(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))
#define _StaticForArgID_ADD89(eval, classname, _id, first,...) eval(classname, _id, first) VA_NARGS_EVAL(_StaticForArgID_ADD88(eval, classname, VA_NARGS_EVAL(_id + 1), __VA_ARGS__))


#define StaticFor(evaluator,...) VA_NARGS_EVAL(_LOOPER_CAT(_StaticFor_ADD, __VA_ARGS__)(evaluator, __VA_ARGS__))
#define StaticForArgID(evaluator, classname, ...) VA_NARGS_EVAL(_LOOPER_CAT(_StaticForArgID_ADD, __VA_ARGS__)(evaluator, classname, 0, __VA_ARGS__))

