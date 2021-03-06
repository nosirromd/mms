#include "TileGraphic.h"

#include <QPair>

#include "Color.h"
#include "Param.h"
#include "State.h"

namespace sim {

TileGraphic::TileGraphic() :
    m_tile(nullptr),
    m_bufferInterface(nullptr),
    m_color(Color::BLACK),
    m_foggy(false) {
}

TileGraphic::TileGraphic(const Tile* tile, BufferInterface* bufferInterface) :
        m_tile(tile),
        m_bufferInterface(bufferInterface),
        m_color(STRING_TO_COLOR.value(P()->tileBaseColor())),
        m_foggy(true) {
}

bool TileGraphic::wallDeclared(Direction direction) const {
    return m_declaredWalls.contains(direction);
}

void TileGraphic::setColor(Color color) {
    m_color = color;
    updateColor();
}

void TileGraphic::declareWall(Direction direction, bool isWall) {
    m_declaredWalls[direction] = isWall;
    updateWall(direction);
}

void TileGraphic::undeclareWall(Direction direction) {
    m_declaredWalls.remove(direction);
    updateWall(direction);
}

void TileGraphic::setFogginess(bool foggy) {
    m_foggy = foggy;
    updateFog();
}

void TileGraphic::setText(const QVector<QString>& rowsOfText) {
    m_rowsOfText = rowsOfText;
    updateText();
}

void TileGraphic::draw() const {

    // Note that the order in which we call insertIntoGraphicCpuBuffer
    // determines the order in which the polygons are drawn. Also note that the
    // *StartingIndex methods in GrahicsUtilities.h depend upon this order.

    // Draw the base of the tile
    m_bufferInterface->insertIntoGraphicCpuBuffer(
        m_tile->getFullPolygon(),
        S()->tileColorsVisible() ? m_color : STRING_TO_COLOR.value(P()->tileBaseColor()),
        1.0);

    // Draw each of the walls of the tile
    for (Direction direction : DIRECTIONS) {
        QPair<Color, float> colorAndAlpha = deduceWallColorAndAlpha(direction);
        m_bufferInterface->insertIntoGraphicCpuBuffer(
            m_tile->getWallPolygon(direction),
            colorAndAlpha.first,
            colorAndAlpha.second);
    }

    // Draw the corners of the tile
    for (Polygon polygon : m_tile->getCornerPolygons()) {
        m_bufferInterface->insertIntoGraphicCpuBuffer(
            polygon,
            STRING_TO_COLOR.value(P()->tileCornerColor()),
            1.0);
    }

    // Draw the fog
    m_bufferInterface->insertIntoGraphicCpuBuffer(
        m_tile->getFullPolygon(),
        STRING_TO_COLOR.value(P()->tileFogColor()),
        m_foggy && S()->tileFogVisible() ? P()->tileFogAlpha() : 0.0);


    // Insert all of the triangle texture objects into the buffer ...
    QPair<int, int> maxRowsAndCols = m_bufferInterface->getTileGraphicTextMaxSize();
    for (int row = 0; row < maxRowsAndCols.first; row += 1) {
        for (int col = 0; col < maxRowsAndCols.second; col += 1) {
            m_bufferInterface->insertIntoTextureCpuBuffer();
        }
    }
    // ... and then populate those triangle texture objects with data
    updateText();
}

void TileGraphic::updateColor() const {
    m_bufferInterface->updateTileGraphicBaseColor(m_tile->getX(), m_tile->getY(),
        S()->tileColorsVisible() ? m_color : STRING_TO_COLOR.value(P()->tileBaseColor()));
}

void TileGraphic::updateWalls() const {
    for (Direction direction : DIRECTIONS) {
        updateWall(direction);
    }
}

void TileGraphic::updateFog() const {
    m_bufferInterface->updateTileGraphicFog(m_tile->getX(), m_tile->getY(),
        m_foggy && S()->tileFogVisible() ? P()->tileFogAlpha() : 0.0);
}

void TileGraphic::updateText() const {

    QVector<QString> rows = m_rowsOfText;
    if (S()->tileDistanceVisible()) {
        rows.insert(rows.begin(), (0 <= m_tile->getDistance() ? QString::number(m_tile->getDistance()) : "inf"));
    }

    QPair<int, int> maxRowsAndCols = m_bufferInterface->getTileGraphicTextMaxSize();
    for (int row = 0; row < maxRowsAndCols.first; row += 1) {
        for (int col = 0; col < maxRowsAndCols.second; col += 1) {
            m_bufferInterface->updateTileGraphicText(
                m_tile->getX(),
                m_tile->getY(),
                std::min(static_cast<int>(rows.size()), maxRowsAndCols.first),
                std::min(static_cast<int>((row < rows.size() ? rows.at(row).size() : 0)), maxRowsAndCols.second),
                row,
                col,
                (S()->tileTextVisible() && row < rows.size() && col < rows.at(row).size() ? rows.at(row).at(col).toLatin1() : ' '));
        }
    }
}

void TileGraphic::updateWall(Direction direction) const {
    QPair<Color, float> colorAndAlpha = deduceWallColorAndAlpha(direction);
    m_bufferInterface->updateTileGraphicWallColor(
        m_tile->getX(),
        m_tile->getY(),
        direction,
        colorAndAlpha.first,
        colorAndAlpha.second
    );
}

QPair<Color, float> TileGraphic::deduceWallColorAndAlpha(Direction direction) const {

    // Declare the wall color and alpha, assign defaults
    Color wallColor = STRING_TO_COLOR.value(P()->tileWallColor());
    float wallAlpha = 1.0;

    // Either draw the true walls of the tile ...
    if (S()->wallTruthVisible()) {
        wallAlpha = m_tile->isWall(direction) ? 1.0 : 0.0;
    }

    // ... or the algorithm's (un)declared walls
    else {

        // If the wall was declared, use the wall color and tile base color ...
        if (wallDeclared(direction)) {
            if (m_declaredWalls.value(direction)) {
                // Correct declaration
                if (m_tile->isWall(direction)) {
                    wallColor = STRING_TO_COLOR.value(P()->tileWallColor());
                }
                // Incorrect declaration
                else {
                    wallColor = STRING_TO_COLOR.value(P()->tileIncorrectlyDeclaredWallColor());
                }
            }
            else {
                // Incorrect declaration
                if (m_tile->isWall(direction)) {
                    wallColor = STRING_TO_COLOR.value(P()->tileIncorrectlyDeclaredNoWallColor());
                }
                // Correct declaration
                else {
                    wallAlpha = 0.0;
                }
            }
        }

        // ... otherwise, use the undeclared walls colors
        else {
            if (m_tile->isWall(direction)) {
                wallColor = STRING_TO_COLOR.value(P()->tileUndeclaredWallColor());
            }
            else {
                wallColor = STRING_TO_COLOR.value(P()->tileUndeclaredNoWallColor());
            }
        }
    }
    
    // If the wall color is the same as the default tile base color,
    // we interpret that to mean that the walls should be transparent
    if (wallColor == STRING_TO_COLOR.value(P()->tileBaseColor())) {
        wallAlpha = 0.0;
    }

    return {wallColor, wallAlpha};
}

} // namespace sim
