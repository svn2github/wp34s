/* This file is part of 34S.
 * 
 * 34S is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * 34S is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with 34S.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __STATS_H__
#define __STATS_H__

#define NUMSIGMAREG (13)

extern void sigma_clear(decimal64 *, decimal64 *, decContext *);
extern void sigma_plus(decContext *);
extern void sigma_minus(decContext *);

extern void stats_mean(decimal64 *, decimal64 *, decContext *);
extern void stats_wmean(decimal64 *, decimal64 *, decContext *);
extern void stats_s(decimal64 *, decimal64 *, decContext *);
extern void stats_sigma(decimal64 *, decimal64 *, decContext *);
extern decNumber *stats_xhat(decNumber *, const decNumber *, decContext *);
extern decNumber *stats_yhat(decNumber *, const decNumber *, decContext *);
extern void stats_correlation(decimal64 *, decimal64 *, decContext *);
extern void stats_LR(decimal64 *, decimal64 *, decContext *);
extern void stats_SErr(decimal64 *, decimal64 *, decContext *);
extern decNumber *stats_sigper(decNumber *, const decNumber *, decContext *);

extern void sigma_N(decimal64 *, decimal64 *, decContext *);
extern void sigma_X(decimal64 *, decimal64 *, decContext *);
extern void sigma_Y(decimal64 *, decimal64 *, decContext *);
extern void sigma_XX(decimal64 *, decimal64 *, decContext *);
extern void sigma_YY(decimal64 *, decimal64 *, decContext *);
extern void sigma_XY(decimal64 *, decimal64 *, decContext *);

extern void sigma_lnX(decimal64 *, decimal64 *, decContext *);
extern void sigma_lnXlnX(decimal64 *, decimal64 *, decContext *);
extern void sigma_lnY(decimal64 *, decimal64 *, decContext *);
extern void sigma_lnYlnY(decimal64 *, decimal64 *, decContext *);
extern void sigma_lnXlnY(decimal64 *, decimal64 *, decContext *);
extern void sigma_XlnY(decimal64 *, decimal64 *, decContext *);
extern void sigma_YlnX(decimal64 *, decimal64 *, decContext *);

extern void sigma_sum(decimal64 *, decimal64 *, decContext *);

extern void stats_mode_expf(decimal64 *nul1, decimal64 *nul2, decContext *ctx);
extern void stats_mode_linf(decimal64 *nul1, decimal64 *nul2, decContext *ctx);
extern void stats_mode_logf(decimal64 *nul1, decimal64 *nul2, decContext *ctx);
extern void stats_mode_pwrf(decimal64 *nul1, decimal64 *nul2, decContext *ctx);
extern void stats_mode_best(decimal64 *nul1, decimal64 *nul2, decContext *ctx);

extern void stats_random(decimal64 *, decimal64 *, decContext *);
extern void stats_sto_random(decimal64 *, decimal64 *, decContext *);

extern decNumber *betai(decNumber *, const decNumber *, const decNumber *, const decNumber *, decContext *);

/* Normal (0, 1) distribution */
extern decNumber *cdf_Q(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_Q(decNumber *, const decNumber *, decContext *);

/* Chi squared n distribution r in register 01 */
extern decNumber *cdf_chi2(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_chi2(decNumber *, const decNumber *, decContext *);

/* Student's T distribution with n df, n in register 01 */
extern decNumber *cdf_T(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_T(decNumber *, const decNumber *, decContext *);

/* F distribution with v1 and v2 df, v1 & v2 in registers 01 and 02 */
extern decNumber *cdf_F(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_F(decNumber *, const decNumber *, decContext *);

/* Weibull distribution with parameters k and lambda in registers 01 and 02 */
extern decNumber *cdf_WB(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_WB(decNumber *, const decNumber *, decContext *);

/* Exponential distribution with parameter lambda in register 01 */
extern decNumber *cdf_EXP(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_EXP(decNumber *, const decNumber *, decContext *);

/* Poisson distribution with parameter lambda (in register 01) */
extern decNumber *cdf_P(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_P(decNumber *, const decNumber *, decContext *);

/* Geometric distribution with parameter p (in register 01) */
extern decNumber *cdf_G(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_G(decNumber *, const decNumber *, decContext *);

/* Binomial distribution (n, p) in registers 01 and 02 */
extern decNumber *cdf_B(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_B(decNumber *, const decNumber *, decContext *);

/* Normal distribution m, sigma from stack (but preserved) */
extern decNumber *cdf_normal(decNumber *, const decNumber *, decContext *);
extern decNumber *qf_normal(decNumber *, const decNumber *, decContext *);

#endif
