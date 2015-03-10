#ifndef _FORMAT_H
#define _FORMAT_H
enum Format
{
     // 1 short
                F10t = 1,
                F10x,
                F11n,
                F11x,
                F12x, // 5
                // 2 shorts
                //F20bc, //6
                F20t,
                F21c, // 7
                F21h,
                F21s, //9
                F21t,
                F22b,
                F22c,
        //      F22cs,
                F22s,
                F22t,
                F22x,
                F23x,//17
                // 3 shorts
                F30t, //18
                F31c,
                F31i,
                F31t,
          //    F32s,
                F32x,
       //       F33x,
                F35c,
               //F35mi,
                F3rc,//26
        //      F3rmi,
        //      F3rms,
                // 4 shorts
                F40sc,//24
                //5 short
                F51l //25       
 };
#endif
