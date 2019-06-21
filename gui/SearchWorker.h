#ifndef SEARCHWORKER_H
#define	SEARCHWORKER_H

#include <QtWidgets>
#include "retro.h"
#include "minimax.h"
#include "alphabeta.h"
#include "handicap.h"
#include "proof.h"

/**
 * Executes the search in a non-GUI thread
 */
class SearchWorker : public QObject {
    Q_OBJECT
    TransTable *transTable;
    AlphaBeta* alphaBeta;
    Handicap* handicap;
    Retro* retro;
    Proof* proof;
    
public:

    static const int DEFAULT_TT_SIZE = 67108859;

    enum RequestType {
        AlphaBetaRequest, HandicapRequest, RetrogradeRequest, ProofNumberRequest, ExactProofNumberRequest
    };

    typedef struct {
        SearchWorker::RequestType type;
        QString variation;
        int maxDepth;
        bool useLamps;

    } SearchRequest;

    SearchWorker();
    ~SearchWorker();

    void setTransTableSize(int);
    int getPlyLimit();
    void setPlyLimit(int limit);

public slots:
    void processColumns(SearchRequest);
    void processSingle(SearchRequest);
    void processComputer(SearchRequest);

signals:
    void update(QString text);
    void resultsReady(QString result);
    void updateLamp(int x, int v);
    void computerMove(char ch);

private:
    int getResult(const SearchRequest& type, const Game& game);
    void alphaBetaReport();
    void handicapReport();
    void proofNumberReport();
};

#endif

