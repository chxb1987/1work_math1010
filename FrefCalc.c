//-------------------- pragmas -------------------------------------------------

//-------------------- include files -------------------------------------------
#include "FrefCalc.h"
#include "Math1.h"
#include <string.h>
#include "GlobalConstPtrs.h"

//-------------------- private definitions ------------------------------------//
#define FREF_ACC_TIME_MIN	0.05
#define FREF_ACC_MIN			0.01


//-------------------- private data -------------------------------------------//

static FREF_Param_t g_sParamActive, g_sParamEdit;								// Parameters
static S_LINK_FNCALLBACK g_sLink = {&FREF_DbCallback, NULL};				// For Callback link
static BOOLEAN g_bParamEditHold = FALSE;


static FLOAT32 g_f32Fref;						         					// Output
static FLOAT64 g_f64Fref;
static BOOLEAN g_bFskip1StayLow;
static BOOLEAN g_bFskip2StayLow;
static BOOLEAN g_bFskip3StayLow;

//-------------------- private functions - definition section -----------------//


//-------------------- public data --------------------------------------------//
const FLOAT32 * const ge_pf32FREF_Fref = &g_f32Fref;					// Output


//-------------------- public functions - implementation section --------------//

/*! \fn			  E_DB_ERROR FREF_Init(void)
 *  \brief		  Initialize 
 *  \return 	  Error type
 */
 
E_DB_ERROR FREF_Init(void)
{
   E_DB_ERROR eRet = E_DBERROR_NO;
   Db_ParamActivate_t pfnDummy = NULL;

   // Register Callback Functions

	eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FMIN, &g_sLink);
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FMAX, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FSKIP1, &g_sLink);
	}	
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FSKIPWIDTH1, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FSKIP2, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FSKIPWIDTH2, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FSKIP3, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FSKIPWIDTH3, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FREQ1, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_FREQ2, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_TACC1, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_TDEC1, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_TACC2, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_TDEC2, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_TACC3, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_TDEC3, &g_sLink);
	}
	if (E_DBERROR_NO == eRet) {
		eRet = DBP_RegisterChangeAction(VFD_PARAM_MCSINDEX_TROUND, &g_sLink);
	}
	

	// Calculate derive Parameters and activate them.
	FREF_ParamDerive(DB_INDEX_NULL, &pfnDummy);
   FREF_ParamActivate();

   // Other jobs.
	FREF_Reset();
		
   return eRet;
}

/*! \fn			  void FREF_Reset(void)
 *  \brief		  Reset
 *  \return 	  None
 */
void FREF_Reset(void)
{
	g_f32Fref = 0.0f;						         					
	g_f64Fref = 0.0;
	g_bFskip1StayLow = TRUE;
	g_bFskip2StayLow = TRUE;
	g_bFskip3StayLow = TRUE;	
   return;
}


/*! \fn			  void FREF_DbCallback(UNSIGNED32 u32Index, U_VALUE *puData)
 *  \brief		  Callback function of parameter update
 *  \param 		  u32Index: Parameter's index
 *  \param 		  puValue:  Returns Parameter's Value
 *  \return 	  None
 */
void FREF_DbCallback(UNSIGNED32 u32Index, U_VALUE *puData)
{
	DBP_ParamDeriveAdd(u32Index, &FREF_ParamDerive);	
	
   return;
}

/*! \fn			  void FREF_ParamDerive(UNSIGNED32 u32Index, Db_ParamActivate pfnActivate)
 *  \brief		  Calculate parameter derive
 *  \param 		  u32Index: Parameter's index
 *  \return 	  None
 */
void FREF_ParamDerive(UNSIGNED32 u32Index, Db_ParamActivate_t *pfnActivate)
{
	FLOAT32	f32AccTsLimit;
	
	g_bParamEditHold = TRUE;
	
	f32AccTsLimit = FREF_ACC_MIN * (*ge_pf32TsMot);
	
	// Load parameters from database
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FMIN        , (U_VALUE *) &g_sParamEdit.f32Fmin        );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FMAX        , (U_VALUE *) &g_sParamEdit.f32Fmax        );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FSKIP1      , (U_VALUE *) &g_sParamEdit.f32Fskip1      );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FSKIPWIDTH1 , (U_VALUE *) &g_sParamEdit.f32FskipWidth1 );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FSKIP2      , (U_VALUE *) &g_sParamEdit.f32Fskip2      );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FSKIPWIDTH2 , (U_VALUE *) &g_sParamEdit.f32FskipWidth2 );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FSKIP3      , (U_VALUE *) &g_sParamEdit.f32Fskip3      );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FSKIPWIDTH3 , (U_VALUE *) &g_sParamEdit.f32FskipWidth3 );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FREQ1       , (U_VALUE *) &g_sParamEdit.f32Freq1       );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_FREQ2       , (U_VALUE *) &g_sParamEdit.f32Freq2       );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_TACC1       , (U_VALUE *) &g_sParamEdit.f32Tacc1       );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_TDEC1       , (U_VALUE *) &g_sParamEdit.f32Tdec1       );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_TACC2       , (U_VALUE *) &g_sParamEdit.f32Tacc2       );   
   DBP_GetParameter(VFD_PARAM_MCSINDEX_TDEC2       , (U_VALUE *) &g_sParamEdit.f32Tdec2       );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_TACC3       , (U_VALUE *) &g_sParamEdit.f32Tacc3       );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_TDEC3       , (U_VALUE *) &g_sParamEdit.f32Tdec3       );
   DBP_GetParameter(VFD_PARAM_MCSINDEX_TROUND      , (U_VALUE *) &g_sParamEdit.f32Tround      );

	// Limit
	g_sParamEdit.f32Fmin 	= Min(g_sParamEdit.f32Fmax, g_sParamEdit.f32Fmin);
	g_sParamEdit.f32Freq1 	= UpLimit(g_sParamEdit.f32Freq1, g_sParamEdit.f32Fmax);
	g_sParamEdit.f32Freq2 	= DualLimit(g_sParamEdit.f32Freq2, g_sParamEdit.f32Fmax, g_sParamEdit.f32Freq1);
	g_sParamEdit.f32Tacc1	= DownLimit(g_sParamEdit.f32Tacc1, FREF_ACC_TIME_MIN);
	g_sParamEdit.f32Tdec1	= DownLimit(g_sParamEdit.f32Tdec1, FREF_ACC_TIME_MIN);
	g_sParamEdit.f32Tacc2	= DownLimit(g_sParamEdit.f32Tacc2, FREF_ACC_TIME_MIN);
	g_sParamEdit.f32Tdec2	= DownLimit(g_sParamEdit.f32Tdec2, FREF_ACC_TIME_MIN);
	g_sParamEdit.f32Tacc3	= DownLimit(g_sParamEdit.f32Tacc3, FREF_ACC_TIME_MIN);
	g_sParamEdit.f32Tdec3	= DownLimit(g_sParamEdit.f32Tdec3, FREF_ACC_TIME_MIN);
	g_sParamEdit.f32Tround	= DownLimit(g_sParamEdit.f32Tround, FREF_ACC_TIME_MIN);

	// Calculate all the derived parameter
   g_sParamEdit.f32Fskip1U = UpLimit  (g_sParamEdit.f32Fskip1 + g_sParamEdit.f32FskipWidth1 * 0.5, g_sParamEdit.f32Fmax);
   g_sParamEdit.f32Fskip1D = DownLimit(g_sParamEdit.f32Fskip1 - g_sParamEdit.f32FskipWidth1 * 0.5, g_sParamEdit.f32Fmin);
   g_sParamEdit.f32Fskip2U = UpLimit  (g_sParamEdit.f32Fskip2 + g_sParamEdit.f32FskipWidth2 * 0.5, g_sParamEdit.f32Fmax);
   g_sParamEdit.f32Fskip2D = DownLimit(g_sParamEdit.f32Fskip2 - g_sParamEdit.f32FskipWidth2 * 0.5, g_sParamEdit.f32Fmin);
   g_sParamEdit.f32Fskip3U = UpLimit  (g_sParamEdit.f32Fskip3 + g_sParamEdit.f32FskipWidth3 * 0.5, g_sParamEdit.f32Fmax);
   g_sParamEdit.f32Fskip3D = DownLimit(g_sParamEdit.f32Fskip3 - g_sParamEdit.f32FskipWidth3 * 0.5, g_sParamEdit.f32Fmin);

	g_sParamEdit.f32AccTs1 = g_sParamEdit.f32Freq1 / g_sParamEdit.f32Tacc1 * (*ge_pf32TsMot);
	g_sParamEdit.f32DecTs1 = - g_sParamEdit.f32Freq1 / g_sParamEdit.f32Tdec1 * (*ge_pf32TsMot);
	
	g_sParamEdit.f32AccTs2 = (g_sParamEdit.f32Freq2 - g_sParamEdit.f32Freq1) / g_sParamEdit.f32Tacc2 * (*ge_pf32TsMot);
	g_sParamEdit.f32DecTs2 = - (g_sParamEdit.f32Freq2 - g_sParamEdit.f32Freq1) / g_sParamEdit.f32Tdec2 * (*ge_pf32TsMot);
	
	g_sParamEdit.f32AccTs3 = (g_sParamEdit.f32Fmax  - g_sParamEdit.f32Freq2) / g_sParamEdit.f32Tacc3 * (*ge_pf32TsMot);
	g_sParamEdit.f32DecTs3 = - (g_sParamEdit.f32Fmax  - g_sParamEdit.f32Freq2) / g_sParamEdit.f32Tdec3 * (*ge_pf32TsMot);
	
	g_sParamEdit.f32AccTs1 = DownLimit(g_sParamEdit.f32AccTs1, f32AccTsLimit);
	g_sParamEdit.f32DecTs1 = UpLimit(g_sParamEdit.f32DecTs1, - f32AccTsLimit);
	g_sParamEdit.f32AccTs2 = DownLimit(g_sParamEdit.f32AccTs2, f32AccTsLimit);
	g_sParamEdit.f32DecTs2 = UpLimit(g_sParamEdit.f32DecTs2, - f32AccTsLimit);
	g_sParamEdit.f32AccTs3 = DownLimit(g_sParamEdit.f32AccTs3, f32AccTsLimit);
	g_sParamEdit.f32DecTs3 = UpLimit(g_sParamEdit.f32DecTs3, - f32AccTsLimit);
	
   // Add parameter activate functions to array
  	g_bParamEditHold = FALSE;

	*pfnActivate = &FREF_ParamActivate;
   return;
}
/*! \fn			  E_PARAM_ACT_ERROR FREF_ParamActivate(void)
 *  \brief		  Activate parameter
 *  \param 		  u32Index: Parameter's index
 *  \param 		  puValue:  Parameter's Value
 *  \return 	  Error code
 */
E_PARAM_ACT_ERROR FREF_ParamActivate(void)
{
	E_PARAM_ACT_ERROR eRet = E_PARAM_ACT_ERROR_NO;
	
	if (FALSE == g_bParamEditHold)
	{
		// Copy from edit to active.
		memcpy(&g_sParamActive, &g_sParamEdit, sizeof(g_sParamEdit));

		// Copy parameters to sub-structure vars.
	
	}
	else
	{
		eRet = E_PARAM_ACT_ERROR_HOLD;
	}
		// No more.
   return eRet;
}
/*! \fn			  void FREF_Calc(FLOAT32 f32Fset)	
 *  \brief		  Calculate Frequency reference
 *  \param       f32Fset: Frequency set point input
 *  \return 	  None
 */

void FREF_Calc(FLOAT32 f32Fset)
{
	FLOAT32 f32Fref, f32AbsFref;
	FLOAT32 f32F_afterLimit;	
	FLOAT32 f32F_afterSkip1, f32F_afterSkip2, f32F_afterSkip3;
	FLOAT32 f32AccTs, f32DecTs;
	FLOAT32 f32Err, f32Step;
	BOOLEAN bBackward;

	f32Fref = (FLOAT32) g_f64Fref;	
	f32AbsFref = fabs(f32Fref);

	bBackward = (f32Fset >= 0.0) ? FALSE : TRUE;
		
	// Limit	
	f32F_afterLimit = DualLimit(fabs(f32Fset), g_sParamActive.f32Fmax, g_sParamActive.f32Fmin);
	
	// Freq skip band 1
	f32F_afterSkip1 = f32F_afterLimit;
	if (0.0 != g_sParamActive.f32FskipWidth1)
	{
		if (f32F_afterLimit > g_sParamActive.f32Fskip1U)
		{
			g_bFskip1StayLow = FALSE;
		}
		else if (f32F_afterLimit < g_sParamActive.f32Fskip1D)
		{
			g_bFskip1StayLow = TRUE;
		}
		else
		{
			f32F_afterSkip1 = (TRUE == g_bFskip1StayLow) ? g_sParamActive.f32Fskip1D : g_sParamActive.f32Fskip1U;
		}
	}
	
	// Freq skip band 2
	f32F_afterSkip2 = f32F_afterSkip1;
	if (0.0 != g_sParamActive.f32FskipWidth2)
	{
		if (f32F_afterSkip1 > g_sParamActive.f32Fskip2U)
		{
			g_bFskip2StayLow = FALSE;
		}
		else if (f32F_afterSkip1 < g_sParamActive.f32Fskip2D)
		{
			g_bFskip2StayLow = TRUE;
		}
		else
		{
			f32F_afterSkip2 = (TRUE == g_bFskip2StayLow) ? g_sParamActive.f32Fskip2D : g_sParamActive.f32Fskip2U;
		}
	}
	
	// Freq skip band 3
   f32F_afterSkip3 = f32F_afterSkip2;
	if (0.0 != g_sParamActive.f32FskipWidth3)
	{
		if (f32F_afterSkip2 > g_sParamActive.f32Fskip3U)
		{
			g_bFskip3StayLow = FALSE;
		}
		else if (f32F_afterSkip2 < g_sParamActive.f32Fskip3D)
		{
			g_bFskip3StayLow = TRUE;
		}
		else
		{
			f32F_afterSkip3 = (TRUE == g_bFskip3StayLow) ? g_sParamActive.f32Fskip3D : g_sParamActive.f32Fskip3U;
		}
	}
	
	f32F_afterSkip3 = (FALSE == bBackward) ? f32F_afterSkip3 : -f32F_afterSkip3;
		
	// Acceleration
	if (f32AbsFref < g_sParamActive.f32Freq1)
	{
		f32AccTs = g_sParamActive.f32AccTs1;
		f32DecTs = g_sParamActive.f32DecTs1;
	}
	else if (f32AbsFref < g_sParamActive.f32Freq2)
	{
		f32AccTs = g_sParamActive.f32AccTs2;
		f32DecTs = g_sParamActive.f32DecTs2;
	}
	else 
	{
		f32AccTs = g_sParamActive.f32AccTs3;
		f32DecTs = g_sParamActive.f32DecTs3;
	}		
					
	f32Err = (FLOAT32)((FLOAT64)f32F_afterSkip3 - g_f64Fref);	
	f32Step = DualLimit(f32Err, f32AccTs, f32DecTs);
	g_f64Fref += (FLOAT64)f32Step;
	g_f32Fref = (FLOAT32)g_f64Fref;

   return;
}

/*! \fn			  void FREF_SetFref(FLOAT32 f32Frefset)
 *  \brief		  Set Frequency reference at start up
 *  \param       f32Fset: Frequency ref set value
 *  \return 	  None
 */

void FREF_SetFref(FLOAT32 f32Frefset)
{
	g_f32Fref = f32Frefset;
	g_f64Fref = (FLOAT64) g_f32Fref;
	
	return;
}

/** @}*/ /* End of group */
