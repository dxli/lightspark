/**************************************************************************
    Lighspark, a free flash player implementation

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

#include "asobjects.h"

ASStage::ASStage():width(640),height(480)
{
	setVariableByName("width",SWFObject(&width,true));
	setVariableByName("height",SWFObject(&height,true));
}

void ASArray::_register()
{
	setVariableByName("constructor",SWFObject(new Function((Function::as_function)constructor),true));
}

void ASArray::constructor(ASArray* th, arguments* args)
{
	LOG(CALLS,"Called Array constructor");
}

void ASObject::_register()
{
	setVariableByName("constructor",SWFObject(new Function((Function::as_function)constructor),true));
}

void ASObject::constructor(ASObject* th, arguments* args)
{
	LOG(CALLS,"Called Object constructor");
}

void ASMovieClip::_register()
{
	setVariableByName("_visible",SWFObject(&_visible,true));
}
