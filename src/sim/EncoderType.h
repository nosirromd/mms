#pragma once

#include <QMap>
#include <QString>

#ifdef _WIN32
    #include "Windows.h"
#endif

#include "ContainerUtilities.h"

namespace sim {

enum class EncoderType {
    ABSOLUTE,
    RELATIVE,
};

static const QMap<EncoderType, QString> ENCODER_TYPE_TO_STRING {
    {EncoderType::ABSOLUTE, "ABSOLUTE"},
    {EncoderType::RELATIVE, "RELATIVE"},
};

static const QMap<QString, EncoderType> STRING_TO_ENCODER_TYPE = 
    ContainerUtilities::inverse(ENCODER_TYPE_TO_STRING);

} // namespace sim
