#pragma once

#include <QString>

// TODO: MACK - come back to cleaning this up

#include "../mouse/IMouseAlgorithm.h" // TODO: MACK - kill this
#include "Direction.h"
#include "Model.h"
#include "MouseInterface.h"
#include "StaticMouseAlgorithmOptions.h"
#include "View.h"

namespace sim {

class Controller {

public:
    Controller(Model* model, View* view);

    StaticMouseAlgorithmOptions getOptions();
    IMouseAlgorithm* getMouseAlgorithm();
    MouseInterface* getMouseInterface();

private:

    StaticMouseAlgorithmOptions m_options;
    IMouseAlgorithm* m_mouseAlgorithm;
    MouseInterface* m_mouseInterface;

    void validateMouseAlgorithm(
        const QString& mouseAlgorithm);

    void validateMouseInterfaceType(
        const QString& mouseAlgorithm,
        const QString& interfaceType);

    void validateMouseInitialDirection(
        const QString& mouseAlgorithm,
        const QString& initialDirection);

    void validateTileTextRowsAndCols(
        const QString& mouseAlgorithm,
        int tileTextNumberOfRows,
        int tileTextNumberOfCols);

    void validateMouseWheelSpeedFraction(
        const QString& mouseAlgorithm,
        double wheelSpeedFraction);

    void initAndValidateMouse(
        const QString& mouseAlgorithm,
        const QString& mouseFile,
        const QString& interfaceType,
        const QString& initialDirection,
        Model* model);

    Direction getInitialDirection(
        const QString& initialDirection,
        Model* model);

};

} // namespace sim
