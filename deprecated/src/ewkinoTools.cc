#include "../interface/ewkinoTools.h"
#include <iostream>

//return run period for 2017 data
unsigned ewk::runPeriod2017(const unsigned long runNb){
    if(runNb < 297020){
        return 0; //Run A
    } else if(runNb < 299337){
        return 1; //Run B
    } else if(runNb < 302030){
        return 2; //Run C
    } else if(runNb < 303435){
        return 3; //Run D
    } else if(runNb < 304911){
        return 4; //Run E
    } else if(runNb < 306827){
        return 5; //Run F
    } else{
        std::cerr << "Error: Unknown run number for 2017 Collisions" << std::endl;
        return 6;
    }
}

unsigned ewk::runPeriod2016(const unsigned long runNb){
    if(runNb < 272006){
        return 0; //Run A
    } else if(runNb < 275656){
        return 1; //Run B
    } else if(runNb < 276314){
        return 2; //Run C
    } else if(runNb <  276830){
        return 3; //Run D
    } else if(runNb < 277771){
        return 4; //Run E
    } else if(runNb < 278819){
        return 5; //Run F
    } else if(runNb < 280918){
        return 6; //Run G
    } else if(runNb < 284045){
        return 7; //Run H
    } else{
        std::cerr << "Error: Unknown run number for 2016 Collisions" << std::endl;
        return 8;
    }
}
