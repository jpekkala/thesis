#ifndef SEARCHWIDGET_H
#define	SEARCHWIDGET_H

#include <QtWidgets>
#include "SearchWorker.h"
#include "BoardWidget.h"

/**
 * Controls the search worker. Also shows the status of the worker and the results
 * when the worker is finished
 */
class SearchWidget : public QWidget {
    Q_OBJECT

    BoardWidget* boardWidget;
    QTextEdit *textEdit;

    SearchWorker* worker;
    QThread* workerThread;

    bool available;
    bool columnMode;
    bool computerMode;

public:
    SearchWidget(BoardWidget*);
    ~SearchWidget();

    virtual QSize sizeHint() const {
        return QSize(200, boardWidget->height() / 2);
    }

public slots:
    void changeTransSize(int);
    void copyAll();
    void setResults(QString text);
    void setText(QString text);
    bool isLocked();
    void doAlphaBeta();
    void doLogical();
    void doProofNumber();
    void doExactProofNumber();
    void doRetrograde();
    void doFullWidth();
    void setLamp(int x, int v);
    void changeFontSize();
    void setComputerMode(bool on);
    void makeComputerMove(char ch);
    void setVariation(QString variation);
    void changeDepthLimit();

    void setColumnMode(bool flag) {
        columnMode = flag;
    }

signals:
    void computerSearchStarted(SearchWorker::SearchRequest);
    void singleSearchStarted(SearchWorker::SearchRequest);
    void columnSearchStarted(SearchWorker::SearchRequest);
    void searchStarted();
    void searchEnded();
    void computerMoveFailed();
    void limitsChanged();

private:
    void lock();
    void unlock();
};


#endif

