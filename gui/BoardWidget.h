#ifndef BOARDWIDGET_H
#define	BOARDWIDGET_H

#include <QtWidgets>
#include "connect4.h"
#include "game.h"

/**
 * Draws a Connect-4 board and reacts to mouse clicks.
 */
class BoardWidget : public QWidget {
    Q_OBJECT

            static const int COLUMNS = BOARD_WIDTH;
    static const int ROWS = BOARD_HEIGHT;



    //the GUI class does not use bitboards because it needs to store more information
    int cells[COLUMNS][ROWS];

    //values used in the cells array, e.g. WHITE | WIN encodes a winning white disc
    static const int EMPTY = 0;
    static const int WHITE = 1;
    static const int RED = 2;
    static const int WINNING = 4;

    //the current state of the game, cells can be initialized using these
    std::vector<char> moves; //allows undoing moves
    std::vector<char> deletedMoves; //allows redoing moves
    int won; //if the game is won, no further moves allowed

    static const int LAMP_OFF = -1;
    int lamps[COLUMNS * 2];
    bool movesAllowed;
    bool computerMode;
    
    Game game;

public:

    BoardWidget();
    bool drop(int n);
    bool pop(int n);
    void undo(int times = 1);
    QString getVariation();
   
    int getPly() {
        return moves.size();
    }
    
    bool hasWon() {
        return won;
    }

    virtual QSize sizeHint() const {
        return QSize(COLUMNS * 100, ROWS * 100);
    }

public slots:
    void restart();
    bool setVariation(const QString& variation);
    void clearLamps();
    void setLamp(int colmun, int value);
    void allowMoves(bool);
    void setComputerMode(bool);

signals:
    void positionChanged(QString variation);

protected:
    virtual void keyPressEvent(QKeyEvent*);
    virtual void paintEvent(QPaintEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void wheelEvent(QWheelEvent*);

private:
    //update the cells array (i.e. set possible winning discs)
    void checkWins();
    void checkLine(int, int, int, int);
    QColor getLampColor(int);

    //contains coordinates and sizes used in the graphical board
    int cellWidth;
    int cellHeight;
    int paddingX;
    int paddingY;
};

#endif

