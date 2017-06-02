/*
�ļ�����: temp.c
����:
    1.������������������صĺ���
����: ������
�޸ļ�¼:
    2017-4-30 �ļ�����
��ע:   void
ע��:   void
 */



/******************************************�궨��*************************************************/
#define CAPATURE_TIMES          10
#define CAP_MAX                 6000000 
#define CAP_MIN                 1
#define RES_ST_VALUE            10000 // ��Ƶ�����ֵ 10K

/*******************************************����**************************************************/
typedef enum{
DisChange,  // ȫ���ŵ�
ChangeS,   // ͨ����Ƶ�����
DisChange2, // ȫ���ŵ�
ChangeT,   // ͨ������������
}enTempStaDef;
static void TempIsr(void);

/*****************************************��������************************************************/
static enTempStaDef enTempSta = DisChange2;
static uint32_t aulCapSt[CAPATURE_TIMES];//�洢ͨ����Ƶ����粶�񵽵� TC(timer counter) 
static uint32_t aulCapTt[CAPATURE_TIMES];//�洢ͨ�����������粶�񵽵� TC(timer counter)
static uint8_t  ucStIndex = 0;//��������aulCapSt[]
static uint8_t  ucTtIndex = 0;//��������aulCapTt[]

/** ��������R/T���ձ� 3950 **/
const int32_t  aulNTC_Data[]=
{
264279,250344,237130,224603,212733,201487,190836,180750,171201,162163,   // -40  --  -31
153610,145516,137858,130614,123761,117280,111149,105351, 99867, 94681,   // -30  --  -21
 89776, 85137, 80750, 76600, 72676, 68963, 65451, 62129, 58986, 56012,   // -20  --  -11
 53198, 50534, 48013, 45627, 43368, 41229, 39204, 37285, 35468, 33747,   // -10  --  -1
 32116, 30570, 29105, 27716, 26399, 25150, 23965, 22842, 21776, 20764,   //  0   --  9
 19783, 18892, 18026, 17204, 16423, 15681, 14976, 14306, 13669, 13063,   //  10  --  19
 12487, 11939, 11418, 10921, 10449, 10000,  9571,  9164,  8775,  8405,   //  20  --  29
  8052,  7716,  7396,  7090,  6798,  6520,  6255,  6002,  5760,  5529,   //  30  --  39
  5309,  5098,  4897,  4704,  4521,  4345,  4177,  4016,  3863,  3716,   //  40  --  49
  3588,  3440,  3311,  3188,  3069,  2956,  2848,  2744,  2644,  2548,   //  50  --  59
  2457,  2369,  2284,  2204,  2126,  2051,  1980,  1911,  1845,  1782,   //  60  --  69
  1721,  1663,  1606,  1552,  1500,  1450,  1402,  1356,  1312,  1269,   //  70  --  79
  1228,  1188,  1150,  1113,  1078,  1044,  1011,   979,   948,   919,   //  80  --  89
   891,   863,   837,   811,   787,   763,   740,   718,   697,   676,   //  90  --  99
   657,   637,   619,   601,   584,   567,                               // 100  --  105
};  

/******************************************��������***********************************************/
/****************************************static��������*********************************************/
/*
����: TempInit()
����:
    1.��������GPIO��ʼ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void TGpioInit(void)
{
    LPC_SC->PCONP |= CLKPWR_PCONP_PCGPIO;

    GpioSetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_OUTPUT);

    GpioSetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_OUTPUT);

    return;
}

/*
����: TTimerInit()
����:
    1.��������timer��ʼ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void TTimerInit(void)
{
    TimerIsrReg(TEMP_TIM_ID, TempIsr);
    return;
}

/*
����: TempInit()
����:
    1.���������ʼ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void TempInit(void)
{
    TGpioInit();
    TTimerInit();
    return;
}

/*
����: TempInit()
����:
    1.���������ʼ��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
static void TempIsr(void)
{
    uint32_t ulTmp = TEMP_TIMERx->IR & 0x3f;
    
    TEMP_TIMERx->IR = ulTmp;
    if (ulTmp & TIM_IR_CLR(TIM_CR0_INT))
    {
        switch(enTempSta)
        {
            case ChangeS:
            {
                aulCapSt[ucStIndex++] = TEMP_TIMERx->CR0;
                ucStIndex %= CAPATURE_TIMES;
            }
            break;

            case ChangeT:
            {
                aulCapTt[ucTtIndex++] = TEMP_TIMERx->CR0; 
                ucTtIndex %= CAPATURE_TIMES;
            }
            break;
            default: break;
        }
    }
    return;
}

/****************************************extern��������*********************************************/
/*
����: TempPro()
����:
    1.�����������ӵĵ��ݳ�/�ŵ纯��
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   
    1.��ر�֤�ú���ÿ50msִ��һ��
*/
void TempPro(void)
{
    static uint8_t ucnt = 0;

    if(ucnt)
    {
        ucnt--;
        return;
    }
    switch(enTempSta)
    {
        case DisChange:
        {
            enTempSta = ChangeS;

            GpioSetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_INPUT);
            GpioSetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_OUTPUT);

            GpioSetBit(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN);

            TEMP_TIMERx->TC = 0;
            break;
        }
        case ChangeS:     //ͨ����Ƶ�����
        {
            enTempSta = DisChange2;

            GpioSetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_OUTPUT);
            GpioSetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_OUTPUT);

            GpioClrBit(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN);
            GpioClrBit(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN);
            break;
        }

        case DisChange2:
        {
            enTempSta = ChangeT;

            GpioSetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_INPUT);
            GpioSetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_OUTPUT);

            GpioSetBit(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN);

            TEMP_TIMERx->TC = 0;
            break;
        }
        case ChangeT:
        {
            enTempSta = DisChange;

            GpioSetDir(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN, GPIO_DIRECTION_OUTPUT);
            GpioSetDir(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN, GPIO_DIRECTION_OUTPUT);

            GpioClrBit(TSENSOR_OUT_ST_PORT, TSENSOR_OUT_ST_PIN);
            GpioClrBit(TSENSOR_OUT_TT_PORT, TSENSOR_OUT_TT_PIN);
            ucnt = 2;
            break;
        }
        default: break;
    }
    return;
}

/*
����: TempGet()
����:
    1.��ȡ�¶�ֵ(0.1���϶�Ϊ��λ)
����:   void
����ֵ: void
����:   void
���:   void
��ע:   void
ע��:   void
*/
int16_t TempGet(void)
{
  uint32_t sum_st = 0 ,sum_tt = 0;
  uint8_t cnt_st,cnt_tt;
  int32_t res_t;
  uint8_t i = 0;
  /** 2�ֲ��� **/
  int16_t cmp_mid,cmp_low,cmp_high;
	
  cnt_st = 0;
  cnt_tt = 0;
  
  for(i = 0; i < 10; i++)
  {
    if( (aulCapSt[i] >= CAP_MIN) && (aulCapSt[i] <= CAP_MAX))
    {
      sum_st += aulCapSt[i];
      cnt_st ++;
    }
  }
  
  for(i = 0; i < 10; i++)
  {
    if( (aulCapTt[i] >= CAP_MIN) && (aulCapTt[i] <= CAP_MAX))
    {
      sum_tt += aulCapTt[i];
      cnt_tt ++;
    }
  }
  
  if( ( cnt_st == 0) || (cnt_tt == 0))
  {
    return 0;
  }
  
    sum_tt /= cnt_tt;
    sum_st /= cnt_st;
  
  res_t = (( RES_ST_VALUE * sum_tt) / sum_st);
  cmp_low = 0;
  cmp_high = SizeOfArray(aulNTC_Data) - 1;
  cmp_mid = (cmp_low + cmp_high) / 2;
  
  while( cmp_low < cmp_high)
  {  
    if( res_t > aulNTC_Data[cmp_mid])
    {
      cmp_high = cmp_mid;
    }
    else if (res_t < aulNTC_Data[cmp_mid])
    {
      cmp_high--;
      cmp_low = cmp_mid;
    }
    else
    {
      break;
    }
    
    cmp_mid = (cmp_low + cmp_high)/2;
  }
  
  if( cmp_mid ==(SizeOfArray(aulNTC_Data) -1) )
  {
    return (float)(SizeOfArray(aulNTC_Data) -1-40);
  }
  else
  {
    if (res_t < aulNTC_Data[cmp_mid])
    {
        cmp_mid++;
    }
    res_t -= aulNTC_Data[cmp_mid];
    
    return (10 * (cmp_mid - 40) + res_t * 10 / ( aulNTC_Data[cmp_mid+1] - aulNTC_Data[cmp_mid]));
  }
}

