// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once

namespace ATProto {

#define SHARED_CONST(type, name, value) \
    static inline const type name = value; \
    Q_PROPERTY(type name MEMBER name CONSTANT FINAL)

}
