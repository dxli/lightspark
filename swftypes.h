/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef SWFTYPES_H
#define SWFTYPES_H

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>

#include "logger.h"
#include <stdlib.h>
#include <string.h>

#define ASFUNCTION(name) \
	static ISWFObject* name(ISWFObject* , arguments* args)
#define ASFUNCTIONBODY(c,name) \
	ISWFObject* c::name(ISWFObject* obj, arguments* args)

enum SWFOBJECT_TYPE { T_OBJECT=0, T_MOVIE, T_REGNUMBER, T_CONSTREF, T_INTEGER, T_NUMBER, T_FUNCTION,
	T_UNDEFINED, T_NULL, T_PLACEOBJECT, T_STRING, T_DEFINABLE, T_BOOLEAN, T_ARRAY, T_PACKAGE, T_INVALID};

enum STACK_TYPE{STACK_NONE=0,STACK_OBJECT,STACK_INT,STACK_NUMBER,STACK_BOOLEAN};

typedef double number_t;

class ISWFObject;
class ASObject;
class arguments;
class IFunction;
struct arrayElem;

class tiny_string
{
private:
	char buf[256];
public:
	tiny_string(){buf[0]=0;}
	tiny_string(const char* s)
	{
		if(strlen(s)>255)
			abort();
		strcpy(buf,s);
	}
	explicit tiny_string(int i)
	{
		sprintf(buf,"%i",i);
	}
	tiny_string& operator=(const std::string& s)
	{
		strncpy(buf,s.c_str(),256);
		return *this;
	}
	tiny_string& operator=(const char* s)
	{
		strncpy(buf,s,256);
		return *this;
	}
	bool operator<(const tiny_string& r) const
	{
		return strncmp(buf,r.buf,256)<0;
	}
	bool operator==(const tiny_string& r) const
	{
		return strncmp(buf,r.buf,256)==0;
	}
	bool operator==(const char* r) const
	{
		return strncmp(buf,r,256)==0;
	}
	operator const char*() const
	{
		return buf;
	}
	char operator[](int i)
	{
		return *(buf+i);
	}
	int len() const
	{
		return strlen(buf);
	}
};

class UI32
{
friend std::istream& operator>>(std::istream& s, UI32& v);
private:
	uint32_t val;
public:
	UI32():val(0){}
	UI32(uint32_t v):val(v){}
	operator uint32_t(){ return val; }
};

class UI16
{
friend std::istream& operator>>(std::istream& s, UI16& v);
private:
	uint16_t val;
public:
	UI16():val(0){}
	UI16(uint16_t v):val(v){}
	operator uint16_t() const { return val; }
	operator UI32() const { return val; }
};

class UI8 
{
friend std::istream& operator>>(std::istream& s, UI8& v);
private:
	uint8_t val;
public:
	UI8():val(0){}
	UI8(uint8_t v):val(v){}
	operator uint8_t() const { return val; }
	operator UI16(){ return val; }
};

class STRING
{
friend std::ostream& operator<<(std::ostream& s, const STRING& r);
friend std::istream& operator>>(std::istream& stream, STRING& v);
friend class ASString;
private:
	std::string String;
public:
	STRING():String(){};
	STRING(const char* s):String(s)
	{
	}
	bool operator==(const STRING& s)
	{
		if(String.size()!=s.String.size())
			return false;
		for(uint32_t i=0;i<String.size();i++)
		{
			if(String[i]!=s.String[i])
				return false;
		}
		return true;
	}
	STRING operator+(const STRING& s)
	{
		STRING ret(*this);
		for(int i=0;i<s.String.size();i++)
			ret.String.push_back(s.String[i]);
		return ret;
	}
	bool isNull() const
	{
		return !String.size();
	}
	operator const std::string&() const
	{
		return String;
	}
	operator const char*() const
	{
		return String.c_str();
	}
	int size()
	{
		return String.size();
	}
};

/*struct Qname
{
	std::string name;
	std::string ns;
	int nskind;
	bool operator<(const Qname& r) const
	{
		return name<r.name;
	}
	Qname(const std::string& s):name(s){}
	Qname(const char* s):name(s){}
	Qname(const STRING& s):name(s){}
	Qname(int i)
	{
		char buf[32];
		sprintf(buf,"%i",i);
		name=buf;
	}
	Qname(const std::string& _ns, const std::string& _name):ns(_ns),name(_name){}
};*/

struct multiname
{
	enum NAME_TYPE {NAME_STRING,NAME_INT};
	NAME_TYPE name_type;
	tiny_string name_s;
	int name_i;
	std::vector<tiny_string> ns;
	std::vector<int> nskind;
	multiname(){count++;}
	~multiname();
	static int count;
};

struct obj_var
{
	ISWFObject* var;
	IFunction* setter;
	IFunction* getter;
	obj_var():var(NULL),setter(NULL),getter(NULL){}
	explicit obj_var(ISWFObject* v):var(v),setter(NULL),getter(NULL){}
	explicit obj_var(ISWFObject* v,IFunction* g,IFunction* s):var(v),setter(s),getter(g){}
};

class Manager
{
friend class ISWFObject;
private:
	std::vector<ISWFObject*> available;
public:
template<class T>
	T* get();
	void put(ISWFObject* o);
};

class ISWFObject
{
friend class MovieClip;
friend class Manager;
friend class ASObject;
private:
	Manager* manager;
	//maps variable name to namespace name and var
	std::multimap<tiny_string,std::pair<tiny_string, obj_var> > Variables;
protected:
	ISWFObject* parent;
	ISWFObject();
	ISWFObject(Manager* m);
	ISWFObject(const ISWFObject& o);
	typedef std::multimap<tiny_string,std::pair<tiny_string, obj_var> >::iterator var_iterator;
	//When findObjVar is invoked with create=true the pointer returned is garanteed to be valid
	obj_var* findObjVar(const tiny_string& name, const tiny_string& ns, bool create=true);
	obj_var* findObjVar(const multiname& mname, bool create=true);
	std::vector<var_iterator> slots_vars;
	SWFOBJECT_TYPE type;
public:
	std::string class_name;
	int ref_count;
	int debug;
	IFunction* constructor;
	int class_index;
	virtual ~ISWFObject();
	void incRef()
	{
		ref_count++;
	}
	void decRef()
	{
		if(ref_count==0)
			abort();
		ref_count--;
		if(ref_count==0)
		{
			if(manager)
				manager->put(this);
			else
				delete this;
		}
	}

	void fake_decRef()
	{
		ref_count--;
	}
	static void s_incRef(ISWFObject* o)
	{
		o->incRef();
	}
	static void s_decRef(ISWFObject* o)
	{
		if(o)
			o->decRef();
	}
	static void s_decRef_safe(ISWFObject* o,ISWFObject* o2)
	{
		if(o && o!=o2)
			o->decRef();
	}
	virtual ISWFObject* getVariableByMultiname(const multiname& name, ISWFObject*& owner);
	virtual intptr_t getVariableByMultiname_i(const multiname& name, ISWFObject*& owner);
	virtual ISWFObject* getVariableByQName(const tiny_string& name, const tiny_string& ns, ISWFObject*& owner);
	virtual ISWFObject* getVariableByString(const std::string& name, ISWFObject*& owner);
	virtual void setVariableByMultiname_i(const multiname& name, intptr_t value);
	virtual void setVariableByMultiname(const multiname& name, ISWFObject* o);
	virtual void setVariableByQName(const tiny_string& name, const tiny_string& ns, ISWFObject* o);
	void setGetterByQName(const tiny_string& name, const tiny_string& ns, IFunction* o);
	void setSetterByQName(const tiny_string& name, const tiny_string& ns, IFunction* o);
	ISWFObject* getSlot(int n)
	{
		return slots_vars[n-1]->second.second.var;
	}
	virtual void setSlot(int n,ISWFObject* o);
	virtual void initSlot(int n,ISWFObject* o, const tiny_string& name, const tiny_string& ns);
	virtual void dumpVariables();
	virtual int numVariables();
	std::string getNameAt(int i);

	SWFOBJECT_TYPE getObjectType() const
	{
		return type;
	}
	virtual tiny_string toString() const;
	virtual int toInt() const;
	virtual double toNumber() const;
	virtual IFunction* toFunction();

	//TODO: check clone semantics, as actually nothing is cloned
	virtual ISWFObject* clone();

	virtual bool isEqual(const ISWFObject* r) const;
	virtual bool isLess(const ISWFObject* r) const;
	virtual bool isGreater(const ISWFObject* r) const;

	virtual void copyFrom(const ISWFObject* o)
	{
		LOG(ERROR,"Copy object of type " << (int)getObjectType() << " from object of type " << (int)o->getObjectType());
		abort();
	}
};

inline void Manager::put(ISWFObject* o)
{
	if(available.size()>10)
		delete o;
	else
		available.push_back(o);
}

template<class T>
T* Manager::get()
{
	if(available.size())
	{
		T* ret=static_cast<T*>(available.back());
		available.pop_back();
		ret->incRef();
		return ret;
	}
	else
		return new T(this);
}

class ConstantReference : public ISWFObject
{
private:
	int index;
public:
	ConstantReference(int i):index(i){type=T_CONSTREF;}
	tiny_string toString() const;
	int toInt() const;
//	double toNumber() const;
	ISWFObject* clone();
/*	ISWFObject* clone()
	{
		return new ConstantReference(*this);
	}*/
};

class RegisterNumber : public ISWFObject
{
private:
	int index;
public:
	RegisterNumber(int i):index(i){type=T_REGNUMBER;}
	tiny_string toString() const;
	//int toInt();
	ISWFObject* clone();
/*	ISWFObject* clone()
	{
		return new RegisterNumber(*this);
	}
	ISWFObject* instantiate();*/
};

class Package : public ISWFObject
{
public:
	Package(){type=T_PACKAGE;}
	ISWFObject* clone()
	{
		abort();
		return NULL;
	}
};

class FLOAT 
{
friend std::istream& operator>>(std::istream& s, FLOAT& v);
private:
	float val;
public:
	FLOAT():val(0){}
	FLOAT(float v):val(v){}
	operator float(){ return val; }
};

class DOUBLE 
{
friend std::istream& operator>>(std::istream& s, DOUBLE& v);
private:
	double val;
public:
	DOUBLE():val(0){}
	DOUBLE(double v):val(v){}
	operator double(){ return val; }
};

class Integer : public ISWFObject
{
friend class Number;
friend class ASArray;
friend class ABCVm;
friend class ABCContext;
friend ISWFObject* abstract_i(intptr_t i);
friend ISWFObject* abstract_i2(intptr_t i);
private:
	int val;
public:
	Integer(int v):val(v){type=T_INTEGER;}
	Integer(Manager* m):val(0),ISWFObject(m){type=T_INTEGER;}
	virtual ~Integer(){}
	Integer& operator=(int v){val=v; return *this; }
	tiny_string toString() const;
	int toInt() const
	{
		return val;
	}
	double toNumber() const
	{
		return val;
	}
	operator int() const{return val;} 
	bool isLess(const ISWFObject* r) const;
	bool isGreater(const ISWFObject* r) const;
	bool isEqual(const ISWFObject* o) const;
	ISWFObject* clone()
	{
		return new Integer(*this);
	}
};

class SI16
{
friend std::istream& operator>>(std::istream& s, SI16& v);
private:
	int16_t val;
public:
	SI16():val(0){}
	SI16(int16_t v):val(v){}
	operator int16_t(){ return val; }
};

typedef UI16 RECORDHEADER;

class RGB
{
public:
	RGB(){};
	RGB(int r,int g, int b):Red(r),Green(g),Blue(b){};
	UI8 Red;
	UI8 Green;
	UI8 Blue;
};

class RGBA
{
public:
	RGBA():Red(0),Green(0),Blue(0),Alpha(255){}
	RGBA(int r, int g, int b, int a):Red(r),Green(g),Blue(b),Alpha(a){}
	UI8 Red;
	UI8 Green;
	UI8 Blue;
	UI8 Alpha;
	RGBA& operator=(const RGB& r)
	{
		Red=r.Red;
		Green=r.Green;
		Blue=r.Blue;
		Alpha=255;
		return *this;
	}
};

typedef UI32 SI32;

typedef UI8 LANGCODE;

std::istream& operator>>(std::istream& s, RGB& v);

inline std::istream& operator>>(std::istream& s, UI8& v)
{
	s.read((char*)&v.val,1);
	return s;
}

inline std::istream& operator>>(std::istream& s, SI16& v)
{
	s.read((char*)&v.val,2);
	return s;
}

inline std::istream& operator>>(std::istream& s, UI16& v)
{
	s.read((char*)&v.val,2);
	return s;
}

inline std::istream& operator>>(std::istream& s, UI32& v)
{
	s.read((char*)&v.val,4);
	return s;
}

inline std::istream& operator>>(std::istream& s, FLOAT& v)
{
	s.read((char*)&v.val,4);
	return s;
}

inline std::istream& operator>>(std::istream& s, DOUBLE& v)
{
	// "Wacky format" is 45670123. Thanks to Ghash :-)
	s.read(((char*)&v.val)+4,4);
	s.read(((char*)&v.val),4);
	return s;
}

inline int min(int a,int b)
{
	return (a<b)?a:b;
}

inline int max(int a,int b)
{
	return (a>b)?a:b;
}

class BitStream
{
public:
	std::istream& f;
	unsigned char buffer;
	unsigned char pos;
public:
	BitStream(std::istream& in):f(in),pos(0){};
	unsigned int readBits(unsigned int num)
	{
		unsigned int ret=0;
		while(num)
		{
			if(!pos)
			{
				pos=8;
				f.read((char*)&buffer,1);
			}
			ret<<=1;
			ret|=(buffer>>(pos-1))&1;
			pos--;
			num--;
		}
		return ret;
	}
};

class FB
{
	int32_t buf;
	int size;
public:
	FB() { buf=0; }
	FB(int s,BitStream& stream):size(s)
	{
		if(s>32)
			LOG(ERROR,"Fixed point bit field wider than 32 bit not supported");
		buf=stream.readBits(s);
		if(buf>>(s-1)&1)
		{
			for(int i=31;i>=s;i--)
				buf|=(1<<i);
		}
	}
	operator float() const
	{
		if(buf>=0)
		{
			int32_t b=buf;
			return b/65536.0f;
		}
		else
		{
			int32_t b=-buf;
			return -(b/65536.0f);
		}
		//return (buf>>16)+(buf&0xffff)/65536.0f;
	}
};

class UB
{
	uint32_t buf;
	int size;
public:
	UB() { buf=0; }
	UB(int s,BitStream& stream):size(s)
	{
/*		if(s%8)
			buf=new uint8_t[s/8+1];
		else
			buf=new uint8_t[s/8];
		int i=0;
		while(!s)
		{
			buf[i]=stream.readBits(min(s,8));
			s-=min(s,8);
			i++;
		}*/
		if(s>32)
			LOG(ERROR,"Unsigned bit field wider than 32 bit not supported");
		buf=stream.readBits(s);
	}
	operator int() const
	{
		return buf;
	}
};

class SB
{
	int32_t buf;
	int size;
public:
	SB() { buf=0; }
	SB(int s,BitStream& stream):size(s)
	{
		if(s>32)
			LOG(ERROR,"Signed bit field wider than 32 bit not supported");
		buf=stream.readBits(s);
		if(buf>>(s-1)&1)
		{
			for(int i=31;i>=s;i--)
				buf|=(1<<i);
		}
	}
	operator int() const
	{
		return buf;
	}
};

class RECT
{
	friend std::ostream& operator<<(std::ostream& s, const RECT& r);
	friend std::istream& operator>>(std::istream& stream, RECT& v);
public:
	UB NBits;
	SB Xmin;
	SB Xmax;
	SB Ymin;
	SB Ymax;
public:
	RECT();
//	RECT(FILE* in);
};

class MATRIX
{
	friend std::istream& operator>>(std::istream& stream, MATRIX& v);
	friend std::ostream& operator<<(std::ostream& s, const MATRIX& r);
public:
	int size;
	UB HasScale;
	UB NScaleBits;
	float ScaleX;
	float ScaleY;
	UB HasRotate;
	UB NRotateBits;
	FB RotateSkew0;
	FB RotateSkew1;
	UB NTranslateBits;
	SB TranslateX;
	SB TranslateY;
public:
	MATRIX():size(0){}
	void get4DMatrix(float matrix[16]);
	//int getSize(){return size;}
};

class GRADRECORD
{
public:
	int version;
	UI8 Ratio;
	RGBA Color;
	bool operator<(const GRADRECORD& g) const
	{
		return Ratio<g.Ratio;
	}
};

class GRADIENT
{
public:
	int version;
	UB SpreadMode;
	UB InterpolationMode;
	UB NumGradient;
	std::vector<GRADRECORD> GradientRecords;
};

class FILLSTYLEARRAY;
class MORPHFILLSTYLE;

class FILLSTYLE
{
	friend std::istream& operator>>(std::istream& s, FILLSTYLEARRAY& v);
	friend std::istream& operator>>(std::istream& s, FILLSTYLE& v);
	friend std::istream& operator>>(std::istream& s, MORPHFILLSTYLE& v);
	friend class DefineTextTag;
	friend class DefineShape2Tag;
	friend class DefineShape3Tag;
	friend class Shape;
private:
	int version;
	UI8 FillStyleType;
	RGBA Color;
	MATRIX GradientMatrix;
	GRADIENT Gradient;
	UI16 BitmapId;
	MATRIX BitmapMatrix;

public:
	virtual void setFragmentProgram() const;
	static void fixedColor(float r, float g, float b);
	virtual void setVertexData(arrayElem* elem);
	virtual ~FILLSTYLE(){}
};

class MORPHFILLSTYLE:public FILLSTYLE
{
public:
	RGBA StartColor;
	RGBA EndColor;
	MATRIX StartGradientMatrix;
	MATRIX EndGradientMatrix;
	UI8 NumGradients;
	std::vector<UI8> StartRatios;
	std::vector<UI8> EndRatios;
	std::vector<RGBA> StartColors;
	std::vector<RGBA> EndColors;
	virtual ~MORPHFILLSTYLE(){}
};

class LINESTYLE
{
public:
	int version;
	UI16 Width;
	RGBA Color;
};

class LINESTYLE2
{
public:
	UI16 Width;
	UB StartCapStyle;
	UB JointStyle;
	UB HasFillFlag;
	UB NoHScaleFlag;
	UB NoVScaleFlag;
	UB PixelHintingFlag;
	UB NoClose;
	UB EndCapStyle;
	UI16 MiterLimitFactor;
	RGBA Color;
	FILLSTYLE FillType;
};

class MORPHLINESTYLE
{
public:
	UI16 StartWidth;
	UI16 EndWidth;
	RGBA StartColor;
	RGBA EndColor;
};

class LINESTYLEARRAY
{
public:
	int version;
	UI8 LineStyleCount;
	LINESTYLE* LineStyles;
	LINESTYLE2* LineStyles2;
};

class MORPHLINESTYLEARRAY
{
public:
	UI8 LineStyleCount;
	MORPHLINESTYLE* LineStyles;
};

class FILLSTYLEARRAY
{
public:
	int version;
	UI8 FillStyleCount;
	FILLSTYLE* FillStyles;
};

class MORPHFILLSTYLEARRAY
{
public:
	UI8 FillStyleCount;
	MORPHFILLSTYLE* FillStyles;
};

class SHAPE;
class SHAPEWITHSTYLE;

class SHAPERECORD
{
public:
	SHAPE* parent;
	UB TypeFlag;
	UB StateNewStyles;
	UB StateLineStyle;
	UB StateFillStyle1;
	UB StateFillStyle0;
	UB StateMoveTo;

	UB MoveBits;
	SB MoveDeltaX;
	SB MoveDeltaY;

	UB FillStyle1;
	UB FillStyle0;
	UB LineStyle;

	//Edge record
	UB StraightFlag;
	UB NumBits;
	UB GeneralLineFlag;
	UB VertLineFlag;
	SB DeltaX;
	SB DeltaY;

	SB ControlDeltaX;
	SB ControlDeltaY;
	SB AnchorDeltaX;
	SB AnchorDeltaY;

	SHAPERECORD* next;

	SHAPERECORD():next(0){};
	SHAPERECORD(SHAPE* p,BitStream& bs);
};

class TEXTRECORD;

class GLYPHENTRY
{
public:
	UB GlyphIndex;
	SB GlyphAdvance;
	TEXTRECORD* parent;
	GLYPHENTRY(TEXTRECORD* p,BitStream& bs);
};

class DefineTextTag;

class TEXTRECORD
{
public:
	UB TextRecordType;
	UB StyleFlagsReserved;
	UB StyleFlagsHasFont;
	UB StyleFlagsHasColor;
	UB StyleFlagsHasYOffset;
	UB StyleFlagsHasXOffset;
	UI16 FontID;
	RGBA TextColor;
	SI16 XOffset;
	SI16 YOffset;
	UI16 TextHeight;
	UI8 GlyphCount;
	std::vector <GLYPHENTRY> GlyphEntries;
	DefineTextTag* parent;
	TEXTRECORD(DefineTextTag* p):parent(p){}
};

class Shape;

class SHAPE
{
	friend std::istream& operator>>(std::istream& stream, SHAPE& v);
	friend std::istream& operator>>(std::istream& stream, SHAPEWITHSTYLE& v);
	friend class SHAPERECORD;
	friend class DefineShapeTag;
	friend class DefineShape2Tag;
	friend class DefineShape3Tag;
	friend class DefineShape4Tag;
	friend class DefineMorphShapeTag;
	friend class DefineTextTag;
	friend class DefineFontTag;
	friend class DefineFont2Tag;
	friend class DefineFont3Tag;
private:
	UB NumFillBits;
	UB NumLineBits;
	SHAPERECORD ShapeRecords;
};

class SHAPEWITHSTYLE : public SHAPE
{
	friend std::istream& operator>>(std::istream& stream, SHAPEWITHSTYLE& v);
public:
	int version;
	FILLSTYLEARRAY FillStyles;
	LINESTYLEARRAY LineStyles;
public:
//	SHAPEWITHSTYLE(){}
};

class CXFORMWITHALPHA
{
	friend std::istream& operator>>(std::istream& stream, CXFORMWITHALPHA& v);
private:
	UB HasAddTerms;
	UB HasMultTerms;
	UB NBits;
	SB RedMultTerm;
	SB GreenMultTerm;
	SB BlueMultTerm;
	SB AlphaMultTerm;
	SB RedAddTerm;
	SB GreenAddTerm;
	SB BlueAddTerm;
	SB AlphaAddTerm;
};

class CXFORM
{
};

class FILTERLIST
{
};

class BUTTONRECORD
{
public:
	UB ButtonReserved;
	UB ButtonHasBlendMode;
	UB ButtonHasFilterList;
	UB ButtonStateHitTest;
	UB ButtonStateDown;
	UB ButtonStateOver;
	UB ButtonStateUp;
	UI16 CharacterID;
	UI16 PlaceDepth;
	MATRIX PlaceMatrix;
	CXFORMWITHALPHA	ColorTransform;
	FILTERLIST FilterList;
	UI8 BlendMode;

	bool isNull() const
	{
		return !(ButtonReserved | ButtonHasBlendMode | ButtonHasFilterList | ButtonStateHitTest | ButtonStateDown | ButtonStateOver | ButtonStateUp);
	}
};

class CLIPEVENTFLAGS
{
public:
	UI32 toParse;
	bool isNull();
};

class CLIPACTIONRECORD
{
public:
	CLIPEVENTFLAGS EventFlags;
	UI32 ActionRecordSize;
	bool isLast();
};

class CLIPACTIONS
{
public:
	UI16 Reserved;
	CLIPEVENTFLAGS AllEventFlags;
	std::vector<CLIPACTIONRECORD> ClipActionRecords;
};

ISWFObject* abstract_i(intptr_t i);
ISWFObject* abstract_i2(intptr_t i);
ISWFObject* abstract_b(bool i);
ISWFObject* abstract_d(number_t i);

std::ostream& operator<<(std::ostream& s, const RECT& r);
std::ostream& operator<<(std::ostream& s, const RGB& r);
std::ostream& operator<<(std::ostream& s, const RGBA& r);
std::ostream& operator<<(std::ostream& s, const STRING& r);
std::ostream& operator<<(std::ostream& s, const multiname& r);
//std::ostream& operator<<(std::ostream& s, const Qname& r);

std::istream& operator>>(std::istream& s, RECT& v);
std::istream& operator>>(std::istream& s, CLIPEVENTFLAGS& v);
std::istream& operator>>(std::istream& s, CLIPACTIONRECORD& v);
std::istream& operator>>(std::istream& s, CLIPACTIONS& v);
std::istream& operator>>(std::istream& s, RGB& v);
std::istream& operator>>(std::istream& s, RGBA& v);
std::istream& operator>>(std::istream& stream, SHAPEWITHSTYLE& v);
std::istream& operator>>(std::istream& stream, SHAPE& v);
std::istream& operator>>(std::istream& stream, FILLSTYLEARRAY& v);
std::istream& operator>>(std::istream& stream, MORPHFILLSTYLEARRAY& v);
std::istream& operator>>(std::istream& stream, LINESTYLEARRAY& v);
std::istream& operator>>(std::istream& stream, MORPHLINESTYLEARRAY& v);
std::istream& operator>>(std::istream& stream, LINESTYLE& v);
std::istream& operator>>(std::istream& stream, LINESTYLE2& v);
std::istream& operator>>(std::istream& stream, MORPHLINESTYLE& v);
std::istream& operator>>(std::istream& stream, FILLSTYLE& v);
std::istream& operator>>(std::istream& stream, MORPHFILLSTYLE& v);
std::istream& operator>>(std::istream& stream, SHAPERECORD& v);
std::istream& operator>>(std::istream& stream, TEXTRECORD& v);
std::istream& operator>>(std::istream& stream, MATRIX& v);
std::istream& operator>>(std::istream& stream, CXFORMWITHALPHA& v);
std::istream& operator>>(std::istream& stream, GLYPHENTRY& v);
std::istream& operator>>(std::istream& stream, STRING& v);
std::istream& operator>>(std::istream& stream, BUTTONRECORD& v);
#endif
