#include <lear/classifier/detectinfo.h>
#include <iomanip>

void lear::DetectInfo::print(std::ostream& o) const{
    using namespace std;
    if (!hardtest){
		o	<< lbound[0] << "," << lbound[1] << "," << extent[0] << "," << extent[1] << "," << score;
	}
	else{
		o   << setw(6) << lbound[0] << ' ' << setw(6) << lbound[1] << ' '
			<< setw(6) << extent[0] << ' ' << setw(6) << extent[1] << ' '
			<< setw(9) << setprecision(5) << scale << ' ' 
			<< setw(13) << setprecision(8) << scientific << score<<fixed;
	}
}

std::ostream& lear::operator<< (std::ostream& o, const lear::DetectInfo& r){
    r.print(o); return o;
}

