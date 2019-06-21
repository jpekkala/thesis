#include "BoardWidget.h"
#include "game.h"
#include <cassert>
#include <iostream>

BoardWidget::BoardWidget() : movesAllowed(true), computerMode(true) {
    restart();
}

void BoardWidget::restart() {
    for (int x = 0; x < COLUMNS; x++) {
        for (int y = 0; y < ROWS; y++) {
            cells[x][y] = EMPTY;
        }
    }
    game.reset();
    clearLamps();
    moves.clear();
    deletedMoves.clear();
    won = 0;
    repaint();
    emit positionChanged(getVariation());
}

bool BoardWidget::setVariation(const QString& variation) {

    restart();
    for (int i = 0; i < variation.length(); i++) {
        char ch = variation.at(i).toLatin1();
        int x;
        if ((x = ch - '1') >= 0 && x < COLUMNS) {
            if (!drop(x)) return false;
        } else if ((x = ch - 'a') >= 0 && x < COLUMNS) {
            if (!drop(x)) return false;
        } else if ((x = ch - 'A') >= 0 && x < COLUMNS) {
            if (!pop(x)) return false;
        } else if (ch == '(') {
            if (i + 2 >= variation.length()) return false;
            ch = variation.at(i + 1).toLatin1();
            char closing = variation.at(i + 2).toLatin1();
            if (closing != ')') return false;
            if ((x = ch - '1') < 0 || x >= COLUMNS) return false;
            if (!pop(x)) return false;
            i += 2;
        } else if (ch == 'P' || ch == '*') {
            if (i + 1 >= variation.length()) return false;
            ch = variation.at(i + 1).toLatin1();
            if ((x = ch - '1') < 0 || x >= COLUMNS) return false;
            if (!pop(x)) return false;
            i++;
        } else {
            return false;
        }
    }
    return true;
}

void BoardWidget::setLamp(int column, int value) {
    lamps[column] = value;
}

void BoardWidget::clearLamps() {
    for (int i = 0; i < COLUMNS * 2; i++) {
        lamps[i] = LAMP_OFF;
    }
}

void BoardWidget::allowMoves(bool on) {
    movesAllowed = on;
}

void BoardWidget::setComputerMode(bool on) {
    computerMode = on;
}

QString BoardWidget::getVariation() {
    QString var;

    foreach(char ch, moves) {
        var.append(ch);
    }
    return var;
}

QPolygon createArrow(int width, int height) {
    QPolygon arrow;
    int tipBaseY = height / 2;
    int middleX = width / 2;
    int shaftWidth = width / 4;
    int tipWidth = width / 2;

    arrow.append(QPoint(middleX, height));
    arrow.append(QPoint(middleX + tipWidth, tipBaseY));
    arrow.append(QPoint(middleX + shaftWidth, tipBaseY));
    arrow.append(QPoint(middleX + shaftWidth, 0));
    arrow.append(QPoint(middleX - shaftWidth, 0));
    arrow.append(QPoint(middleX - shaftWidth, tipBaseY));
    arrow.append(QPoint(middleX - tipWidth, tipBaseY));
    return arrow;
}

QColor BoardWidget::getLampColor(int i) {
    using namespace Connect4;

    bool whiteToMove = getPly() % 2 == 0;
    if (lamps[i] == DRAW) return QColor::fromRgb(255, 255, 0);
    else if (lamps[i] == (whiteToMove ? WIN : LOSS)) return QColor::fromRgb(255, 255, 255);
    else if (lamps[i] == (whiteToMove ? LOSS : WIN)) return QColor::fromRgb(255, 0, 0);
    else if (lamps[i] == (whiteToMove ? DRAW_OR_WIN : DRAW_OR_LOSS)) return QColor::fromRgb(255, 255, 128);
    else if (lamps[i] == (whiteToMove ? DRAW_OR_LOSS : DRAW_OR_WIN)) return QColor::fromRgb(255, 128, 0);
    else if (lamps[i] == DRAW_BY_REPEAT) return QColor::fromRgb(255, 255, 0);
    else return QColor::fromRgb(0, 0, 0);
}

void BoardWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);

    QColor bgColor = QColor::fromRgb(0x83afda);
    //QColor bgColor = QColor::fromRgb(0xffffff);
    //draw background
    painter.setBrush(bgColor);
    painter.drawRect(0, 0, width(), height());

    //calculate the placement of different elements based on widget width and heigh
    this->cellWidth = width() / (COLUMNS + 2);
    this->cellHeight = height() / (ROWS + 2);
    double holeRatio = 0.8; //how big the cell hole is compared to cell size
    int holeWidth = holeRatio*cellWidth;
    int holeHeight = holeRatio*cellHeight;
    int edgeX = cellWidth * 0.2; //the edge around the cell area
    int edgeY = cellHeight * 0.15;
    int emptyX = (width() - cellWidth * COLUMNS - 2 * edgeX) / 2;
    int emptyY = (height() - cellHeight * ROWS - 2 * edgeY) / 2;
    this->paddingX = emptyX + edgeX;
    this->paddingY = emptyY + edgeY;

    //draw board
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor::fromRgb(0x002890));
    QRect rect(emptyX, emptyY, width() - 2 * emptyX, height() - 2 * emptyY);
    painter.drawRoundedRect(rect, 18.0, 18.0, Qt::RelativeSize);


    for (int column = 0; column < COLUMNS; column++) {
        for (int row = 0; row < ROWS; row++) {
            int x = emptyX + edgeX + column * cellWidth + (1 - holeRatio) * cellWidth / 2;
            int y = emptyY + edgeY + (ROWS - row - 1) * cellHeight + (1 - holeRatio) * cellHeight / 2;

            QLinearGradient gradient(x, y, x + holeWidth * 2, y + holeHeight * 2);
            switch (cells[column][row]) {
                case EMPTY:
                    gradient.setColorAt(0, bgColor);
                    break;
                case WHITE:
                    gradient.setColorAt(0, QColor::fromRgb(0xffffff));
                    gradient.setColorAt(1, QColor::fromRgb(0x808080));
                    //gradient.setColorAt(0, QColor::fromRgb(0xffffff));
                    //gradient.setColorAt(1, QColor::fromRgb(0x000000));
                     //gradient.setColorAt(0, QColor::fromRgb(0x888888));
                   // gradient.setColorAt(1, QColor::fromRgb(0x000000));
                    break;
                case RED:
                    gradient.setColorAt(0, QColor::fromRgb(0xff0000));
                    gradient.setColorAt(0.5, QColor::fromRgb(0x880000));
                    
                     //gradient.setColorAt(0, QColor::fromRgb(0x880000));
                    //gradient.setColorAt(0.5, QColor::fromRgb(0x000000));
                    break;
                case (WHITE | WINNING):
                case (RED | WINNING):
                    gradient.setColorAt(0, QColor::fromRgb(0xffff00));
                    gradient.setColorAt(1, QColor::fromRgb(0x808080));
            }

            painter.setBrush(gradient);
            painter.drawEllipse(x, y, holeWidth, holeHeight);


        }
    }

    //draw a green marker for previous move
    if (moves.size() > 0) {

        char ch = moves.back();
        int lastX, lastY;
        bool wasPop = false;
        if (ch >= 'a') {
            lastX = ch - 'a';
            for (lastY = ROWS - 1; cells[lastX][lastY] == EMPTY; lastY--);
        } else {
            lastX = ch - 'A';
            lastY = 0;
            wasPop = true;
        }

        double dotRatio = 0.4;

        int dotX = emptyX + edgeX + lastX * cellWidth + (1 - dotRatio) * cellWidth / 2;
        int dotY = emptyY + edgeY + (ROWS - lastY - 1) * cellHeight + (1 - dotRatio) * cellHeight / 2;
        int dotWidth = dotRatio * cellWidth;
        int dotHeight = dotRatio * cellHeight;

        QLinearGradient gradient(dotX, dotY, dotWidth, dotHeight);
        gradient.setColorAt(0, QColor::fromRgb(0x00ff00));
        gradient.setColorAt(1, QColor::fromRgb(0x00b200));
        painter.setBrush(gradient);

        if (wasPop) {
            dotHeight *= 1.1;

            QPolygon arrow = createArrow(dotWidth, dotHeight * 1.1);
            arrow.translate(dotX, dotY);
            painter.drawPolygon(arrow);
        } else {

            painter.drawEllipse(dotX, dotY, dotWidth, dotHeight);
        }
    }

    //draw lamps
    for (int i = 0; i < COLUMNS; i++) {
        if (lamps[i] != LAMP_OFF) {
            painter.setBrush(getLampColor(i));
            int x = emptyX + edgeX + cellWidth * i;
            int y = emptyY - cellHeight * 0.4;
            painter.drawRect(x + cellWidth * 0.15, y, cellWidth * 0.7, cellHeight * 0.25);
        }

        if (lamps[i + COLUMNS] != LAMP_OFF) {
            painter.setBrush(getLampColor(i + COLUMNS));
            int x = emptyX + edgeX + cellWidth * i + cellWidth * 0.15;
            int y = emptyY + 3.5 * edgeY + cellHeight * ROWS;
            QPolygon arrow = createArrow(cellWidth * 0.7, cellHeight * 0.35);
            arrow.translate(x, y);
            painter.drawPolygon(arrow);
            //painter.drawRect(x + cellWidth * 0.15, y, cellWidth * 0.7, cellHeight * 0.25);
        }
    }
}

/**
 * Drop on left mouse click, pop on right mouse click
 */
void BoardWidget::mousePressEvent(QMouseEvent* e) {
    if (!movesAllowed) return;
    double x = ((double) e->x() - paddingX) / cellWidth;
    //double y = (height() - paddingY - (double) e->y()) / cellHeight;
    if (x < 0) return; //check negative before casting to int because of rounding
    int n = (int) x;
    if (n >= COLUMNS) return;
    if (e->button() == Qt::LeftButton) drop(n);
    else if (e->button() == Qt::RightButton) pop(n);
}

/**
 * Undo or redo moves on mouse wheel event
 */
void BoardWidget::wheelEvent(QWheelEvent* e) {
    if (!movesAllowed) return;
    if (e->orientation() == Qt::Vertical) {
        if (e->delta() < 0) {
            if (computerMode) {
                undo(2);
            } else {
                undo();
            }
        } else if (deletedMoves.size() > 0) {
            char ch = deletedMoves.back();
            if (ch >= 'a') {
                drop(ch - 'a');
            } else {
                pop(ch - 'A');
            }
        }
    }
}

void BoardWidget::keyPressEvent(QKeyEvent* e) {
    if (e->key() == Qt::Key_Space) {
        std::cout << Connect4::toString(game.getPosition()) << std::endl;
        std::cout << "Position code: " << game.getPosition() << std::endl;
        std::cout << "Ply: " << game.getPly() << std::endl;
    }
}

/**
 * Make a drop move.
 * 
 * @param x     the column index (first index is 0)
 * @return      true if the move could be done
 */
bool BoardWidget::drop(int x) {
    if (won) return false;

    for (int y = 0; y < ROWS; y++) {
        if (cells[x][y] == EMPTY) {
            cells[x][y] = getPly() % 2 == 0 ? WHITE : RED;
            char ch = 'a' + x;
            moves.push_back(ch);
            if (deletedMoves.size() > 0 && deletedMoves.back() == ch) {
                deletedMoves.pop_back();
            } else {
                deletedMoves.erase(deletedMoves.begin(), deletedMoves.end());
            }
            clearLamps();
            checkWins();
            repaint();
            game.drop(x);
            emit positionChanged(getVariation());
            return true;
        }
    }
    return false;
}

/**
 * Make a pop move.
 * 
 * @param x     the column index (first index is 0)
 * @return      true if the move could be done
 */
bool BoardWidget::pop(int x) {
    if (won) return false;

    if (cells[x][0] != (getPly() % 2 == 0 ? WHITE : RED)) return false;

    for (int y = 0; y < ROWS - 1; y++) {
        cells[x][y] = cells[x][y + 1];
    }
    cells[x][ROWS - 1] = EMPTY;
    char ch = 'A' + x;
    moves.push_back(ch);
    if (deletedMoves.size() > 0 && deletedMoves.back() == ch) {
        deletedMoves.pop_back();
    } else {
        deletedMoves.erase(deletedMoves.begin(), deletedMoves.end());
    }
    clearLamps();
    checkWins();
    repaint();
    game.pop(x);
    emit positionChanged(getVariation());
    return true;
}

void BoardWidget::undo(int times) {
    if (moves.size() == 0) return;

    for (int i = 0; i < times; i++) {
        if(moves.size() == 0) break;

        char ch = moves.back();
        moves.pop_back();
        deletedMoves.push_back(ch);
        if (ch >= 'a') {
            int x = ch - 'a';
            for (int y = ROWS - 1; y >= 0; y--) {
                if (cells[x][y] != EMPTY) {
                    cells[x][y] = EMPTY;
                    break;
                }
            }
            game.undrop(x);

        } else {
            int x = ch - 'A';
            for (int y = ROWS - 1; y > 0; y--) {
                cells[x][y] = cells[x][y - 1];
            }
            cells[x][0] = getPly() % 2 == 0 ? WHITE : RED;
            game.unpop(x);
        }

        if (won) {
            for (int x = 0; x < COLUMNS; x++) {
                for (int y = 0; y < ROWS; y++) {

                    cells[x][y] &= ~WINNING;
                }
            }
            won = 0;
        }
    }
    clearLamps();
    repaint();
    emit positionChanged(getVariation());
}

void BoardWidget::checkWins() {
    //vertical
    for (int x = 0; x < COLUMNS; x++) {
        checkLine(x, 0, 0, 1);
    }

    //horizontal
    for (int y = 0; y < ROWS; y++) {
        checkLine(0, y, 1, 0);
    }

    //diagonals
    for (int x = -ROWS + 4; x < COLUMNS - 3; x++) {
        if (x < 0) {
            checkLine(0, -x, 1, 1);
            checkLine(0, ROWS - 1 + x, 1, -1);
        } else {
            checkLine(x, 0, 1, 1);
            checkLine(x, ROWS - 1, 1, -1);
        }

    }

    //if both players have four in row, the winner is the player whose move it was
    int both = WHITE | RED;
    if ((won & both) == both) {
        int loser = getPly() % 2 == 0 ? WHITE : RED;
        for (int x = 0; x < COLUMNS; x++) {
            for (int y = 0; y < ROWS; y++) {
                if ((cells[x][y] & loser) != 0) {
                    //remove any winning cell the loser might have

                    cells[x][y] &= ~WINNING;
                }
            }
        }
    }
}

void BoardWidget::checkLine(int x, int y, int dispX, int dispY) {
    int lastColor = EMPTY;
    int count = 0;

    for (; (x >= 0 && x < COLUMNS) && (y >= 0 && y < ROWS); x += dispX, y += dispY) {
        int d = cells[x][y];

        if (d == EMPTY) {
            lastColor = EMPTY;
            count = 0;
            continue;
        }

        d &= ~WINNING;

        if (lastColor == d) {
            int winDisc = d | WINNING;
            count++;
            if (count == 4) {
                won |= winDisc;
                for (int i = 0; i < 4; i++) {
                    cells[x - i * dispX][y - i * dispY] = winDisc;
                }
            } else if (count > 4) {
                cells[x][y] = winDisc;
            }
        } else {
            lastColor = d;
            count = 1;
        }
    }
}
