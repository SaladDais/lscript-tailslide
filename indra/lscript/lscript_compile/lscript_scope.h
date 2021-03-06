/** 
 * @file lscript_scope.h
 * @brief builds nametable and checks scope
 *
 * $LicenseInfo:firstyear=2002&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2010, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#ifndef LL_LSCRIPT_SCOPE_H
#define LL_LSCRIPT_SCOPE_H

#include <map>
#include "llstringtable.h"
#include "lscript_byteformat.h"

typedef enum e_lscript_identifier_type
{
	LIT_INVALID,
	LIT_GLOBAL,
	LIT_VARIABLE,
	LIT_FUNCTION,
	LIT_LABEL,
	LIT_STATE,
	LIT_HANDLER,
	LIT_LIBRARY_FUNCTION,
	LIT_EOF
} LSCRIPTIdentifierType;

const char LSCRIPTFunctionTypeStrings[LST_EOF] =	 	/*Flawfinder: ignore*/
{
	'0',
	'i',
	'f',
	's',
	'k',
	'v',
	'q',
	'l',
	'0'
};

const char * const LSCRIPTListDescription[LST_EOF] =	/*Flawfinder: ignore*/
{
   "PUSHARGB 0",
   "PUSHARGB 1",
   "PUSHARGB 2",
   "PUSHARGB 3",
   "PUSHARGB 4",
   "PUSHARGB 5",
   "PUSHARGB 6",
   "PUSHARGB 7",
   "PUSHARGB 0"
};

const char * const LSCRIPTTypePush[LST_EOF] = 	/*Flawfinder: ignore*/
{
	"INVALID",
	"PUSHE",
	"PUSHE",
	"PUSHE",
	"PUSHE",
	"PUSHEV",
	"PUSHEQ",
	"PUSHE",
	"undefined"
};

const char * const LSCRIPTTypeReturn[LST_EOF] = 	/*Flawfinder: ignore*/
{
	"INVALID",
	"LOADP -12",
	"LOADP -12",
	"STORES -12\nPOP",
	"STORES -12\nPOP",
	"LOADVP -20",
	"LOADQP -24",
	"LOADLP -12",
	"undefined"
};

const char * const LSCRIPTTypePop[LST_EOF] = 	/*Flawfinder: ignore*/
{
	"INVALID",
	"POP",
	"POP",
	"POPS",
	"POPS",
	"POPV",
	"POPQ",
	"POPL",
	"undefined"
};

const char * const LSCRIPTTypeDuplicate[LST_EOF] = 	 	/*Flawfinder: ignore*/
{
	"INVALID",
	"DUP",
	"DUP",
	"DUPS",
	"DUPS",
	"DUPV",
	"DUPQ",
	"DUPL",
	"undefined"
};

const char * const LSCRIPTTypeLocalStore[LST_EOF] = 	/*Flawfinder: ignore*/
{
	"INVALID",
	"STORE ",
	"STORE ",
	"STORES ",
	"STORES ",
	"STOREV ",
	"STOREQ ",
	"STOREL ",
	"undefined"
};

const char * const LSCRIPTTypeLocalDeclaration[LST_EOF] = 	 	/*Flawfinder: ignore*/
{
	"INVALID",
	"STOREP ",
	"STOREP ",
	"STORESP ",
	"STORESP ",
	"STOREVP ",
	"STOREQP ",
	"STORELP ",
	"undefined"
};

const char * const LSCRIPTTypeGlobalStore[LST_EOF] = 	/*Flawfinder: ignore*/
{
	"INVALID",
	"STOREG ",
	"STOREG ",
	"STORESG ",
	"STORESG ",
	"STOREGV ",
	"STOREGQ ",
	"STORELG ",
	"undefined"
};

const char * const LSCRIPTTypeLocalPush[LST_EOF] = 	 	/*Flawfinder: ignore*/
{
	"INVALID",
	"PUSH ",
	"PUSH ",
	"PUSHS ",
	"PUSHS ",
	"PUSHV ",
	"PUSHQ ",
	"PUSHL ",
	"undefined"
};

const char * const LSCRIPTTypeLocalPush1[LST_EOF] = 	 	/*Flawfinder: ignore*/
{
	"INVALID",
	"PUSHARGI 1",
	"PUSHARGF 1",
	"undefined",
	"undefined",
	"undefined",
	"undefined",
	"undefined",
	"undefined"
};

const char * const LSCRIPTTypeGlobalPush[LST_EOF] = 	/*Flawfinder: ignore*/
{
	"INVALID",
	"PUSHG ",
	"PUSHG ",
	"PUSHGS ",
	"PUSHGS ",
	"PUSHGV ",
	"PUSHGQ ",
	"PUSHGL ",
	"undefined"
};

class LLScriptSimpleAssignable;

class LLScriptArgString
{
public:
	LLScriptArgString() : mString(NULL) {}
	~LLScriptArgString() { delete [] mString; }

	LSCRIPTType getType(S32 count)
	{
		if (!mString)
			return LST_NULL;
		S32 length = (S32)strlen(mString);	 	/*Flawfinder: ignore*/
		if (count >= length)
		{
			return LST_NULL;
		}
		switch(mString[count])
		{
		case 'i':
			return LST_INTEGER;
		case 'f':
			return LST_FLOATINGPOINT;
		case 's':
			return LST_STRING;
		case 'k':
			return LST_KEY;
		case 'v':
			return LST_VECTOR;
		case 'q':
			return LST_QUATERNION;
		case 'l':
			return LST_LIST;
		default:
			return LST_NULL;
		}
	}

	void addType(LSCRIPTType type)
	{
		S32 count = 0;
		if (mString)
		{
			count = (S32)strlen(mString);	 	/*Flawfinder: ignore*/
			char *temp = new char[count + 2];
			memcpy(temp, mString, count);	 	/*Flawfinder: ignore*/
			delete [] mString;
			mString = temp;
			mString[count + 1] = 0;
		}
		else
		{
			mString = new char[count + 2];
			mString[count + 1] = 0;
		}
		mString[count++] = LSCRIPTFunctionTypeStrings[type];
	}

	S32 getNumber()
	{
		if (mString)
			return (S32)strlen(mString);	 	/*Flawfinder: ignore*/
		else
			return 0;
	}

	char *mString;
};

class LLScriptScopeEntry
{
public:
	LLScriptScopeEntry(const char *identifier, LSCRIPTIdentifierType idtype, LSCRIPTType type, S32 count = 0)
		: mIdentifier(identifier), mIDType(idtype), mType(type), mOffset(0), mSize(0), mAssignable(NULL), mCount(count), mLibraryNumber(0)
	{
	}

	~LLScriptScopeEntry() {}

	const char					*mIdentifier;
	LSCRIPTIdentifierType		mIDType;
	LSCRIPTType					mType;
	S32							mOffset;
	S32							mSize;
	LLScriptSimpleAssignable	*mAssignable;
	S32							mCount; // NOTE: Index for locals in CIL.
	U16							mLibraryNumber;
	LLScriptArgString			mFunctionArgs;
	LLScriptArgString			mLocals;
};

class LLScriptScope
{
public:
	LLScriptScope(LLStringTable *stable)
		: mParentScope(NULL), mSTable(stable), mFunctionCount(0), mStateCount(0)
	{ 
	}

	~LLScriptScope()	
	{
		delete_and_clear(mEntryMap);
	}

	LLScriptScopeEntry *addEntry(const char *identifier, LSCRIPTIdentifierType idtype, LSCRIPTType type)
	{
		const char *name = mSTable->addString(identifier);
		if (mEntryMap.find(name) == mEntryMap.end())
		{
			if (idtype == LIT_FUNCTION)
				mEntryMap[name] = new LLScriptScopeEntry(name, idtype, type, mFunctionCount++);
			else if (idtype == LIT_STATE)
				mEntryMap[name] = new LLScriptScopeEntry(name, idtype, type, mStateCount++);
			else
				mEntryMap[name] = new LLScriptScopeEntry(name, idtype, type);
			return mEntryMap[name];
		}
		else
		{
			// identifier already exists at this scope
			return NULL;
		}
	}

	bool checkEntry(const char *identifier)
	{
		const char *name = mSTable->addString(identifier);
		return mEntryMap.find(name) != mEntryMap.end();
	}

	LLScriptScopeEntry *findEntry(const char *identifier)
	{
		const char		*name = mSTable->addString(identifier);
		LLScriptScope	*scope = this;

		while (scope)
		{
			entry_map_t::iterator found_it = scope->mEntryMap.find(name);
			if (found_it != scope->mEntryMap.end())
			{
				// cool, we found it at this scope
				return found_it->second;
			}
			scope = scope->mParentScope;
		}
		return NULL;
	}

	LLScriptScopeEntry *findEntryTyped(const char *identifier, LSCRIPTIdentifierType idtype)
	{
		const char		*name = mSTable->addString(identifier);
		LLScriptScope	*scope = this;

		while (scope)
		{
			entry_map_t::iterator found_it = scope->mEntryMap.find(name);
			if (found_it != scope->mEntryMap.end())
			{
				// need to check type, and if type is function we need to check both types
				if (idtype == LIT_FUNCTION)
				{
					if (found_it->second->mIDType == LIT_FUNCTION)
					{
						return (found_it->second);
					}
					else if (found_it->second->mIDType == LIT_LIBRARY_FUNCTION)
					{
						return (found_it->second);
					}
				}
				else if (found_it->second->mIDType == idtype)
				{
					// cool, we found it at this scope
					return (found_it->second);
				}
			}
			scope = scope->mParentScope;
		}
		return NULL;
	}

	void addParentScope(LLScriptScope *scope)
	{
		mParentScope = scope;
	}

	typedef std::map<const char *, LLScriptScopeEntry *> entry_map_t;
	entry_map_t		mEntryMap;
	LLScriptScope*	mParentScope;
	LLStringTable*	mSTable;
	S32				mFunctionCount;
	S32				mStateCount;
};

extern LLStringTable *gScopeStringTable;



#endif
