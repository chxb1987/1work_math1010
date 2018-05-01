#define  SQRT3_INT  	(1774)
#define  SQRT_INT_BIT  	(10)
#define	 PWM_V2T_KX		(2494)
#define SINCOSTABLE_AMP_BIT		   (15)

void PWM_Out_Cal(s32 Vq_Ref ,s32  Vd_Ref,s32 Theta,u32 Pwm_PR)
{
	s32 Va=0,Vb=0,Vc=0,V_Alpha = 0,V_Beta = 0,T_va = 0,T_vb = 0,T_vc = 0,Ta = 0, Tb = 0,Tc = 0,T1 = 0,
			T2 = 0,T1T2Temp = 0;
	u16 Phase_Block=0;
	s32 VDC_Bus=24;//直流母线电压
	static u32 s_BrT = 0;
	_iq Angle_Pu,Cosine,Sine;
	
	Angle_Pu = _IQdiv(Theta,_IQ(PI2));//转化为标幺值pu
	// Using look-up IQ sine table
	Sine = _IQsinPU(Angle_Pu);
	Cosine = _IQcosPU(Angle_Pu);
	
	//i-Park变换
	V_Alpha = (Vd_Ref * Cosine - Vq_Ref * Sine) >> SINCOSTABLE_AMP_BIT;
	V_Beta  = (Vd_Ref * Sine + Vq_Ref * Cosine) >> SINCOSTABLE_AMP_BIT;

	//扇区判断，变换出3个虚拟轴
	Va = V_Beta;
	Vb = ( (V_Alpha * SQRT3_INT >> SQRT_INT_BIT )- V_Beta) >> 1;
	Vc = -(( (V_Alpha * SQRT3_INT >> SQRT_INT_BIT )+ V_Beta) >> 1);

	//相带判断
	if(Va > 0)
		Phase_Block += 1;
	if(Vb > 0)
		Phase_Block += 2;
	if(Vc > 0)
		Phase_Block += 4;

	//电压转换为矢量计数
	T_va = (s32)(Va *  PWM_V2T_KX/ VDC_Bus);
	T_vb = - (s32)(Vc * PWM_V2T_KX / VDC_Bus);
	T_vc = - (s32)(Vb * PWM_V2T_KX / VDC_Bus);
	//通过相带判断输出比较单元的值来输出pwm
	switch(Phase_Block)
	{
	case 1:
		T1 = T_vc;
		T2 = T_vb;

		if( (T1 + T2) > (s32)Pwm_PR)
		{
			T1T2Temp = T1 + T2;
			T1 = T1 * (s32)Pwm_PR / T1T2Temp ;
			T2 = T2 * (s32)Pwm_PR / T1T2Temp ;
		}

		Tb = (Pwm_PR - T1 - T2) >> 2;
		Ta = Tb + (T1 >> 1) ;
		Tc = Ta + (T2 >> 1) ;
		break;
	case 2:
		T1 = T_vb;
		T2 = -T_va;
		if((T1 + T2) > (s32)Pwm_PR )
		{
			T1T2Temp = T1 + T2;
			T1 = T1 * (s32)Pwm_PR / T1T2Temp ;
			T2 = T2 * (s32)Pwm_PR / T1T2Temp ;
		}

		Ta = ((s32)Pwm_PR - T1 - T2) >> 2;
		Tc = Ta + (T1 >> 1) ;
		Tb = Tc + (T2 >> 1) ;
		break;
	case 3:
		T1 = -T_vc;
		T2 = T_va;
		if((T1 + T2) > (s32)Pwm_PR )
		{
			T1T2Temp = T1 + T2;
			T1 = T1 * (s32)Pwm_PR / T1T2Temp ;
			T2 = T2 * (s32)Pwm_PR / T1T2Temp ;
		}

		Ta = ((s32)Pwm_PR - T1 - T2) >> 2;
		Tb = Ta + (T1 >> 1);
		Tc = Tb + (T2 >> 1);
		break;
	case 4:
		T1 = -T_va;
		T2 = T_vc;
		if((T1 + T2) > (s32)Pwm_PR )
		{
			T1T2Temp = T1 + T2;
			T1 = T1 * (s32)Pwm_PR / T1T2Temp ;
			T2 = T2 * (s32)Pwm_PR / T1T2Temp ;
		}

		Tc = ((s32)Pwm_PR - T1 - T2) >> 2;
		Tb = Tc + (T1 >> 1);
		Ta = Tb + (T2 >> 1);
		break;
	case 5:
		T1 = T_va;
		T2 = -T_vb;
		if((T1 + T2) > (s32)Pwm_PR )
		{
			T1T2Temp = T1 + T2;
			T1 = T1 * (s32)Pwm_PR / T1T2Temp ;
			T2 = T2 * (s32)Pwm_PR / T1T2Temp ;
		}

		Tb= ((s32)Pwm_PR - T1 - T2) >> 2;
		Tc= Tb + (T1 >> 1);
		Ta= Tc + (T2 >> 1);
		break;
	case 6:
		T1 = -T_vb;
		T2 = -T_vc;
		if((T1 + T2) > (s32)Pwm_PR )
		{
			T1T2Temp = T1 + T2;
			T1 = T1 * (s32)Pwm_PR / T1T2Temp ;
			T2 = T2 * (s32)Pwm_PR / T1T2Temp ;
		}

		Tc = ((s32)Pwm_PR - T1 - T2) >> 2;
		Ta = Tc + (T1 >> 1);
		Tb = Ta + (T2 >> 1);
		break;
	}
	// 当检测到使能时候，正常输出PWM波

	TIM8->CCR1 = Ta;
	TIM8->CCR2 = Tb;
	TIM8->CCR3 = Tc;
}