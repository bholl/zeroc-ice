// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Endpoint.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(Endpoint* p) { p->__incRef(); }
void IceInternal::decRef(Endpoint* p) { p->__decRef(); }
