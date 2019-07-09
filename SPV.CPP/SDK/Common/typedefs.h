// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TYPEDEFS_H__
#define __ELASTOS_SDK_TYPEDEFS_H__

#include "uchar_vector.h"
#include <vector>
#include <set>
#include <boost/shared_ptr.hpp>

typedef uchar_vector bytes_t;
typedef std::set<bytes_t> hashset_t;

typedef boost::shared_ptr<bytes_t> bytes_ptr;

#endif //__ELASTOS_SDK_TYPEDEFS_H__
