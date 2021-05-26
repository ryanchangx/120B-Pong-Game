/*	Author: ryan
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "timer.h"
#include "scheduler.h"

unsigned char D[5][8] = {
                {0,0,0,0,0,0,0,0},
                {1,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,1},
                {1,0,0,0,0,0,0,1},
                {0,0,0,0,0,0,0,0}
            };

unsigned char leftPad[5] = {0,1,1,1,0};
unsigned char rightPad[5] = {0,1,1,1,0};

enum p1controlSM{waitp1, down1, up1};
int p1ButtonTick(int state){
    unsigned short i = 0;
    unsigned char tmpA = ~PINA;
    switch(state){
        case waitp1:
            if(tmpA == 0x03 || tmpA == 0x00){
                state = waitp1;
            }
            if(tmpA == 0x01){
                state = up1;
                if(D[0][7] != 1){
                    for(i = 0; i < 4; ++i){
                        D[i][7] = D[i + 1][7];
                    }
                    D[4][7] = 0;
                }
            }
            if(tmpA == 0x02){
                state = down1;
                if(D[4][7] != 1){
                    for(i = 4; i > 0; --i){
                        D[i][7] = D[i-1][7];
                    }
                    D[0][7] = 0;
                }
            }
            break;
        case down1:
            state = (tmpA == 0x00)? waitp1 : down1;
            break;
        case up1:
            state = (tmpA == 0x00)? waitp1 : up1;
            break;
    }
    return state;
}

enum ballDirection{east,west,northeast,southeast,northwest,southwest};
enum ballSM{startball,moveball};

int ballMotionTick(int state){
    //start
    //bounce (combinational logic??) need to remember previous direction of travel
    static unsigned short balliPrev = 2;
    static unsigned short balljPrev = 3;
    static unsigned short balliNext = 0;
    static unsigned short balljNext = 0;
    static int currDirection = east;
    balliPrev = balliNext;
    balljPrev = balljNext;
    D[balliPrev][balljPrev] = 0;
    switch(state){
        case startball:
            balliNext = 2;
            balljNext = 3;
            break;
        case moveball:
            //check for at col 1 or 6 (this should either bounce or fail)
                //if its at 1 or 6 then the current direction needs to either change or game needs to stop
            //otherwise, continue moving in the direction
            //just get east/west to work first, then work on edge cases
            
            break;
    }
    D[balliNext][balljNext] = 1;
    return state;
}

enum displaySM{disp1,disp2,disp3,disp4,disp5};
int displayMult(int state){
    int m;
    unsigned char pattern = 0x00;
    unsigned char row;
    switch(state){
        case disp1:
            for(m = 0; m < 8; ++m){
                pattern |= (D[0][m] << (7-m));
            }
            row = ~(0x01);
            state = disp2;
            break;
        case disp2:
            for(m = 0; m < 8; ++m){
                pattern |= (D[1][m] << (7-m));
            }
            row = ~(0x02);
            state = disp3;
            break;
        case disp3:
            for(m = 0; m < 8; ++m){
                pattern |= (D[2][m] << (7-m));
            }
            row = ~(0x04);
            state = disp4;
            break;
        case disp4:
            for(m = 0; m < 8; ++m){
                pattern |= (D[3][m] << (7-m));
            }
            row = ~(0x08);
            state = disp5;
            break;
        case disp5:
            for(m = 0; m < 8; ++m){
                pattern |= (D[4][m] << (7-m));
            }
            row = ~(0x10);
            state = disp1;
            break;
    }
    PORTC = pattern;
    PORTD = row;
    return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0xFF; PORTD = 0x00;

    /* Insert your solution below */
    static task task1, task2, task3;
    task *tasks[] = {&task1, &task2, &task3};
    const unsigned short numTasks = sizeof(tasks)/sizeof(task*);    
    unsigned short i;

    task1.state = disp1;
    task1.period = 1;
    task1.elapsedTime = task1.period;
    task1.TickFct = &displayMult;

    task2.state = waitp1;
    task2.period = 10;
    task2.elapsedTime = task2.period;
    task2.TickFct = &p1ButtonTick;

    task3.state = ball1;
    task3.period = 150;
    task3.elapsedTime = task3.period;
    task3.TickFct = &ballMotionTick;

    unsigned long gcd = tasks[0]->period;
    for(i = 1; i < numTasks; ++i){
        gcd = findGCD(gcd,tasks[i]->period);
    }

    TimerSet(gcd);
    TimerOn();
    
    while(1){
        for(i = 0; i < numTasks; ++i){
            if(tasks[i]->elapsedTime >= tasks[i]->period){
                tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
                tasks[i]->elapsedTime = 0;
            }
            tasks[i]->elapsedTime += gcd;
        }
        while(!TimerFlag);
        TimerFlag = 0;
    }
    return 1;
}
