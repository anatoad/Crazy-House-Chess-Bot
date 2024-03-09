#include "Bot.h"

#include <bits/stdc++.h>

const std::string Bot::BOT_NAME = "sigsegv";

/**
 *  Initialize board and reset engine's parameters.
*/
Bot::Bot() {
    initBoard();

    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 2; j++)
            castlePossible[i][j] = true;

    for (int i = 0; i < 5; i++)
        pool[WHITE][i] = pool[BLACK][i] = 0;

    botPlaySide = BLACK;

    nextMove = nullptr;
    lastRecordedMove = nullptr;

    moveCount = 0;

    mode = NORMAL_MODE;
}

/**
 * Record move received from xboard into internal chess board representation.
 * @param move move
 * @param sideToMove side to move
*/
void Bot::recordMove(Move* move, PlaySide sideToMove) {
    lastRecordedMove = Move::copyMove(move);

    if (move->isDropIn()) {
        position dst = getMovePosition(move->getDestination());
        Piece piece = move->getReplacement().value();
        pool[sideToMove][piece]--;
        board[dst.x][dst.y] = getBoardPiece(piece, sideToMove);
        return;

    } else if (move->isPromotion()) {
        position src = getMovePosition(move->getSource());
        position dst = getMovePosition(move->getDestination());
        Piece piece = move->getReplacement().value();

        if (board[dst.x][dst.y] != EMPTY) {
            Piece capturedPiece = (board[dst.x][dst.y] < 0) ? PAWN : getPiece(board[dst.x][dst.y]);
            pool[sideToMove][capturedPiece]++;
        }

        board[src.x][src.y] = EMPTY;
        board[dst.x][dst.y] = - getBoardPiece(piece, sideToMove);
        return;     
    }
 
    position src = getMovePosition(move->getSource());
    position dst = getMovePosition(move->getDestination());

    movePiece(board, src, dst, sideToMove);
}

/**
 * Calculate the bot's next move.
 * @returns the next move of the bot
*/
Move* Bot::calculateNextMove() {
    /* check if in check, if so defend yourself */
    if (inCheck(board, botPlaySide)) {
        defendCheck(board, botPlaySide);

    } else {
        nextMove = nullptr;
        /* first check if castling is possible */
        if (!castle(board, botPlaySide)) {
            minimax(board, 0);
        }
    }

    if (!nextMove) { /* stalemate (no legal moves) */
        std::cout << "1/2-1/2 {Stalemate}\n";
    }
    
    /* record move */
    if (nextMove->isDropIn()) {
        position dst = getMovePosition(nextMove->getDestination());
        Piece piece = nextMove->getReplacement().value();

        board[dst.x][dst.y] = getBoardPiece(piece, botPlaySide);
        pool[botPlaySide][piece]--;

        if (piece == PAWN)
            moveCount = 0;
    } else {
        position src = getMovePosition(nextMove->getSource());
        position dst = getMovePosition(nextMove->getDestination());

        movePiece(board, src, dst, botPlaySide);

        if (nextMove->isPromotion()) {
            Piece promotedPiece = nextMove->getReplacement().value();
            board[dst.x][dst.y] = - getBoardPiece(promotedPiece, botPlaySide);
        }

    }

    if (moveCount >= 50) {
        std::cout << "1/2-1/2 {Draw by repetition}\n";
    }

    return nextMove;
}

/**
 * Get the name of the bot.
*/
std::string Bot::getBotName() {
    return Bot::BOT_NAME;
}

/**
 * Initialize the chess board representation, with all pieces in the starting position.
*/
void Bot::initBoard() {
    for (int i = 3; i <= 6; i++)
        for (int j = 1; j <= 8; j++)
            board[i][j] = EMPTY;

    for (int j = 1; j <= 8; j++)
        board[2][j] = WHITE_PAWN;

    board[1][1] = board[1][8] = WHITE_ROOK;
    board[1][2] = board[1][7] = WHITE_KNIGHT;
    board[1][3] = board[1][6] = WHITE_BISHOP;
    board[1][4] = WHITE_QUEEN;
    board[1][5] = WHITE_KING;

    for (int j = 1; j <= 8; j++)
        board[7][j] = BLACK_PAWN;

    board[8][1] = board[8][8] = BLACK_ROOK;
    board[8][2] = board[8][7] = BLACK_KNIGHT;
    board[8][3] = board[8][6] = BLACK_BISHOP;
    board[8][4] = BLACK_QUEEN;
    board[8][5] = BLACK_KING;
} 

/**
 * Get the bot's playSide.
*/
PlaySide Bot::getBotPlaySide() {
    return botPlaySide;
}

/**
 * Set the bot's playSide.
 * @param playSide play side
*/
void Bot::setBotPlaySide(PlaySide playSide) {
    this->botPlaySide = playSide;
}

/**
 * Set the bot's playMode.
 * @param playMode playMode
*/
void Bot::setMode(PlayMode playMode) {
    this->mode = playMode;
}

/**
 * Get play side of the opponent.
 * @param playSide side to move
 * @returns opponent's play side
*/
PlaySide Bot::getOpponentPlaySide(PlaySide playSide) {
    switch (playSide) {
        case BLACK: return WHITE;
        case WHITE: return BLACK;
        default: return NONE;
    }
}

/**
 *  Get BoardPiece value.
 * @param piece piece
 * @param playSide side to move
 * @returns BoardPiece value of piece of color playSide
*/
int Bot::getBoardPiece(Piece piece, PlaySide playSide) {
    return piece + (playSide == BLACK) * 6 + 1;
}

/**
 * Generate a string representation of pos, in coordinate notation (e.g. b4, e8).
 * @param pos position on the chess board
 * @returns string representation of pos
*/
std::string Bot::toString(position pos) {
    std::string str;
    str +=  (char)('a' + pos.y - 1);
    str += (char)('0' + pos.x);
    return str;
}

/**
 * Move piece at src to dst, record changes on board.
 * @param src source position
 * @param dst destination position
 * @param playSide side to move
*/
void Bot::movePiece(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], position src, position dst, PlaySide playSide) {
    Piece pieceToMove = getPiece(board[src.x][src.y]);
    if (pieceToMove == PAWN)
        moveCount = 0;
    
    if (board[dst.x][dst.y] != EMPTY) {  /* capture piece */
        Piece capturedPiece = board[dst.x][dst.y] < 0 ? PAWN : getPiece(board[dst.x][dst.y]);
        pool[playSide][capturedPiece]++;
        moveCount = 0;
    } else if (pieceToMove == Piece::PAWN && src.y != dst.y) { /* en passant */
        pool[playSide][Piece::PAWN]++;
        board[src.x][dst.y] = EMPTY;
        moveCount = 0;
    } else {
        moveCount++;
    }

    /* for castling */
    if (pieceToMove == Piece::KING) {
        castlePossible[playSide][0] = castlePossible[playSide][1] = false;

        if (abs(src.y - dst.y) > 1) { /* move rook when castling */
            int col = (src.y - dst.y > 0) ? 1 : 8;
            int diff = (src.y - dst.y > 0) ? 3 : -2;
            board[src.x][col + diff] = board[src.x][col];
            board[src.x][col] = EMPTY;
        }
    } 
    
    if (pieceToMove == Piece::ROOK) {
        if (src.y == 1)  /* Queen side rook */
            castlePossible[playSide][0] = false;
        else if (src.y == 8)  /* King side rook */
            castlePossible[playSide][1] = false;
    }

    board[dst.x][dst.y] = board[src.x][src.y];
    board[src.x][src.y] = EMPTY;
}

/**
 * Parse position from a string.
 * @param pos string representation of position in coordinate notation (e.g. a1, f5).
*/
position Bot::getMovePosition(std::optional<std::string> pos) {
    if (!pos.has_value())
        return {0,0};

    position p;
    p.x = pos.value()[1] - '0';
    p.y = pos.value()[0] - 'a' + 1;

    return p;
}

/**
 * Check if position is valid (within the borders of the chess board).
 * @param pos position
 * @returns true if position is valid, false otherwise
*/
bool Bot::isPositionValid(position pos) {
    return (pos.x > 0 && pos.x <= BOARD_SIZE && pos.y > 0 && pos.y <= BOARD_SIZE);
}

/**
 * Returns the side of the piece with given value.
 * @param value value of piece on the board
*/
PlaySide Bot::getPlaySide(int value) {
    if (value == 0)
        return PlaySide::NONE;

    if (abs(value) <= 6)
        return PlaySide::WHITE;

    return PlaySide::BLACK;
}

/**
 * Returns a Piece with given value.
 * @param value value of piece on the board
*/
Piece Bot::getPiece(int value) {
    int mod = abs(value) % 6;
    switch (mod) {
        case 1: return Piece::PAWN;
        case 2: return Piece::ROOK;
        case 3: return Piece::BISHOP;
        case 4: return Piece::KNIGHT;
        case 5: return Piece::QUEEN;
        default: return Piece::KING;
    }
}

/**
 * Returns the number of points of a piece with given value.
 * @param piece piece
*/
int Bot::getPiecePoints(int value) {
    Piece piece = getPiece(value);
    switch (piece) {
        case PAWN: return POINTS_PAWN;
        case KNIGHT: return POINTS_KNIGHT;
        case BISHOP: return POINTS_BISHOP;
        case ROOK: return POINTS_ROOK;
        case QUEEN: return POINTS_QUEEN;
        case KING: return POINTS_KING;
        default: return 0;
    }
}

/**
 * Find the King's position on the board.
 * @param board board configuration
 * @param playSide side to move
 * @returns a position with the King's coordinates on the board
*/
position Bot::getKingPosition(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide) {
    position king_position = {0,0};
    for (int x = 1; x <= BOARD_SIZE; x++)
        for (int y = 1; y <= BOARD_SIZE; y++)
            if (board[x][y] == 6 + (1 - playSide) * 6) {
                king_position = {x, y};
                break;
            }
    
    return king_position;
}

/**
 * Check if piece at src position is attacked by piece at position dst.
 * @param src source position
 * @param dst destination position
 * @return true if piece at src is attacked by piece at dst, false otherwise
*/
bool Bot::isAttacked(position src, position dst) {
    PlaySide src_side = getPlaySide(board[src.x][src.y]);
    PlaySide dst_side = getPlaySide(board[dst.x][dst.y]);

    if (src_side == dst_side || dst_side == PlaySide::NONE)
        return false;

    Piece piece = getPiece(board[dst.x][dst.y]);

    if (piece == Piece::PAWN) {
        int row_diff = (dst_side == PlaySide::WHITE) ? 1 : -1;
        return (src.x - dst.x == row_diff && abs(src.y - dst.y) == 1);
    
    } else if (piece == Piece::ROOK) {
        return (src.x == dst.x || src.y == dst.y);

    } else if (piece == Piece::BISHOP) {
        return (abs(src.x - dst.x) == abs(src.y - dst.y));

    } else if (piece == Piece::KNIGHT) {
        return (abs(src.x - dst.x) > 0 && abs(src.y - dst.y) > 0 && abs(src.x - dst.x) + abs(src.y - dst.y) == 3);

    } else if (piece == Piece::QUEEN) {
        return (src.x == dst.x || src.y == dst.y || abs(src.x - dst.x) == abs(src.y - dst.y));

    } else if (piece == Piece::KING) {
        return (abs(src.x - dst.x) <= 1 && abs(src.y - dst.y) <= 1);
    }

    return false;
}

/**
 * Check if bot is in check.
 * @param board board configuration
 * @param sideToMove side to move
 * @return true if bot's King is in check, false otherwise
*/
bool Bot::inCheck(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide sideToMove) {
    int row, col;
    dir p;

    /* find the king position on the board, then check if it is attacked by an opponent's piece */
    position king_position = getKingPosition(board, sideToMove);

    std::queue<dir> queue;
    bool vis[BOARD_SIZE + 1][BOARD_SIZE + 1] = {false};

    /* first check if any knight is attacking the king */
    for (int i = 0; i < DIRECTIONS; i++) {
        row = king_position.x + knight_dx[i];
        col = king_position.y + knight_dy[i];

        if (isPositionValid({row, col})) {
            Piece piece = getPiece(board[row][col]);
            PlaySide piece_color = getPlaySide(board[row][col]);

            if (piece_color != PlaySide::NONE) {
                if (piece == Piece::KNIGHT && piece_color != sideToMove)
                    return true;
                
                vis[row][col] = true;
            }
        }
    }

    /* then use BFS to search the board */
    vis[king_position.x][king_position.y] = true;

    for (int i = 0; i < DIRECTIONS; i++) {
        row = king_position.x + dx[i];
        col = king_position.y + dy[i];

        if (isPositionValid({row, col}) && getPlaySide(board[row][col]) != sideToMove) {
            queue.push({{row, col}, i});
            vis[row][col] = true;
        }
    }

    while (!queue.empty()) {
        p = queue.front();
        queue.pop();
        
        if (isAttacked(king_position, p.pos)) {
           return true;
        }

        if (getPlaySide(board[p.pos.x][p.pos.y]) != NONE)
            continue;

        row = p.pos.x + dx[p.dir];
        col = p.pos.y + dy[p.dir];
        
        if (!isPositionValid({row, col}))
            continue;

        PlaySide new_side = getPlaySide(board[row][col]);

        if (!vis[row][col] && new_side != sideToMove) {
            queue.push({{row, col}, p.dir});
            vis[row][col] = true;
        }
    }

    return false;
}

/**
 * Generate a nextMove that defends King from check.
 * @param board board configuration
 * @param sideToMove side to move
*/
void Bot::defendCheck(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide sideToMove) {
    /* generate all possible moves */
    std::vector<Move*> moves = generateAllMoves(board, sideToMove);
    int size = moves.size();

    /* no legal move found to get out of chess */
    if (size == 0)
        return;  /* should NEVER get here, xboard should stop the game if check mate */

    nextMove = Move::copyMove(moves[0]);

    for (int i = 0; i < size; i++)
        delete moves[i];
}

/**
 * Check if the move from src to dst, with given replacement piece replc, lands into check.
 * @param board board configuration
 * @param src source position
 * @param dst destination position
 * @param replc replacement piece (if move is a dropIn, EMPTY otherwise)
 * @return true if move lands into check, false otherwise
*/
bool Bot::landsInCheck(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], position src, position dst, BoardPiece replc) {
    Move *move = nullptr;
    if (replc == EMPTY)  /* no replacement piece, normal move */
        move = Move::moveTo(toString(src), toString(dst));
    else
        move = Move::dropIn(toString(dst), getPiece(replc));

    /* make the move, check if it lands in check, then undo the move */
    int captured = makeMove(move, board, botPlaySide);

    bool result = inCheck(board, botPlaySide);

    undoMove(move, board, captured, botPlaySide);

    delete move;

    return result;
}

/**
 * Generate a string representation of the given Move.
 * @param move move
 * @return a string representation of move in coordinate notation (e.g. e2e4, e7e8q) 
*/
std::string Bot::moveToString(Move* move) {
    if (move->isNormal())
        return move->getSource().value() + move->getDestination().value();

    if (move->isDropIn()) {
        std::string pieceCode = "";
        switch (move->getReplacement().value()) {
            case Piece::BISHOP: pieceCode = "B"; break;
            case Piece::KNIGHT: pieceCode = "N"; break;
            case Piece::ROOK: pieceCode = "R"; break;
            case Piece::QUEEN: pieceCode = "Q"; break;
            case Piece::PAWN: pieceCode = "P"; break;
            default: break;
        };
        return pieceCode + "@" + move->getDestination().value();
    }

    return "resign";
}

/**
 * Compare two positions.
 * @param p, q positions
 * @return true if positions' coordinates are equal, false otherwise
*/
bool Bot::positionEquals(position p, position q) {
    return (p.x == q.x) && (p.y == q.y);
}

/**
 * Rules for castling:
 * - your king and rook can NOT have moved, once your king or rook moves, you can no longer castle
 * - your king can NOT be in check
 * - your king can NOT pass through check - if any square the king moves over or moves onto
 *   would put you in check, you can't castle
 * - no pieces can be between the king and rook
 * @param board board configuration
 * @param playSide side to move
 * @param type type 0 -> Queen side castle, type 1 -> King side castle
 * @returns true if castling of given type is possible, false otherwise
*/
bool Bot::spaceForCastle(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide, int type) {
    int row = (playSide == PlaySide::WHITE) ? 1 : 8, start, end;

    if (type == 0) {
        start = 2;
        end = 4;
    } else {
        start = 6;
        end = 7;
    }

    for (int i = start; i <= end; i++) {
        if (board[row][i] != BoardPiece::EMPTY)
            return false;

        if (landsInCheck(board, getKingPosition(board, playSide), {row, i}, EMPTY))
            return false;
    }

    return true;
}

/**
 * Check if castling is possible, if so, choose to perform it.
 * @param board board configuration
 * @param playSide side to move
 * @returns true if castling has been performed, false otherwise
*/
bool Bot::castle(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide) {
    position king_position = getKingPosition(board, playSide);

    /* castle KING side */
    if (castlePossible[playSide][1] && !inCheck(board, playSide) && spaceForCastle(board, botPlaySide, 1)) {
        nextMove = Move::moveTo(toString(king_position), toString({king_position.x, king_position.y + 2}));
        castlePossible[playSide][0] = castlePossible[playSide][1] = false;

        return true;
    }

    /* castle QUEEN side */
    if (castlePossible[playSide][0] && !inCheck(board, playSide) && spaceForCastle(board, botPlaySide, 0)) {
        nextMove = Move::moveTo(toString(king_position), toString({king_position.x, king_position.y - 2}));
        castlePossible[playSide][0] = castlePossible[playSide][1] = false;

        return true;
    }

    return false;
}

/**
 * Compute the difference in points between bot and opponent pieces on the board.
 * @param board board configuration
 * @returns the difference of points between bot and opponent
*/
int Bot::getPiecePointsDiff(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1]) {
    int botPoints = 0, opponentPoints = 0;
    PlaySide playSide, opponentPlaySide = getOpponentPlaySide(botPlaySide);

    for (int x = 1; x <= BOARD_SIZE; x++) {
        for (int y = 1; y <= BOARD_SIZE; y++) {
            playSide = getPlaySide(board[x][y]);

            if (playSide == botPlaySide)
                botPoints += getPiecePoints(board[x][y]);
            else if (playSide == opponentPlaySide)
                opponentPoints += getPiecePoints(board[x][y]);
        }
    }

    return botPoints - opponentPoints;
}

/**
 * Evaluation function for minimax.
 * @param board board configuration
 * @returns the heuristic value of the board configuration
*/
int Bot::evaluate(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1]) {
    /* Currently: naive approach, calculate the difference between the number of points of the two sides */
    return getPiecePointsDiff(board);
}

/**
 * Push all valid moves (guaranteed not to lang in check) from src position to the moves vector.
 * @param board board configuration
 * @param src source position
 * @param moves vector containing all possible moves of piece at src
 * @param directions number of directions the piece at src can move to
 * @param dx x coordinate increments from src to all possible directions piece at src can move to
 * @param dy y coordinate increments from src to all possible directions piece ar src can move to
*/
void Bot::pushPieceMoves(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], position src, std::vector<Move*> &moves, \
                    int directions, const int dx[], const int dy[]) {
    int row, col, vis[BOARD_SIZE+1][BOARD_SIZE+1] = {false};
    std::queue<dir> queue;
    dir p;

    Piece piece = getPiece(board[src.x][src.y]);
    PlaySide playSide = getPlaySide(board[src.x][src.y]), newSide;

    vis[src.x][src.y] = true;

    for (int i = 0; i < directions; i++) {
        row = src.x + dx[i];
        col = src.y + dy[i];
        if (!isPositionValid({row, col}))
            continue;

        newSide = getPlaySide(board[row][col]);
        
        if (!vis[row][col] && newSide != playSide) {
            if (!landsInCheck(board, src, {row, col}, EMPTY))
                moves.push_back(Move::moveTo(toString(src), toString({row, col})));
            
            vis[row][col] = true;
            if (newSide == NONE)
                queue.push({{row, col}, i});
        }
    }

    /* knight and king can only move one step */
    if (piece == KNIGHT || piece == KING)
        return;

    /* use BFS to search all possible directions reachable by piece at src */
    while (!queue.empty()) {
        p = queue.front();
        queue.pop();

        row = p.pos.x + dx[p.dir];
        col = p.pos.y + dy[p.dir];
        if (!isPositionValid({row, col}))
            continue;

        newSide = getPlaySide(board[row][col]);

        if (!vis[row][col] && newSide != playSide) {
            if (!landsInCheck(board, src, {row, col}, EMPTY))
                moves.push_back(Move::moveTo(toString(src), toString({row, col})));

            vis[row][col] = true;
            if (newSide == NONE)
                queue.push({{row, col}, p.dir});
        }
    }
}

/**
 * Generate all possible moves of playSide.
 * @param board board configuration
 * @param playSide side to move
 * @returns a vector containing all possible moves of playSide, given the current board configuration
*/
std::vector<Move*> Bot::generateAllMoves(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide) {
    PlaySide opponentPlaySide = getOpponentPlaySide(playSide);
    std::vector<Move*> moves;

    /* White moves its pawns upward on the board, Black downward */
    int dir = (playSide == WHITE) ? 1 : -1;
    int start, stop, inc;
    if (playSide == WHITE) {
        start = 8;
        stop = 0;
        inc = -1;
    } else {
        start = 1;
        stop = 9;
        inc = 1;
    }

    /* generate drop-ins */
    for (int i = 0; i < 5; i++) {
        if (pool[playSide][i] > 0) {
            int xStart = 1, xEnd = 8;
            if (i == 0) {  /* Pawns cannot be placed on rows 1 and 8 */
                xStart = 2;
                xEnd = 7;
            }

            for (int x = xStart; x <= xEnd; x++) {
                for (int y = 1; y <= BOARD_SIZE; y++) {
                    if (board[x][y] == EMPTY && !landsInCheck(board, {0, 0}, {x, y}, BoardPiece(i + (playSide == BLACK) * 6 + 1)))
                        moves.push_back(Move::dropIn(toString({x, y}), Piece(i)));
                }
            }
        }
    }

    int promotionRow = (playSide == WHITE) ? 8 : 1;

    /* generate all possible moves for all pieces of playSide on the current board */
    for (int x = start; x != stop; x += inc) {
      for (int y = 1; y <= BOARD_SIZE; y++) {
        if (getPlaySide(board[x][y]) != playSide)
            continue;

        Piece piece = getPiece(board[x][y]);

        if (piece == PAWN) {
            /* check if pawn can capture neighboring pieces */
            if (isPositionValid({x+dir, y-1})) {
                if (getPlaySide(board[x+dir][y-1]) == opponentPlaySide && !landsInCheck(board, {x, y}, {x+dir, y-1}, EMPTY)) {
                    if (x+dir == promotionRow) {
                        moves.push_back(Move::promote(toString({x, y}), toString({x+dir, y-1}), QUEEN));
                    } else {
                        moves.push_back(Move::moveTo(toString({x, y}), toString({x+dir, y-1})));
                    }
                }

                /* check for en Passant rights */
                if (getPlaySide(board[x][y-1]) == opponentPlaySide && enPassantRights(board, {x, y}, {x+dir, y-1}))
                    moves.push_back(Move::moveTo(toString({x, y}), toString({x+dir, y-1})));
            }

            if (isPositionValid({x+dir, y+1})) {
                if (getPlaySide(board[x+dir][y+1]) == opponentPlaySide && !landsInCheck(board, {x, y}, {x+dir, y+1}, EMPTY)) {
                    if (x+dir == promotionRow) {
                        moves.push_back(Move::promote(toString({x, y}), toString({x+dir, y+1}), QUEEN));
                    } else {
                        moves.push_back(Move::moveTo(toString({x, y}), toString({x+dir, y+1})));
                    }
                }

                /* check for en Passant rights */
                if (getPlaySide(board[x][y+1]) == opponentPlaySide && enPassantRights(board, {x, y}, {x+dir, y+1}))
                    moves.push_back(Move::moveTo(toString({x, y}), toString({x+dir, y+1})));
            }

       
            /* check if pawn can move one square */
            if (isPositionValid({x+dir, y}) && board[x+dir][y] == EMPTY) {
                if (!landsInCheck(board, {x, y}, {x+dir, y}, EMPTY)) {
                    if (x+dir == promotionRow) {
                        moves.push_back(Move::promote(toString({x, y}), toString({x+dir, y}), QUEEN));
                    } else {
                        moves.push_back(Move::moveTo(toString({x, y}), toString({x+dir, y})));
                    }
                }
            }

            /* check if pawn can move two squares */
            if ((playSide == WHITE && x == 2) || (playSide == BLACK && x == 7)) {
                if (isPositionValid({x+2*dir, y}) && board[x+2*dir][y] == EMPTY && board[x+dir][y] == EMPTY)
                    if (!landsInCheck(board, {x, y}, {x+2*dir, y}, EMPTY))
                        moves.push_back(Move::moveTo(toString({x,y}), toString({x+2*dir, y})));
            }

        } else if (piece == KNIGHT) {
            pushPieceMoves(board, {x, y}, moves, KNIGHT_DIRECTIONS, knight_dx, knight_dy);

        } else if (piece == ROOK) {
            pushPieceMoves(board, {x, y}, moves, ROOK_DIRECTIONS, rook_dx, rook_dy);

        } else if (piece == BISHOP) {
            pushPieceMoves(board, {x, y}, moves, BISHOP_DIRECTIONS, bishop_dx, bishop_dy);

        } else if (piece == QUEEN) {
            pushPieceMoves(board, {x, y}, moves, DIRECTIONS, dx, dy);

        } else if (piece == KING) {
            pushPieceMoves(board, {x, y}, moves, DIRECTIONS, dx, dy);
        }
      }
    }

    return moves;
}

/**
 * Check if en Passant is possible.
 * @param board board configuration
 * @param src source position
 * @param dst destination position
 * @returns true if en Passant is possible, false otherwise
*/
bool Bot::enPassantRights(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], position src, position dst) {
    position lastSrc = getMovePosition(lastRecordedMove->getSource());
    position lastDst = getMovePosition(lastRecordedMove->getDestination());
    Piece pieceMoved = getPiece(board[lastDst.x][lastDst.y]);

    if (positionEquals(lastDst, {src.x, dst.y}) && abs(lastDst.x - lastSrc.x) == 2 && pieceMoved == PAWN && !landsInCheck(board, src, dst, EMPTY))
        return true;

    
    return false;
}

/**
 * Use minimax algorithm to find the best move according to the board evaluation heuristics.
 * Save found move to nextMove.
 * @param board board configuration
 * @param depth current search depth
 * @returns best possible heuristic score
*/
int Bot::minimax(int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], int depth) {
    if (inCheck(board, botPlaySide))  /* bot is in check */
        return -1000;

    if (inCheck(board, getOpponentPlaySide(botPlaySide)))  /* opponent is in check */
        return 1000;

    if (depth == MAX_DEPTH - 1)  /* depth-limited minimax */
        return evaluate(board);

    int score = 0;

    if (depth % 2 == 0) { /* maxPlayer, the bot's turn to move */
        /* generate all possible moves */
        std::vector<Move*> moves = generateAllMoves(board, botPlaySide);
        int maxScore = -INF, size = moves.size();

        /* evaluate the board for each possible move and choose to perform
        * the move with the highest possible score */
        for (int i = 0; i < size; i++) {
            Move *currentMove = Move::copyMove(moves[i]);
            delete moves[i];

            /* perform move */
            int captured = makeMove(currentMove, board, botPlaySide);

            score = minimax(board, depth+1);

            if (score > maxScore) {
                maxScore = score;
                if (depth == 0) {
                    nextMove = Move::copyMove(currentMove);
                }
            } 
            
            /* undo move */
            undoMove(currentMove, board, captured, botPlaySide);

            delete currentMove;
        }

        return maxScore;
    } else {  /* minPlayer, the opponent's turn to move */
        std::vector<Move*> moves = generateAllMoves(board, getOpponentPlaySide(botPlaySide));
        int minScore = INF, size = moves.size();

        /* evaluate the board for each possible move and choose to perform
        * the move with the lowest possible score */
        for (int i = 0; i < size; i++) {
            Move *currentMove = Move::copyMove(moves[i]);
            delete moves[i];

            /* perform move */
            int captured = makeMove(currentMove, board, getOpponentPlaySide(botPlaySide));

            score = minimax(board, depth+1);

            if (score < minScore)
                minScore = score;
            
            /* undo move */
            undoMove(currentMove, board, captured, getOpponentPlaySide(botPlaySide));
            delete currentMove;
        }

        return minScore;
    }
}

/**
 * Perform move and record changes on board.
 * @param move move
 * @param board board configuration
 * @param playSide side to move
 * @returns value of the captured piece, 0 if no piece is captured
*/
int Bot::makeMove(Move *move, int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], PlaySide playSide) {
    int captured = 0;
    position src, dst = getMovePosition(move->getDestination());

    if (move->isNormal()) {
        src = getMovePosition(move->getSource());
        captured = board[dst.x][dst.y];
        Piece pieceToMove = getPiece(board[src.x][src.y]);
        
        if (board[dst.x][dst.y] != EMPTY) {/* capture piece */
            Piece capturedPiece = board[dst.x][dst.y] < 0 ? PAWN : getPiece(board[dst.x][dst.y]);
            pool[playSide][capturedPiece]++;
        } else if (pieceToMove == PAWN && src.y != dst.y) { /* en passant */
            pool[playSide][PAWN]++;
            board[src.x][dst.y] = EMPTY;
        }

        board[dst.x][dst.y] = board[src.x][src.y];
        board[src.x][src.y] = EMPTY;    
    } else if (move->isDropIn()) {
        Piece piece = move->getReplacement().value();
        board[dst.x][dst.y] = getBoardPiece(piece, playSide);
        pool[playSide][piece]--;
    } else if (move->isPromotion()) {
        src = getMovePosition(move->getSource());
        Piece piece = move->getReplacement().value();

        if (board[dst.x][dst.y] != EMPTY) {
            captured = board[dst.x][dst.y];
            /* promoted piece turns into PAWN */
            Piece capturedPiece = board[dst.x][dst.y] < 0 ? PAWN : getPiece(board[dst.x][dst.y]);
            pool[playSide][capturedPiece]++;
        }

        board[src.x][src.y] = EMPTY;
        board[dst.x][dst.y] = getBoardPiece(piece, playSide);
    }

    return captured;
}

/**
 * Return board to the previous configuration, before move was performed.
 * @param move move
 * @param board board configuration
 * @param captured value of the captured piece, 0 if no piece was captured
 * @param playSide side to move
*/
void Bot::undoMove(Move *move, int (&board)[BOARD_SIZE+1][BOARD_SIZE+1], int captured, PlaySide playSide) {   
    position src, dst = getMovePosition(move->getDestination());
    
    if (move->isNormal()) {
        src = getMovePosition(move->getSource());
        Piece pieceToMove = getPiece(board[dst.x][dst.y]);

        if (captured != 0) {  /* capture piece */
            if (captured < 0)
                pool[playSide][PAWN]--;
            else
                pool[playSide][getPiece(captured)]--;
        } else if (pieceToMove == PAWN && src.y != dst.y) { /* en Passant */
            pool[playSide][PAWN]--;
            board[src.x][dst.y] = getBoardPiece(PAWN, getOpponentPlaySide(playSide));
        }

        board[src.x][src.y] = board[dst.x][dst.y];
        board[dst.x][dst.y] = captured;

    } else if (move->isDropIn()) {
        Piece piece = move->getReplacement().value();
        board[dst.x][dst.y] = EMPTY;
        pool[playSide][piece]++;

    } else if (move->isPromotion()) {
        src = getMovePosition(move->getSource());
        if (captured != 0) {
            if (captured < 0)
                pool[playSide][PAWN]--;
            else
                pool[playSide][getPiece(captured)]--;
        }

        board[src.x][src.y] = getBoardPiece(PAWN, playSide);
        board[dst.x][dst.y] = captured;
    }
}
