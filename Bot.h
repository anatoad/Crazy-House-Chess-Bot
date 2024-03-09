#ifndef BOT_H
#define BOT_H
#include <bits/stdc++.h>

#include "Move.h"
#include "PlaySide.h"

#define BOARD_SIZE 8
#define MAX_DEPTH 4 

#define DIRECTIONS 8
#define ROOK_DIRECTIONS 4
#define BISHOP_DIRECTIONS 4
#define KNIGHT_DIRECTIONS 8

#define INF 1000000000

enum BoardPiece { 
    WHITE_PAWN = 1, WHITE_ROOK = 2, WHITE_BISHOP = 3,
    WHITE_KNIGHT = 4, WHITE_QUEEN = 5, WHITE_KING = 6,
    BLACK_PAWN = 7, BLACK_ROOK = 8, BLACK_BISHOP = 9,
    BLACK_KNIGHT = 10, BLACK_QUEEN = 11, BLACK_KING = 12,
    EMPTY = 0
};

enum PiecePoints {
    POINTS_PAWN = 1, POINTS_KNIGHT = 3, POINTS_BISHOP = 3,
    POINTS_ROOK = 5, POINTS_QUEEN = 9, POINTS_KING = 100
};

enum Direction {
    UPPER_LEFT = 0, UP = 1, UPPER_RIGHT = 2,
    RIGHT = 3,
    LOWER_RIGHT = 4, DOWN = 5, LOWER_LEFT = 6,
    LEFT = 7
};

enum PlayMode {
    NORMAL_MODE = 0, FORCE_MODE = 1
};

/* all possible directions, also used by the queen, king */
const int dx[] = {-1, -1, -1,  0,  1,  1,  1,  0};
const int dy[] = {-1,  0,  1,  1,  1,  0, -1, -1};

const int knight_dx[] = {-2, -2, -1,  1,  2,  2,  1, -1};
const int knight_dy[] = {-1,  1,  2,  2,  1, -1, -2, -2};

const int rook_dx[] = {-1, 0, 1,  0};
const int rook_dy[] = { 0, 1, 0, -1};

const int bishop_dx[] = {-1,  1,  1, -1};
const int bishop_dy[] = {-1, -1,  1, 1};

typedef struct {
    int x, y;
} position;

typedef struct {
    position pos;
    int dir;
} dir;

class Bot {
 private:
    static const std::string BOT_NAME;

    PlaySide botPlaySide;
    PlaySide sideToMoveNext;

    bool castlePossible[2][2];  /* 0 - Queen side castle, 1 - King side castle
                                   castlePossible[PlaySide::WHITE][] - for WHITE's castling rights
                                   castlePossible[PlaySide::BLACK][] - for BLACK's castling rights */

    Move *nextMove; /* next move generated */
    Move *lastRecordedMove; /* last recorded move */

    int moveCount; /* number of moves (from the beginning of a game) without any captured pieces or pawns moved */

    PlayMode mode;

    int board[BOARD_SIZE + 1][BOARD_SIZE + 1];

    int pool[2][5];  /* number of pieces captured and can be dropped */
                     /* pool[PlaySide::BLACK] - black's pool
                        pool[PlaySide::WHITE] - white's pool */


    void initBoard();

    std::string toString(position pos);

    position getMovePosition(std::optional<std::string> pos);

    void movePiece(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], position src, position dst, PlaySide playside);

    int getBoardPiece(Piece piece, PlaySide playSide);

    PlaySide getPlaySide(int value);

    PlaySide getOpponentPlaySide(PlaySide playSide);

    position getKingPosition(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide sideToMove);

    bool isPositionValid(position pos);

    bool inCheck(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide);

    bool landsInCheck(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], position src, position dst, BoardPiece replc);

    bool isAttacked(position src, position dst);

    Piece getPiece(int value);

    int getPiecePoints(int value);

    bool spaceForCastle(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide, int type);

    bool castle(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide);

    void defendCheck(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide sideToMove);

    std::vector<Move*> generateAllMoves(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide);

    bool enPassantRights(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], position src, position dst);

    void pushPieceMoves(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], position src, std::vector<Move*> &moves, \
                        int directions, const int dx[], const int dy[]);

    int evaluate(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1]);

    int getPiecePointsDiff(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1]);

    int minimax(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], int depth);

    int makeMove(Move *move, int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide);

    void undoMove(Move *move, int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], int captured, PlaySide playSide);

 public:
    static std::string moveToString(Move* move);

    bool positionEquals(position p, position q);

    PlaySide getBotPlaySide();

    void setBotPlaySide(PlaySide playSide);

    void setMode(PlayMode playMode);

    Bot();

    /**
     * Record move (either by enemy in normal mode, or by either side
     * in force mode) in custom structures
     * @param move received move
     * @param sideToMode side to move
     */
    void recordMove(Move* move, PlaySide sideToMove);

    /**
     * Calculates next move, in response to enemyMove
     * @param enemyMove the enemy's last move
     *                  null if this is the opening move, or previous
     *                  move has been recorded in force mode
     * @return your move
     */
    Move* calculateNextMove();

    static std::string getBotName();
};
#endif
