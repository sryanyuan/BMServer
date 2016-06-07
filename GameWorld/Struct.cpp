#include "Struct.h"
//////////////////////////////////////////////////////////////////////////
BYTEBUFFER_STRUCT_READ(DelayedProcess)
{
	_xBuffer
		>> _var.uOp
		>> _var.uParam0
		>> _var.uParam1
		>> _var.uParam2
		>> _var.uParam3;
	return _xBuffer;
}

BYTEBUFFER_STRUCT_WRITE(DelayedProcess)
{
	_xBuffer
		<< _var.uOp
		<< _var.uParam0
		<< _var.uParam1
		<< _var.uParam2
		<< _var.uParam3;
	return _xBuffer;
}

BYTEBUFFER_STRUCT_READ(DelayedDBProcess)
{
	_xBuffer
		>> _var.uOp
		>> _var.uParam0
		>> _var.uParam1
		>> _var.uParam2
		>> _var.uParam3;
	return _xBuffer;
}

BYTEBUFFER_STRUCT_WRITE(DelayedDBProcess)
{
	_xBuffer
		<< _var.uOp
		<< _var.uParam0
		<< _var.uParam1
		<< _var.uParam2
		<< _var.uParam3;
	return _xBuffer;
}