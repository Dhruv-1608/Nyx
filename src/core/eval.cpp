#include "eval.h"
#include "board.h"
#include "types.h"
#include "movegen.h"

namespace {
    constexpr int16_t MG_PAWN_PST[64] = {
         0,   0,   0,   0,   0,   0,   0,   0,
        50,  50,  50,  50,  50,  50,  50,  50,
        10,  10,  20,  30,  30,  20,  10,  10,
         5,   5,  10,  25,  25,  10,   5,   5,
         0,   0,   0,  20,  20,   0,   0,   0,
         5,  -5, -10,   0,   0, -10,  -5,   5,
         5,  10,  10, -20, -20,  10,  10,   5,
         0,   0,   0,   0,   0,   0,   0,   0
    };

    constexpr int16_t MG_KNIGHT_PST[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   5,   5,   0, -20, -40,
        -30,   5,  10,  15,  15,  10,   5, -30,
        -30,   0,  15,  20,  20,  15,   0, -30,
        -30,   5,  15,  20,  20,  15,   5, -30,
        -30,   0,  10,  15,  15,  10,   0, -30,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    };

    constexpr int16_t MG_BISHOP_PST[64] = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   5,   0,   0,   0,   0,   5, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   5,   5,  10,  10,   5,   5, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    constexpr int16_t MG_ROOK_PST[64] = {
         0,   0,   0,   5,   5,   0,   0,   0,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
         5,  10,  10,  10,  10,  10,  10,   5,
         0,   0,   0,   0,   0,   0,   0,   0
    };

    constexpr int16_t MG_QUEEN_PST[64] = {
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,   5,   5,   5,   0, -10,
         -5,   0,   5,   5,   5,   5,   0,  -5,
          0,   0,   5,   5,   5,   5,   0,  -5,
        -10,   5,   5,   5,   5,   5,   0, -10,
        -10,   0,   5,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20
    };

    constexpr int16_t MG_KING_PST[64] = {
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -10, -20, -20, -20, -20, -20, -20, -10,
         20,  20,   0,   0,   0,   0,  20,  20,
         20,  30,  10,   0,   0,  10,  30,  20
    };

   constexpr int16_t EG_KING_PST[64] = {
        -50, -40, -30, -20, -20, -30, -40, -50,
        -30, -20, -10,   0,   0, -10, -20, -30,
        -30, -10,  20,  30,  30,  20, -10, -30,
        -20,   0,  30,  40,  40,  30,   0, -20,
        -20,   0,  30,  40,  40,  30,   0, -20,
        -30, -10,  20,  30,  30,  20, -10, -30,
        -30, -20, -10,   0,   0, -10, -20, -30,
        -50, -40, -30, -20, -20, -30, -40, -50
    };

   constexpr int16_t EG_PAWN_PST[64] = {
         0,   0,   0,   0,   0,   0,   0,   0,
        50,  50,  50,  50,  50,  50,  50,  50,
        10,  10,  20,  30,  30,  20,  10,  10,
         5,   5,  10,  25,  25,  10,   5,   5,
         0,   0,   0,  20,  20,   0,   0,   0,
         5,  -5, -10,   0,   0, -10,  -5,   5,
         5,  10,  10, -20, -20,  10,  10,   5,
         0,   0,   0,   0,   0,   0,   0,   0
    };

    constexpr int16_t EG_KNIGHT_PST[64] = {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   5,   5,   0, -20, -40,
        -30,   5,  10,  15,  15,  10,   5, -30,
        -30,   0,  15,  20,  20,  15,   0, -30,
        -30,   5,  15,  20,  20,  15,   5, -30,
        -30,   0,  10,  15,  15,  10,   0, -30,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    };

    constexpr int16_t EG_BISHOP_PST[64] = {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   5,   0,   0,   0,   0,   5, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   5,   5,  10,  10,   5,   5, -10,
        -10,   0,   5,  10,  10,   5,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    };

    constexpr int16_t EG_ROOK_PST[64] = {
         0,   0,   0,   5,   5,   0,   0,   0,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
        -5,   0,   0,   0,   0,   0,   0,  -5,
         5,  10,  10,  10,  10,  10,  10,   5,
         0,   0,   0,   0,   0,   0,   0,   0
    };

    constexpr int16_t EG_QUEEN_PST[64] = {
        -20, -10, -10,  -5,  -5, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   5,   5,   5,   5,   0, -10,
         -5,   0,   5,   5,   5,   5,   0,  -5,
          0,   0,   5,   5,   5,   5,   0,  -5,
        -10,   5,   5,   5,   5,   5,   0, -10,
        -10,   0,   5,   0,   0,   0,   0, -10,
        -20, -10, -10,  -5,  -5, -10, -10, -20
    };
}

// Define static members
const int16_t Evaluator::MG_PST[NUM_PIECES][NUM_SQUARES] = {
    { MG_PAWN_PST[0], MG_PAWN_PST[1], MG_PAWN_PST[2], MG_PAWN_PST[3], MG_PAWN_PST[4], MG_PAWN_PST[5], MG_PAWN_PST[6], MG_PAWN_PST[7],
      MG_PAWN_PST[8], MG_PAWN_PST[9], MG_PAWN_PST[10], MG_PAWN_PST[11], MG_PAWN_PST[12], MG_PAWN_PST[13], MG_PAWN_PST[14], MG_PAWN_PST[15],
      MG_PAWN_PST[16], MG_PAWN_PST[17], MG_PAWN_PST[18], MG_PAWN_PST[19], MG_PAWN_PST[20], MG_PAWN_PST[21], MG_PAWN_PST[22], MG_PAWN_PST[23],
      MG_PAWN_PST[24], MG_PAWN_PST[25], MG_PAWN_PST[26], MG_PAWN_PST[27], MG_PAWN_PST[28], MG_PAWN_PST[29], MG_PAWN_PST[30], MG_PAWN_PST[31],
      MG_PAWN_PST[32], MG_PAWN_PST[33], MG_PAWN_PST[34], MG_PAWN_PST[35], MG_PAWN_PST[36], MG_PAWN_PST[37], MG_PAWN_PST[38], MG_PAWN_PST[39],
      MG_PAWN_PST[40], MG_PAWN_PST[41], MG_PAWN_PST[42], MG_PAWN_PST[43], MG_PAWN_PST[44], MG_PAWN_PST[45], MG_PAWN_PST[46], MG_PAWN_PST[47],
      MG_PAWN_PST[48], MG_PAWN_PST[49], MG_PAWN_PST[50], MG_PAWN_PST[51], MG_PAWN_PST[52], MG_PAWN_PST[53], MG_PAWN_PST[54], MG_PAWN_PST[55],
      MG_PAWN_PST[56], MG_PAWN_PST[57], MG_PAWN_PST[58], MG_PAWN_PST[59], MG_PAWN_PST[60], MG_PAWN_PST[61], MG_PAWN_PST[62], MG_PAWN_PST[63] },
    { MG_KNIGHT_PST[0], MG_KNIGHT_PST[1], MG_KNIGHT_PST[2], MG_KNIGHT_PST[3], MG_KNIGHT_PST[4], MG_KNIGHT_PST[5], MG_KNIGHT_PST[6], MG_KNIGHT_PST[7],
      MG_KNIGHT_PST[8], MG_KNIGHT_PST[9], MG_KNIGHT_PST[10], MG_KNIGHT_PST[11], MG_KNIGHT_PST[12], MG_KNIGHT_PST[13], MG_KNIGHT_PST[14], MG_KNIGHT_PST[15],
      MG_KNIGHT_PST[16], MG_KNIGHT_PST[17], MG_KNIGHT_PST[18], MG_KNIGHT_PST[19], MG_KNIGHT_PST[20], MG_KNIGHT_PST[21], MG_KNIGHT_PST[22], MG_KNIGHT_PST[23],
      MG_KNIGHT_PST[24], MG_KNIGHT_PST[25], MG_KNIGHT_PST[26], MG_KNIGHT_PST[27], MG_KNIGHT_PST[28], MG_KNIGHT_PST[29], MG_KNIGHT_PST[30], MG_KNIGHT_PST[31],
      MG_KNIGHT_PST[32], MG_KNIGHT_PST[33], MG_KNIGHT_PST[34], MG_KNIGHT_PST[35], MG_KNIGHT_PST[36], MG_KNIGHT_PST[37], MG_KNIGHT_PST[38], MG_KNIGHT_PST[39],
      MG_KNIGHT_PST[40], MG_KNIGHT_PST[41], MG_KNIGHT_PST[42], MG_KNIGHT_PST[43], MG_KNIGHT_PST[44], MG_KNIGHT_PST[45], MG_KNIGHT_PST[46], MG_KNIGHT_PST[47],
      MG_KNIGHT_PST[48], MG_KNIGHT_PST[49], MG_KNIGHT_PST[50], MG_KNIGHT_PST[51], MG_KNIGHT_PST[52], MG_KNIGHT_PST[53], MG_KNIGHT_PST[54], MG_KNIGHT_PST[55],
      MG_KNIGHT_PST[56], MG_KNIGHT_PST[57], MG_KNIGHT_PST[58], MG_KNIGHT_PST[59], MG_KNIGHT_PST[60], MG_KNIGHT_PST[61], MG_KNIGHT_PST[62], MG_KNIGHT_PST[63] },
    { MG_BISHOP_PST[0], MG_BISHOP_PST[1], MG_BISHOP_PST[2], MG_BISHOP_PST[3], MG_BISHOP_PST[4], MG_BISHOP_PST[5], MG_BISHOP_PST[6], MG_BISHOP_PST[7],
      MG_BISHOP_PST[8], MG_BISHOP_PST[9], MG_BISHOP_PST[10], MG_BISHOP_PST[11], MG_BISHOP_PST[12], MG_BISHOP_PST[13], MG_BISHOP_PST[14], MG_BISHOP_PST[15],
      MG_BISHOP_PST[16], MG_BISHOP_PST[17], MG_BISHOP_PST[18], MG_BISHOP_PST[19], MG_BISHOP_PST[20], MG_BISHOP_PST[21], MG_BISHOP_PST[22], MG_BISHOP_PST[23],
      MG_BISHOP_PST[24], MG_BISHOP_PST[25], MG_BISHOP_PST[26], MG_BISHOP_PST[27], MG_BISHOP_PST[28], MG_BISHOP_PST[29], MG_BISHOP_PST[30], MG_BISHOP_PST[31],
      MG_BISHOP_PST[32], MG_BISHOP_PST[33], MG_BISHOP_PST[34], MG_BISHOP_PST[35], MG_BISHOP_PST[36], MG_BISHOP_PST[37], MG_BISHOP_PST[38], MG_BISHOP_PST[39],
      MG_BISHOP_PST[40], MG_BISHOP_PST[41], MG_BISHOP_PST[42], MG_BISHOP_PST[43], MG_BISHOP_PST[44], MG_BISHOP_PST[45], MG_BISHOP_PST[46], MG_BISHOP_PST[47],
      MG_BISHOP_PST[48], MG_BISHOP_PST[49], MG_BISHOP_PST[50], MG_BISHOP_PST[51], MG_BISHOP_PST[52], MG_BISHOP_PST[53], MG_BISHOP_PST[54], MG_BISHOP_PST[55],
      MG_BISHOP_PST[56], MG_BISHOP_PST[57], MG_BISHOP_PST[58], MG_BISHOP_PST[59], MG_BISHOP_PST[60], MG_BISHOP_PST[61], MG_BISHOP_PST[62], MG_BISHOP_PST[63] },
    { MG_ROOK_PST[0], MG_ROOK_PST[1], MG_ROOK_PST[2], MG_ROOK_PST[3], MG_ROOK_PST[4], MG_ROOK_PST[5], MG_ROOK_PST[6], MG_ROOK_PST[7],
      MG_ROOK_PST[8], MG_ROOK_PST[9], MG_ROOK_PST[10], MG_ROOK_PST[11], MG_ROOK_PST[12], MG_ROOK_PST[13], MG_ROOK_PST[14], MG_ROOK_PST[15],
      MG_ROOK_PST[16], MG_ROOK_PST[17], MG_ROOK_PST[18], MG_ROOK_PST[19], MG_ROOK_PST[20], MG_ROOK_PST[21], MG_ROOK_PST[22], MG_ROOK_PST[23],
      MG_ROOK_PST[24], MG_ROOK_PST[25], MG_ROOK_PST[26], MG_ROOK_PST[27], MG_ROOK_PST[28], MG_ROOK_PST[29], MG_ROOK_PST[30], MG_ROOK_PST[31],
      MG_ROOK_PST[32], MG_ROOK_PST[33], MG_ROOK_PST[34], MG_ROOK_PST[35], MG_ROOK_PST[36], MG_ROOK_PST[37], MG_ROOK_PST[38], MG_ROOK_PST[39],
      MG_ROOK_PST[40], MG_ROOK_PST[41], MG_ROOK_PST[42], MG_ROOK_PST[43], MG_ROOK_PST[44], MG_ROOK_PST[45], MG_ROOK_PST[46], MG_ROOK_PST[47],
      MG_ROOK_PST[48], MG_ROOK_PST[49], MG_ROOK_PST[50], MG_ROOK_PST[51], MG_ROOK_PST[52], MG_ROOK_PST[53], MG_ROOK_PST[54], MG_ROOK_PST[55],
      MG_ROOK_PST[56], MG_ROOK_PST[57], MG_ROOK_PST[58], MG_ROOK_PST[59], MG_ROOK_PST[60], MG_ROOK_PST[61], MG_ROOK_PST[62], MG_ROOK_PST[63] },
    { MG_QUEEN_PST[0], MG_QUEEN_PST[1], MG_QUEEN_PST[2], MG_QUEEN_PST[3], MG_QUEEN_PST[4], MG_QUEEN_PST[5], MG_QUEEN_PST[6], MG_QUEEN_PST[7],
      MG_QUEEN_PST[8], MG_QUEEN_PST[9], MG_QUEEN_PST[10], MG_QUEEN_PST[11], MG_QUEEN_PST[12], MG_QUEEN_PST[13], MG_QUEEN_PST[14], MG_QUEEN_PST[15],
      MG_QUEEN_PST[16], MG_QUEEN_PST[17], MG_QUEEN_PST[18], MG_QUEEN_PST[19], MG_QUEEN_PST[20], MG_QUEEN_PST[21], MG_QUEEN_PST[22], MG_QUEEN_PST[23],
      MG_QUEEN_PST[24], MG_QUEEN_PST[25], MG_QUEEN_PST[26], MG_QUEEN_PST[27], MG_QUEEN_PST[28], MG_QUEEN_PST[29], MG_QUEEN_PST[30], MG_QUEEN_PST[31],
      MG_QUEEN_PST[32], MG_QUEEN_PST[33], MG_QUEEN_PST[34], MG_QUEEN_PST[35], MG_QUEEN_PST[36], MG_QUEEN_PST[37], MG_QUEEN_PST[38], MG_QUEEN_PST[39],
      MG_QUEEN_PST[40], MG_QUEEN_PST[41], MG_QUEEN_PST[42], MG_QUEEN_PST[43], MG_QUEEN_PST[44], MG_QUEEN_PST[45], MG_QUEEN_PST[46], MG_QUEEN_PST[47],
      MG_QUEEN_PST[48], MG_QUEEN_PST[49], MG_QUEEN_PST[50], MG_QUEEN_PST[51], MG_QUEEN_PST[52], MG_QUEEN_PST[53], MG_QUEEN_PST[54], MG_QUEEN_PST[55],
      MG_QUEEN_PST[56], MG_QUEEN_PST[57], MG_QUEEN_PST[58], MG_QUEEN_PST[59], MG_QUEEN_PST[60], MG_QUEEN_PST[61], MG_QUEEN_PST[62], MG_QUEEN_PST[63] },
    { MG_KING_PST[0], MG_KING_PST[1], MG_KING_PST[2], MG_KING_PST[3], MG_KING_PST[4], MG_KING_PST[5], MG_KING_PST[6], MG_KING_PST[7],
      MG_KING_PST[8], MG_KING_PST[9], MG_KING_PST[10], MG_KING_PST[11], MG_KING_PST[12], MG_KING_PST[13], MG_KING_PST[14], MG_KING_PST[15],
      MG_KING_PST[16], MG_KING_PST[17], MG_KING_PST[18], MG_KING_PST[19], MG_KING_PST[20], MG_KING_PST[21], MG_KING_PST[22], MG_KING_PST[23],
      MG_KING_PST[24], MG_KING_PST[25], MG_KING_PST[26], MG_KING_PST[27], MG_KING_PST[28], MG_KING_PST[29], MG_KING_PST[30], MG_KING_PST[31],
      MG_KING_PST[32], MG_KING_PST[33], MG_KING_PST[34], MG_KING_PST[35], MG_KING_PST[36], MG_KING_PST[37], MG_KING_PST[38], MG_KING_PST[39],
      MG_KING_PST[40], MG_KING_PST[41], MG_KING_PST[42], MG_KING_PST[43], MG_KING_PST[44], MG_KING_PST[45], MG_KING_PST[46], MG_KING_PST[47],
      MG_KING_PST[48], MG_KING_PST[49], MG_KING_PST[50], MG_KING_PST[51], MG_KING_PST[52], MG_KING_PST[53], MG_KING_PST[54], MG_KING_PST[55],
      MG_KING_PST[56], MG_KING_PST[57], MG_KING_PST[58], MG_KING_PST[59], MG_KING_PST[60], MG_KING_PST[61], MG_KING_PST[62], MG_KING_PST[63] }
};

const int16_t Evaluator::EG_PST[NUM_PIECES][NUM_SQUARES] = {
    { EG_PAWN_PST[0], EG_PAWN_PST[1], EG_PAWN_PST[2], EG_PAWN_PST[3], EG_PAWN_PST[4], EG_PAWN_PST[5], EG_PAWN_PST[6], EG_PAWN_PST[7],
      EG_PAWN_PST[8], EG_PAWN_PST[9], EG_PAWN_PST[10], EG_PAWN_PST[11], EG_PAWN_PST[12], EG_PAWN_PST[13], EG_PAWN_PST[14], EG_PAWN_PST[15],
      EG_PAWN_PST[16], EG_PAWN_PST[17], EG_PAWN_PST[18], EG_PAWN_PST[19], EG_PAWN_PST[20], EG_PAWN_PST[21], EG_PAWN_PST[22], EG_PAWN_PST[23],
      EG_PAWN_PST[24], EG_PAWN_PST[25], EG_PAWN_PST[26], EG_PAWN_PST[27], EG_PAWN_PST[28], EG_PAWN_PST[29], EG_PAWN_PST[30], EG_PAWN_PST[31],
      EG_PAWN_PST[32], EG_PAWN_PST[33], EG_PAWN_PST[34], EG_PAWN_PST[35], EG_PAWN_PST[36], EG_PAWN_PST[37], EG_PAWN_PST[38], EG_PAWN_PST[39],
      EG_PAWN_PST[40], EG_PAWN_PST[41], EG_PAWN_PST[42], EG_PAWN_PST[43], EG_PAWN_PST[44], EG_PAWN_PST[45], EG_PAWN_PST[46], EG_PAWN_PST[47],
      EG_PAWN_PST[48], EG_PAWN_PST[49], EG_PAWN_PST[50], EG_PAWN_PST[51], EG_PAWN_PST[52], EG_PAWN_PST[53], EG_PAWN_PST[54], EG_PAWN_PST[55],
      EG_PAWN_PST[56], EG_PAWN_PST[57], EG_PAWN_PST[58], EG_PAWN_PST[59], EG_PAWN_PST[60], EG_PAWN_PST[61], EG_PAWN_PST[62], EG_PAWN_PST[63] },
    { EG_KNIGHT_PST[0], EG_KNIGHT_PST[1], EG_KNIGHT_PST[2], EG_KNIGHT_PST[3], EG_KNIGHT_PST[4], EG_KNIGHT_PST[5], EG_KNIGHT_PST[6], EG_KNIGHT_PST[7],
      EG_KNIGHT_PST[8], EG_KNIGHT_PST[9], EG_KNIGHT_PST[10], EG_KNIGHT_PST[11], EG_KNIGHT_PST[12], EG_KNIGHT_PST[13], EG_KNIGHT_PST[14], EG_KNIGHT_PST[15],
      EG_KNIGHT_PST[16], EG_KNIGHT_PST[17], EG_KNIGHT_PST[18], EG_KNIGHT_PST[19], EG_KNIGHT_PST[20], EG_KNIGHT_PST[21], EG_KNIGHT_PST[22], EG_KNIGHT_PST[23],
      EG_KNIGHT_PST[24], EG_KNIGHT_PST[25], EG_KNIGHT_PST[26], EG_KNIGHT_PST[27], EG_KNIGHT_PST[28], EG_KNIGHT_PST[29], EG_KNIGHT_PST[30], EG_KNIGHT_PST[31],
      EG_KNIGHT_PST[32], EG_KNIGHT_PST[33], EG_KNIGHT_PST[34], EG_KNIGHT_PST[35], EG_KNIGHT_PST[36], EG_KNIGHT_PST[37], EG_KNIGHT_PST[38], EG_KNIGHT_PST[39],
      EG_KNIGHT_PST[40], EG_KNIGHT_PST[41], EG_KNIGHT_PST[42], EG_KNIGHT_PST[43], EG_KNIGHT_PST[44], EG_KNIGHT_PST[45], EG_KNIGHT_PST[46], EG_KNIGHT_PST[47],
      EG_KNIGHT_PST[48], EG_KNIGHT_PST[49], EG_KNIGHT_PST[50], EG_KNIGHT_PST[51], EG_KNIGHT_PST[52], EG_KNIGHT_PST[53], EG_KNIGHT_PST[54], EG_KNIGHT_PST[55],
      EG_KNIGHT_PST[56], EG_KNIGHT_PST[57], EG_KNIGHT_PST[58], EG_KNIGHT_PST[59], EG_KNIGHT_PST[60], EG_KNIGHT_PST[61], EG_KNIGHT_PST[62], EG_KNIGHT_PST[63] },
    { EG_BISHOP_PST[0], EG_BISHOP_PST[1], EG_BISHOP_PST[2], EG_BISHOP_PST[3], EG_BISHOP_PST[4], EG_BISHOP_PST[5], EG_BISHOP_PST[6], EG_BISHOP_PST[7],
      EG_BISHOP_PST[8], EG_BISHOP_PST[9], EG_BISHOP_PST[10], EG_BISHOP_PST[11], EG_BISHOP_PST[12], EG_BISHOP_PST[13], EG_BISHOP_PST[14], EG_BISHOP_PST[15],
      EG_BISHOP_PST[16], EG_BISHOP_PST[17], EG_BISHOP_PST[18], EG_BISHOP_PST[19], EG_BISHOP_PST[20], EG_BISHOP_PST[21], EG_BISHOP_PST[22], EG_BISHOP_PST[23],
      EG_BISHOP_PST[24], EG_BISHOP_PST[25], EG_BISHOP_PST[26], EG_BISHOP_PST[27], EG_BISHOP_PST[28], EG_BISHOP_PST[29], EG_BISHOP_PST[30], EG_BISHOP_PST[31],
      EG_BISHOP_PST[32], EG_BISHOP_PST[33], EG_BISHOP_PST[34], EG_BISHOP_PST[35], EG_BISHOP_PST[36], EG_BISHOP_PST[37], EG_BISHOP_PST[38], EG_BISHOP_PST[39],
      EG_BISHOP_PST[40], EG_BISHOP_PST[41], EG_BISHOP_PST[42], EG_BISHOP_PST[43], EG_BISHOP_PST[44], EG_BISHOP_PST[45], EG_BISHOP_PST[46], EG_BISHOP_PST[47],
      EG_BISHOP_PST[48], EG_BISHOP_PST[49], EG_BISHOP_PST[50], EG_BISHOP_PST[51], EG_BISHOP_PST[52], EG_BISHOP_PST[53], EG_BISHOP_PST[54], EG_BISHOP_PST[55],
      EG_BISHOP_PST[56], EG_BISHOP_PST[57], EG_BISHOP_PST[58], EG_BISHOP_PST[59], EG_BISHOP_PST[60], EG_BISHOP_PST[61], EG_BISHOP_PST[62], EG_BISHOP_PST[63] },
    { EG_ROOK_PST[0], EG_ROOK_PST[1], EG_ROOK_PST[2], EG_ROOK_PST[3], EG_ROOK_PST[4], EG_ROOK_PST[5], EG_ROOK_PST[6], EG_ROOK_PST[7],
      EG_ROOK_PST[8], EG_ROOK_PST[9], EG_ROOK_PST[10], EG_ROOK_PST[11], EG_ROOK_PST[12], EG_ROOK_PST[13], EG_ROOK_PST[14], EG_ROOK_PST[15],
      EG_ROOK_PST[16], EG_ROOK_PST[17], EG_ROOK_PST[18], EG_ROOK_PST[19], EG_ROOK_PST[20], EG_ROOK_PST[21], EG_ROOK_PST[22], EG_ROOK_PST[23],
      EG_ROOK_PST[24], EG_ROOK_PST[25], EG_ROOK_PST[26], EG_ROOK_PST[27], EG_ROOK_PST[28], EG_ROOK_PST[29], EG_ROOK_PST[30], EG_ROOK_PST[31],
      EG_ROOK_PST[32], EG_ROOK_PST[33], EG_ROOK_PST[34], EG_ROOK_PST[35], EG_ROOK_PST[36], EG_ROOK_PST[37], EG_ROOK_PST[38], EG_ROOK_PST[39],
      EG_ROOK_PST[40], EG_ROOK_PST[41], EG_ROOK_PST[42], EG_ROOK_PST[43], EG_ROOK_PST[44], EG_ROOK_PST[45], EG_ROOK_PST[46], EG_ROOK_PST[47],
      EG_ROOK_PST[48], EG_ROOK_PST[49], EG_ROOK_PST[50], EG_ROOK_PST[51], EG_ROOK_PST[52], EG_ROOK_PST[53], EG_ROOK_PST[54], EG_ROOK_PST[55],
      EG_ROOK_PST[56], EG_ROOK_PST[57], EG_ROOK_PST[58], EG_ROOK_PST[59], EG_ROOK_PST[60], EG_ROOK_PST[61], EG_ROOK_PST[62], EG_ROOK_PST[63] },
    { EG_QUEEN_PST[0], EG_QUEEN_PST[1], EG_QUEEN_PST[2], EG_QUEEN_PST[3], EG_QUEEN_PST[4], EG_QUEEN_PST[5], EG_QUEEN_PST[6], EG_QUEEN_PST[7],
      EG_QUEEN_PST[8], EG_QUEEN_PST[9], EG_QUEEN_PST[10], EG_QUEEN_PST[11], EG_QUEEN_PST[12], EG_QUEEN_PST[13], EG_QUEEN_PST[14], EG_QUEEN_PST[15],
      EG_QUEEN_PST[16], EG_QUEEN_PST[17], EG_QUEEN_PST[18], EG_QUEEN_PST[19], EG_QUEEN_PST[20], EG_QUEEN_PST[21], EG_QUEEN_PST[22], EG_QUEEN_PST[23],
      EG_QUEEN_PST[24], EG_QUEEN_PST[25], EG_QUEEN_PST[26], EG_QUEEN_PST[27], EG_QUEEN_PST[28], EG_QUEEN_PST[29], EG_QUEEN_PST[30], EG_QUEEN_PST[31],
      EG_QUEEN_PST[32], EG_QUEEN_PST[33], EG_QUEEN_PST[34], EG_QUEEN_PST[35], EG_QUEEN_PST[36], EG_QUEEN_PST[37], EG_QUEEN_PST[38], EG_QUEEN_PST[39],
      EG_QUEEN_PST[40], EG_QUEEN_PST[41], EG_QUEEN_PST[42], EG_QUEEN_PST[43], EG_QUEEN_PST[44], EG_QUEEN_PST[45], EG_QUEEN_PST[46], EG_QUEEN_PST[47],
      EG_QUEEN_PST[48], EG_QUEEN_PST[49], EG_QUEEN_PST[50], EG_QUEEN_PST[51], EG_QUEEN_PST[52], EG_QUEEN_PST[53], EG_QUEEN_PST[54], EG_QUEEN_PST[55],
      EG_QUEEN_PST[56], EG_QUEEN_PST[57], EG_QUEEN_PST[58], EG_QUEEN_PST[59], EG_QUEEN_PST[60], EG_QUEEN_PST[61], EG_QUEEN_PST[62], EG_QUEEN_PST[63] },
    { EG_KING_PST[0], EG_KING_PST[1], EG_KING_PST[2], EG_KING_PST[3], EG_KING_PST[4], EG_KING_PST[5], EG_KING_PST[6], EG_KING_PST[7],
      EG_KING_PST[8], EG_KING_PST[9], EG_KING_PST[10], EG_KING_PST[11], EG_KING_PST[12], EG_KING_PST[13], EG_KING_PST[14], EG_KING_PST[15],
      EG_KING_PST[16], EG_KING_PST[17], EG_KING_PST[18], EG_KING_PST[19], EG_KING_PST[20], EG_KING_PST[21], EG_KING_PST[22], EG_KING_PST[23],
      EG_KING_PST[24], EG_KING_PST[25], EG_KING_PST[26], EG_KING_PST[27], EG_KING_PST[28], EG_KING_PST[29], EG_KING_PST[30], EG_KING_PST[31],
      EG_KING_PST[32], EG_KING_PST[33], EG_KING_PST[34], EG_KING_PST[35], EG_KING_PST[36], EG_KING_PST[37], EG_KING_PST[38], EG_KING_PST[39],
      EG_KING_PST[40], EG_KING_PST[41], EG_KING_PST[42], EG_KING_PST[43], EG_KING_PST[44], EG_KING_PST[45], EG_KING_PST[46], EG_KING_PST[47],
      EG_KING_PST[48], EG_KING_PST[49], EG_KING_PST[50], EG_KING_PST[51], EG_KING_PST[52], EG_KING_PST[53], EG_KING_PST[54], EG_KING_PST[55],
      EG_KING_PST[56], EG_KING_PST[57], EG_KING_PST[58], EG_KING_PST[59], EG_KING_PST[60], EG_KING_PST[61], EG_KING_PST[62], EG_KING_PST[63] }
};

Evaluator::Evaluator() {}

int Evaluator::evaluate(const Board& board) const {
    Color us = board.side_to_move();
    Color them = static_cast<Color>(1 - us);

    int us_material_mg = evaluate_material(board, us, MG_PIECE_VALUE);
    int them_material_mg = evaluate_material(board, them, MG_PIECE_VALUE);
    int us_material_eg = evaluate_material(board, us, EG_PIECE_VALUE);
    int them_material_eg = evaluate_material(board, them, EG_PIECE_VALUE);

    int us_position = evaluate_position(board, us);
    int them_position = evaluate_position(board, them);
    int position = us_position - them_position;

    int us_mobility = evaluate_mobility(board, us);
    int them_mobility = evaluate_mobility(board, them);
    int mobility = us_mobility - them_mobility;

    int us_king = evaluate_king_safety(board, us);
    int them_king = evaluate_king_safety(board, them);
    int king = us_king - them_king;

    int p = phase(board);
    int mg_score = (us_material_mg - them_material_mg) + position + mobility + king;
    int eg_score = (us_material_eg - them_material_eg) + position + mobility + king;

    return interpolate(mg_score, eg_score, p);
}

int Evaluator::evaluate_material(const Board& board, Color c, const int* piece_values) const {
    if (piece_values == nullptr) {
        piece_values = MG_PIECE_VALUE;
    }
    int score = 0;
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        Bitboard bb = board.pieces(static_cast<PieceType>(pt), c);
        int count = BitOps::popcountll(bb);
        score += piece_values[pt] * count;
    }
    return score;
}

int Evaluator::evaluate_position(const Board& board, Color c) const {
    int score = 0;
    int p = phase(board);

    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        Bitboard bb = board.pieces(static_cast<PieceType>(pt), c);
        while (bb) {
            Square sq = static_cast<Square>(BitOps::ctzll(bb));
            bb &= bb - 1;

            Square mirror_sq = (c == BLACK) ? static_cast<Square>(63 - sq) : sq;
            score += MG_PST[pt][mirror_sq];
            score += ((EG_PST[pt][mirror_sq] - MG_PST[pt][mirror_sq]) * p) / 24;
        }
    }

    return score;
}

int Evaluator::evaluate_mobility(const Board& board, Color c) const {
    (void)c;
    int phase = this->phase(board);
    MoveGenerator mg(board);
    MoveList moves = mg.generate_pseudo_legal();
    int mobility = 0;

    for (size_t i = 0; i < moves.size(); ++i) {
        const Move& m = moves[i];
        if (m.piece() != PAWN) {
            mobility++;
        }
    }

    // Less mobility bonus in endgame (pieces naturally more active)
    int multiplier = (phase <= 12) ? 15 : (phase >= 18 ? 5 : 10);
    return mobility * multiplier;
}

int Evaluator::evaluate_king_safety(const Board& board, Color c) const {
    Square ksq = board.find_king(c);
    if (ksq == 64) return 0;

    int phase = this->phase(board);
    int score = 0;

    Bitboard our_pawns = board.pieces(PAWN, c);
    int x = file_of(ksq);
    int y = rank_of(ksq);
    int pawn_dir = (c == WHITE) ? 1 : -1;
    int pawns_near = 0;

    for (int dx = -1; dx <= 1; ++dx) {
        int nx = x + dx;
        int ny = y + pawn_dir;
        if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
            Square s = static_cast<Square>(ny * 8 + nx);
            if (our_pawns & (1ULL << s)) {
                pawns_near++;
            }
        }
    }

    // Pawn shield matters less in endgame
    int shield_bonus = (phase <= 12) ? 25 : (phase >= 18 ? 10 : 20);
    score += pawns_near * shield_bonus;

    // Penalty for exposed king (fewer pawns)
    if (pawns_near < 2) {
        score -= (phase <= 12) ? 50 : (phase >= 18 ? 10 : 30);
    }

    return score;
}

int Evaluator::phase(const Board& board) const {
    int total = 0;
    for (int pt = 0; pt < NUM_PIECES; ++pt) {
        if (pt == KING) continue;
        Bitboard w = board.pieces(static_cast<PieceType>(pt), WHITE);
        Bitboard b = board.pieces(static_cast<PieceType>(pt), BLACK);
        total += (BitOps::popcountll(w) + BitOps::popcountll(b)) * PHASE_WEIGHTS[pt];
    }
    // Phase: 0 = opening/middlegame, 24 = endgame
    return std::min(24, total);
}

int Evaluator::interpolate(int mg, int eg, int p) const {
    if (p <= 0) return mg;
    if (p >= 24) return eg;
    return mg + ((eg - mg) * p) / 24;
}

Bitboard Evaluator::attacked_squares(const Board& board, Color c) const {
    Bitboard attacks = 0;
    Bitboard all_occ = board.all_pieces();

    // Pawns
    Bitboard pawns = board.pieces(PAWN, c);
    int pawn_dir = (c == WHITE) ? 1 : -1;
    while (pawns) {
        Square sq = static_cast<Square>(BitOps::ctzll(pawns));
        pawns &= pawns - 1;
        int x = file_of(sq);
        int y = rank_of(sq);
        if (x > 0) attacks |= 1ULL << make_square(x - 1, y + pawn_dir);
        if (x < 7) attacks |= 1ULL << make_square(x + 1, y + pawn_dir);
    }

    // Knights
    Bitboard knights = board.pieces(KNIGHT, c);
    static constexpr int knight_dx[8] = { -2, -1, 1, 2, 2, 1, -1, -2 };
    static constexpr int knight_dy[8] = { 1, 2, 2, 1, -1, -2, -2, -1 };
    while (knights) {
        Square sq = static_cast<Square>(BitOps::ctzll(knights));
        knights &= knights - 1;
        int x = file_of(sq);
        int y = rank_of(sq);
        for (int i = 0; i < 8; ++i) {
            int nx = x + knight_dx[i];
            int ny = y + knight_dy[i];
            if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                attacks |= 1ULL << make_square(nx, ny);
            }
        }
    }

    // Bishops and Queens (diagonals)
    Bitboard bishops = board.pieces(BISHOP, c);
    Bitboard queens = board.pieces(QUEEN, c);
    Bitboard diag_pieces = bishops | queens;

    const int bishop_dirs[4] = { -9, -7, 7, 9 };
    while (diag_pieces) {
        Square sq = static_cast<Square>(BitOps::ctzll(diag_pieces));
        diag_pieces &= diag_pieces - 1;
        for (int dir : bishop_dirs) {
            int s = static_cast<int>(sq) + dir;
            while (s >= 0 && s < 64) {
                attacks |= 1ULL << s;
                if (all_occ & (1ULL << s)) break;
                s += dir;
                if (dir == -7 && (s % 8 == 7)) break;
                if (dir == 7 && (s % 8 == 0)) break;
                if (dir == -9 && (s % 8 == 0)) break;
                if (dir == 9 && (s % 8 == 7)) break;
            }
        }
    }

    // Rooks and Queens (orthogonal)
    Bitboard rooks = board.pieces(ROOK, c);
    Bitboard ortho_pieces = rooks | queens;

    const int rook_dirs[4] = { -8, -1, 1, 8 };
    while (ortho_pieces) {
        Square sq = static_cast<Square>(BitOps::ctzll(ortho_pieces));
        ortho_pieces &= ortho_pieces - 1;
        for (int dir : rook_dirs) {
            int s = static_cast<int>(sq) + dir;
            while (s >= 0 && s < 64) {
                attacks |= 1ULL << s;
                if (all_occ & (1ULL << s)) break;
                s += dir;
                if (dir == -1 && (s % 8 == 7)) break;
                if (dir == 1 && (s % 8 == 0)) break;
            }
        }
    }

    // King
    Square king_sq = board.find_king(c);
    if (king_sq != 64) {
        int kx = file_of(king_sq);
        int ky = rank_of(king_sq);
        static constexpr int king_dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
        static constexpr int king_dy[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };
        for (int i = 0; i < 8; ++i) {
            int nx = kx + king_dx[i];
            int ny = ky + king_dy[i];
            if (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                attacks |= 1ULL << make_square(nx, ny);
            }
        }
    }

    return attacks;
}
