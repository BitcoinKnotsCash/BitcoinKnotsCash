// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2022 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util/check.h>

/** One-off target shift applied to the first block under a new PoW algorithm.
 *
 * Shared by GetNextWorkRequired and PermittedDifficultyTransition so the value
 * that gets produced and the value that gets accepted cannot drift apart.
 */
static unsigned int ApplyBlake2bTargetShift(unsigned int nBits, const Consensus::Params& params)
{
    arith_uint256 bnNew;
    bnNew.SetCompact(nBits);
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    const unsigned int shift{params.Blake2bTargetShift};
    if (bnNew > (bnPowLimit >> shift)) {
        bnNew = bnPowLimit;
    } else {
        bnNew <<= shift;
    }
    return bnNew.GetCompact();
}

// XBTC: ASERT (aserti3-2d) difficulty, anchored at genesis. Ported from Friotaioch/BCHN (MIT).
// Integer-only, consensus-critical. The one-time BLAKE2b target shift (activation at height 1)
// is folded into the anchor target, since every block from height 1 is BLAKE2b.
unsigned int GetNextWorkRequiredASERT(const CBlockIndex* pindexPrev,
                                      const Consensus::Params& params,
                                      const CBlockIndex* pindexAnchor)
{
    const arith_uint256 powLimit = UintToArith256(params.powLimit);
    const int64_t nHalfLife = params.nASERTHalfLife;

    const int64_t nAnchorHeight = pindexAnchor->nHeight;          // 0 (genesis)
    const int64_t nAnchorTime   = pindexAnchor->GetBlockTime();   // genesis nTime

    // Anchor target = genesis nBits, shifted once into BLAKE2b space (blocks >=1 are BLAKE2b).
    arith_uint256 anchorTarget;
    anchorTarget.SetCompact(ApplyBlake2bTargetShift(pindexAnchor->nBits, params));

    const int64_t nHeightDiff    = int64_t(pindexPrev->nHeight) - nAnchorHeight;
    const int64_t nTimeDiff      = pindexPrev->GetBlockTime() - nAnchorTime;
    const int64_t nIdealTimespan = params.nPowTargetSpacing * (nHeightDiff + 1);

    int64_t exponent = ((nTimeDiff - nIdealTimespan) * 65536) / nHalfLife;

    const int64_t num_shifts = exponent >> 16;
    exponent -= num_shifts * 65536;
    const uint64_t frac = uint64_t(exponent);

    const uint64_t factor = 65536 +
        ((195766423245049ULL * frac +
          971821376ULL * frac * frac +
          5127ULL * frac * frac * frac +
          (1ULL << 47)) >> 48);

    arith_uint256 next = anchorTarget;
    next *= (uint32_t)factor;

    if (num_shifts <= -256) {
        return arith_uint256(1).GetCompact();
    } else if (num_shifts < 0) {
        next >>= (unsigned)(-num_shifts);
    } else if (num_shifts >= 15) {
        return powLimit.GetCompact();
    } else {
        next <<= (unsigned)num_shifts;
    }
    next >>= 16;

    if (next == 0) return arith_uint256(1).GetCompact();
    if (next > powLimit) return powLimit.GetCompact();
    return next.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    assert(pindexLast != nullptr);

    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    if (params.fPowAllowMinDifficultyBlocks &&
        pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 2) {
        return UintToArith256(params.powLimit).GetCompact();
    }

    // XBTC: per-block ASERT anchored at genesis (height 0). Genesis is SHA256d;
    // every block from height 1 is BLAKE2b (shift folded into the anchor above).
    const CBlockIndex* pindexAnchor = pindexLast->GetAncestor(0);
    assert(pindexAnchor != nullptr);
    return GetNextWorkRequiredASERT(pindexLast, params, pindexAnchor);
}

unsigned int CalculateNextWorkRequired(const CBlockIndex* pindexLast, int64_t nFirstBlockTime, const Consensus::Params& params)
{
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan/4)
        nActualTimespan = params.nPowTargetTimespan/4;
    if (nActualTimespan > params.nPowTargetTimespan*4)
        nActualTimespan = params.nPowTargetTimespan*4;

    // Retarget
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;

    // Special difficulty rule for Testnet4
    if (params.enforce_BIP94) {
        // Here we use the first block of the difficulty period. This way
        // the real difficulty is always preserved in the first block as
        // it is not allowed to use the min-difficulty exception.
        int nHeightFirst = pindexLast->nHeight - (params.DifficultyAdjustmentInterval()-1);
        const CBlockIndex* pindexFirst = pindexLast->GetAncestor(nHeightFirst);
        bnNew.SetCompact(pindexFirst->nBits);
    } else {
        bnNew.SetCompact(pindexLast->nBits);
    }

    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

// Check that on difficulty adjustments, the new difficulty does not increase
// or decrease beyond the permitted limits.
bool PermittedDifficultyTransition(const Consensus::Params& params, int64_t height, uint32_t old_nbits, uint32_t new_nbits)
{
    // XBTC: ASERT retargets every block, so the legacy "within 4x per 2016 blocks"
    // invariant no longer applies. The exact required nBits is enforced contextually
    // via GetNextWorkRequired() in ContextualCheckBlockHeader().
    (void)params; (void)height; (void)old_nbits; (void)new_nbits;
    return true;
}

// Bypasses the actual proof of work check during fuzz testing with a simplified validation checking whether
// the most significant bit of the last byte of the hash is set.
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    if constexpr (G_FUZZING) return (hash.data()[31] & 0x80) == 0;
    return CheckProofOfWorkImpl(hash, nBits, params);
}

std::optional<arith_uint256> DeriveTarget(unsigned int nBits, const uint256 pow_limit)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(pow_limit))
        return {};

    return bnTarget;
}

bool CheckProofOfWorkImpl(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    auto bnTarget{DeriveTarget(nBits, params.powLimit)};
    if (!bnTarget) return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}
