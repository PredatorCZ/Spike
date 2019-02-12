# PreCore
PreCore is a library with helper classes.
It contains IO classes, logging services, algorhitms, data types, extensions for other libraries and more.

Library contains following folders:
## _samples
Contains practical usage of classes.
## data
Basic classes:
### allocator_hybrid
Is a modified std::allocator, where can be assigned already existant buffer.
Usage is in data.cpp.
### bincore
An internal base class for Binreader/Binwritter.
Usage is in data.cpp.

Macros:

- getBlockSize(classname, startval, endval): gets size between one struct member and another (including sizes of both members)

Functions:

- Seek : seek stream
- Skip : skips bytes in stream
- Tell : position within stream
- SetStream : sets external stream object
- GetStream : gets stream
- SetRelativeOrigin : sets absolute position within stream as relative zero, affects Seek and Tell functions by default
- ResetRelativeOrigin : resets relative position as absolute
- Open : opens file
- IsValid : is opened file valid one
- SwapEndian : set data reading in opposite endian
- SwappedEndian : uses opposite endian
- Encryptor : setting, creating and getting ecryption class (only XOR so far, only if class is present)
### binreader
Reading binary data from stream object.
Usage is in data.cpp.

Functions:

- Savepos : saves current stream position away
- Restorepos : seeks into saved stream position called by Savepos()
- GetSize : Gets size of opened stream/file
- ReadBuffer : reads a raw buffer
- ReadContainer : reads container class (vector, string, wstring, etc)
- ReadString : read pure string with \0 ending char, supports both string and wstring
- Read : reads any type

Read functions can handle endianness (more in endian section), and xor encoding if available.

### binwritter
Writing binary data into stream object.
Usage is in data.cpp.

Functions:

- WriteBuffer : writes raw buffer into stream
- WriteContainer : writes any container class (vector, string, wstring, etc)
- WriteContainerWCount : writes any container class (vector, string, wstring, etc) with number of elements
- WriteT : writes pure text
- Write : writes any type

Write functions can handle endianness (more in endian section), and xor encoding if available.

### blowfish & blowfish2
Blowfish block encryption algo, supports ECB, CBC, PCBC, CFB, OFB modes.

### disabler
Disabler/enabler of functions for template interface classes.
Usage is in data.cpp.

### endian
FByteSwapper function for swapping endianness on classes and data types.

Can detect if input class have SwapEndian function, if it can find it, it'll call this function, where you can swap members individually. Otherwise it'll swap entire class.

Usage is in data.cpp.

### esString
Universal string class, can contain classic and unicode string, as-well convert between them.
Usage is in data.cpp.

### fileinfo
Class for filtering filepath elements, uses classic and unicode strings.
Usage is in data.cpp.

### flags
Classes for bitflag management.
Usage is in data.cpp.

Classes:

- t_Flags: base class for managing bit flags
- EnumFlags : uses enumeration as bit flags
- esEnum : enum with explicit size

### halfFloat
Converting half float (16 bit floating point) into float.

### jenkinsHash
Compile time Jenkins One At Time function.

### macroloop
Compile time for loop, 70 of maximum iterations.

### masterPrinter
Multithreaded logging service.
Usage is in data.cpp.

Functions:

- operator << : appends data into stream, works like cout, wcout, istream
- operator >> : flushes stream data, >> 1 : inserts newline, >> 0 : only flushes
- FlushAll : works like operator >> 0
- PrintThreadID : Allows to print thread ID before message
- AddPrinterFunction : Adds cdecl function as output, functions like printf, wprintf, or custom ones

### matrix33simple
Transformation matrix3x3 class.

### reflector
Class reflection header.
Usage is in data.cpp.

### reflectorRegistry
Header for registering enum classes.
Usage is in data.cpp.

### vectors
2,3,4 component vectors, vectors can store any datatype.
Usage is in data.cpp.

### xorenc
XOR encoder.
Usage is in data.cpp.
