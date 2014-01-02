/*
    Copyright (C) 2013 Commtech, Inc.

    This file is part of fscc-linux.

    fscc-linux is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    fscc-linux is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with fscc-linux.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
//#include "../fsccdrv.h"
#define result_array_size 512

struct ResultStruct {
    double target;
    double freq;
    double errorPPM;
    int VCO_Div;
    int refDiv;
    int outDiv;
    int failed;
};

struct IcpRsStruct {
    double pdf;
    double nbw;
    double ratio;
    double df;
    unsigned long Rs;
    double icp;
    unsigned long icpnum;   //I have to use this in the switch statement because 8.75e-6 becomes 874
};

int GetICS30703Data(unsigned long desired, unsigned long ppm, struct ResultStruct *theOne, struct IcpRsStruct *theOther, unsigned char *progdata);

int calculate_clock_bits(unsigned long freq,unsigned long ppm, unsigned char *progbytes)
{
    int t;
    int i;
    unsigned long desiredppm;
    unsigned char progwords[20];
    struct ResultStruct solutiona;  //final results for ResultStruct data calculations
    struct IcpRsStruct solutionb;   //final results for IcpRsStruct data calculations

    //printf("desired freq:%ld ppm:%ld\n",freq,ppm);
    desiredppm = ppm;

    memset(&solutiona,0,sizeof(struct ResultStruct));
    memset(&solutionb,0,sizeof(struct IcpRsStruct));
    t=GetICS30703Data(freq, desiredppm, &solutiona, &solutionb, &progwords[0]);
        switch(t)
        {
        case 0:
            //printf("ICS30703: GetICS30703Data returned successfully.\n");
            //printf("programming word:\n");
            //for(i=19;i>=0;i--) printf("%x:",progwords[i]);
            //printf("\n");
            break;
        case 1:
            //printf("ICS30703: Rs case error\n");
            goto drop;
            break;
        case 2:
            //printf("ICS30703: no solutions found, try increasing ppm\n");
            goto drop;
            break;
        case 3:
            //printf("ICS30703: Table 1: Input Divider is out of rante.\n");
            goto drop;
            break;
        case 4:
            //printf("ICS30703: Table 2: VCODivider is out of range.\n");
            goto drop;
            break;
        case 5:
            //printf("ICS30703: Table 4: LoopFilterResistor is incorrect.\n");
            goto drop;
            break;
        case 6:
            //printf("ICS30703: Table 3: Charge Pump Current is incorrect.\n");
            goto drop;
            break;
        case 7:
            //printf("ICS30703: Table 5: OutputDividerOut1 is out of range.\n");
            goto drop;
            break;
        default:
            //printf("ICS30703: Unknown error number.\n");
            goto drop;
        }

        /*
        printk("The One:\n");
        printk(" RD = %4i, ",solutiona.refDiv);
        printk("VD = %4i, ",solutiona.VCO_Div);
        printk("OD = %4i, ",solutiona.outDiv);
        printk("freq_MHz = %12.3f, ",solutiona.freq);
        printk(" error_PPM= %4.3f\n",solutiona.errorPPM);

          printk(" Rs=%5d, ",solutionb.Rs);
          printk("Icp=%6.2f, ",(solutionb.icp)*10e5);
          printk("pdf=%12.2f, ",solutionb.pdf);
          printk("nbw=%15.3f, ",solutionb.nbw);
          printk("ratio=%5.4f, ",solutionb.ratio);
          printk("df=%6.3f\n\n",solutionb.df);
        */

        /*      printk("ICS30703: Programming word is: \n  0x");
        for(t=20;t>0;t--)
        {
        printk("%2.2X ", progwords[t-1]);
        }
        */
    for(i=0;i<20;i++) progbytes[i]=progwords[i];
drop:

if(t==0)    return 0;
else return 1;
}


int GetICS30703Data(unsigned long desired, unsigned long ppm, struct ResultStruct *theOne, struct IcpRsStruct *theOther, unsigned char *progdata)
{
    //  double inputfreq=18432000.0;
    double inputfreq=24000000.0;

    unsigned long od=0; //Output Divider
    unsigned r=0;
    unsigned v=0;
    unsigned V_Divstart=0;
    double freq=0;
    unsigned maxR;
    unsigned minR;
    unsigned max_V=2055;
    unsigned min_V=12;
    double allowable_error;
    double freq_err;
    struct ResultStruct Results;
    unsigned long i,j;
    struct IcpRsStruct IRStruct;
    unsigned count;
    unsigned long Rs;
    double rule1, rule2;
    int tempint;

    int InputDivider=0;
    int VCODivider=0;
    unsigned long ChargePumpCurrent=0;
    unsigned long LoopFilterResistor=0;
    unsigned long OutputDividerOut1=0;
    unsigned long temp=0;
    unsigned long requestedppm;

    if (desired < 15000 || desired > 270000000)
        return 1;

    requestedppm=ppm;

    if( inputfreq == 18432000.0) 
    {
        maxR = 921;
        minR = 1;
    }
    else if( inputfreq == 24000000.0) 
    {
        maxR = 1200;
        minR = 1;
    }

    ppm=0;
increaseppm:
    //printf("ICS30703: ppm = %ld\n",ppm);
    allowable_error  = ppm * desired/1e6; // * 1e6

    for( r = minR; r <= maxR; r++ )
    {
        rule2 = inputfreq /(double)r;
        if ( (rule2 < 20000.0) || (rule2 > 100000000.0) )
        {
            //          printk("Rule2(r=%d): 20,000<%f<100000000\n",r,rule2);
            continue;   //next r
        }

        od=8232;
        while(od > 1)
        {
            //set starting VCO setting with output freq just below target
            V_Divstart = (int) (((desired - (allowable_error) ) * r * od) / (inputfreq));

            //check if starting VCO setting too low
            if (V_Divstart < min_V)
                V_Divstart = min_V;

            //check if starting VCO setting too high
            else if (V_Divstart > max_V)
                V_Divstart = max_V;

            /** Loop thru VCO divide settings**/
            //Loop through all VCO divider ratios
            for( v = V_Divstart; v <= max_V; v++ ) //Check all Vco divider settings
            {
                rule1 = (inputfreq * ((double)v / (double)r) );

                        if(od==2)
                {
                    if( (rule1 < 90000000.0) || (rule1 > 540000000.0)  )
                    {
                        continue;   //next VCO_Div
                    }
                }
                else if(od==3)
                {
                    if( (rule1 < 90000000.0) || (rule1 > 720000000.0)  )
                    {
                        continue;   //next VCO_Div
                    }
                }
                else if( (od>=38) && (od<=1029) )
                {
                    if( (rule1 < 90000000.0) || (rule1 > 570000000.0)  )
                    {
                        continue;   //next VCO_Div
                    }
                }
                else
                {
                    if( (rule1 < 90000000.0) || (rule1 > 730000000.0)  )
                    {
                        continue;   //next VCO_Div
                    }
                }

                freq = (inputfreq * ((double)v / ((double)r * (double)od)));
                freq_err    = fabs(freq - desired) ; //Hz

                if ((freq_err) > allowable_error)
                {
                    continue; //next VCO_Div
                }
                else if((freq_err) <= allowable_error)
                {
                    count=0;
                    for(i=0;i<4;i++)
                    {
                        switch(i)
                        {
                        case 0:
                            Rs = 64000;
                            break;
                        case 1:
                            Rs = 52000;
                            break;
                        case 2:
                            Rs = 16000;
                            break;
                        case 3:
                            Rs = 4000;
                            break;
                        default:
                            return 1;
                        }

                        for(j=0;j<20;j++)
                        {
                            IRStruct.Rs=Rs;
                            switch(j)
                            {
                            case 0:
                                IRStruct.icp=1.25e-6;
                                IRStruct.icpnum=125;
                                break;
                            case 1:
                                IRStruct.icp=2.5e-6;
                                IRStruct.icpnum=250;
                                break;
                            case 2:
                                IRStruct.icp=3.75e-6;
                                IRStruct.icpnum=375;
                                break;
                            case 3:
                                IRStruct.icp=5.0e-6;
                                IRStruct.icpnum=500;
                                break;
                            case 4:
                                IRStruct.icp=6.25e-6;
                                IRStruct.icpnum=625;
                                break;
                            case 5:
                                IRStruct.icp=7.5e-6;
                                IRStruct.icpnum=750;
                                break;
                            case 6:
                                IRStruct.icp=8.75e-6;
                                IRStruct.icpnum=875;
                                break;
                            case 7:
                                IRStruct.icp=10.0e-6;
                                IRStruct.icpnum=1000;
                                break;
                            case 8:
                                IRStruct.icp=11.25e-6;
                                IRStruct.icpnum=1125;
                                break;
                            case 9:
                                IRStruct.icp=12.5e-6;
                                IRStruct.icpnum=1250;
                                break;
                            case 10:
                                IRStruct.icp=15.0e-6;
                                IRStruct.icpnum=1500;
                                break;
                            case 11:
                                IRStruct.icp=17.5e-6;
                                IRStruct.icpnum=1750;
                                break;
                            case 12:
                                IRStruct.icp=18.75e-6;
                                IRStruct.icpnum=1875;
                                break;
                            case 13:
                                IRStruct.icp=20.0e-6;
                                IRStruct.icpnum=2000;
                                break;
                            case 14:
                                IRStruct.icp=22.5e-6;
                                IRStruct.icpnum=2250;
                                break;
                            case 15:
                                IRStruct.icp=25.0e-6;
                                IRStruct.icpnum=2500;
                                break;
                            case 16:
                                IRStruct.icp=26.25e-6;
                                IRStruct.icpnum=2625;
                                break;
                            case 17:
                                IRStruct.icp=30.0e-6;
                                IRStruct.icpnum=3000;
                                break;
                            case 18:
                                IRStruct.icp=35.0e-6;
                                IRStruct.icpnum=3500;
                                break;
                            case 19:
                                IRStruct.icp=40.0e-6;
                                IRStruct.icpnum=4000;
                                break;
                            default:
                                printf("ICS30703: switch(j:icp) - You shouldn't get here! %ld\n",j);
                            }//end switch(j)


                            //                          printf("Rs=%5d ",IRStruct.Rs);
                            //                          printf("Icp=%2.2f ",IRStruct.icp*10e5);

                            IRStruct.pdf = (inputfreq / (double)r) ;
                            //                          printf("pdf=%12.2f ",IRStruct.pdf);

                            IRStruct.nbw = ( ((double)IRStruct.Rs * IRStruct.icp * 310.0e6) / (2.0 * 3.14159 * (double)v) );
                            //                          printf("nbw=%15.3f ",IRStruct.nbw);

                            IRStruct.ratio = (IRStruct.pdf/IRStruct.nbw);

                            tempint = (int)(IRStruct.ratio*10.0);
                            if((IRStruct.ratio*10.0)-tempint>=0.0) tempint++;
                            IRStruct.ratio = (double)tempint/10.0;

                            //                          IRStruct.ratio = ceil(IRStruct.ratio*10.0); //these two statements make the
                            //                          IRStruct.ratio = IRStruct.ratio/10.0;       //ratio a little nicer to compare

                            //                          printf("ratio=%2.4f ",IRStruct.ratio);

                            IRStruct.df = ( ((double)IRStruct.Rs / 2) * (sqrt( ((IRStruct.icp * 0.093) / (double)v))) );
                            //                          printf("ndf=%12.3f\n",IRStruct.df);

                            count++;
                            if( (IRStruct.ratio>30) || (IRStruct.ratio<7) || (IRStruct.df>2.0) || (IRStruct.df<0.2) )
                            {
                                continue;
                            }
                            else
                            {
                                Results.target    = desired;
                                Results.freq      = freq;
                                Results.errorPPM     = freq_err / desired * 1.0e6 ;
                                Results.VCO_Div   = v;
                                Results.refDiv    = r;
                                Results.outDiv    = od;
                                Results.failed = 1;
                                goto finished;
                            }
                        }//end for(j=0;j<20;j++)
                    }//end for(i=0;i<4;i++)
                }
            }//end of for( v = V_Divstart; v < max_V; v++ )

            if(od<=1030)
                od--;
            else if(od<=2060)
                od=od-2;
            else if(od<=4120)
                od=od-4;
            else od=od-8;

        }//end of while(od <= 8232)
    }//end of for( r = maxR, *saved_result_num = 0; r >= minR; r-- )

    ppm++;
    if(ppm>requestedppm)
    {
        return 2;
    }
    else
    {
        //      printk("ICS30703: increasing ppm to %d\n",ppm);
        goto increaseppm;
    }

finished:

    memcpy(theOne,&Results,sizeof(struct ResultStruct));

    memcpy(theOther,&IRStruct,sizeof(struct IcpRsStruct));
    /*
    printf("ICS30703: Best result is \n");
    printf("\tRD = %4i,",Results.refDiv);
    printf(" VD = %4i,",Results.VCO_Div);
    printf(" OD = %4i,",Results.outDiv);
    printf(" freq_Hz = %ld,\n",(unsigned long)Results.freq);

    printf("\tRs = %5ld, ",IRStruct.Rs);
    printf("Icp = %ld, ",(unsigned long)(IRStruct.icp*1e6));
    //  printk("pdf = %d, ",(ULONG)IRStruct.pdf);
    //  printk("nbw = %d, ",(ULONG)IRStruct.nbw);
    printf("ratio = %ld, ",(unsigned long)IRStruct.ratio);
    printf("df = %ld\n",(unsigned long)IRStruct.df*1000);
    */
    /*
    first, choose the best dividers (V, R, and OD) with

      1st key best accuracy
      2nd key lowest reference divide
      3rd key highest VCO frequency (OD)

        then choose the best loop filter with

          1st key best PDF/NBW ratio (between 7 and 30, 15 is optimal)
          2nd key best damping factor (between 0.2 and 2, 0.7 is optimal)
    */
    /* this is 1MHz
    progdata[19]=0xff;
    progdata[18]=0xff;
    progdata[17]=0xff;
    progdata[16]=0xf0;
    progdata[15]=0x00;
    progdata[14]=0x01;
    progdata[13]=0x43;
    progdata[12]=0x1a;
    progdata[11]=0x9c;
    progdata[10]=0x00;
    progdata[9]=0x00;
    progdata[8]=0x00;
    progdata[7]=0x00;
    progdata[6]=0x00;
    progdata[5]=0x00;
    progdata[4]=0x00;
    progdata[3]=0x00;
    progdata[2]=0x0c;
    progdata[1]=0xdf;
    progdata[0]=0xee;
    goto doitnow;
    */
    /* 10 MHz
    progdata[19]=0xff;
    progdata[18]=0xff;
    progdata[17]=0xff;
    progdata[16]=0x00;
    progdata[15]=0x80;
    progdata[14]=0x01;
    progdata[13]=0x00;
    progdata[12]=0x66;
    progdata[11]=0x38;
    progdata[10]=0x00;
    progdata[9]=0x00;
    progdata[8]=0x00;
    progdata[7]=0x00;
    progdata[6]=0x00;
    progdata[5]=0x00;
    progdata[4]=0x00;
    progdata[3]=0x00;
    progdata[2]=0x07;
    progdata[1]=0x20;
    progdata[0]=0x12;
    goto doitnow;
    */

    progdata[19]=0xff;
    progdata[18]=0xff;
    progdata[17]=0xff;
    progdata[16]=0x00;
    progdata[15]=0x04;//change to xtal, as per.(now overridden in the driver)
    progdata[14]=0x00;
    progdata[13]=0x00;
    progdata[12]=0x00;
    progdata[11]=0x00;
    progdata[10]=0x00;
    progdata[9]=0x00;
    progdata[8]=0x00;
    progdata[7]=0x00;
    progdata[6]=0x00;
    progdata[5]=0x00;
    progdata[4]=0x00;
    progdata[3]=0x00;
    progdata[2]=0x00;
    progdata[1]=0x00;
    progdata[0]=0x00;

    //  progdata[16]|=0x02; //enable CLK3
    //  progdata[15]&=0xef; //CLK3 source select: 1=CLK1, 0=CLK1 before OD
    //  progdata[15]|=0x08; //CLK2 source select: 1=CLK1, 0=CLK1 before OD
    //  progdata[15]|=0x40; //reference source is: 1=crystal, 0=clock
    progdata[14]|=0x01; //1=Power up, 0=power down feedback counter, charge pump and VCO
    //  progdata[13]|=0x80; //enable CLK2
    progdata[13]|=0x40; //enable CLK1

    InputDivider = theOne->refDiv;
    VCODivider = theOne->VCO_Div;
    ChargePumpCurrent = theOther->icpnum;
    LoopFilterResistor = theOther->Rs;
    OutputDividerOut1 = theOne->outDiv;

    //InputDivider=2;
    //VCODivider=60;
    //OutputDividerOut1 = 45;
    //LoopFilterResistor=16000;
    //ChargePumpCurrent=3500;

    /* Table 1: Input Divider */

    if( (InputDivider==1)||(InputDivider==2) )
    {
        switch(InputDivider)
        {
        case 1:
            progdata[0]&=0xFC;
            progdata[1]&=0xF0;
            break;
        case 2:
            progdata[0]&=0xFC;
            progdata[0]|=0x01;
            progdata[1]&=0xF0;
            break;
        }
        //  printf("1 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);
    }
    else if( (InputDivider>=3) && (InputDivider<=17) )
    {
        temp=~(InputDivider-2);
        temp = (temp << 2);
        progdata[0]=(unsigned char)temp&0xff;

        progdata[0]&=0x3e;  //set bit 0 to a 0
        progdata[0]|=0x02;  //set bit 1 to a 1

        //      printf("2 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);
    }
    else if( (InputDivider>=18) && (InputDivider<=2055) )
    {
        temp=InputDivider-8;
        temp = (temp << 2);
        progdata[0]=(unsigned char)temp&0xff;
        progdata[1]=(unsigned char)((temp>>8)&0xff);

        progdata[0]|=0x03;  //set bit 0 and 1 to a 1

        //      printf("3 0x%2.2X,0x%2.2X\n",progdata[1],progdata[0]);

    }
    else
        return 3;

    /* Table 2 VCO Divider */

    if( (VCODivider >= 12) && (VCODivider <=2055) )
    {
        temp=VCODivider-8;
        temp=(temp << 5);
        progdata[1]|=temp&0xff;
        progdata[2]|=((temp>>8)&0xff);
        //      printf("4 0x%2.2X,0x%2.2X\n",progdata[2],progdata[1]);
    }
    else return 4;

    /* Table 4 Loop Filter Resistor */

    switch(LoopFilterResistor)
    {
    case 64000:
        progdata[11]&=0xf9; //bit 89 and 90 = 0
        break;
    case 52000:
        progdata[11]&=0xf9; //bit 89 = 0
        progdata[11]|=0x04; //bit 90 = 1
        break;
    case 16000:
        progdata[11]&=0xf9; //bit 90 = 0
        progdata[11]|=0x02; //bit 89 = 1
        break;
    case 4000:
        progdata[11]|=0x06; //bit 89 and 90 = 1
        break;
    default:
        return 5;
    }
    //  printf("5 0x%2.2X\n",progdata[11]);

    /* Table 3 Charge Pump Current */

    switch(ChargePumpCurrent)
    {
    case 125:
        progdata[11]|=0x38;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        //      printf("125\n");
        break;

    case 250:
        progdata[11]|=0x38;
        progdata[15]|=0x80;
        progdata[16]&=0xfe;
        break;

    case 375:
        progdata[11]|=0x38;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 500:
        progdata[11]|=0x38;
        progdata[15]|=0x80;
        progdata[16]|=0x01;
        break;

    case 625:
        progdata[11]|=0x18;
        progdata[11]&=0xdf;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        break;

    case 750:
        progdata[11]|=0x10;
        progdata[11]&=0xd7;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        break;

    case 875:
        progdata[11]|=0x08;
        progdata[11]&=0xcf;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        break;

    case 1000:
        progdata[11]&=0xc7;
        progdata[15]&=0x7f;
        progdata[16]&=0xfe;
        break;

    case 1125:
        progdata[11]|=0x28;
        progdata[11]&=0xef;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 1250:
        progdata[11]|=0x18;
        progdata[11]&=0xdf;
        progdata[15]|=0x80;
        progdata[16]&=0xfe;
        break;

    case 1500:
        progdata[11]|=0x28;
        progdata[11]&=0xef;
        progdata[15]|=0x80;
        progdata[16]|=0x01;
        break;


    case 1750:
        progdata[11]|=0x08;
        progdata[11]&=0xcf;
        progdata[15]|=0x80;
        progdata[16]&=0xfe;
        break;

    case 1875:
        progdata[11]|=0x18;
        progdata[11]&=0xdf;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 2000:
        progdata[11]&=0xc7;
        progdata[15]|=0x80;
        progdata[16]&=0xfe;
        break;

    case 2250:
        progdata[11]|=0x10;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 2500:
        progdata[11]|=0x18;
        progdata[11]&=0xdf;
        progdata[15]|=0x80;
        progdata[16]|=0x01;
        break;

    case 2625:
        progdata[11]|=0x08;
        progdata[11]&=0xcf;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 3000:
        progdata[11]&=0xc7;
        progdata[15]&=0x7f;
        progdata[16]|=0x01;
        break;

    case 3500:
        progdata[11]|=0x08;
        progdata[11]&=0xcf;
        progdata[15]|=0x80;
        progdata[16]|=0x01;
        break;

    case 4000:
        progdata[11]&=0xc7;
        progdata[15]|=0x80;
        progdata[16]|=0x01;
        break;

    default:
        return 6;
    }//end switch(j)
    //  printf("6 0x%2.2X, 0x%2.2X, 0x%2.2X\n",progdata[16],progdata[15],progdata[11]);

    /* Table 5 Output Divider for Output 1 */
    //OutputDividerOut1=38;
    if( (OutputDividerOut1 >= 2) && (OutputDividerOut1 <= 8232) )
    {
        switch(OutputDividerOut1)
        {
        case 2:
            progdata[11]&=0x7f;
            progdata[12]&=0x00;
            progdata[13]&=0xc0;
            break;

        case 3:
            progdata[11]|=0x80;
            progdata[12]&=0x00;
            progdata[13]&=0xc0;
            break;

        case 4:
            progdata[11]&=0x7f;
            progdata[12]|=0x04;
            progdata[13]&=0xc0;
            break;

        case 5:
            progdata[11]&=0x7f;
            progdata[12]|=0x01;
            progdata[13]&=0xc0;
            break;

        case 6:
            progdata[11]|=0x80;
            progdata[12]|=0x04;
            progdata[13]&=0xc0;
            break;

        case 7:
            progdata[11]|=0x80;
            progdata[12]|=0x01;
            progdata[13]&=0xc0;
            break;

        case 11:
            progdata[11]|=0x80;
            progdata[12]|=0x09;
            progdata[13]&=0xc0;
            break;

        case 9:
            progdata[11]|=0x80;
            progdata[12]|=0x05;
            progdata[13]&=0xc0;
            break;

        case 13:
            progdata[11]|=0x80;
            progdata[12]|=0x0d;
            progdata[13]&=0xc0;
            break;

        case 8: case 10: case 12: case 14: case 15: case 16: case 17:case 18: case 19:
        case 20: case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28:
        case 29: case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
            temp = ~(OutputDividerOut1-6);
            temp = (temp << 2);
            progdata[12] = (unsigned char)temp & 0x7f;

            progdata[11]&=0x7f;
            progdata[12]&=0xfe;
            progdata[12]|=0x02;
            progdata[13]&=0xc0;
            break;

        default:

            for(i=0;i<512;i++)
            {
                if( OutputDividerOut1 == ((((i+3)*2)+0)*(1)) )
                {
                    //                  printf("1 x=%d, y=0, z=0\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xe7;
                    progdata[12]|=0x04;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+0)*(2)) )
                {
                    //                  printf("2 x=%d, y=0, z=1\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xef;
                    progdata[12]|=0x0c;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+0)*(4)) )
                {
                    //                  printf("3 x=%d, y=0, z=2\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xf7;
                    progdata[12]|=0x14;
                    break;
                }

                else if( OutputDividerOut1 == ( (((i+3)*2)+0)*(8)) )
                {
                    //                  printf("4 x=%d, y=0, z=3\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]|=0x1c;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+1)*(1)) )
                {
                    //                  printf("5 x=%d, y=1, z=0\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xe3;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+1)*(2)) )
                {
                    //                  printf("6 x=%d, y=1, z=1\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xeb; //power of 1
                    progdata[12]|=0x08;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+1)*(4)) )
                {
                    //                  printf("7 x=%d, y=1, z=2\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xf7;
                    progdata[12]|=0x10;
                    break;
                }

                else if( OutputDividerOut1 == ((((i+3)*2)+1)*(8)) )
                {
                    //                  printf("8 x=%d, y=1, z=3\n",i);
                    temp = (i<< 5);
                    progdata[12]|=(temp & 0xff);
                    progdata[13]|=(temp >> 8)&0xff;

                    progdata[12]&=0xfb;
                    progdata[12]|=0x18;
                    break;
                }
            }
            progdata[11]|=0x80; //1
            progdata[12]&=0xfe; //0
            progdata[12]|=0x02; //1
        }
        //      printf("0x%2.2x, 0x%2.2x, 0x%2.2x\n\n",progdata[13]&0x3f, progdata[12], progdata[11]&0x80);
    }
    else return 7;
    //doitnow:

    /*  progdata[15]|=0x03; //this will set
    progdata[14]|=0xc0; //the OD of clock 3
    progdata[11]&=0xbf; //to 2
    */
    return 0;

}//end of GetICS30703Bits
