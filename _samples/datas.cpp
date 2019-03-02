#include <tchar.h>
#include <stdio.h>
#include <vector>
#include <thread>
//some headers should be included first, to activate macro definitions for others
#include "../datas/reflector.hpp"
#include "../datas/endian.hpp"

#include "../datas/allocator_hybrid.hpp"
#include "../datas/vectors.hpp"
#include "../datas/disabler.hpp"
#include "../datas/esstring.h"
#include "../datas/fileinfo.hpp"
#include "../datas/flags.hpp"
#include "../datas/halfFloat.hpp"
#include "../datas/masterprinter.hpp"
#include "../datas/Matrix33Simple.hpp"
#include "../datas/reflectorRegistry.hpp"
#include "../datas/xorenc.hpp"
#include "../datas/blowfish2.h"

//some headers should be included last, to activate all possible macro contitionals
#include "../datas/binreader.hpp"
#include "../datas/binwritter.hpp"

/************************************************************************/
/************************ REFLECTOR SAMPLE ******************************/
/************************************************************************/

REFLECTOR_ENUM(EnumWrap00, 0xd587b2c1, E1, E2, E3) //declaring enum with class like encapsulation; E1, E2, E3 can be accessed only via EnumWrap00::

REFLECTOR_ENUM_NAKED(EnumWrap01, 0xd587b2c1, EnumWrap01_E1, EnumWrap01_E2, EnumWrap01_E3) // declaring classic enum; EnumWrap01_E1, EnumWrap01_E2, EnumWrap01_E3 can be accessed globally

class enumClass00
{
	REFLECTOR_ENUM_INCLASS(EnumWrap02, E1, E2, E3) // declaring classic enum inside class, no registering needed
};

struct subrefl
{
	int data;
	float data2;

	void SwapEndian() //swapper function for fbyteswapper called from reflClass class
	{
		FByteswapper(data);
		FByteswapper(data2);
	}
};

struct _ReflClassData
{
	bool test1;
	char test2;
	uchar test3;
	short test4;
	ushort test5;
	int test6;
	uint test7;
	_LONGLONG test8;
	_ULONGLONG test9;
	float test10;
	double test11;
	EnumFlags<uchar, EnumWrap00> test12;

	EnumWrap00 test13;
	esEnum<short, EnumWrap00> test14;

	Vector test15;
	Vector2 test16;
	Vector4 test17;

	subrefl test18;
	
	void SwapEndian() //swapper function for fbyteswapper called from bincore classes
	{
		FByteswapper(test4);
		FByteswapper(test5);
		FByteswapper(test6);
		FByteswapper(test7);
		FByteswapper(test8);
		FByteswapper(test9);
		FByteswapper(test10);
		FByteswapper(test11);
		FByteswapper(test12);
		FByteswapper(test13);
		FByteswapper(test14);
		FByteswapper(test15);
		FByteswapper(test16);
		FByteswapper(test17);
		FByteswapper(test18);
	}
};

struct reflClass : Reflector, _ReflClassData
{
	DECLARE_REFLECTOR;
};

REFLECTOR_START(reflClass, test1, test2, test3, test4, test5, test6, test7, test8, test9, test10, test11, test12, test13, test14, test15, test16, test17, test18.data, test18.data2)


//uses reflector, esstring as convertor, masterprinter
reflClass ReflectorTest()
{
	REGISTER_ENUM(EnumWrap00);

	reflClass test = {};
	test.ConstructReflection();

	//Previews of reflType entries
	const reflType *types = reflClass::types;
	const reflType *types2 = reflClass::types + 1;
	const reflType *types3 = reflClass::types + 2;
	const reflType *types4 = reflClass::types + 3;
	const reflType *types5 = reflClass::types + 4;
	const reflType *types6 = reflClass::types + 5;
	const reflType *types7 = reflClass::types + 6;
	const reflType *types8 = reflClass::types + 7;
	const reflType *types9 = reflClass::types + 8;
	const reflType *types10 = reflClass::types + 9;
	const reflType *types11 = reflClass::types + 10;
	const reflType *types12 = reflClass::types + 11;
	const reflType *types13 = reflClass::types + 12;
	const reflType *types14 = reflClass::types + 13;
	const reflType *types15 = reflClass::types + 14;
	const reflType *types16 = reflClass::types + 15;
	const reflType *types17 = reflClass::types + 16;

	//setting values
	test.SetReflectedValue("test1", "true");
	test.SetReflectedValue("test2", "-107");
	test.SetReflectedValue("test3", "157");
	test.SetReflectedValue("test4", "-15798");
	test.SetReflectedValue("test5", "15785");

	test.SetReflectedValue("test6", "-15798078");
	test.SetReflectedValue("test7", "4294967290");


	test.SetReflectedValue("test8", "-9223372036854775800");
	test.SetReflectedValue("test9", "12233720368547758004");


	test.SetReflectedValue("test10", "1.59178762832786378278276817837");
	test.SetReflectedValue("test11", "1.5978317365453265638626274637212");

	test.SetReflectedValue("test12", "E1 | E2");

	test.SetReflectedValue("test13", "E2");

	test.SetReflectedValue("test14", "E3");

	test.SetReflectedValue("test15", "[1.5, 2.8, 5.4]");
	test.SetReflectedValue("test16", "[5.81, 2.1]");
	test.SetReflectedValue("test17", "[1.15, 2.631, 4.1, 15161.196541]");

	test.SetReflectedValue("test18.data", "182");
	test.SetReflectedValue("test18.data2", "182.8744");

	//Gets name and value pair, names are working only if DECLARE_REFLECTOR_WNAMES is used
	Reflector::KVPair test1val = test.GetReflectedPair(16);

	printline("Printing all reflected values")

	for (int r = 0; r < test.GetNumReflectedValues(); r++)
	{
		//printer << esString(test.GetReflectedValue(r)) >> 1;
	}

	return test;
}

/************************************************************************/
/******** BINWRITTER, BINREADER, TFILEINFO, MASTERPRINTER SAMPLE ********/
/************************************************************************/
void FileIO(const TCHAR *folderPath)
{
	std::wstring fnamele = folderPath;
	fnamele.append(_T("tesfile.le"));
	
	reflClass rclass = ReflectorTest();

	printline("Saving: ", << fnamele);

	{
		BinWritter wr(fnamele);
		wr.Write(static_cast<_ReflClassData&>(rclass));
	}

	TFileInfo fle(fnamele.c_str());
	std::wstring fnamebe = fle.GetPath() + fle.GetFileName() + _T(".be");

	printline("Saving: ", << fnamebe);

	{
		BinWritter wr(fnamebe);
		wr.SwapEndian(true); // continue write as big endian from now on
		wr.Write(static_cast<_ReflClassData&>(rclass));
	}

	reflClass rclassle;

	printline("Loading: ", << fnamele);

	{
		BinReader wr(fnamele);
		wr.Read(static_cast<_ReflClassData&>(rclassle));
	}

	reflClass rclassbe;

	printline("Loading: ", << fnamebe);

	{
		BinReader wr(fnamebe);
		wr.SwapEndian(true); // continue loading as big endian from now on
		wr.Read(static_cast<_ReflClassData&>(rclassbe));
	}
}

/************************************************************************/
/*********************** allocator_hybrid SAMPLE ************************/
/************************************************************************/
void HybridVector()
{
	char *externalBuffer = static_cast<char*>(malloc(125));
	memset(externalBuffer, 89, 125);

	std::vector<char, std::allocator_hybrid<char>> vectr;
	vectr.resize(85);

	//assigning externalBuffer as internal buffer for vector
	//equivalent to: vectr = std::vector<char, std::allocator_hybrid<char>>(externalBuffer, externalBuffer + 125, std::allocator_hybrid<char>(externalBuffer));
	vectr = decltype(vectr)(externalBuffer, externalBuffer + 125, decltype(vectr)::allocator_type(externalBuffer));
	

	vectr[12] = 12;

	//NOTE: this will still use externalBuffer!!
	vectr.clear();
	// but if vector is resized with size greater than sizeof externalBuffer, then externalBuffer will be dropped
	vectr.resize(200);

	//This is a great alternative to drop externalBuffer, it'll just reset the whole vector
	vectr.~vector();

	//doing normal vector stuff afterwards
	vectr.push_back(75); 

	free(externalBuffer);
}

/************************************************************************/
/*********************** DISABLER SAMPLE ********************************/
/************************************************************************/
struct class01
{
	const int iType = 12;
};

struct class02
{
	void noType();
};

struct ImasterClass
{
	virtual int Func01() = 0;
};

template<class _parent>
struct masterClass_t : _parent
{
	ADD_DISABLERS(_parent, noType);

	enabledFunction(noType, int) Func01()
	{
		return iType;
	}

	disabledFunction(noType, int) Func01()
	{
		return -1;
	}
};

template<class _Ty>
struct masterClass: ImasterClass
{
	typedef masterClass_t<_Ty> typuh;
	typuh Data;

	masterClass() {}

	int Func01() { return Data.Func01(); }
};

void DisablerTest()
{
	std::auto_ptr<ImasterClass> cls01(new masterClass<class01>);
	std::auto_ptr<ImasterClass> cls02(new masterClass<class02>);

	int cls1out = cls01->Func01(); // = 12;
	int cls2out = cls02->Func01(); // = -1;
}

/************************************************************************/
/******** BINWRITTER, BINREADER, XORENCODER, MASTERPRINTER SAMPLE *******/
/************************************************************************/
void EncryptorTest(const TCHAR *folderPath)
{
	std::wstring fnameenc = folderPath;
	fnameenc.append(_T("tesfile.enc"));

	reflClass rclass = ReflectorTest();

	const char *key = "Sample Key";

	printline("Saving: ", << fnameenc);

	{
		BinWritter wr(fnameenc);
		wr.Encryptor<XOREncoder>();
		wr.Encryptor()->SetKey(key, sizeof(key) - 1);
		wr.Write(static_cast<_ReflClassData&>(rclass));
	}

	reflClass rclassenc;

	printline("Loading: ", << fnameenc);

	{
		BinReader wr(fnameenc);
		wr.Encryptor<XOREncoder>();
		wr.Encryptor()->SetKey(key, sizeof(key) - 1);
		wr.Read(static_cast<_ReflClassData&>(rclassenc));
	}
}

/************************************************************************/
/*********************** ESSTRING, MASTERPRINTER SAMPLE *****************/
/************************************************************************/
void ESstringTest()
{
	esString wide = L"Samle Text utf16";
	esString normal = "Sample Text default";

	printf(static_cast<std::string>(wide).c_str());
	printf("\n");
	printer << wide >> 1;

	printf(static_cast<std::string>(normal).c_str());
	printf("\n");
	printer << normal >> 1;
}

/************************************************************************/
/*********************** MASTERPRINTER THREAD SAMPLE ********************/
/************************************************************************/

void ThreadFunc()
{
	printline("Hello, thread.")
}

void MasterprinterTest()
{
	const int nthreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threadedfuncs(nthreads);
	printer.PrintThreadID(true);

	for (int t = 0; t < nthreads; t++)
		threadedfuncs[t] = std::thread(ThreadFunc);

	for (int t = 0; t < nthreads; t++)
		threadedfuncs[t].join();

	printer.PrintThreadID(false);
}

/************************************************************************/
/*********************** MAIN *******************************************/
/************************************************************************/

int main()
{
	printer.AddPrinterFunction(wprintf); // adding console print function for masterprinter service

	DisablerTest();
	HybridVector();
	ReflectorTest();

	TSTRING folderpath;

	FileIO(folderpath.c_str());
	EncryptorTest(folderpath.c_str());

	ESstringTest();

	MasterprinterTest();

	return 0;
}