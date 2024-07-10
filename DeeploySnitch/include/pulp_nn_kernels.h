/*
 * pulp_nn_kernels.h
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


void pulp_nn_add_i8_i8_i8(
                       uint8_t * pIn1,
                       uint8_t * pIn2,
                       uint8_t * pOut,
                       int32_t in_mult1,
                       int32_t in_add1,
                       uint16_t in_shift1,
                       int32_t in_mult2,
                       int32_t in_add2,
                       uint16_t in_shift2,
                       int32_t out_mult,
                       int32_t out_add,
                       uint16_t out_shift,
                       uint16_t dim_im_in_x,
                       uint16_t dim_im_in_y,
                       uint16_t ch_im_in,
                       int      out_requant_flag);



#endif