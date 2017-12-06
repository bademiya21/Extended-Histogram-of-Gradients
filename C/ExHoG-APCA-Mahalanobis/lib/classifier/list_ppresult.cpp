#include <iostream>
#include <lear/exception.h>

#include <lear/classifier/list_ppresult.h>

lear::List_PPResult::List_PPResult(
        const std::string& listfile_,
        const int marker,
		const bool hardtest)
            :
    Parent(),
    to(listfile_.c_str()), to1(listfile_), marker(marker), hardtest(hardtest)
{
    using namespace std;
    if (hardtest){
	if (!to) {
		throw lear::Exception("List_PPResult::constructor()",
                  "Unable to open output file " + listfile_);
	}
    }
}

void lear::List_PPResult::write( 
    const_iterator s, const_iterator e, const std::string filename)
{// {{{
     using namespace std;
	 if (!hardtest){
		 string file_list = filename;
		 string str = "txt";
		 string::iterator it = file_list.end()-3;
		 file_list.erase(it,file_list.end());
		 file_list.append(str);
			 
		 it = file_list.end() - 22;
		 file_list.erase(file_list.begin(),it);
		 string file_list1 = to1;
		 file_list1.append(file_list);
			 
		 ofstream file(file_list1.c_str());	
		 for (; s != e; ++s) 
			file  << *s << endl;
	 }
	 else{
		to  << setw(6) << 0 << ' ' << setw(6) << 0 << ' ' 
			<< setw(6) << 0 << ' ' << setw(6) << 0 << ' ' 
			<< setw(6) << 0 << ' ' << setw(6) << 0 << ' '
			<< setw(6) << 0 << '\n';
		for (; s != e; ++s) 
			to  << *s << setw(4) << marker << '\n';
	 }

}// }}}

