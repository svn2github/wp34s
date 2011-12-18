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

/*
 *  Define register block
 */
typedef struct _stat_data {
	// The next four are higher precision
	decimal128 sX2Y;
	decimal128 sX2;		
	decimal128 sY2;		
	decimal128 sXY;

	decimal64 sX;		
	decimal64 sY;		
	decimal64 slnX;		
	decimal64 slnXlnX;	
	decimal64 slnY;		
	decimal64 slnYlnY;	
	decimal64 slnXlnY;	
	decimal64 sXlnY;	
	decimal64 sYlnX;

	unsigned int sN;		
} STAT_DATA;

extern STAT_DATA *StatRegs;

extern int  sigmaCheck(void);
extern void sigmaDeallocate(void);
extern int  sigmaCopy(void *source);
extern void sigma_clear(decimal64 *, decimal64 *, enum nilop);
extern void sigma_plus(void);
extern void sigma_minus(void);

extern void stats_mean(decimal64 *, decimal64 *, enum nilop);
extern void stats_wmean(decimal64 *, decimal64 *, enum nilop);
extern void stats_gmean(decimal64 *, decimal64 *, enum nilop);
extern void stats_deviations(decimal64 *, decimal64 *, enum nilop);
extern void stats_wdeviations(decimal64 *, decimal64 *, enum nilop);
extern decNumber *stats_xhat(decNumber *, const decNumber *);
extern decNumber *stats_yhat(decNumber *, const decNumber *);
extern void stats_correlation(decimal64 *, decimal64 *, enum nilop);
extern void stats_COV(decimal64 *, decimal64 *, enum nilop);
extern void stats_LR(decimal64 *, decimal64 *, enum nilop);
extern void stats_SErr(decimal64 *, decimal64 *, enum nilop);
extern decNumber *stats_sigper(decNumber *, const decNumber *);

extern void sigma_val(decimal64 *, decimal64 *, enum nilop);

extern void sigma_sum(decimal64 *, decimal64 *, enum nilop);

extern void stats_mode(decimal64 *nul1, decimal64 *nul2, enum nilop op);

extern void stats_random(decimal64 *, decimal64 *, enum nilop);
extern void stats_sto_random(decimal64 *, decimal64 *, enum nilop);

extern decNumber *betai(decNumber *, const decNumber *, const decNumber *, const decNumber *);

/* Normal (0, 1) distribution */
extern decNumber *pdf_Q(decNumber *, const decNumber *);
extern decNumber *cdf_Q(decNumber *, const decNumber *);
extern decNumber *qf_Q(decNumber *, const decNumber *);

/* Chi squared n distribution r in register J */
extern decNumber *pdf_chi2(decNumber *, const decNumber *);
extern decNumber *cdf_chi2(decNumber *, const decNumber *);
extern decNumber *qf_chi2(decNumber *, const decNumber *);

/* Student's T distribution with n df, n in register J */
extern decNumber *pdf_T(decNumber *, const decNumber *);
extern decNumber *cdf_T(decNumber *, const decNumber *);
extern decNumber *qf_T(decNumber *, const decNumber *);

/* F distribution with v1 and v2 df, v1 & v2 in registers J and K */
extern decNumber *pdf_F(decNumber *, const decNumber *);
extern decNumber *cdf_F(decNumber *, const decNumber *);
extern decNumber *qf_F(decNumber *, const decNumber *);

/* Weibull distribution with parameters k and lambda in registers J and K */
extern decNumber *pdf_WB(decNumber *, const decNumber *);
extern decNumber *cdf_WB(decNumber *, const decNumber *);
extern decNumber *qf_WB(decNumber *, const decNumber *);

/* Exponential distribution with parameter lambda in register J */
extern decNumber *pdf_EXP(decNumber *, const decNumber *);
extern decNumber *cdf_EXP(decNumber *, const decNumber *);
extern decNumber *qf_EXP(decNumber *, const decNumber *);

/* Poisson distribution with parameter lambda (in register J) */
extern decNumber *pdf_P(decNumber *, const decNumber *);
extern decNumber *cdf_P(decNumber *, const decNumber *);
extern decNumber *qf_P(decNumber *, const decNumber *);

/* Geometric distribution with parameter p (in register J) */
extern decNumber *pdf_G(decNumber *, const decNumber *);
extern decNumber *cdf_G(decNumber *, const decNumber *);
extern decNumber *qf_G(decNumber *, const decNumber *);

/* Binomial distribution (n, p) in registers J and K */
extern decNumber *pdf_B(decNumber *, const decNumber *);
extern decNumber *cdf_B(decNumber *, const decNumber *);
extern decNumber *qf_B(decNumber *, const decNumber *);

/* Normal distribution m, sigma in registers J and K */
extern decNumber *pdf_normal(decNumber *, const decNumber *);
extern decNumber *cdf_normal(decNumber *, const decNumber *);
extern decNumber *qf_normal(decNumber *, const decNumber *);

/* Lognormal distribution m, sigma in registers J and K */
extern decNumber *pdf_lognormal(decNumber *, const decNumber *);
extern decNumber *cdf_lognormal(decNumber *, const decNumber *);
extern decNumber *qf_lognormal(decNumber *, const decNumber *);

/* Lognormal distribution m, sigma in registers J and K */
extern decNumber *pdf_logistic(decNumber *, const decNumber *);
extern decNumber *cdf_logistic(decNumber *, const decNumber *);
extern decNumber *qf_logistic(decNumber *, const decNumber *);

/* Cauchy distribution x0, gamma in registers J and K */
extern decNumber *pdf_cauchy(decNumber *, const decNumber *);
extern decNumber *cdf_cauchy(decNumber *, const decNumber *);
extern decNumber *qf_cauchy(decNumber *, const decNumber *);

#endif
