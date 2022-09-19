
#include "../interface/ewkinoSearchRegions.h"

//Determine the MET SR index
unsigned metSR(const double met){
    if(met < 100){
        return 0;
    } else if(met < 150){
        return 1;
    } else if(met < 200){
        return 2;
    } else{
        return 3;
    }
}


unsigned ewkino::SR_EWK_3lOSSF_old(const double mt, const double met, const double mll){ //3 light leptons, OSSF
    unsigned metI = metSR(met), sr = 0;
    if(mt < 100 || mt > 160){
        if(met > 550){
            metI = 6;
        } else if(met > 400){
            metI = 5;
        } else if(met > 250){
            metI = 4;
        }
    }
    if(mll < 75){
        if(mt < 100){
            if(metI > 4) metI = 4;
        } else if(mt < 160){
            sr += 5;
        } else{
            sr += 9;
            if(metI > 4) metI = 4;
        }
    } else if(mll < 105){
        sr += 14;
        if(mt > 100 && mt <160){
            sr += 7;
        } else if(mt > 160){
            sr += 11;
            if(metI > 5) metI = 5;
        }
    } else{
        sr += 31;
        if(mt < 100){
            if(metI > 4) metI = 4;
        } else if(mt < 160){
            sr += 5;
        } else{
            sr += 9;
            if(metI > 3) metI = 3;
        }
    }
    return sr + metI;
}


//Search region defitintions for 
unsigned ewkino::SR_EWK_3lOSSF_combinationPaper( const double mt, const double met, const double mll, const double ht){
    
    //search regions 1 to 14 (indices 0 to 13 )
    if( mll < 75. ){
        if( ht < 200 ){
            if( mt < 100 ){
                if( met < 100 ){
                    return 0;
                } else if( met < 150 ){
                    return 1;
                } else if( met < 200 ){
                    return 2;
                } else {
                    return 3;
                }
            } else if( mt < 160 ){
                if( met < 100 ){
                    return 4;
                } else if( met < 150 ){
                    return 5;
                } else {
                    return 6;
                }
            } else{
                if( met < 100 ){
                    return 7;
                } else if( met < 150 ){
                    return 8;
                } else if( met < 200 ){
                    return 9;
                } else {
                    return 10;
                }
            }
        } else {
            if( mt < 100 ){
                return 11;
            } else if( mt < 160 ){
                return 12;
            } else {
                return 13;
            }          
        }

    //search regions 16 to 56 (numbers 1 higher than in AN-2017-141 because we include WZ CR here 
    } else if( mll < 105. ){
        if( ht < 100 ){
            if( mt < 100 ){
                if( met < 100 ){
                    return 14;
                } else if( met < 150 ){
                    return 15; 
                } else if( met < 200 ){
                    return 16;
                } else if( met < 250 ){
                    return 17;
                } else{
                    return 18;
                }
            } else if( mt < 160 ){
                if( met < 100 ){
                    return 19;
                } else if( met < 150 ){
                    return 20;
                } else if( met < 200 ){
                    return 21;
                } else {
                    return 22;
                }
            } else {
                if( met < 100 ){
                    return 23;
                } else if( met < 150 ){
                    return 24;
                } else if( met < 200 ){
                    return 25;
                } else {
                    return 26;
                } 
            }
        } else if( ht < 200 ){
            if( mt < 100 ){
                if( met < 100 ){
                    return 27;
                } else if( met < 150 ){
                    return 28;
                } else if( met < 200 ){
                    return 29;
                } else if( met < 250 ){
                    return 30;
                } else {
                    return 31;
                }
            } else if( mt < 160 ){
                if( met < 100 ){
                    return 32; 
                } else if( met < 150 ){
                    return 33;
                } else if( met < 200 ){
                    return 34;
                } else {
                    return 35;
                }
            } else {
                if( met < 100 ){
                    return 36;
                } else if( met < 150 ){
                    return 37;
                } else if( met < 200 ){
                    return 38;
                } else{
                    return 39;
                }
            }
        } else {
           if( mt < 100 ){
               if( met < 150 ){
                   return 40;
               } else if( met < 250 ){
                   return 41;
               } else if( met < 350 ){
                   return 42;
               } else {
                   return 43;
               }
           } else if( mt < 160 ){
               if( met < 100 ){
                   return 44;
               } else if( met < 150 ){
                   return 45;
               } else if( met < 200 ){
                   return 46;
               } else if( met < 250 ){
                   return 47;
               } else if( met < 300 ){
                   return 48;
               } else {
                   return 49;
               }
           } else {
               if( met < 100 ){
                   return 50;
               } else if( met < 150 ){
                   return 51;
               } else if( met < 200 ){
                   return 52;
               } else if( met < 250 ){
                   return 53;
               } else if( met < 300 ){
                   return 54;
               } else {
                   return 55;
               }
           }
        }

    //search regions 57 - 59
    } else {
        if( mt < 100 ){
            return 56;
        } else if( mt < 160 ){
            return 57;
        } else {
            return 58;
        }
    }
}

unsigned ewkino::SR_EWK_3lOSSF_new( const double mll, const double mt, const double mt3l, const double met, const double ht){
    if( mll < 50 ){
        if( mt < 100 ){
            if( mt3l < 50 ){
                return 1;
            } else if ( mt3l < 100 ){
                return 2;
            } else {
                return 3;
            }
        } else if( mt < 200 ){
            return 4;
        } else {
            return 5;
        }

    } else if( mll < 75 ){
        if( mt < 100 ){
            if(mt3l < 100 ){
                return 6;
            } else if(mt3l < 400 ){
                return 7;
            } else{
                return 8;
            }
        } else if( mt < 200 ){
            if( mt3l < 400 ){
                return 9;
            } else{
                return 10;
            }
        } else{
            if( mt3l < 400 ){
                return 11;
            } else {
                return 12;
            }
        }


    //on-Z 
    } else if( mll < 105 ){
        if( ht < 100 ){
            if( mt < 100 ){
                if( met < 100 ){
                    return 25;
                } else if( met < 150 ){
                    return 26;
                } else if( met < 200 ){
                    return 27;
                } else if( met < 250 ){
                    return 28;
                } else{
                    return 29;
                }
            } else if( mt < 160 ){
                if( met < 100 ){
                    return 30;
                } else if( met < 150 ){
                    return 31;
                } else if( met < 200 ){
                    return 32;
                } else {
                    return 33;
                }
            } else {
                if( met < 100 ){
                    return 34;
                } else if( met < 150 ){
                    return 35;
                } else if( met < 200 ){
                    return 36;
                } else {
                    return 37;
                }
            }
        } else if( ht < 200 ){
            if( mt < 100 ){
                if( met < 100 ){
                    return 38;
                } else if( met < 150 ){
                    return 39;
                } else if( met < 200 ){
                    return 40;
                } else if( met < 250 ){
                    return 41;
                } else {
                    return 42;
                }
            } else if( mt < 160 ){
                if( met < 100 ){
                    return 43;
                } else if( met < 150 ){
                    return 44;
                } else if( met < 200 ){
                    return 45;
                } else {
                    return 46;
                }
            } else {
                if( met < 100 ){
                    return 47;
                } else if( met < 150 ){
                    return 48;
                } else if( met < 200 ){
                    return 49;
                } else{
                    return 50;
                }
            }
        } else {
           if( mt < 100 ){
               if( met < 150 ){
                   return 51;
               } else if( met < 250 ){
                   return 52;
               } else if( met < 350 ){
                   return 53;
               } else {
                   return 54;
               }
           } else if( mt < 160 ){
               if( met < 100 ){
                   return 55;
               } else if( met < 150 ){
                   return 56;
               } else if( met < 200 ){
                   return 57;
               } else if( met < 250 ){
                   return 58;
               } else if( met < 300 ){
                   return 59;
               } else {
                   return 60;
               }
           } else {
               if( met < 100 ){
                   return 61;
               } else if( met < 150 ){
                   return 62;
               } else if( met < 200 ){
                   return 63;
               } else if( met < 250 ){
                   return 64;
               } else if( met < 300 ){
                   return 65;
               } else {
                   return 66;
               }
           }
        }

    } else if( mll < 250 ){
        if( mt < 100 ){
            if( mt3l < 400 ){
                return 13;
            } else{
                return 14;
            }
        } else if( mt < 200 ){
            if( mt3l < 400 ){
                return 15;
            } else{
                return 16;
            }
        } else{
            if( mt3l < 400 ){
                return 17;
            } else if( mt3l < 600 ){
                return 18;
            } else{
                return 19;
            }
        }


    } else {
        if( mt < 100 ){
            if(mt3l < 400 ){
                return 20;
            } else {
                return 21;
            }
        } else if( mt < 200 ){
            return 22;
        } else{
            if( mt3l < 400 ){
                return 23;
            } else {
                return 24;
            }
        }
    }
}
