#ifndef SCALECOMMAND_H
#define SCALECOMMAND_H

#include <QString>
enum class ScaleCommand {
    ImmediateQ,  // Request weighing data immediately (Q)
    ImmediateRW, // Request weighing data immediately (RW)
    ImmediateSI, // Request weighing data immediately (SI)
    Stable,      // Request weighing data when stabilized (S)
    StableEscP,  // Request weighing data when stabilized (<ESC>P)
    Continuous,  // Request continuous weighing data (SIR)
    Cancel       // Cancel ongoing request (C)
};

enum class ScaleError {
    NoError,
    PortNotConnected,
    WriteError,
    InvalidDataFormat,
    UnstableData,
    WeightConversionError,
    UnitNotFound
};

// Convert ScaleCommand to string for sending
inline QString scaleCommandToString(ScaleCommand command)
{
    switch (command) {
    case ScaleCommand::ImmediateQ:
        return "Q";
    case ScaleCommand::ImmediateRW:
        return "RW";
    case ScaleCommand::ImmediateSI:
        return "SI";
    case ScaleCommand::Stable:
        return "S";
    case ScaleCommand::StableEscP:
        return "\x1BP";
    case ScaleCommand::Continuous:
        return "SIR";
    case ScaleCommand::Cancel:
        return "C";
    default:
        return "";
    }
}
#endif // SCALECOMMAND_H
