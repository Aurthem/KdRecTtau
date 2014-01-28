
#include "typedefs.h"
#include "utilities.h"

#ifndef aurthem_criteria
#define aurthem_criteria

class Criteria {
public:
	Criteria(void);

	Criteria& fill(const T_msvpdd& dummy);
	
	bool check(const T_s& key, const double& value, const T_s& si="") const;
private:
	T_msvpdd thresholds;

	friend std::ostream& operator<< (std::ostream&,const Criteria&);
};

#endif	//aurthem_criteria
