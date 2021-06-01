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
#include <stdlib.h>
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
unsigned short balliPrev = 2;
unsigned short balljPrev = 3;
unsigned short balliNext = 2;
unsigned short balljNext = 3;
// unsigned char D[5] = {0,1,1,1,0};
// unsigned char rightPad[5] = {0,1,1,1,0};

unsigned char unlocked = 0x00;

enum p1controlSM{waitp1, down1, up1};
int p1ButtonTick(int state){
    unsigned short i = 0;
    unsigned char tmpA = ~PINA;
    if(unlocked == 0x00){
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
    }
    return state;
}

enum p2controlSM{follow,away};
int p2ButtonTick(int state){
    if(unlocked == 0x00){
        switch(state){
            case follow:
                if(balliNext == 0 || balliNext == 1){
                    D[0][0] = D[1][0] = D[2][0] = 1;
                    D[3][0] = D[4][0] = 0;
                }
                else if(balliNext == 2){
                    D[1][0] = D[2][0] = D[3][0] = 1;
                    D[0][0] = D[4][0] = 0;
                }
                else if(balliNext == 3 || balliNext == 4){
                    D[2][0] = D[3][0] = D[4][0] = 1;
                    D[0][0] = D[1][0] = 0;
                }
                break;
            case away:
                if(balliNext == 3 || balliNext == 4){
                    D[0][0] = D[1][0] = D[2][0] = 1;
                    D[3][0] = D[4][0] = 0;
                }
                else if(balliNext == 2){
                    D[1][0] = D[2][0] = D[3][0] = 1;
                    D[0][0] = D[4][0] = 0;
                }
                else if(balliNext == 0 || balliNext == 1){
                    D[2][0] = D[3][0] = D[4][0] = 1;
                    D[0][0] = D[1][0] = 0;
                }
                break;
        }
        state = (rand() % 2 == 1)? follow : away;
    }
    return state;
}

enum ballDirection{east,west,northeast,southeast,northwest,southwest,idle};
enum ballSM{startball,moveball,stopball,play};

int ballMotionTick(int state){
    //start
    //bounce (combinational logic??) need to remember previous direction of travel
    unsigned char tmpA = ~PINA;
    static int currDirection = idle;
    unsigned short k;
    //current paddle location
    unsigned char p1[5] = {D[0][7], D[1][7], D[2][7], D[3][7], D[4][7]};
    unsigned char p2[5] = {D[0][0], D[1][0], D[2][0], D[3][0], D[4][0]};
    balliPrev = balliNext;
    balljPrev = balljNext;
    D[balliPrev][balljPrev] = 0;
    switch(state){
        case stopball:
            state = (tmpA == 0x04)? play : stopball;
            break;
        case play:
            balliNext = 2;
            balljNext = 3;
            //reset p1 paddle
            for(k = 0; k < 5; ++k){
                if(k == 0 || k == 4){
                    D[k][7] = 0;
                }
                else{
                    D[k][7] = 1;
                }
            }
            currDirection = idle;
            state = (tmpA == 0x00)? startball : play;
            break;
        case startball:
            state = moveball;
            currDirection = east;
            unlocked = 0x00;
            break;
        case moveball:
            //check for at col 1 or 6 (this should either bounce or fail)
                //if its at 1 or 6 then the current direction needs to either change or game needs to stop
            //otherwise, continue moving in the direction
            //just get east/west to work first, then work on edge cases
            //directions work, how do we take into account the paddles now?
            // pad is static
                //middle of pad and surface:
                    // ball moves in opposite x- direction, while keeping y direction
                //side of pad
                    // split into two parts: side corner and side surface
            // pad is dynamic
            state = (tmpA == 0x04)? play : moveball;
            switch(currDirection){
                case idle:
                    break;
                case east:
                    balliNext = balliPrev;
                    balljNext = balljPrev + 1;
                    currDirection = (balljNext == 6)? west : east;
                    if(balljNext == 6){
                        //opposite direction, depends on where pad is
                        //upper
                        if(p1[0] == 1){
                            if(balliNext == 0){
                                currDirection = southwest;
                            }
                            else if(balliNext == 1){
                                currDirection = west;
                            }
                            else if(balliNext == 2){
                                currDirection = southwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        //center
                        else if(p1[1] == 1){
                            if(balliNext == 1){
                                currDirection = northwest;
                            }
                            else if(balliNext == 2){
                                currDirection = west;
                            }
                            else if(balliNext == 3){
                                currDirection = southwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        //lower
                        else{
                            if(balliNext == 2){
                                currDirection = northwest;
                            }
                            else if(balliNext == 3){
                                currDirection = west;
                            }
                            else if(balliNext == 4){
                                currDirection = northwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                    }
                    break;
                case west:
                    balliNext = balliPrev;
                    balljNext = balljPrev - 1;
                    // currDirection = (balljNext == 1)? east : west;
                    if(balljNext == 1){
                        if(p2[0] == 1){
                            if(balliNext == 0){
                                currDirection = southeast;
                            }
                            else if(balliNext == 1){
                                currDirection = east;
                            }
                            else if(balliNext == 2){
                                currDirection = southeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        else if(p2[1] == 1){
                            if(balliNext == 1){
                                currDirection = northeast;
                            }
                            else if(balliNext == 2){
                                currDirection = east;
                            }
                            else if(balliNext == 3){
                                currDirection = southeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        else{
                            if(balliNext == 2){
                                currDirection = northeast;
                            }
                            else if(balliNext == 3){
                                currDirection = east;
                            }
                            else if(balliNext == 4){
                                currDirection = northeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                    }
                    break;
                case northeast:
                    balliNext = (balliPrev > 0)? balliPrev - 1 : 0;
                    balljNext = balljPrev + 1;
                    if(balljNext == 6){
                        if(p1[0] == 1){
                            if(balliNext == 0){
                                currDirection = southwest;
                            }
                            else if(balliNext == 1){
                                currDirection = west;
                            }
                            else if(balliNext == 2){
                                currDirection = southwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        //center
                        else if(p1[1] == 1){
                            if(balliNext == 1){
                                currDirection = northwest;
                            }
                            else if(balliNext == 2){
                                currDirection = west;
                            }
                            else if(balliNext == 3){
                                currDirection = southwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        //lower
                        else{
                            if(balliNext == 2){
                                currDirection = northwest;
                            }
                            else if(balliNext == 3){
                                currDirection = west;
                            }
                            else if(balliNext == 4){
                                currDirection = northwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                    }
                    else if(balliNext == 0){
                        currDirection = southeast;
                    }
                    else{
                        currDirection = northeast;
                    }
                    break;
                case southeast:
                    balliNext = balliPrev + 1;
                    balljNext = balljPrev + 1;
                    if(balljNext == 6){
                        if(p1[0] == 1){
                            if(balliNext == 0){
                                currDirection = southwest;
                            }
                            else if(balliNext == 1){
                                currDirection = west;
                            }
                            else if(balliNext == 2){
                                currDirection = southwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        //center
                        else if(p1[1] == 1){
                            if(balliNext == 1){
                                currDirection = northwest;
                            }
                            else if(balliNext == 2){
                                currDirection = west;
                            }
                            else if(balliNext == 3){
                                currDirection = southwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        //lower
                        else{
                            if(balliNext == 2){
                                currDirection = northwest;
                            }
                            else if(balliNext == 3){
                                currDirection = west;
                            }
                            else if(balliNext == 4){
                                currDirection = northwest;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                    }
                    else if(balliNext == 4){
                        currDirection = northeast;
                    }
                    else{
                        currDirection = southeast;
                    }
                    break;
                case northwest:
                    balliNext = balliPrev - 1;
                    balljNext = balljPrev - 1;
                    if(balljNext == 1){
                        if(p2[0] == 1){
                            if(balliNext == 0){
                                currDirection = southeast;
                            }
                            else if(balliNext == 1){
                                currDirection = northeast;
                            }
                            else if(balliNext == 2){
                                currDirection = southeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        else if(p2[1] == 1){
                            if(balliNext == 1){
                                currDirection = northeast;
                            }
                            else if(balliNext == 2){
                                currDirection = northeast;
                            }
                            else if(balliNext == 3){
                                currDirection = southeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        else{
                            if(balliNext == 2){
                                currDirection = northeast;
                            }
                            else if(balliNext == 3){
                                currDirection = northeast;
                            }
                            else if(balliNext == 4){
                                currDirection = northeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                    }
                    else if(balliNext == 0){
                        currDirection = southwest;
                    }
                    else{
                        currDirection = northwest;
                    }
                    break;
                case southwest:
                    balliNext = balliPrev + 1;
                    balljNext = balljPrev - 1;
                    if(balljNext == 1){
                        if(p2[0] == 1){
                            if(balliNext == 0){
                                currDirection = southeast;
                            }
                            else if(balliNext == 1){
                                currDirection = southeast;
                            }
                            else if(balliNext == 2){
                                currDirection = southeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        else if(p2[1] == 1){
                            if(balliNext == 1){
                                currDirection = northeast;
                            }
                            else if(balliNext == 2){
                                currDirection = southeast;
                            }
                            else if(balliNext == 3){
                                currDirection = southeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                        else{
                            if(balliNext == 2){
                                currDirection = northeast;
                            }
                            else if(balliNext == 3){
                                currDirection = southeast;
                            }
                            else if(balliNext == 4){
                                currDirection = northeast;
                            }
                            else{
                                currDirection = idle;
                                state = stopball;
                            }
                        }
                    }
                    else if(balliNext == 4){
                        currDirection = northwest;
                    }
                    else{
                        currDirection = southwest;
                    }
                    break;
                // in any case where ball travels along i axis, then check for edges, make sure they bounce, and reflect in the right way.
                // also need to check for losing games
            }
            break;
    }
    if(state == stopball){
        unlocked = 0xFF;
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
    static task task1, task2, task3, task4;
    task *tasks[] = {&task1, &task2, &task3, &task4};
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

    task3.state = follow;
    task3.period = 300;
    task3.elapsedTime = task3.period;
    task3.TickFct = &p2ButtonTick;

    task4.state = stopball;
    task4.period = 150;
    task4.elapsedTime = task4.period;
    task4.TickFct = &ballMotionTick;

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
