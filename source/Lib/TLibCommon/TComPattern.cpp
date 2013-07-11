/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2013, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     TComPattern.cpp
    \brief    neighboring pixel access classes
*/

#include "TComPic.h"
#include "TComPattern.h"
#include "TComDataCU.h"

//! \ingroup TLibCommon
//! \{

// ====================================================================================================================
// Tables
// ====================================================================================================================

const UChar TComPattern::m_intraFilter[5] =
{
    10, //4x4
    7, //8x8
    1, //16x16
    0, //32x32
    10, //64x64
};
// ====================================================================================================================
// Public member functions (TComPatternParam)
// ====================================================================================================================

/** \param  piTexture     pixel data
 \param  roiWidth     pattern width
 \param  roiHeight    pattern height
 \param  stride       buffer stride
 \param  offsetLeft   neighbour offset (left)
 \param  offsetAbove  neighbour offset (above)
 */
Void TComPatternParam::setPatternParamPel(Pel* piTexture,
                                          Int  roiWidth,
                                          Int  roiHeight,
                                          Int  stride,
                                          Int  offsetLeft,
                                          Int  offsetAbove)
{
    m_patternOrigin = piTexture;
    m_roiWidth       = roiWidth;
    m_roiHeight      = roiHeight;
    m_patternStride  = stride;
    m_offsetLeft     = offsetLeft;
    m_offsetAbove    = offsetAbove;
}

/**
 \param  cu          CU data structure
 \param  comp         component index (0=Y, 1=Cb, 2=Cr)
 \param  roiWidth     pattern width
 \param  roiHeight    pattern height
 \param  stride       buffer stride
 \param  offsetLeft   neighbour offset (left)
 \param  offsetAbove  neighbour offset (above)
 \param  absPartIdx  part index
 */
Void TComPatternParam::setPatternParamCU(TComDataCU* cu,
                                         UChar       comp,
                                         UChar       roiWidth,
                                         UChar       roiHeight,
                                         Int         offsetLeft,
                                         Int         offsetAbove,
                                         UInt        absPartIdx)
{
    m_offsetLeft   = offsetLeft;
    m_offsetAbove  = offsetAbove;

    m_roiWidth     = roiWidth;
    m_roiHeight    = roiHeight;

    UInt absZOrderIdx = cu->getZorderIdxInCU() + absPartIdx;

    if (comp == 0)
    {
        m_patternStride  = cu->getPic()->getStride();
        m_patternOrigin = cu->getPic()->getPicYuvRec()->getLumaAddr(cu->getAddr(), absZOrderIdx) - m_offsetAbove * m_patternStride - m_offsetLeft;
    }
    else
    {
        m_patternStride = cu->getPic()->getCStride();
        if (comp == 1)
        {
            m_patternOrigin = cu->getPic()->getPicYuvRec()->getCbAddr(cu->getAddr(), absZOrderIdx) - m_offsetAbove * m_patternStride - m_offsetLeft;
        }
        else
        {
            m_patternOrigin = cu->getPic()->getPicYuvRec()->getCrAddr(cu->getAddr(), absZOrderIdx) - m_offsetAbove * m_patternStride - m_offsetLeft;
        }
    }
}

// ====================================================================================================================
// Public member functions (TComPattern)
// ====================================================================================================================

Void TComPattern::initPattern(Pel* y,
                              Pel* cb,
                              Pel* cr,
                              Int  roiWidth,
                              Int  roiHeight,
                              Int  stride,
                              Int  offsetLeft,
                              Int  offsetAbove)
{
    m_patternY.setPatternParamPel(y,  roiWidth,      roiHeight,      stride,      offsetLeft,      offsetAbove);
    m_patternCb.setPatternParamPel(cb, roiWidth >> 1, roiHeight >> 1, stride >> 1, offsetLeft >> 1, offsetAbove >> 1);
    m_patternCr.setPatternParamPel(cr, roiWidth >> 1, roiHeight >> 1, stride >> 1, offsetLeft >> 1, offsetAbove >> 1);
}

Void TComPattern::initPattern(TComDataCU* cu, UInt partDepth, UInt absPartIdx)
{
    Int   uiOffsetLeft  = 0;
    Int   uiOffsetAbove = 0;

    UChar width          = cu->getWidth(0) >> partDepth;
    UChar height         = cu->getHeight(0) >> partDepth;

    UInt  absZOrderIdx   = cu->getZorderIdxInCU() + absPartIdx;
    UInt  uiCurrPicPelX    = cu->getCUPelX() + g_rasterToPelX[g_zscanToRaster[absZOrderIdx]];
    UInt  uiCurrPicPelY    = cu->getCUPelY() + g_rasterToPelY[g_zscanToRaster[absZOrderIdx]];

    if (uiCurrPicPelX != 0)
    {
        uiOffsetLeft = 1;
    }

    if (uiCurrPicPelY != 0)
    {
        uiOffsetAbove = 1;
    }

    m_patternY.setPatternParamCU(cu, 0, width,      height,      uiOffsetLeft, uiOffsetAbove, absPartIdx);
    m_patternCb.setPatternParamCU(cu, 1, width >> 1, height >> 1, uiOffsetLeft, uiOffsetAbove, absPartIdx);
    m_patternCr.setPatternParamCU(cu, 2, width >> 1, height >> 1, uiOffsetLeft, uiOffsetAbove, absPartIdx);
}

Void TComPattern::initAdiPattern(TComDataCU* cu, UInt zOrderIdxInPart, UInt partDepth, Pel* adiBuf, Int strideOrig, Int heightOrig)
{
    Pel*  roiOrigin;
    Pel*  adiTemp;
    UInt  cuWidth   = cu->getWidth(0) >> partDepth;
    UInt  cuHeight  = cu->getHeight(0) >> partDepth;
    UInt  uiCuWidth2  = cuWidth << 1;
    UInt  uiCuHeight2 = cuHeight << 1;
    UInt  width;
    UInt  height;
    Int   picStride = cu->getPic()->getStride();
    Int   unitSize = 0;
    Int   numUnitsInCU = 0;
    Int   totalUnits = 0;
    Bool  bNeighborFlags[4 * MAX_NUM_SPU_W + 1];
    Int   numIntraNeighbor = 0;

    UInt partIdxLT, partIdxRT, partIdxLB;

    cu->deriveLeftRightTopIdxAdi(partIdxLT, partIdxRT, zOrderIdxInPart, partDepth);
    cu->deriveLeftBottomIdxAdi(partIdxLB,              zOrderIdxInPart, partDepth);

    unitSize      = g_maxCUWidth >> g_maxCUDepth;
    numUnitsInCU  = cuWidth / unitSize;
    totalUnits    = (numUnitsInCU << 2) + 1;

    bNeighborFlags[numUnitsInCU * 2] = isAboveLeftAvailable(cu, partIdxLT);
    numIntraNeighbor  += (Int)(bNeighborFlags[numUnitsInCU * 2]);
    numIntraNeighbor  += isAboveAvailable(cu, partIdxLT, partIdxRT, bNeighborFlags + (numUnitsInCU * 2) + 1);
    numIntraNeighbor  += isAboveRightAvailable(cu, partIdxLT, partIdxRT, bNeighborFlags + (numUnitsInCU * 3) + 1);
    numIntraNeighbor  += isLeftAvailable(cu, partIdxLT, partIdxLB, bNeighborFlags + (numUnitsInCU * 2) - 1);
    numIntraNeighbor  += isBelowLeftAvailable(cu, partIdxLT, partIdxLB, bNeighborFlags + numUnitsInCU   - 1);

    width = uiCuWidth2 + 1;
    height = uiCuHeight2 + 1;

    if (((width << 2) > strideOrig) || ((height << 2) > heightOrig))
    {
        return;
    }

    roiOrigin = cu->getPic()->getPicYuvRec()->getLumaAddr(cu->getAddr(), cu->getZorderIdxInCU() + zOrderIdxInPart);
    adiTemp   = adiBuf;

    fillReferenceSamples(g_bitDepthY, roiOrigin, adiTemp, bNeighborFlags, numIntraNeighbor, unitSize, numUnitsInCU, totalUnits, cuWidth, cuHeight, width, height, picStride);

    Int   i;
    // generate filtered intra prediction samples
    Int iBufSize = uiCuHeight2 + uiCuWidth2 + 1; // left and left above border + above and above right border + top left corner = length of 3. filter buffer

    UInt uiWH = ADI_BUF_STRIDE * height;       // number of elements in one buffer

    Pel* piFilteredBuf1 = adiBuf + uiWH;      // 1. filter buffer
    Pel* piFilteredBuf2 = piFilteredBuf1 + uiWH; // 2. filter buffer
    Pel* piFilterBuf = piFilteredBuf2 + uiWH;   // buffer for 2. filtering (sequential)
    Pel* piFilterBufN = piFilterBuf + iBufSize; // buffer for 1. filtering (sequential)

    Int l = 0;
    // left border from bottom to top
    for (i = 0; i < uiCuHeight2; i++)
    {
        piFilterBuf[l++] = adiTemp[ADI_BUF_STRIDE * (uiCuHeight2 - i)];
    }

    // top left corner
    piFilterBuf[l++] = adiTemp[0];

    // above border from left to right
    memcpy(&piFilterBuf[l], &adiTemp[1], uiCuWidth2 * sizeof(*piFilterBuf));

    if (cu->getSlice()->getSPS()->getUseStrongIntraSmoothing())
    {
        Int blkSize = 32;
        Int bottomLeft = piFilterBuf[0];
        Int topLeft = piFilterBuf[uiCuHeight2];
        Int topRight = piFilterBuf[iBufSize - 1];
        Int threshold = 1 << (g_bitDepthY - 5);
        Bool bilinearLeft = abs(bottomLeft + topLeft - 2 * piFilterBuf[cuHeight]) < threshold;
        Bool bilinearAbove  = abs(topLeft + topRight - 2 * piFilterBuf[uiCuHeight2 + cuHeight]) < threshold;

        if (cuWidth >= blkSize && (bilinearLeft && bilinearAbove))
        {
            Int shift = g_convertToBit[cuWidth] + 3; // log2(uiCuHeight2)
            piFilterBufN[0] = piFilterBuf[0];
            piFilterBufN[uiCuHeight2] = piFilterBuf[uiCuHeight2];
            piFilterBufN[iBufSize - 1] = piFilterBuf[iBufSize - 1];
            //TODO: Performance Primitive???
            for (i = 1; i < uiCuHeight2; i++)
            {
                piFilterBufN[i] = ((uiCuHeight2 - i) * bottomLeft + i * topLeft + cuHeight) >> shift;
            }

            for (i = 1; i < uiCuWidth2; i++)
            {
                piFilterBufN[uiCuHeight2 + i] = ((uiCuWidth2 - i) * topLeft + i * topRight + cuWidth) >> shift;
            }
        }
        else
        {
            // 1. filtering with [1 2 1]
            piFilterBufN[0] = piFilterBuf[0];
            piFilterBufN[iBufSize - 1] = piFilterBuf[iBufSize - 1];
            for (i = 1; i < iBufSize - 1; i++)
            {
                piFilterBufN[i] = (piFilterBuf[i - 1] + 2 * piFilterBuf[i] + piFilterBuf[i + 1] + 2) >> 2;
            }
        }
    }
    else
    {
        // 1. filtering with [1 2 1]
        piFilterBufN[0] = piFilterBuf[0];
        piFilterBufN[iBufSize - 1] = piFilterBuf[iBufSize - 1];
        for (i = 1; i < iBufSize - 1; i++)
        {
            piFilterBufN[i] = (piFilterBuf[i - 1] + 2 * piFilterBuf[i] + piFilterBuf[i + 1] + 2) >> 2;
        }
    }

    // fill 1. filter buffer with filtered values
    l = 0;
    for (i = 0; i < uiCuHeight2; i++)
    {
        piFilteredBuf1[ADI_BUF_STRIDE * (uiCuHeight2 - i)] = piFilterBufN[l++];
    }

    piFilteredBuf1[0] = piFilterBufN[l++];
    memcpy(&piFilteredBuf1[1], &piFilterBufN[l], uiCuWidth2 * sizeof(*piFilteredBuf1));
}

//Overloaded initialiation of ADI buffers to support buffered references for xpredIntraAngBufRef
Void TComPattern::initAdiPattern(TComDataCU* cu, UInt zOrderIdxInPart, UInt partDepth, Pel* adiBuf, Int strideOrig, Int heightOrig, Pel* refAbove, Pel* refLeft, Pel* refAboveFlt, Pel* refLeftFlt)
{
    initAdiPattern(cu, zOrderIdxInPart, partDepth, adiBuf, strideOrig, heightOrig);
    UInt  cuWidth   = cu->getWidth(0) >> partDepth;
    UInt  cuHeight  = cu->getHeight(0) >> partDepth;
    UInt  uiCuWidth2  = cuWidth << 1;
    UInt  uiCuHeight2 = cuHeight << 1;

    refAbove += cuWidth - 1;
    refAboveFlt += cuWidth - 1;
    refLeft += cuWidth - 1;
    refLeftFlt += cuWidth - 1;

    //  ADI_BUF_STRIDE * (2 * height + 1);
    memcpy(refAbove, adiBuf, (uiCuWidth2 + 1) * sizeof(Pel));
    memcpy(refAboveFlt, adiBuf + ADI_BUF_STRIDE * (2 * cuHeight + 1), (uiCuWidth2 + 1) * sizeof(Pel));

    for (int k = 0; k < uiCuHeight2 + 1; k++)
    {
        refLeft[k] = adiBuf[k * ADI_BUF_STRIDE];
        refLeftFlt[k] = (adiBuf + ADI_BUF_STRIDE * (uiCuHeight2 + 1))[k * ADI_BUF_STRIDE];   // Smoothened
    }
}

Void TComPattern::initAdiPatternChroma(TComDataCU* cu, UInt zOrderIdxInPart, UInt partDepth, Pel* adiBuf, Int strideOrig, Int heightOrig)
{
    Pel*  roiOrigin;
    Pel*  adiTemp;
    UInt  cuWidth  = cu->getWidth(0) >> partDepth;
    UInt  cuHeight = cu->getHeight(0) >> partDepth;
    UInt  width;
    UInt  height;
    Int   picStride = cu->getPic()->getCStride();

    Int   unitSize = 0;
    Int   numUnitsInCU = 0;
    Int   totalUnits = 0;
    Bool  bNeighborFlags[4 * MAX_NUM_SPU_W + 1];
    Int   numIntraNeighbor = 0;

    UInt partIdxLT, partIdxRT, partIdxLB;

    cu->deriveLeftRightTopIdxAdi(partIdxLT, partIdxRT, zOrderIdxInPart, partDepth);
    cu->deriveLeftBottomIdxAdi(partIdxLB,              zOrderIdxInPart, partDepth);

    unitSize      = (g_maxCUWidth >> g_maxCUDepth) >> 1; // for chroma
    numUnitsInCU  = (cuWidth / unitSize) >> 1;          // for chroma
    totalUnits    = (numUnitsInCU << 2) + 1;

    bNeighborFlags[numUnitsInCU * 2] = isAboveLeftAvailable(cu, partIdxLT);
    numIntraNeighbor  += (Int)(bNeighborFlags[numUnitsInCU * 2]);
    numIntraNeighbor  += isAboveAvailable(cu, partIdxLT, partIdxRT, bNeighborFlags + (numUnitsInCU * 2) + 1);
    numIntraNeighbor  += isAboveRightAvailable(cu, partIdxLT, partIdxRT, bNeighborFlags + (numUnitsInCU * 3) + 1);
    numIntraNeighbor  += isLeftAvailable(cu, partIdxLT, partIdxLB, bNeighborFlags + (numUnitsInCU * 2) - 1);
    numIntraNeighbor  += isBelowLeftAvailable(cu, partIdxLT, partIdxLB, bNeighborFlags + numUnitsInCU   - 1);

    cuWidth = cuWidth >> 1; // for chroma
    cuHeight = cuHeight >> 1; // for chroma

    width = cuWidth * 2 + 1;
    height = cuHeight * 2 + 1;

    if ((4 * width > strideOrig) || (4 * height > heightOrig))
    {
        return;
    }

    // get Cb pattern
    roiOrigin = cu->getPic()->getPicYuvRec()->getCbAddr(cu->getAddr(), cu->getZorderIdxInCU() + zOrderIdxInPart);
    adiTemp   = adiBuf;

    fillReferenceSamples(g_bitDepthC, roiOrigin, adiTemp, bNeighborFlags, numIntraNeighbor, unitSize, numUnitsInCU, totalUnits, cuWidth, cuHeight, width, height, picStride);

    // get Cr pattern
    roiOrigin = cu->getPic()->getPicYuvRec()->getCrAddr(cu->getAddr(), cu->getZorderIdxInCU() + zOrderIdxInPart);
    adiTemp   = adiBuf + ADI_BUF_STRIDE * height;

    fillReferenceSamples(g_bitDepthC, roiOrigin, adiTemp, bNeighborFlags, numIntraNeighbor, unitSize, numUnitsInCU, totalUnits, cuWidth, cuHeight, width, height, picStride);
}

Void TComPattern::fillReferenceSamples(Int bitDepth, Pel* roiOrigin, Pel* adiTemp, Bool* bNeighborFlags, Int numIntraNeighbor, Int unitSize, Int numUnitsInCU, Int totalUnits, UInt cuWidth, UInt cuHeight, UInt width, UInt height, Int picStride)
{
    Pel* piRoiTemp;
    Int  i, j;
    Int  iDCValue = 1 << (bitDepth - 1);

    if (numIntraNeighbor == 0)
    {
        // Fill border with DC value
        for (i = 0; i < width; i++)
        {
            adiTemp[i] = iDCValue;
        }

        for (i = 1; i < height; i++)
        {
            adiTemp[i * ADI_BUF_STRIDE] = iDCValue;
        }
    }
    else if (numIntraNeighbor == totalUnits)
    {
        // Fill top-left border with rec. samples
        piRoiTemp = roiOrigin - picStride - 1;
        adiTemp[0] = piRoiTemp[0];

        // Fill left border with rec. samples
        // Fill below left border with rec. samples
        piRoiTemp = roiOrigin - 1;

        for (i = 0; i < 2 * cuHeight; i++)
        {
            adiTemp[(1 + i) * ADI_BUF_STRIDE] = piRoiTemp[0];
            piRoiTemp += picStride;
        }

        // Fill top border with rec. samples
        // Fill top right border with rec. samples
        piRoiTemp = roiOrigin - picStride;
        memcpy(&adiTemp[1], piRoiTemp, 2 * cuWidth * sizeof(*adiTemp));
    }
    else // reference samples are partially available
    {
        Int  iNumUnits2 = numUnitsInCU << 1;
        Int  iTotalSamples = totalUnits * unitSize;
        Pel  piAdiLine[5 * MAX_CU_SIZE];
        Pel  *piAdiLineTemp;
        Bool *pbNeighborFlags;
        Int  iNext, iCurr;
        Pel  piRef = 0;

        // Initialize
        for (i = 0; i < iTotalSamples; i++)
        {
            piAdiLine[i] = iDCValue;
        }

        // Fill top-left sample
        piRoiTemp = roiOrigin - picStride - 1;
        piAdiLineTemp = piAdiLine + (iNumUnits2 * unitSize);
        pbNeighborFlags = bNeighborFlags + iNumUnits2;
        if (*pbNeighborFlags)
        {
            piAdiLineTemp[0] = piRoiTemp[0];
            for (i = 1; i < unitSize; i++)
            {
                piAdiLineTemp[i] = piAdiLineTemp[0];
            }
        }

        // Fill left & below-left samples
        piRoiTemp += picStride;
        piAdiLineTemp--;
        pbNeighborFlags--;
        for (j = 0; j < iNumUnits2; j++)
        {
            if (*pbNeighborFlags)
            {
                for (i = 0; i < unitSize; i++)
                {
                    piAdiLineTemp[-i] = piRoiTemp[i * picStride];
                }
            }
            piRoiTemp += unitSize * picStride;
            piAdiLineTemp -= unitSize;
            pbNeighborFlags--;
        }

        // Fill above & above-right samples
        piRoiTemp = roiOrigin - picStride;
        piAdiLineTemp = piAdiLine + ((iNumUnits2 + 1) * unitSize);
        pbNeighborFlags = bNeighborFlags + iNumUnits2 + 1;
        for (j = 0; j < iNumUnits2; j++)
        {
            if (*pbNeighborFlags)
            {
                memcpy(piAdiLineTemp, piRoiTemp, unitSize * sizeof(*adiTemp));
            }
            piRoiTemp += unitSize;
            piAdiLineTemp += unitSize;
            pbNeighborFlags++;
        }

        // Pad reference samples when necessary
        iCurr = 0;
        iNext = 1;
        piAdiLineTemp = piAdiLine;
        while (iCurr < totalUnits)
        {
            if (!bNeighborFlags[iCurr])
            {
                if (iCurr == 0)
                {
                    while (iNext < totalUnits && !bNeighborFlags[iNext])
                    {
                        iNext++;
                    }

                    piRef = piAdiLine[iNext * unitSize];
                    // Pad unavailable samples with new value
                    while (iCurr < iNext)
                    {
                        for (i = 0; i < unitSize; i++)
                        {
                            piAdiLineTemp[i] = piRef;
                        }

                        piAdiLineTemp += unitSize;
                        iCurr++;
                    }
                }
                else
                {
                    piRef = piAdiLine[iCurr * unitSize - 1];
                    for (i = 0; i < unitSize; i++)
                    {
                        piAdiLineTemp[i] = piRef;
                    }

                    piAdiLineTemp += unitSize;
                    iCurr++;
                }
            }
            else
            {
                piAdiLineTemp += unitSize;
                iCurr++;
            }
        }

        // Copy processed samples
        piAdiLineTemp = piAdiLine + height + unitSize - 2;
        memcpy(adiTemp, piAdiLineTemp, width * sizeof(*adiTemp));

        piAdiLineTemp = piAdiLine + height - 1;
        for (i = 1; i < height; i++)
        {
            adiTemp[i * ADI_BUF_STRIDE] = piAdiLineTemp[-i];
        }
    }
}

Pel* TComPattern::getAdiOrgBuf(Int /*cuWidth*/, Int /*cuHeight*/, Pel* adiBuf)
{
    return adiBuf;
}

Pel* TComPattern::getAdiCbBuf(Int /*cuWidth*/, Int /*cuHeight*/, Pel* adiBuf)
{
    return adiBuf;
}

Pel* TComPattern::getAdiCrBuf(Int cuWidth, Int cuHeight, Pel* adiBuf)
{
    return adiBuf + ADI_BUF_STRIDE * (cuHeight * 2 + 1);
}

/** Get pointer to reference samples for intra prediction
 * \param dirMode     prediction mode index
 * \param log2BlkSize size of block (2 = 4x4, 3 = 8x8, 4 = 16x16, 5 = 32x32, 6 = 64x64)
 * \param adiBuf    pointer to unfiltered reference samples
 * \return            pointer to (possibly filtered) reference samples
 *
 * The prediction mode index is used to determine whether a smoothed reference sample buffer is returned.
 */
Pel* TComPattern::getPredictorPtr(UInt dirMode, UInt log2BlkSize, Pel* adiBuf)
{
    Pel* piSrc;

    assert(log2BlkSize >= 2 && log2BlkSize < 7);
    Int diff = min<Int>(abs((Int)dirMode - HOR_IDX), abs((Int)dirMode - VER_IDX));
    UChar ucFiltIdx = diff > m_intraFilter[log2BlkSize - 2] ? 1 : 0;
    if (dirMode == DC_IDX)
    {
        ucFiltIdx = 0; //no smoothing for DC or LM chroma
    }

    assert(ucFiltIdx <= 1);

    Int width  = 1 << log2BlkSize;
    Int height = 1 << log2BlkSize;

    piSrc = getAdiOrgBuf(width, height, adiBuf);

    if (ucFiltIdx)
    {
        piSrc += ADI_BUF_STRIDE * (2 * height + 1);
    }

    return piSrc;
}

Bool TComPattern::isAboveLeftAvailable(TComDataCU* cu, UInt partIdxLT)
{
    Bool bAboveLeftFlag;
    UInt uiPartAboveLeft;
    TComDataCU* pcCUAboveLeft = cu->getPUAboveLeft(uiPartAboveLeft, partIdxLT);

    if (cu->getSlice()->getPPS()->getConstrainedIntraPred())
    {
        bAboveLeftFlag = (pcCUAboveLeft && pcCUAboveLeft->getPredictionMode(uiPartAboveLeft) == MODE_INTRA);
    }
    else
    {
        bAboveLeftFlag = (pcCUAboveLeft ? true : false);
    }
    return bAboveLeftFlag;
}

Int TComPattern::isAboveAvailable(TComDataCU* cu, UInt partIdxLT, UInt partIdxRT, Bool *bValidFlags)
{
    const UInt uiRasterPartBegin = g_zscanToRaster[partIdxLT];
    const UInt uiRasterPartEnd = g_zscanToRaster[partIdxRT] + 1;
    const UInt uiIdxStep = 1;
    Bool *pbValidFlags = bValidFlags;
    Int iNumIntra = 0;

    for (UInt uiRasterPart = uiRasterPartBegin; uiRasterPart < uiRasterPartEnd; uiRasterPart += uiIdxStep)
    {
        UInt uiPartAbove;
        TComDataCU* pcCUAbove = cu->getPUAbove(uiPartAbove, g_rasterToZscan[uiRasterPart]);
        if (cu->getSlice()->getPPS()->getConstrainedIntraPred())
        {
            if (pcCUAbove && pcCUAbove->getPredictionMode(uiPartAbove) == MODE_INTRA)
            {
                iNumIntra++;
                *pbValidFlags = true;
            }
            else
            {
                *pbValidFlags = false;
            }
        }
        else
        {
            if (pcCUAbove)
            {
                iNumIntra++;
                *pbValidFlags = true;
            }
            else
            {
                *pbValidFlags = false;
            }
        }
        pbValidFlags++;
    }

    return iNumIntra;
}

Int TComPattern::isLeftAvailable(TComDataCU* cu, UInt partIdxLT, UInt partIdxLB, Bool *bValidFlags)
{
    const UInt uiRasterPartBegin = g_zscanToRaster[partIdxLT];
    const UInt uiRasterPartEnd = g_zscanToRaster[partIdxLB] + 1;
    const UInt uiIdxStep = cu->getPic()->getNumPartInWidth();
    Bool *pbValidFlags = bValidFlags;
    Int iNumIntra = 0;

    for (UInt uiRasterPart = uiRasterPartBegin; uiRasterPart < uiRasterPartEnd; uiRasterPart += uiIdxStep)
    {
        UInt uiPartLeft;
        TComDataCU* pcCULeft = cu->getPULeft(uiPartLeft, g_rasterToZscan[uiRasterPart]);
        if (cu->getSlice()->getPPS()->getConstrainedIntraPred())
        {
            if (pcCULeft && pcCULeft->getPredictionMode(uiPartLeft) == MODE_INTRA)
            {
                iNumIntra++;
                *pbValidFlags = true;
            }
            else
            {
                *pbValidFlags = false;
            }
        }
        else
        {
            if (pcCULeft)
            {
                iNumIntra++;
                *pbValidFlags = true;
            }
            else
            {
                *pbValidFlags = false;
            }
        }
        pbValidFlags--; // opposite direction
    }

    return iNumIntra;
}

Int TComPattern::isAboveRightAvailable(TComDataCU* cu, UInt partIdxLT, UInt partIdxRT, Bool *bValidFlags)
{
    const UInt numUnitsInPU = g_zscanToRaster[partIdxRT] - g_zscanToRaster[partIdxLT] + 1;
    Bool *pbValidFlags = bValidFlags;
    Int iNumIntra = 0;

    for (UInt offset = 1; offset <= numUnitsInPU; offset++)
    {
        UInt uiPartAboveRight;
        TComDataCU* pcCUAboveRight = cu->getPUAboveRightAdi(uiPartAboveRight, partIdxRT, offset);
        if (cu->getSlice()->getPPS()->getConstrainedIntraPred())
        {
            if (pcCUAboveRight && pcCUAboveRight->getPredictionMode(uiPartAboveRight) == MODE_INTRA)
            {
                iNumIntra++;
                *pbValidFlags = true;
            }
            else
            {
                *pbValidFlags = false;
            }
        }
        else
        {
            if (pcCUAboveRight)
            {
                iNumIntra++;
                *pbValidFlags = true;
            }
            else
            {
                *pbValidFlags = false;
            }
        }
        pbValidFlags++;
    }

    return iNumIntra;
}

Int TComPattern::isBelowLeftAvailable(TComDataCU* cu, UInt partIdxLT, UInt partIdxLB, Bool *bValidFlags)
{
    const UInt numUnitsInPU = (g_zscanToRaster[partIdxLB] - g_zscanToRaster[partIdxLT]) / cu->getPic()->getNumPartInWidth() + 1;
    Bool *pbValidFlags = bValidFlags;
    Int iNumIntra = 0;

    for (UInt offset = 1; offset <= numUnitsInPU; offset++)
    {
        UInt uiPartBelowLeft;
        TComDataCU* pcCUBelowLeft = cu->getPUBelowLeftAdi(uiPartBelowLeft, partIdxLB, offset);
        if (cu->getSlice()->getPPS()->getConstrainedIntraPred())
        {
            if (pcCUBelowLeft && pcCUBelowLeft->getPredictionMode(uiPartBelowLeft) == MODE_INTRA)
            {
                iNumIntra++;
                *pbValidFlags = true;
            }
            else
            {
                *pbValidFlags = false;
            }
        }
        else
        {
            if (pcCUBelowLeft)
            {
                iNumIntra++;
                *pbValidFlags = true;
            }
            else
            {
                *pbValidFlags = false;
            }
        }
        pbValidFlags--; // opposite direction
    }

    return iNumIntra;
}

//! \}
