
//Determine the MET SR index
unsigned metSR(const double met);

namespace ewkino{
    unsigned SR_EWK_3lOSSF_old(const double mt, const double met, const double mll);
    unsigned SR_EWK_3lOSSF_combinationPaper(const double mt, const double met, const double mll, const double ht);
    unsigned SR_EWK_3lOSSF_new( const double mll, const double mt, const double mt3l, const double met, const double ht);
}
