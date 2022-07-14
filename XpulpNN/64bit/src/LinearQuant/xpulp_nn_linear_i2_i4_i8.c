/*
 * xpulp_nn_linear_i2_i4_i8.c
 * Nazareno Bruschi <nazareno.bruschi@unibo.it>
 *
 * Copyright (C) 2019-2020 University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pmsis.h"
#include "pulp_nn_utils.h"



void __attribute__((noinline)) xpulp_nn_linear_i2_i4_i8(
                        int8_t *pIn,
                        int8_t *pBias,
                        int8_t *pOut,
                        int8_t *pWeight,
                        int64_t *pKappa,
                        int64_t *pLambda,
                        uint16_t out_mult,
                        uint16_t out_shift,
                        uint16_t dim_vec,
                        uint16_t num_o_neurons,
                        uint8_t flag_relu,
                        uint8_t flag_batch_norm)
{
  int8_t mask = 0xf0;
  int8_t n_mask = ~ mask;
  int8_t off = 0x04;
  uint16_t dim_vec_in = PACK_INT2_SIZE(dim_vec);
  uint16_t dim_vec_wt = PACK_INT8_SIZE(dim_vec);

  int core_id = pi_core_id();
  int Log2Core = log2(NUM_CORES);
  int chunk = (num_o_neurons >> Log2Core) + ((num_o_neurons & (NUM_CORES-1))!=0);
  int start = min((chunk << (chunk == 1)) * core_id, num_o_neurons);
  int stop = min(start + (chunk << (chunk == 1)), num_o_neurons);

  v4s vecB[4];
  v4s vecA[4];
  v4s vecA2[4];

  int8_t *pOutBuffer = (int8_t *) pOut + (start >> 1);

  int i;

  int64_t *k1 = pKappa + start;
  int64_t *lambda1 = pLambda + start;

  for(i=start; i<stop; i+=2)
  {
    int sum = 0;
    int sum2 = 0;

    if (pBias != NULL)
    {
      sum = *(int32_t *)(pBias + 4*i);
      sum2 = (pBias[i + 1]);
    }

    int8_t *pB = pIn;
    int8_t *pA = pWeight + (i * dim_vec_wt);
    int8_t *pA2 = pA + dim_vec_wt;
    int32_t *ptrA  = (int32_t *) pA ;
    int32_t *ptrA2  = (int32_t *) pA2 ;
    pB  = pulp_nn_i2_to_i8(pB , vecB);

    int32_t *startB;

    asm volatile("mv %0, %1":"=r"(startB):"r"(vecB));

    int32_t *ptrB  = (int32_t *) vecB;

    ptrA  = MacLoadInit(1, 0, 0, 0, ptrA);
    ptrA2  = MacLoadInit(1, 0, 1, 0, ptrA2);

    ptrB  = MacLoadInit(0, 1, 0, 0, ptrB);


    for(int j=0; j < (dim_vec >> 4); j++)
    {
      sum = MacLoads4(1, 0, 0, 0, ptrA, sum);
      ptrA = MacLoadUpdate(ptrA);
      sum2 = MacLoads4(1, 0, 1, 0, ptrA2, sum2);
      ptrA2 = MacLoadUpdate(ptrA2);

      ptrB  = MacLoadInit(0, 1, 0, 0, ptrB);

      sum = MacLoads4(1, 0, 0, 0, ptrA, sum);
      ptrA = MacLoadUpdate(ptrA);
      sum2 = MacLoads4(1, 0, 1, 0, ptrA2, sum2);
      ptrA2 = MacLoadUpdate(ptrA2);

      ptrB  = MacLoadInit(0, 1, 0, 0, ptrB);

      sum = MacLoads4(1, 0, 0, 0, ptrA, sum);
      ptrA = MacLoadUpdate(ptrA);
      sum2 = MacLoads4(1, 0, 1, 0, ptrA2, sum2);
      ptrA2 = MacLoadUpdate(ptrA2);

      ptrB  = MacLoadInit(0, 1, 0, 0, ptrB);
      pB  = pulp_nn_i2_to_i8(pB , vecB);

      ptrB   = MacLoadAssign(startB);
      sum = MacLoads4(1, 0, 0, 0, ptrA, sum);
      ptrA = MacLoadUpdate(ptrA);
      sum2 = MacLoads4(1, 0, 1, 0, ptrA2, sum2);
      ptrA2 = MacLoadUpdate(ptrA2);

      ptrB  = MacLoadInit(0, 1, 0, 0, ptrB);
    }
    uint16_t col_cnt = dim_vec & 0xf;
    if(col_cnt)
    {
      pA=((dim_vec >> 4) << 4);
      pA2+=((dim_vec >> 4) << 4);
      pB-=4;
      do
      {
        int8_t inB =  (int8_t) bitext((int32_t) *pB, 2, 0);
        int8_t inB2 = (int8_t) bitext((int32_t) *pB, 2, 2);
        int8_t inB3 = (int8_t) bitext((int32_t) *pB, 2, 4);
        int8_t inB4 = (int8_t) bitext((int32_t) *pB, 2, 6);
        pB++;
        int8_t inA = *pA;
        pA++;
        int8_t inA2 = *pA;
        pA++;
        int8_t inA3 = *pA;
        pA++;
        int8_t inA4 = *pA;
        pA++;
        sum += inA * inB;
        sum += inA2 * inB2;
        sum += inA3 * inB3;
        sum += inA4 * inB4;
        inA = *pA2;
        pA2++;
        inA2 = *pA2;
        pA2++;
        inA3 = *pA2;
        pA2++;
        inA4 = *pA2;
        pA2++;
        sum2 += inA * inB;
        sum2 += inA2 * inB2;
        sum2 += inA3 * inB3;
        sum2 += inA4 * inB4;
        col_cnt-=4;
      }while (col_cnt);
    }
    if (flag_batch_norm && flag_relu)
    {
      sum = pulp_nn_bn_quant_i4(sum, *k1, *lambda1, out_shift);
      sum2 = pulp_nn_bn_quant_i4(sum2, *(k1 + 1), *(lambda1 + 1), out_shift);
      *pOutBuffer = bitins(sum, n_mask, sum2, mask, off);
      pOutBuffer++;
      k1+=2;
      lambda1+=2;
    }
    else
    {
      if (flag_relu == 1)
      {
        sum = pulp_nn_quant_i4(sum, out_mult, out_shift);
        sum2 = pulp_nn_quant_i4(sum2, out_mult, out_shift);
        *pOutBuffer = bitins(sum, n_mask, sum2, mask, off);
        pOutBuffer++;
      }
      else
      {
        sum = (int8_t) clips4(sum >> out_shift);
        sum2 = (int8_t) clips4(sum2 >> out_shift);
        *pOutBuffer = bitins(sum, n_mask, sum2, mask, off);
        pOutBuffer++;
      }
    }
  }
  pi_cl_team_barrier(0);
}
